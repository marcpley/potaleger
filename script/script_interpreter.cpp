#include "script_interpreter.h"

/*
  Complete implementation of the small interpreter with:
  - C-style block syntax using { } for if/else/while/for
  - Error location reporting by selecting the offending token in a QPlainTextEdit (if provided)
  - Thread-safe modal dialogs via Qt
  - SQL integration via QSqlDatabase passed to runScript
*/

namespace {

// Helpers
static inline QString trim(const QString &s) { return s.trimmed(); }
static inline QString lower(const QString &s) { return s.toLower(); }

// Remove comments starting with -- until EOL.
// Produces the filtered script (noComments) and a mapping vector origPosMap where
// origPosMap[i] is the index in original src that corresponds to noComments[i].
QString removeCommentsWithMap(const QString &src, QVector<int> &origPosMap) {
    QString out;
    origPosMap.clear();
    const int n = src.size();
    int i = 0;
    while (i < n) {
        // if we find "--", skip to end of line
        if (i + 1 < n && src[i] == '-' && src[i+1] == '-') {
            // skip until end of line or end of input
            while (i < n && src[i] != '\n') ++i;
            // if newline present, include it (to preserve line numbers)
            if (i < n && src[i] == '\n') {
                out += '\n';
                origPosMap.append(i);
                ++i;
            }
            continue;
        }
        // regular char -> append and map position
        out += src[i];
        origPosMap.append(i);
        ++i;
    }
    return out;
}

// Find matching brace index starting at pos of '{'.
// Returns index of matching '}' or -1 on error. Handles strings '...' and nested braces.
int findMatchingBrace(const QString &s, int startPos) {
    if (startPos < 0 || startPos >= s.size() || s[startPos] != '{') return -1;
    int depth = 0;
    bool inSingle = false;
    for (int i = startPos; i < s.size(); ++i) {
        QChar c = s[i];
        if (c == '\'' ) {
            // toggle string, handle doubled '' escape
            if (!inSingle) { inSingle = true; continue; }
            // if inSingle
            if (i+1 < s.size() && s[i+1] == '\'') { ++i; continue; }
            inSingle = false;
            continue;
        }
        if (inSingle) continue;
        if (c == '{') ++depth;
        else if (c == '}') {
            --depth;
            if (depth == 0) return i;
        }
    }
    return -1;
}

// Split into top-level statements. Semicolon ';' separates statements when not inside string and not inside a top-level brace group.
// For control blocks (if/else/while/for) we include the whole if(...) { ... } [else if(...) { ... }] [else { ... }] sequence as one statement.
QVector<QString> splitStatements(const QString &script, QVector<int> &starts) {
    QVector<QString> res;
    starts.clear();
    QString cur;
    bool inSingle = false;
    int i = 0;
    int n = script.size();
    int curStart = 0;
    while (i < n) {
        QChar c = script[i];
        if (cur.isEmpty()) curStart = i;
        // string literal handling
        if (c == '\'') {
            cur += c;
            ++i;
            while (i < n) {
                cur += script[i];
                if (script[i] == '\'') {
                    if (i+1 < n && script[i+1] == '\'') { // escape
                        cur += script[i+1];
                        i += 2;
                        continue;
                    } else { ++i; break; }
                }
                ++i;
            }
            continue;
        }
        // control block detection: if/while/for starting at current position (allow spaces before keyword)
        // we detect "if", "while", "for" tokens when not inside a string
        // check for keyword at current i (skipping leading whitespace in cur)
        if (!inSingle) {
            // peek ahead for keyword only if cur is empty or contains only whitespace (so we are at top-level start)
            if (cur.trimmed().isEmpty()) {
                int j = i;
                // skip whitespace
                while (j < n && script[j].isSpace()) { cur += script[j]; ++j; }
                // if "if" or "while" or "for" starts at j
                QString remaining = script.mid(j).toLower();
                if (remaining.startsWith("if") || remaining.startsWith("while") || remaining.startsWith("for")) {
                    // ensure keyword is followed by space or '('
                    int kwLen = remaining.startsWith("if") ? 2 : remaining.startsWith("while") ? 5 : 3;
                    if (j+kwLen < n && (script[j+kwLen].isSpace() || script[j+kwLen] == '(')) {
                        // copy until we find the opening '{' of the first block
                        int k = j;
                        // move until we find the first '{' that starts the block (handle strings)
                        bool foundBrace = false;
                        for (; k < n; ++k) {
                            QChar cc = script[k];
                            if (cc == '\'') {
                                // copy string
                                cur += script.mid(i, k - i); // copy chunk until string
                                // append string properly
                                int sPos = k;
                                cur += script[sPos];
                                ++sPos;
                                while (sPos < n) {
                                    cur += script[sPos];
                                    if (script[sPos] == '\'') {
                                        if (sPos+1 < n && script[sPos+1] == '\'') { cur += script[sPos+1]; sPos += 2; continue; }
                                        ++sPos; break;
                                    }
                                    ++sPos;
                                }
                                k = sPos - 1;
                                continue;
                            }
                            if (cc == '{') {
                                // find matching brace from k
                                int match = findMatchingBrace(script, k);
                                if (match < 0) {
                                    // malformed; include rest and break
                                    cur += script.mid(i);
                                    i = n;
                                    break;
                                }
                                // include up to match
                                cur += script.mid(i, match - i + 1);
                                i = match + 1;
                                // Now check for trailing else/else if clauses, include them if present
                                while (true) {
                                    // skip spaces
                                    int p = i;
                                    while (p < n && script[p].isSpace()) ++p;
                                    // check if 'else' starts at p
                                    if (p + 4 <= n && script.mid(p,4).toLower() == "else") {
                                        // include 'else' token and possible 'if' header and its block
                                        // find next '{' after p
                                        int nextBrace = -1;
                                        int q = p;
                                        while (q < n) {
                                            if (script[q] == '{') { nextBrace = q; break; }
                                            if (script[q] == '\'') {
                                                // skip string
                                                ++q;
                                                while (q < n) {
                                                    if (script[q] == '\'') {
                                                        if (q+1 < n && script[q+1] == '\'') { q += 2; continue; }
                                                        ++q; break;
                                                    }
                                                    ++q;
                                                }
                                                continue;
                                            }
                                            ++q;
                                        }
                                        if (nextBrace < 0) {
                                            // malformed: include rest and stop
                                            cur += script.mid(i);
                                            i = n;
                                            break;
                                        }
                                        int match2 = findMatchingBrace(script, nextBrace);
                                        if (match2 < 0) { cur += script.mid(i); i = n; break; }
                                        // include everything up to match2
                                        cur += script.mid(i, match2 - i + 1);
                                        i = match2 + 1;
                                        continue; // check for further else
                                    }
                                    break;
                                }
                                foundBrace = true;
                                break;
                            }
                        }
                        if (!foundBrace && i < n) {
                            // didn't find a brace, fallback to accumulate char and continue
                            cur += script[i];
                            ++i;
                            continue;
                        }
                        // we've already advanced i to after the block(s); next loop iteration will set new curStart
                        // finalize statement
                        QString t = trim(cur);
                        if (!t.isEmpty()) { res.append(t); starts.append(curStart); }
                        cur.clear();
                        continue;
                    }
                }
                // if not control keyword, fall through to normal char append
            }
        }

        // normal char handling
        if (!inSingle && c == ';') {
            QString t = trim(cur);
            if (!t.isEmpty()) { res.append(t); starts.append(curStart); }
            cur.clear();
            ++i;
            continue;
        } else {
            cur += c;
            ++i;
        }
    }
    QString t = trim(cur);
    if (!t.isEmpty()) { res.append(t); starts.append(curStart); }
    return res;
}

// Expression tokenizer and evaluator (supports integers, strings, variables and operators)
enum TokenType { T_NUMBER, T_STRING, T_IDENT, T_OP, T_LP, T_RP };

struct Token {
    TokenType type;
    QString text;
};

// Tokenize expression. If an unexpected character is found, set errorMsg and (if errorPos non-null) *errorPos = position in expr.
QVector<Token> tokenizeExpr(const QString &expr, QString &errorMsg, int *errorPos = nullptr) {
    QVector<Token> out;
    int i = 0;
    const int n = expr.size();
    while (i < n) {
        QChar c = expr[i];
        if (c.isSpace()) { ++i; continue; }
        // string literal
        if (c == '\'') {
            QString s;
            ++i;
            while (i < n) {
                if (expr[i] == '\'') {
                    if (i+1 < n && expr[i+1] == '\'') { s += '\''; i += 2; continue; } // escaped ''
                    ++i; break;
                } else {
                    s += expr[i]; ++i;
                }
            }
            out.append({T_STRING, s});
            continue;
        }
        // number (integer)
        if (c.isDigit()) {
            QString num;
            while (i < n && expr[i].isDigit()) { num += expr[i]; ++i; }
            out.append({T_NUMBER, num});
            continue;
        }
        // identifier (variable or true/false)
        if (c.isLetter() || c == '_') {
            QString id;
            while (i < n && (expr[i].isLetterOrNumber() || expr[i] == '_' || expr[i]=='.' || expr[i]=='[' || expr[i]==']' || expr[i]=='"' || expr[i]=='\'')) {
                id += expr[i];
                ++i;
            }
            out.append({T_IDENT, id});
            continue;
        }
        // parentheses
        if (c == '(') { out.append({T_LP, "("}); ++i; continue; }
        if (c == ')') { out.append({T_RP, ")"}); ++i; continue; }
        // two-char operators
        if (i+1 < n) {
            QString two = expr.mid(i,2);
            if (two == "||" || two == "==" || two == "!=" || two == "<=" || two == ">=") {
                out.append({T_OP, two});
                i += 2; continue;
            }
        }
        // single-char operators
        if (QString("+-*/<>").contains(c)) {
            out.append({T_OP, QString(c)});
            ++i; continue;
        }
        // unknown char
        if (errorPos) *errorPos = i;
        errorMsg = QString("Unexpected character in expression: '%1'").arg(c);
        return {};
    }
    return out;
}

int opPrecedence(const QString &op) {
    if (op == "||") return 1;
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") return 2;
    if (op == "+" || op == "-") return 3;
    if (op == "*" || op == "/") return 4;
    return 0;
}

bool isRightAssoc(const QString &op) { Q_UNUSED(op); return false; }

bool toRPN(const QVector<Token> &tokens, QVector<Token> &outRPN, QString &err) {
    QVector<Token> stack;
    for (const Token &t : tokens) {
        if (t.type == T_NUMBER || t.type == T_STRING || t.type == T_IDENT) {
            outRPN.append(t);
        } else if (t.type == T_OP) {
            while (!stack.isEmpty() && stack.last().type == T_OP) {
                QString o1 = t.text;
                QString o2 = stack.last().text;
                int p1 = opPrecedence(o1);
                int p2 = opPrecedence(o2);
                if (( !isRightAssoc(o1) && p1 <= p2) || (isRightAssoc(o1) && p1 < p2)) {
                    outRPN.append(stack.takeLast());
                } else break;
            }
            stack.append(t);
        } else if (t.type == T_LP) {
            stack.append(t);
        } else if (t.type == T_RP) {
            bool found = false;
            while (!stack.isEmpty()) {
                Token top = stack.takeLast();
                if (top.type == T_LP) { found = true; break; }
                outRPN.append(top);
            }
            if (!found) { err = "Mismatched parentheses"; return false; }
        }
    }
    while (!stack.isEmpty()) {
        if (stack.last().type == T_LP || stack.last().type == T_RP) { err = "Mismatched parentheses"; return false; }
        outRPN.append(stack.takeLast());
    }
    return true;
}

// Helper to get variable value with simple dot/bracket support (one level)
QVariant getVariableValue(const QMap<QString,QVariant> &vars, const QString &ident) {
    int dot = ident.indexOf('.');
    if (dot < 0) {
        return vars.value(ident);
    }
    QString varname = ident.left(dot);
    QString rest = ident.mid(dot+1);
    QVariant v = vars.value(varname);
    if (v.type() == QVariant::Map) {
        QVariantMap m = v.toMap();
        if (rest.startsWith('[')) {
            int q1 = rest.indexOf('"');
            int q2 = rest.lastIndexOf('"');
            if (q1 >= 0 && q2 > q1) {
                QString key = rest.mid(q1+1, q2-q1-1);
                return m.value(key);
            }
            if (rest.startsWith("['") && rest.endsWith("']")) {
                QString key = rest.mid(2, rest.size()-4);
                return m.value(key);
            }
            return QVariant();
        } else {
            return m.value(rest);
        }
    }
    return QVariant();
}

// Evaluate RPN stack and return QVariant. error in err.
QVariant evaluateRPN(const QVector<Token> &rpn, const QMap<QString,QVariant> &vars, QString &err) {
    QVector<QVariant> stack;
    for (const Token &t : rpn) {
        if (t.type == T_NUMBER) {
            stack.append(t.text.toInt());
        } else if (t.type == T_STRING) {
            stack.append(t.text);
        } else if (t.type == T_IDENT) {
            QString id = t.text;
            if (id.compare("true", Qt::CaseInsensitive) == 0) { stack.append(true); continue; }
            if (id.compare("false", Qt::CaseInsensitive) == 0) { stack.append(false); continue; }
            QVariant vv = getVariableValue(vars, id);
            stack.append(vv);
        } else if (t.type == T_OP) {
            QString op = t.text;
            if (stack.size() < 1) { err = "Malformed expression"; return {}; }
            if (op == "||") {
                if (stack.size() < 2) { err = "Operator || needs two operands"; return {}; }
                QVariant b = stack.takeLast(); QVariant a = stack.takeLast();
                stack.append(a.toString() + b.toString());
                continue;
            }
            if (stack.size() < 2) { err = QString("Operator %1 requires two operands").arg(op); return {}; }
            QVariant vb = stack.takeLast(); QVariant va = stack.takeLast();
            if (op == "+") {
                if (va.type() == QVariant::String || vb.type() == QVariant::String) stack.append(va.toString() + vb.toString());
                else stack.append(va.toInt() + vb.toInt());
            } else if (op == "-") {
                stack.append(va.toInt() - vb.toInt());
            } else if (op == "*") {
                stack.append(va.toInt() * vb.toInt());
            } else if (op == "/") {
                int denom = vb.toInt();
                if (denom == 0) { err = "Division by zero"; return {}; }
                stack.append(va.toInt() / denom);
            } else if (op == "==") {
                stack.append(va.toString() == vb.toString());
            } else if (op == "!=") {
                stack.append(va.toString() != vb.toString());
            } else if (op == "<") {
                stack.append(va.toInt() < vb.toInt());
            } else if (op == ">") {
                stack.append(va.toInt() > vb.toInt());
            } else if (op == "<=") {
                stack.append(va.toInt() <= vb.toInt());
            } else if (op == ">=") {
                stack.append(va.toInt() >= vb.toInt());
            } else {
                err = QString("Unsupported operator: %1").arg(op);
                return {};
            }
        }
    }
    if (stack.isEmpty()) return QVariant();
    return stack.last();
}

// Evaluate an expression with error position reporting.
// baseOffset is absolute offset in noComments string where expr starts.
// outErrorPos receives absolute offset in noComments if a tokenization error occurs.
QVariant evalExpression(const QString &expr, const QMap<QString,QVariant> &vars, QString &err, int baseOffset = 0, int *outErrorPos = nullptr) {
    QString e = expr.trimmed();
    if (e.isEmpty()) return QVariant();
    int localErrPos = -1;
    QVector<Token> tokens = tokenizeExpr(e, err, &localErrPos);
    if (!err.isEmpty()) {
        if (outErrorPos && localErrPos >= 0) *outErrorPos = baseOffset + localErrPos;
        return {};
    }
    QVector<Token> rpn;
    if (!toRPN(tokens, rpn, err)) {
        if (outErrorPos) *outErrorPos = baseOffset;
        return {};
    }
    QVariant res = evaluateRPN(rpn, vars, err);
    if (!err.isEmpty()) {
        if (outErrorPos && localErrPos >= 0) *outErrorPos = baseOffset + localErrPos;
    }
    return res;
}

// GUI helpers (thread-safe)
void showMessageBox(QWidget *parent, const QString &title, const QString &text) {
    if (QThread::currentThread() == qApp->thread()) {
        QMessageBox::information(parent, title, text);
    } else {
        QMetaObject::invokeMethod(qApp, [parent,title,text](){
            QMessageBox::information(parent, title, text);
        }, Qt::BlockingQueuedConnection);
    }
}

bool showOkCancel(QWidget *parent, const QString &title, const QString &text) {
    bool result = false;
    if (QThread::currentThread() == qApp->thread()) {
        QMessageBox::StandardButton b = QMessageBox::question(parent, title, text, QMessageBox::Ok|QMessageBox::Cancel);
        result = (b == QMessageBox::Ok);
    } else {
        QMetaObject::invokeMethod(qApp, [&result,parent,title,text](){
            QMessageBox::StandardButton b = QMessageBox::question(parent, title, text, QMessageBox::Ok|QMessageBox::Cancel);
            result = (b == QMessageBox::Ok);
        }, Qt::BlockingQueuedConnection);
    }
    return result;
}

// SQL helpers
bool executeSqlMod(QSqlDatabase &db, const QString &sql, int &affectedRows, QString &err) {
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        err = q.lastError().text();
        return false;
    }
    affectedRows = q.numRowsAffected();
    return true;
}

