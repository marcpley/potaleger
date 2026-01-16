#include "script_interpreter.h"

/*
  Complete implementation of the small interpreter.

  Notes:
  - The interpreter is intentionally compact and pragmatic, not a full language.
  - Statements are separated by ';' (semicolon). Strings use single quotes '...'
    and a quote inside a string is escaped by doubling: 'it''s ok'.
  - Comments: from -- to end of line (removed before parsing).
  - runScript returns error position via errorLine/errorColumn when available.
  - This file is self-contained and consistent with the header above.
*/

namespace {

// Helpers
static inline QString trim(const QString &s) { return s.trimmed(); }
static inline QString lower(const QString &s) { return s.toLower(); }

// Remove comments starting with -- until EOL
QString removeComments(const QString &src) {
    QString out;
    QTextStream in(const_cast<QString*>(&src), QIODevice::ReadOnly);
    QString line;
    while (true) {
        line = in.readLine();
        if (line.isNull()) break;
        int idx = line.indexOf("--");
        if (idx >= 0) line = line.left(idx);
        out += line;
        out += '\n';
    }
    return out;
}

// Split into statements by semicolon ; but ignore semicolons inside single quotes.
// Returns vector of trimmed statements and fills starts with each statement's start offset inside script.
QVector<QString> splitStatements(const QString &script, QVector<int> &starts) {
    QVector<QString> res;
    starts.clear();
    QString cur;
    bool inSingle = false;
    int curStart = 0;
    for (int i = 0; i < script.size(); ++i) {
        QChar c = script[i];
        if (cur.isEmpty()) curStart = i;
        if (c == '\'') {
            // detect escape by doubling ''
            if (inSingle && i+1 < script.size() && script[i+1] == '\'') {
                cur += "''";
                ++i;
                continue;
            }
            inSingle = !inSingle;
            cur += c;
            continue;
        }
        if (!inSingle && c == ';') {
            QString t = trim(cur);
            if (!t.isEmpty()) {
                res.append(t);
                starts.append(curStart);
            }
            cur.clear();
            continue;
        }
        cur += c;
    }
    QString t = trim(cur);
    if (!t.isEmpty()) {
        res.append(t);
        starts.append(curStart);
    }
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
        // support rest like field or ["field"]
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
// baseOffset is the absolute offset in the full script where expr starts; on tokenization error or rpn error we can report baseOffset + localPos.
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
        // toRPN doesn't provide local pos; we attempt to set outErrorPos to baseOffset
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

// Control flow detection helpers
bool startsIf(const QString &s) { return lower(s).startsWith("if(") || lower(s).startsWith("if "); }
bool startsElseIf(const QString &s) { QString t = lower(s); return t.startsWith("else if(") || t.startsWith("elseif(") || t == "else if" || t.startsWith("elseif "); }
bool isElse(const QString &s) { return lower(s) == "else"; }
bool isEndIf(const QString &s) { return lower(s) == "endif"; }
bool startsWhile(const QString &s) { return lower(s).startsWith("while(") || lower(s).startsWith("while "); }
bool isEndWhile(const QString &s) { return lower(s) == "endwhile"; }
bool startsFor(const QString &s) { return lower(s).startsWith("for(") || lower(s).startsWith("for "); }
bool isEndFor(const QString &s) { return lower(s) == "endfor"; }

// Forward
bool executeStatements(const QVector<QString> &stmts, const QVector<int> &stmtStarts, int &idx, QMap<QString,QVariant> &vars, QWidget *parent, QSqlDatabase &db, QString &err, int *outErrorPos = nullptr);

// Execute a single statement. rawStmtStart is the statement's start offset in the full script (noComments).
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

// Main statements executor. stmtStarts provides absolute start offsets for each statement.
bool executeStatements(const QVector<QString> &stmts, const QVector<int> &stmtStarts, int &idx, QMap<QString,QVariant> &vars, QWidget *parent, QSqlDatabase &db, QString &err, int *outErrorPos) {
    while (idx < stmts.size()) {
        QString s = trim(stmts[idx]);
        if (s.isEmpty()) { ++idx; continue; }

        // IF ... ENDIF
        if (startsIf(s)) {
            int lp = s.indexOf('('), rp = s.lastIndexOf(')');
            if (lp < 0 || rp < 0 || rp <= lp) { err = "Malformed if condition"; return false; }
            QString condExpr = s.mid(lp+1, rp-lp-1).trimmed();
            int posCond = s.indexOf(condExpr);
            int condErrPos = -1;
            QString eErr;
            QVariant condVal = evalExpression(condExpr, vars, eErr, stmtStarts[idx] + (posCond>=0?posCond:0), &condErrPos);
            if (!eErr.isEmpty()) { err = eErr; if (outErrorPos && condErrPos>=0) *outErrorPos = condErrPos; return false; }
            bool condTrue = condVal.toBool();

            // find matching endif and manage possible else/else if segments
            int endifPos = -1;
            int depth = 0;
            for (int k = idx + 1; k < stmts.size(); ++k) {
                QString t = trim(stmts[k]);
                if (startsIf(t)) ++depth;
                if (isEndIf(t)) {
                    if (depth == 0) { endifPos = k; break; }
                    --depth;
                }
            }
            if (endifPos < 0) { err = "Missing endif"; return false; }

            // iterate segments between idx+1 and endifPos; evaluate sequentially to pick the branch to run
            int k = idx + 1;
            bool executedBranch = false;
            // The first segment is the if-body; subsequent separators are "else if(...)" or "else"
            while (k < endifPos) {
                int segStart = k;
                int segEnd = segStart;
                QString sepLower;
                for (; segEnd < endifPos; ++segEnd) {
                    QString tt = trim(stmts[segEnd]);
                    QString lowt = lower(tt);
                    if (lowt.startsWith("else if") || lowt.startsWith("elseif") || lowt == "else") {
                        sepLower = lowt;
                        break;
                    }
                }
                // evaluate whether this segment should run
                bool shouldRun = false;
                if (segStart == idx + 1) {
                    if (condTrue) shouldRun = true;
                } else {
                    // separator is at segStart-1
                    QString sep = trim(stmts[segStart-1]);
                    QString sepL = lower(sep);
                    if (sepL == "else") shouldRun = true;
                    else {
                        // else if(cond)
                        int lp2 = sep.indexOf('('), rp2 = sep.lastIndexOf(')');
                        if (lp2 < 0 || rp2 < 0 || rp2 <= lp2) { err = "Malformed else if condition"; return false; }
                        QString cond2 = sep.mid(lp2+1, rp2-lp2-1).trimmed();
                        int pos2 = sep.indexOf(cond2);
                        int condErrPos2 = -1;
                        QString eErr2;
                        QVariant v2 = evalExpression(cond2, vars, eErr2, stmtStarts[segStart-1] + (pos2>=0?pos2:0), &condErrPos2);
                        if (!eErr2.isEmpty()) { err = eErr2; if (outErrorPos && condErrPos2>=0) *outErrorPos = condErrPos2; return false; }
                        if (v2.toBool()) shouldRun = true;
                    }
                }
                if (shouldRun && !executedBranch) {
                    int subIdx = segStart;
                    while (subIdx < segEnd) {
                        if (!executeStatements(stmts, stmtStarts, subIdx, vars, parent, db, err, outErrorPos)) return false;
                    }
                    executedBranch = true;
                }
                // move k to segEnd + 1 (skip separator)
                k = segEnd + 1;
            }
            idx = endifPos + 1;
            continue;
        }

        // WHILE ... ENDWHILE
        if (startsWhile(s)) {
            int lp = s.indexOf('('), rp = s.lastIndexOf(')');
            if (lp < 0 || rp < 0 || rp <= lp) { err = "Malformed while condition"; return false; }
            QString condExpr = s.mid(lp+1, rp-lp-1).trimmed();
            int endwhilePos = -1;
            int depth = 0;
            for (int k = idx + 1; k < stmts.size(); ++k) {
                if (startsWhile(stmts[k])) ++depth;
                if (isEndWhile(trim(stmts[k]))) {
                    if (depth == 0) { endwhilePos = k; break; }
                    --depth;
                }
            }
            if (endwhilePos < 0) { err = "Missing endwhile"; return false; }
            while (true) {
                int posCond = s.indexOf(condExpr);
                int condErrPos = -1;
                QString eErr;
                QVariant condVal = evalExpression(condExpr, vars, eErr, stmtStarts[idx] + (posCond>=0?posCond:0), &condErrPos);
                if (!eErr.isEmpty()) { err = eErr; if (outErrorPos && condErrPos>=0) *outErrorPos = condErrPos; return false; }
                if (!condVal.toBool()) break;
                int subIdx = idx + 1;
                while (subIdx < endwhilePos) {
                    if (!executeStatements(stmts, stmtStarts, subIdx, vars, parent, db, err, outErrorPos)) return false;
                }
            }
            idx = endwhilePos + 1;
            continue;
        }

        // FOR(init; cond; after) ... ENDFOR
        if (startsFor(s)) {
            int lp = s.indexOf('('), rp = s.lastIndexOf(')');
            if (lp < 0 || rp < 0 || rp <= lp) { err = "Malformed for header"; return false; }
            QString inside = s.mid(lp+1, rp-lp-1);
            QStringList parts = inside.split(';');
            if (parts.size() < 3) { err = "for(...) must have init;cond;after"; return false; }
            QString init = parts[0].trimmed();
            QString cond = parts[1].trimmed();
            QString after = parts[2].trimmed();
            int endforPos = -1;
            int depth = 0;
            for (int k = idx + 1; k < stmts.size(); ++k) {
                if (startsFor(stmts[k])) ++depth;
                if (isEndFor(trim(stmts[k]))) {
                    if (depth == 0) { endforPos = k; break; }
                    --depth;
                }
            }
            if (endforPos < 0) { err = "Missing endfor"; return false; }
            // init
            if (!init.isEmpty()) {
                int posInit = s.indexOf(init);
                int initErrPos = -1;
                QString eErr;
                QVariant v = evalExpression(init, vars, eErr, stmtStarts[idx] + (posInit>=0?posInit:0), &initErrPos);
                if (!eErr.isEmpty()) {
                    // try as statement (assignment)
                    QString tmpErr;
                    if (!executeSingleStatement(init + ";", stmtStarts[idx], vars, parent, db, tmpErr, nullptr, outErrorPos)) { err = tmpErr; return false; }
                }
            }
            while (true) {
                if (!cond.isEmpty()) {
                    int posCond = s.indexOf(cond);
                    int condErrPos = -1;
                    QString eErr;
                    QVariant vcond = evalExpression(cond, vars, eErr, stmtStarts[idx] + (posCond>=0?posCond:0), &condErrPos);
                    if (!eErr.isEmpty()) { err = eErr; if (outErrorPos && condErrPos>=0) *outErrorPos = condErrPos; return false; }
                    if (!vcond.toBool()) break;
                }
                int subIdx = idx + 1;
                while (subIdx < endforPos) {
                    if (!executeStatements(stmts, stmtStarts, subIdx, vars, parent, db, err, outErrorPos)) return false;
                }
                if (!after.isEmpty()) {
                    int posAfter = s.indexOf(after);
                    int afterErrPos = -1;
                    QString eErr;
                    QVariant v = evalExpression(after, vars, eErr, stmtStarts[idx] + (posAfter>=0?posAfter:0), &afterErrPos);
                    if (!eErr.isEmpty()) {
                        QString tmpErr;
                        if (!executeSingleStatement(after + ";", stmtStarts[idx], vars, parent, db, tmpErr, nullptr, outErrorPos)) { err = tmpErr; return false; }
                    }
                }
            }
            idx = endforPos + 1;
            continue;
        }

        // end tokens to bubble up
        if (isEndIf(s) || isEndWhile(s) || isEndFor(s) || s.toLower().startsWith("else")) {
            return true;
        }

        // handle patterns like var = (SELECT)  followed by SELECT ...; or inline parenthesized SQL handled by executeSingleStatement
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

        // finally execute a single statement
        int stmtAbsStart = (idx < stmtStarts.size() ? stmtStarts[idx] : 0);
        int errPosLocal = -1;
        if (!executeSingleStatement(s, stmtAbsStart, vars, parent, db, err, nullptr, outErrorPos)) return false;
        ++idx;
    }
    return true;
}

} // anonymous namespace

// Public API
bool runScript(const QString &script, QWidget *parent, QSqlDatabase db, int *errorLine, int *errorColumn) {
    QString noComments = removeComments(script);
    QVector<int> stmtStarts;
    QVector<QString> stmts = splitStatements(noComments, stmtStarts);

    QMap<QString,QVariant> vars;
    int idx = 0;
    QString err;
    int errorPos = -1;
    bool ok = executeStatements(stmts, stmtStarts, idx, vars, parent, db, err, &errorPos);
    if (!ok) {
        qWarning() << "Script error:" << err;
        if (parent) {
            QMetaObject::invokeMethod(qApp, [parent, err](){
                QMessageBox::critical(parent, "Script error", err);
            }, Qt::QueuedConnection);
        }
        // compute line/col (1-based) from errorPos in noComments
        if (errorPos >= 0) {
            int line = 1;
            int col = 1;
            int pos = 0;
            for (int i = 0; i < noComments.size() && pos < errorPos; ++i) {
                if (noComments[i] == '\n') { ++line; col = 1; ++pos; continue; }
                ++col; ++pos;
            }
            if (errorLine) *errorLine = line;
            if (errorColumn) *errorColumn = col;
        } else {
            if (errorLine) *errorLine = -1;
            if (errorColumn) *errorColumn = -1;
        }
    } else {
        if (errorLine) *errorLine = -1;
        if (errorColumn) *errorColumn = -1;
    }
    return ok;
}