bool executeSqlSelectAndShow(QSqlDatabase &db, const QString &sql, QWidget *parent, QMap<QString,QVariant> &vars, const QString &assignVar, QString &err) {
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        err = q.lastError().text();
        return false;
    }

    QStandardItemModel *model = new QStandardItemModel();
    QSqlRecord rec = q.record();
    int cols = rec.count();
    QStringList headers;
    for (int c=0;c<cols;++c) headers << rec.fieldName(c);
    model->setColumnCount(cols);
    model->setHorizontalHeaderLabels(headers);

    int row = 0;
    while (q.next()) {
        model->setRowCount(row+1);
        for (int c=0;c<cols;++c) {
            QVariant v = q.value(c);
            QStandardItem *it = new QStandardItem(v.toString());
            model->setItem(row, c, it);
        }
        ++row;
    }

    QDialog dlg(parent);
    dlg.setWindowTitle("Résultat SELECT");
    QVBoxLayout *lay = new QVBoxLayout(&dlg);
    QTableView *tv = new QTableView(&dlg);
    tv->setModel(model);
    tv->setSelectionBehavior(QAbstractItemView::SelectRows);
    tv->setSelectionMode(QAbstractItemView::SingleSelection);
    tv->horizontalHeader()->setStretchLastSection(true);
    lay->addWidget(tv);
    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dlg);
    QAbstractEventDispatcher::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    lay->addWidget(bb);
    dlg.exec();

    if (!assignVar.isEmpty()) {
        QModelIndexList sel = tv->selectionModel()->selectedRows();
        int selRow = -1;
        if (!sel.isEmpty()) selRow = sel.first().row();
        else if (model->rowCount() == 1) selRow = 0;
        if (selRow >= 0) {
            QVariantMap m;
            for (int c=0;c<cols;++c) m.insert(headers[c], model->item(selRow, c)->text());
            vars.insert(assignVar, m);
        } else {
            vars.insert(assignVar, QVariantMap());
        }
    }

    delete model;
    return true;
}

// Execute a system command and return exit code
int executeSystemCommand(const QString &cmd) {
    QProcess p;
#ifdef Q_OS_WIN
    QString program = "cmd.exe";
    QStringList args = { "/C", cmd };
#else
    QString program = "/bin/sh";
    QStringList args = { "-c", cmd };
#endif
    p.start(program, args);
    bool ok = p.waitForFinished(-1);
    if (!ok) return -1;
    return p.exitCode();
}

// Forward declaration of main executor
bool executeStatements(const QVector<QString> &stmts, const QVector<int> &stmtStarts, int &idx, QMap<QString,QVariant> &vars, QWidget *parent, QSqlDatabase &db, QString &err, int *outErrorPos = nullptr);

// Execute script source (a chunk) given its base offset in noComments; this helper splits and transforms starts to absolute offsets.
bool executeFromSource(const QString &source, int baseOffset, QMap<QString,QVariant> &vars, QWidget *parent, QSqlDatabase &db, QString &err, const QVector<int> &origStmtStarts, int *outErrorPos = nullptr) {
    Q_UNUSED(origStmtStarts);
    QVector<int> localStarts;
    QVector<QString> localStmts = splitStatements(source, localStarts);
    // convert localStarts to absolute offsets
    QVector<int> absStarts;
    absStarts.reserve(localStarts.size());
    for (int s : localStarts) absStarts.append(baseOffset + s);
    int idx = 0;
    return [&] (const QVector<QString> &stmts, const QVector<int> &stmtStarts, int &idx, QMap<QString,QVariant> &vars, QWidget *parent, QSqlDatabase &db, QString &err, int *outErr) -> bool {
        // forward to main executor (defined below). We'll use a function pointer call by name.
        return executeStatements(stmts, stmtStarts, idx, vars, parent, db, err, outErr);
    } (localStmts, absStarts, idx, vars, parent, db, err, outErrorPos);
}


// Execute a single statement. rawStmtStart is the statement's start offset in the noComments script.
bool executeSingleStatement(const QString &rawStmt, int rawStmtStart, QMap<QString,QVariant> &vars, QWidget *parent, QSqlDatabase &db, QString &err, QString *outAssignedVar = nullptr, int *outErrorPos = nullptr) {
    QString s = trim(rawStmt);
    if (s.isEmpty()) return true;

    // assignment: name = expr
    QRegularExpression assignRe("^([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*(.+)$");
    QRegularExpressionMatch m = assignRe.match(s);
    if (m.hasMatch()) {
        QString name = m.captured(1);
        QString rhs = trim(m.captured(2));
        // inline parenthesized SQL: name = (UPDATE ...); name = (SELECT ...)
        if (rhs.startsWith('(') && rhs.endsWith(')')) {
            QString inner = rhs.mid(1, rhs.size()-2).trimmed();
            QString low = lower(inner);
            if (low.startsWith("update") || low.startsWith("insert") || low.startsWith("delete")) {
                int affected = 0;
                if (!executeSqlMod(db, inner, affected, err)) return false;
                vars.insert(name, affected);
                if (outAssignedVar) *outAssignedVar = name;
                return true;
            } else if (low.startsWith("select")) {
                if (!executeSqlSelectAndShow(db, inner, parent, vars, name, err)) return false;
                if (outAssignedVar) *outAssignedVar = name;
                return true;
            }
        }
        // normal expression; compute position of rhs in rawStmt to compute absolute base
        int localPos = rawStmt.indexOf(rhs);
        int base = rawStmtStart + (localPos >= 0 ? localPos : 0);
        int exprErrPos = -1;
        QString evalErr;
        QVariant val = evalExpression(rhs, vars, evalErr, base, &exprErrPos);
        if (!evalErr.isEmpty()) {
            err = QString("Expression error: %1").arg(evalErr);
            if (outErrorPos && exprErrPos >= 0) *outErrorPos = exprErrPos;
            return false;
        }
        vars.insert(name, val);
        return true;
    }

    // messageBox(title, text)
    if (lower(s).startsWith("messagebox")) {
        int lp = s.indexOf('('), rp = s.lastIndexOf(')');
        if (lp < 0 || rp < 0 || rp <= lp) { err = "Invalid messageBox syntax"; return false; }
        QString args = s.mid(lp+1, rp-lp-1);
        QStringList parts;
        QString cur; bool inS=false;
        for (int i=0;i<args.size();++i){
            QChar c = args[i];
            if (c == '\'') { inS = !inS; cur += c; continue; }
            if (c == ',' && !inS) { parts << cur.trimmed(); cur.clear(); continue; }
            cur += c;
        }
        if (!cur.isEmpty()) parts << cur.trimmed();
        if (parts.size() < 2) { err = "messageBox requires two arguments"; return false; }
        int pos0 = rawStmt.indexOf(parts[0]);
        int pos1 = rawStmt.indexOf(parts[1], pos0>=0?pos0:0);
        int errPos = -1;
        QString eErr;
        QVariant a = evalExpression(parts[0], vars, eErr, rawStmtStart + (pos0>=0?pos0:0), &errPos); if (!eErr.isEmpty()) { err = eErr; if (outErrorPos && errPos>=0) *outErrorPos = errPos; return false; }
        QVariant b = evalExpression(parts[1], vars, eErr, rawStmtStart + (pos1>=0?pos1:0), &errPos); if (!eErr.isEmpty()) { err = eErr; if (outErrorPos && errPos>=0) *outErrorPos = errPos; return false; }
        showMessageBox(parent, a.toString(), b.toString());
        return true;
    }

    // okCancelBox(title, text) -> returns boolean if assigned
    if (lower(s).startsWith("okcancelbox")) {
        int lp = s.indexOf('('), rp = s.lastIndexOf(')');
        if (lp < 0 || rp < 0 || rp <= lp) { err = "Invalid okCancelBox syntax"; return false; }
        QString args = s.mid(lp+1, rp-lp-1);
        QStringList parts;
        QString cur; bool inS=false;
        for (int i=0;i<args.size();++i){
            QChar c = args[i];
            if (c == '\'') { inS = !inS; cur += c; continue; }
            if (c == ',' && !inS) { parts << cur.trimmed(); cur.clear(); continue; }
            cur += c;
        }
        if (!cur.isEmpty()) parts << cur.trimmed();
        if (parts.size() < 2) { err = "okCancelBox requires two arguments"; return false; }
        int pos0 = rawStmt.indexOf(parts[0]);
        int pos1 = rawStmt.indexOf(parts[1], pos0>=0?pos0:0);
        int errPos = -1;
        QString eErr;
        QVariant a = evalExpression(parts[0], vars, eErr, rawStmtStart + (pos0>=0?pos0:0), &errPos); if (!eErr.isEmpty()) { err = eErr; if (outErrorPos && errPos>=0) *outErrorPos = errPos; return false; }
        QVariant b = evalExpression(parts[1], vars, eErr, rawStmtStart + (pos1>=0?pos1:0), &errPos); if (!eErr.isEmpty()) { err = eErr; if (outErrorPos && errPos>=0) *outErrorPos = errPos; return false; }
        bool res = showOkCancel(parent, a.toString(), b.toString());
        Q_UNUSED(res);
        return true;
    }

    // system('command') or run('command')
    if (lower(s).startsWith("system(") || lower(s).startsWith("run(")) {
        int lp = s.indexOf('('), rp = s.lastIndexOf(')');
        if (lp < 0 || rp < 0 || rp <= lp) { err = "Invalid system(...) syntax"; return false; }
        QString inner = s.mid(lp+1, rp-lp-1).trimmed();
        int posInner = rawStmt.indexOf(inner);
        int errPos = -1;
        QString evalErr;
        QVariant cmd = evalExpression(inner, vars, evalErr, rawStmtStart + (posInner>=0?posInner:0), &errPos); if (!evalErr.isEmpty()) { err = evalErr; if (outErrorPos && errPos>=0) *outErrorPos = errPos; return false; }
        int exitCode = executeSystemCommand(cmd.toString());
        Q_UNUSED(exitCode);
        return true;
    }

    // SQL: SELECT -> show, UPDATE/INSERT/DELETE -> execute
    QString low = lower(s);
    if (low.startsWith("select")) {
        if (!executeSqlSelectAndShow(db, s, parent, vars, QString(), err)) return false;
        return true;
    }
    if (low.startsWith("update") || low.startsWith("insert") || low.startsWith("delete")) {
        int affected = 0;
        if (!executeSqlMod(db, s, affected, err)) return false;
        return true;
    }

    err = QString("Unknown statement: %1").arg(s);
    return false;
}

// Main executor for a list of statements where stmtStarts are absolute offsets in noComments text.
bool executeStatements(const QVector<QString> &stmts, const QVector<int> &stmtStarts, int &idx, QMap<QString,QVariant> &vars, QWidget *parent, QSqlDatabase &db, QString &err, int *outErrorPos) {
    while (idx < stmts.size()) {
        QString s = trim(stmts[idx]);
        if (s.isEmpty()) { ++idx; continue; }

        // IF (...) { ... } [ else if(...) { ... } ] [ else { ... } ]
        if (lower(s).startsWith("if") && s.contains('{')) {
            // parse first condition
            int parenL = s.indexOf('(');
            int parenR = s.indexOf(')', parenL);
            if (parenL < 0 || parenR < 0) { err = "Malformed if condition"; return false; }
            QString condExpr = s.mid(parenL+1, parenR-parenL-1).trimmed();
            int condPos = stmtStarts[idx] + s.indexOf(condExpr);
            int condErrPos = -1;
            QString eErr;
            QVariant condVal = evalExpression(condExpr, vars, eErr, condPos, &condErrPos);
            if (!eErr.isEmpty()) { err = eErr; if (outErrorPos && condErrPos>=0) *outErrorPos = condErrPos; return false; }

            // find bodies for if / else if / else inside the same statement string s
            // We'll iterate over s to extract sequences of (cond, bodySource, bodyStartOffset)
            struct Branch { QString cond; int condAbsPos; QString body; int bodyAbsStart; bool isElse; };
            QVector<Branch> branches;

            int pos = 0;
            // parse initial if
            pos = s.indexOf('{', parenR);
            if (pos < 0) { err = "Malformed if block"; return false; }
            int match = findMatchingBrace(s, pos);
            if (match < 0) { err = "Malformed if block braces"; return false; }
            QString body = s.mid(pos+1, match-pos-1);
            int bodyAbsStart = stmtStarts[idx] + pos + 1;
            branches.append({condExpr, static_cast<int>(stmtStarts[idx] + s.indexOf(condExpr)), body, bodyAbsStart, false});
            pos = match + 1;
            // handle chained else if / else
            while (true) {
                // skip whitespace
                while (pos < s.size() && s[pos].isSpace()) ++pos;
                if (pos >= s.size()) break;
                if (s.mid(pos,4).toLower() == "else") {
                    pos += 4;
                    // skip whitespace
                    while (pos < s.size() && s[pos].isSpace()) ++pos;
                    if (pos < s.size() && s.mid(pos,2).toLower() == "if") {
                        // else if
                        int pIfStart = pos;
                        int pParenL = s.indexOf('(', pos);
                        int pParenR = s.indexOf(')', pParenL);
                        if (pParenL < 0 || pParenR < 0) { err = "Malformed else if"; return false; }
                        QString cond2 = s.mid(pParenL+1, pParenR-pParenL-1).trimmed();
                        int pBodyL = s.indexOf('{', pParenR);
                        if (pBodyL < 0) { err = "Malformed else if block"; return false; }
                        int pMatch = findMatchingBrace(s, pBodyL);
                        if (pMatch < 0) { err = "Malformed else if braces"; return false; }
                        QString body2 = s.mid(pBodyL+1, pMatch-pBodyL-1);
                        int body2Abs = stmtStarts[idx] + pBodyL + 1;
                        branches.append({cond2, static_cast<int>(stmtStarts[idx] + s.indexOf(cond2, pParenL)), body2, body2Abs, false});
                        pos = pMatch + 1;
                        continue;
                    } else {
                        // else { ... }
                        int pBodyL = s.indexOf('{', pos);
                        if (pBodyL < 0) { err = "Malformed else block"; return false; }
                        int pMatch = findMatchingBrace(s, pBodyL);
                        if (pMatch < 0) { err = "Malformed else braces"; return false; }
                        QString body2 = s.mid(pBodyL+1, pMatch-pBodyL-1);
                        int body2Abs = stmtStarts[idx] + pBodyL + 1;
                        branches.append({QString(), -1, body2, body2Abs, true});
                        pos = pMatch + 1;
                        continue;
                    }
                } else break;
            }

            // execute first branch whose cond is true (or else)
            bool executed = false;
            for (const Branch &b : branches) {
                if (b.isElse) {
                    if (!executed) {
                        // execute body
                        if (!executeFromSource(b.body, b.bodyAbsStart, vars, parent, db, eErr, stmtStarts, outErrorPos)) { err = eErr; return false; }
                        executed = true;
                        break;
                    }
                } else {
                    int localErr = -1;
                    QString evalErr;
                    QVariant v = evalExpression(b.cond, vars, evalErr, b.condAbsPos, &localErr);
                    if (!evalErr.isEmpty()) { err = evalErr; if (outErrorPos && localErr>=0) *outErrorPos = localErr; return false; }
                    if (v.toBool()) {
                        if (!executeFromSource(b.body, b.bodyAbsStart, vars, parent, db, eErr, stmtStarts, outErrorPos)) { err = eErr; return false; }
                        executed = true;
                        break;
                    }
                }
            }
            ++idx;
            continue;
        }

        // WHILE (...) { ... }
        if (lower(s).startsWith("while") && s.contains('{')) {
            int parenL = s.indexOf('(');
            int parenR = s.indexOf(')', parenL);
            if (parenL < 0 || parenR < 0) { err = "Malformed while condition"; return false; }
            QString condExpr = s.mid(parenL+1, parenR-parenL-1).trimmed();
            int bodyL = s.indexOf('{', parenR);
            if (bodyL < 0) { err = "Malformed while body"; return false; }
            int match = findMatchingBrace(s, bodyL);
            if (match < 0) { err = "Malformed while braces"; return false; }
            QString body = s.mid(bodyL+1, match-bodyL-1);
            int bodyAbs = stmtStarts[idx] + bodyL + 1;
            while (true) {
                int condPos = stmtStarts[idx] + s.indexOf(condExpr);
                int condErrPos = -1;
                QString eErr;
                QVariant condVal = evalExpression(condExpr, vars, eErr, condPos, &condErrPos);
                if (!eErr.isEmpty()) { err = eErr; if (outErrorPos && condErrPos>=0) *outErrorPos = condErrPos; return false; }
                if (!condVal.toBool()) break;
                if (!executeFromSource(body, bodyAbs, vars, parent, db, eErr, stmtStarts, outErrorPos)) { err = eErr; return false; }
            }
            ++idx;
            continue;
        }

        // FOR(init; cond; after) { ... }
        if (lower(s).startsWith("for") && s.contains('{')) {
            int parenL = s.indexOf('(');
            int parenR = s.indexOf(')', parenL);
            if (parenL < 0 || parenR < 0) { err = "Malformed for header"; return false; }
            QString inside = s.mid(parenL+1, parenR-parenL-1);
            QStringList parts = inside.split(';');
            if (parts.size() < 3) { err = "for(...) must have init;cond;after"; return false; }
            QString init = parts[0].trimmed();
            QString cond = parts[1].trimmed();
            QString after = parts[2].trimmed();
            int bodyL = s.indexOf('{', parenR);
            if (bodyL < 0) { err = "Malformed for body"; return false; }
            int match = findMatchingBrace(s, bodyL);
            if (match < 0) { err = "Malformed for braces"; return false; }
            QString body = s.mid(bodyL+1, match-bodyL-1);
            int bodyAbs = stmtStarts[idx] + bodyL + 1;
            // init
            if (!init.isEmpty()) {
                int posInit = stmtStarts[idx] + s.indexOf(init);
                int initErrPos = -1;
                QString eErr;
                QVariant v = evalExpression(init, vars, eErr, posInit, &initErrPos);
                if (!eErr.isEmpty()) {
                    // try as statement (assignment)
                    QString tmpErr;
                    if (!executeSingleStatement(init + ";", stmtStarts[idx] + s.indexOf(init), vars, parent, db, tmpErr, nullptr, outErrorPos)) { err = tmpErr; return false; }
                }
            }
            // loop
            while (true) {
                if (!cond.isEmpty()) {
                    int posCond = stmtStarts[idx] + s.indexOf(cond);
                    int condErrPos = -1;
                    QString eErr;
                    QVariant vcond = evalExpression(cond, vars, eErr, posCond, &condErrPos);
                    if (!eErr.isEmpty()) { err = eErr; if (outErrorPos && condErrPos>=0) *outErrorPos = condErrPos; return false; }
                    if (!vcond.toBool()) break;
                }
                if (!executeFromSource(body, bodyAbs, vars, parent, db, err, stmtStarts, outErrorPos)) return false;
                if (!after.isEmpty()) {
                    int posAfter = stmtStarts[idx] + s.indexOf(after);
                    int afterErrPos = -1;
                    QString eErr;
                    QVariant va = evalExpression(after, vars, eErr, posAfter, &afterErrPos);
                    if (!eErr.isEmpty()) {
                        // try as statement
                        QString tmpErr;
                        if (!executeSingleStatement(after + ";", stmtStarts[idx] + s.indexOf(after), vars, parent, db, tmpErr, nullptr, outErrorPos)) { err = tmpErr; return false; }
                    }
                }
            }
            ++idx;
            continue;
        }

        // Patterns like var = (SELECT) followed by SELECT statement - we keep same behavior as before:
        QRegularExpression assignNextRe("^([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*\\(\\s*SELECT\\s*\\)$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch m2 = assignNextRe.match(s);
        if (m2.hasMatch()) {
            QString varname = m2.captured(1);
            if (idx + 1 >= stmts.size()) { err = "Expected SELECT after assignment marker"; return false; }
            QString next = trim(stmts[idx+1]);
            if (!lower(next).startsWith("select")) { err = "Expected SELECT after assignment marker"; return false; }
            if (!executeSqlSelectAndShow(db, next, parent, vars, varname, err)) return false;
            idx += 2; continue;
        }
        QRegularExpression assignNextUpdRe("^([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*\\(\\s*UPDATE\\s*\\)$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch m3 = assignNextUpdRe.match(s);
        if (m3.hasMatch()) {
            QString varname = m3.captured(1);
            if (idx + 1 >= stmts.size()) { err = "Expected UPDATE after assignment marker"; return false; }
            QString next = trim(stmts[idx+1]);
            if (!lower(next).startsWith("update")) { err = "Expected UPDATE after assignment marker"; return false; }
            int affected = 0;
            if (!executeSqlMod(db, next, affected, err)) return false;
            vars.insert(varname, affected);
            idx += 2; continue;
        }

        // Finally execute a simple statement
        int stmtAbsStart = (idx < stmtStarts.size() ? stmtStarts[idx] : 0);
        if (!executeSingleStatement(s, stmtAbsStart, vars, parent, db, err, nullptr, outErrorPos)) return false;
        ++idx;
    }
    return true;
}

} // anonymous namespace

// Public API
bool runScript(const QString &script, QWidget *parent, QSqlDatabase db, QPlainTextEdit *SQLEdit) {
    // Build noComments and mapping to original script positions
    QVector<int> origMap; // origMap[i] = index in original script corresponding to noComments[i]
    QString noComments = removeCommentsWithMap(script, origMap);

    QVector<int> stmtStarts;
    QVector<QString> stmts = splitStatements(noComments, stmtStarts);

    QMap<QString,QVariant> vars;
    int idx = 0;
    QString err;
    int errorPosInNoComments = -1;
    bool ok = executeStatements(stmts, stmtStarts, idx, vars, parent, db, err, &errorPosInNoComments);
    if (!ok) {
        qWarning() << "Script error:" << err;
        if (parent) {
            QMetaObject::invokeMethod(qApp, [parent, err](){
                QMessageBox::critical(parent, "Script error", err);
            }, Qt::QueuedConnection);
        }
        // Map errorPosInNoComments to original script position using origMap
        if (SQLEdit && errorPosInNoComments >= 0 && errorPosInNoComments < origMap.size()) {
            int origPos = origMap[errorPosInNoComments];
            // Select token around origPos: expand left/right on identifier chars
            int start = origPos;
            while (start > 0) {
                QChar ch = script[start-1];
                if (ch.isLetterOrNumber() || ch == '_' || ch == '.' || ch == '\'' ) start--; else break;
            }
            int end = origPos;
            while (end < script.size()) {
                QChar ch = script[end];
                if (ch.isLetterOrNumber() || ch == '_' || ch == '.' || ch == '\'' ) end++; else break;
            }
            // apply selection in SQLEdit (positions are in chars)
            QTextCursor cursor = SQLEdit->textCursor();
            cursor.setPosition(start);
            cursor.setPosition(end, QTextCursor::KeepAnchor);
            SQLEdit->setTextCursor(cursor);
            SQLEdit->setFocus();
        }
    } else {
        // clear selection?
    }
    return ok;
}
