#include "script_interpreter.h"

// NOTE: This is a compact, illustrative interpreter. It favors clarity over performance.
// It is not hardened for production use (no sandboxing, limited error reporting).
// Use it as a starting point and extend/robustify as required.

namespace {

// Helper trim/lower
static inline QString trim(const QString &s) { return s.trimmed(); }
static inline QString lower(const QString &s) { return s.toLower(); }

// Remove SQL-style comments: --  until end of line
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
// Returns vector of trimmed statement strings (without the trailing ;)
QVector<QString> splitStatements(const QString &script) {
    QVector<QString> res;
    QString cur;
    bool inSingle = false;
    for (int i = 0; i < script.size(); ++i) {
        QChar c = script[i];
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
            if (!t.isEmpty()) res.append(t);
            cur.clear();
            continue;
        }
        cur += c;
    }
    QString t = trim(cur);
    if (!t.isEmpty()) res.append(t);
    return res;
}

// Expression evaluation (shunting-yard -> RPN).
// Tokens: numbers, identifiers, strings '...',
// operators: ||, ==, !=, <=, >=, <, >, +, -, *, /, parentheses
enum TokenType { T_NUMBER, T_STRING, T_IDENT, T_OP, T_LP, T_RP };

struct Token {
    TokenType type;
    QString text;
};

QVector<Token> tokenizeExpr(const QString &expr, QString &err) {
    QVector<Token> out;
    int i = 0;
    const int n = expr.size();
    while (i < n) {
        QChar c = expr[i];
        if (c.isSpace()) { ++i; continue; }
        // string
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
        // number (int only for simplicity)
        if (c.isDigit()) {
            QString num;
            while (i < n && (expr[i].isDigit())) { num += expr[i]; ++i; }
            out.append({T_NUMBER, num});
            continue;
        }
        // identifier (variable) or keywords
        if (c.isLetter() || c == '_') {
            QString id;
            while (i < n && (expr[i].isLetterOrNumber() || expr[i] == '_' || expr[i] == '.'
                             || expr[i] == '[' || expr[i]==']' || expr[i]=='"' || expr[i]=='\'')) {
                // We'll keep simple: allow dot access as part of identifier text; bracket access will be handled in eval
                id += expr[i];
                ++i;
            }
            out.append({T_IDENT, id});
            continue;
        }
        // parentheses
        if (c == '(') { out.append({T_LP, "("}); ++i; continue; }
        if (c == ')') { out.append({T_RP, ")"}); ++i; continue; }
        // multi-char operators: ||, ==, !=, <=, >=
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
        err = QString("Unexpected character in expression: '%1' (at pos %2)").arg(c).arg(i);
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

// Helper to get variable field (support dot notation: var.field) or bracket ["field"]
QVariant getVariableValue(const QMap<QString,QVariant> &vars, const QString &ident) {
    // parse something like name or rec.field or rec["field"]
    // We'll split at first dot.
    int dot = ident.indexOf('.');
    if (dot < 0) {
        // no dot: direct variable
        return vars.value(ident);
    }
    QString varname = ident.left(dot);
    QString rest = ident.mid(dot+1);
    QVariant v = vars.value(varname);
    // support nested dot only once for simplicity, and bracket ["field"]
    if (v.type() == QVariant::Map) {
        QVariantMap m = v.toMap();
        // rest may be like foo.bar (nested) or ["name"] or name
        if (rest.startsWith('[')) {
            // ["field"]
            int firstQuote = rest.indexOf('"');
            int lastQuote = rest.lastIndexOf('"');
            if (firstQuote >=0 && lastQuote > firstQuote) {
                QString key = rest.mid(firstQuote+1, lastQuote-firstQuote-1);
                return m.value(key);
            }
            if (rest.startsWith("['") && rest.endsWith("']")) {
                QString key = rest.mid(2, rest.size()-4);
                return m.value(key);
            }
            return QVariant();
        } else {
            // simple field
            return m.value(rest);
        }
    }
    // if v is a list or string, maybe interpret differently; keep simple:
    return QVariant();
}

// Evaluate RPN values; returns QVariant (int or QString or bool)
QVariant evaluateRPN(const QVector<Token> &rpn, const QMap<QString,QVariant> &vars, QString &err) {
    QVector<QVariant> stack;
    for (const Token &t : rpn) {
        if (t.type == T_NUMBER) {
            stack.append(t.text.toInt());
        } else if (t.type == T_STRING) {
            stack.append(t.text);
        } else if (t.type == T_IDENT) {
            // variable or literal true/false?
            QString id = t.text;
            if (id.compare("true", Qt::CaseInsensitive) == 0) { stack.append(true); continue; }
            if (id.compare("false", Qt::CaseInsensitive) == 0) { stack.append(false); continue; }
            QVariant vv = getVariableValue(vars, id);
            stack.append(vv);
        } else if (t.type == T_OP) {
            QString op = t.text;
            if (stack.size() < 1) { err = "Malformed expression (not enough operands)"; return {}; }
            if (op == "!" ) { Q_UNUSED(op); } // not used
            if (op == "||") {
                if (stack.size() < 2) { err = "Operator || requires two operands"; return {}; }
                QVariant b = stack.takeLast(); QVariant a = stack.takeLast();
                QString sa = a.toString(); QString sb = b.toString();
                stack.append(sa + sb);
                continue;
            }
            // binary arithmetic/comparison
            if (stack.size() < 2) { err = QString("Operator %1 requires two operands").arg(op); return {}; }
            QVariant vb = stack.takeLast(); QVariant va = stack.takeLast();
            // if any is string and op is + or || treat accordingly
            if (op == "+" ) {
                // if either is string -> string concat
                if (va.type() == QVariant::String || vb.type() == QVariant::String) {
                    stack.append(va.toString() + vb.toString());
                } else {
                    stack.append(va.toInt() + vb.toInt());
                }
                continue;
            } else if (op == "-") {
                stack.append(va.toInt() - vb.toInt()); continue;
            } else if (op == "*") {
                stack.append(va.toInt() * vb.toInt()); continue;
            } else if (op == "/") {
                int denom = vb.toInt();
                if (denom == 0) { err = "Division by zero"; return {}; }
                stack.append(va.toInt() / denom); continue;
            } else if (op == "==") {
                stack.append(va.toString() == vb.toString()); continue;
            } else if (op == "!=") {
                stack.append(va.toString() != vb.toString()); continue;
            } else if (op == "<") {
                stack.append(va.toInt() < vb.toInt()); continue;
            } else if (op == ">") {
                stack.append(va.toInt() > vb.toInt()); continue;
            } else if (op == "<=") {
                stack.append(va.toInt() <= vb.toInt()); continue;
            } else if (op == ">=") {
                stack.append(va.toInt() >= vb.toInt()); continue;
            } else {
                err = QString("Unsupported operator: %1").arg(op);
                return {};
            }
        }
    }
    if (stack.isEmpty()) return QVariant();
    return stack.last();
}

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

QVariant evalExpression(const QString &expr, const QMap<QString,QVariant> &vars, QString &err) {
    QString e = expr.trimmed();
    if (e.isEmpty()) return QVariant();
    QVector<Token> tokens = tokenizeExpr(e, err);
    if (!err.isEmpty()) return {};
    QVector<Token> rpn;
    if (!toRPN(tokens, rpn, err)) return {};
    return evaluateRPN(rpn, vars, err);
}

// Show a modal message box (OK)
void showMessageBox(QWidget *parent, const QString &title, const QString &text) {
    if (QThread::currentThread() == qApp->thread()) {
        QMessageBox::information(parent, title, text);
    } else {
        QMetaObject::invokeMethod(qApp, [parent,title,text](){
            QMessageBox::information(parent, title, text);
        }, Qt::BlockingQueuedConnection);
    }
}

// Show modal OK/Cancel; returns true if OK
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

// Execute SQL modification (UPDATE/INSERT/DELETE) and return affected rows, using provided db
bool executeSqlMod(QSqlDatabase &db, const QString &sql, int &affectedRows, QString &err) {
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        err = q.lastError().text();
        return false;
    }
    affectedRows = q.numRowsAffected();
    return true;
}

// Execute SELECT and show modal QTableView. If assignVar is not empty, fill vars[assignVar] with selected row as QVariantMap.
// If SELECT result is one col one row, table is not displayed and the var is assigned directly.
bool executeSqlSelectAndShow(QSqlDatabase &db, const QString &sql, QWidget *parent, QMap<QString,QVariant> &vars, const QString &assignVar, QString &err) {
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        err = q.lastError().text();
        return false;
    }

    // Build model
    QStandardItemModel *model = new QStandardItemModel();
    // headers
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

    if (row==1 and cols==1) {
        vars.insert(assignVar, q.value(0));
        return true;
    }

    // Dialog with QTableView and Ok
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
    // modal
    dlg.exec();

    // If variable assignment requested: get selected row (if no selection but single row exists, choose first)
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
            // no selection => assign empty map
            vars.insert(assignVar, QVariantMap());
        }
    }

    // cleanup: model will be deleted with parent? we allocated without parent; attach to dlg as parentless so we delete explicitly
    delete model;
    return true;
}

// Execute a system command (blocking), returns exit code
int executeSystemCommand(const QString &cmd) {
    // For simplicity, use QProcess with shell
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

// A small helper: identify whether a statement is starting a control block
bool startsIf(const QString &s) { return lower(s).startsWith("if(") || lower(s).startsWith("if "); }
bool startsElseIf(const QString &s) { QString t = lower(s); return t.startsWith("else if(") || t.startsWith("elseif(") || t == "else if" || t.startsWith("elseif "); }
bool isElse(const QString &s) { return lower(s) == "else"; }
bool isEndIf(const QString &s) { return lower(s) == "endif"; }
bool startsWhile(const QString &s) { return lower(s).startsWith("while(") || lower(s).startsWith("while "); }
bool isEndWhile(const QString &s) { return lower(s) == "endwhile"; }
bool startsFor(const QString &s) { return lower(s).startsWith("for(") || lower(s).startsWith("for "); }
bool isEndFor(const QString &s) { return lower(s) == "endfor"; }

// Evaluate a single statement or a block of statements (recursive).
// stmts: list of statements (each originally ended by ';')
// idx: current index; advanced by the function (by reference)
// vars: variable store
// parent/db used for dialogs and DB operations
bool executeStatements(const QVector<QString> &stmts, int &idx, QMap<QString,QVariant> &vars, QWidget *parent, QSqlDatabase &db, QString &err);

// Helper to find matching endif/endfor/endwhile with nesting
int findMatchingEnd(const QVector<QString> &stmts, int startIdx, const QString &startTokenLower, const QString &endTokenLower) {
    int depth = 0;
    for (int i = startIdx; i < stmts.size(); ++i) {
        QString s = trim(stmts[i]);
        QString low = lower(s);
        if (low.startsWith(startTokenLower)) ++depth;
        if (low == endTokenLower) {
            if (depth == 0) return i;
            --depth;
        }
    }
    return -1;
}

// Execute single (non-block) statement
bool executeSingleStatement(const QString &rawStmt, QMap<QString,QVariant> &vars, QWidget *parent, QSqlDatabase &db, QString &err, QString *outAssignedVar = nullptr) {
    QString s = trim(rawStmt);
    if (s.isEmpty()) return true;

    // assignment pattern: name = expr OR name = (SELECT ...) OR name = (UPDATE ...)
    QRegularExpression assignRe("^([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*(.+)$");
    QRegularExpressionMatch m = assignRe.match(s);
    if (m.hasMatch()) {
        QString name = m.captured(1);
        QString rhs = trim(m.captured(2));
        // if rhs begins with '(' and contains SELECT or UPDATE etc, we treat specially by returning the assigned var name to caller,
        // because the actual SQL will be the next statement or contained. Here we handle rhs that is a parenthesized inline SQL too.
        if (rhs.startsWith('(') && rhs.endsWith(')')) {
            QString inner = rhs.mid(1, rhs.size()-2).trimmed();
            // inline SQL e.g. name = (UPDATE ...); or name = (SELECT ...);
            if (lower(inner).startsWith("update") || lower(inner).startsWith("insert") || lower(inner).startsWith("delete")) {
                // execute immediate SQL and assign affected rows
                int affected = 0;
                if (!executeSqlMod(db, inner, affected, err)) return false;
                vars.insert(name, affected);
                if (outAssignedVar) *outAssignedVar = name;
                return true;
            } else if (lower(inner).startsWith("select")) {
                // execute select immediately and assign first row? We'll show the modal and assign selection
                if (!executeSqlSelectAndShow(db, inner, parent, vars, name, err)) return false;
                if (outAssignedVar) *outAssignedVar = name;
                return true;
            }
        }
        // otherwise evaluate rhs as expression
        QString evalErr;
        QVariant val = evalExpression(rhs, vars, evalErr);
        if (!evalErr.isEmpty()) { err = "Expression error: " + evalErr; return false; }
        vars.insert(name, val);
        return true;
    }

    // control keywords / functions
    // messageBox('Title','text')
    if (lower(s).startsWith("messagebox")) {
        // parse args inside parentheses naive split by comma (taking care of quotes)
        int lp = s.indexOf('(');
        int rp = s.lastIndexOf(')');
        if (lp < 0 || rp < 0 || rp <= lp) { err = "Invalid messageBox syntax"; return false; }
        QString args = s.mid(lp+1, rp-lp-1);
        // split two args by comma not inside quotes
        QStringList parts;
        QString cur; bool inS=false;
        for (int i=0;i<args.size();++i){
            QChar c = args[i];
            if (c == '\'') {
                inS = !inS;
                cur += c; continue;
            }
            if (c == ',' && !inS) { parts << cur.trimmed(); cur.clear(); continue; }
            cur += c;
        }
        if (!cur.isEmpty()) parts << cur.trimmed();
        if (parts.size() < 2) { err = "messageBox requires two arguments"; return false; }
        // evaluate both
        QString eErr;
        QVariant at = evalExpression(parts[0], vars, eErr); if (!eErr.isEmpty()) { err = eErr; return false; }
        QVariant bt = evalExpression(parts[1], vars, eErr); if (!eErr.isEmpty()) { err = eErr; return false; }
        showMessageBox(parent, at.toString(), bt.toString());
        return true;
    }

    // okCancelBox -> returns bool, but if used alone it's not stored; user likely writes var = okCancelBox(...)
    if (lower(s).startsWith("okcancelbox")) {
        int lp = s.indexOf('(');
        int rp = s.lastIndexOf(')');
        if (lp < 0 || rp < 0 || rp <= lp) { err = "Invalid okCancelBox syntax"; return false; }
        QString args = s.mid(lp+1, rp-lp-1);
        QStringList parts;
        QString cur; bool inS=false;
        for (int i=0;i<args.size();++i){
            QChar c = args[i];
            if (c == '\'') {
                inS = !inS;
                cur += c; continue;
            }
            if (c == ',' && !inS) { parts << cur.trimmed(); cur.clear(); continue; }
            cur += c;
        }
        if (!cur.isEmpty()) parts << cur.trimmed();
        if (parts.size() < 2) { err = "okCancelBox requires two arguments"; return false; }
        QString eErr;
        QVariant at = evalExpression(parts[0], vars, eErr); if (!eErr.isEmpty()) { err = eErr; return false; }
        QVariant bt = evalExpression(parts[1], vars, eErr); if (!eErr.isEmpty()) { err = eErr; return false; }
        bool res = showOkCancel(parent, at.toString(), bt.toString());
        // if user wanted to capture, they should have assigned variable; here we just ignore
        // To support var = okCancelBox(...); assignment was already handled above.
        Q_UNUSED(res);
        return true;
    }

    // system('command')
    if (lower(s).startsWith("system(") || lower(s).startsWith("run(")) {
        int lp = s.indexOf('(');
        int rp = s.lastIndexOf(')');
        if (lp < 0 || rp < 0 || rp <= lp) { err = "Invalid system(...) syntax"; return false; }
        QString inner = s.mid(lp+1, rp-lp-1).trimmed();
        QString evalErr;
        QVariant cmd = evalExpression(inner, vars, evalErr); if (!evalErr.isEmpty()) { err = evalErr; return false; }
        int exitCode = executeSystemCommand(cmd.toString());
        // user can capture exit code with assignment syntax handled earlier
        Q_UNUSED(exitCode);
        return true;
    }

    // SQL statements (starting with SELECT / UPDATE / INSERT / DELETE)
    QString low = lower(s);
    if (low.startsWith("select")) {
        // show result; no variable assignment in this statement itself
        if (!executeSqlSelectAndShow(db, s, parent, vars, QString(), err)) return false;
        return true;
    }
    if (low.startsWith("update") || low.startsWith("insert") || low.startsWith("delete")) {
        int affected = 0;
        if (!executeSqlMod(db, s, affected, err)) return false;
        // no assignment unless user wrote var=(UPDATE ...); which was handled earlier as inline parenthesized RHS.
        return true;
    }

    // If unknown statement: return error
    err = QString("Unknown statement: %1").arg(s);
    return false;
}

// Main executor: executes stmts from idx until end (or until caller asks to stop by idx change)
bool executeStatements(const QVector<QString> &stmts, int &idx, QMap<QString,QVariant> &vars, QWidget *parent, QSqlDatabase &db, QString &err) {
    while (idx < stmts.size()) {
        QString s = trim(stmts[idx]);
        if (s.isEmpty()) { ++idx; continue; }

        // control structures
        if (startsIf(s)) {
            // parse condition inside parentheses
            int lp = s.indexOf('(');
            int rp = s.lastIndexOf(')');
            if (lp < 0 || rp < 0 || rp <= lp) { err = "Malformed if condition"; return false; }
            QString condExpr = s.mid(lp+1, rp-lp-1);
            QString eErr;
            QVariant condVal = evalExpression(condExpr, vars, eErr);
            if (!eErr.isEmpty()) { err = eErr; return false; }
            bool condTrue = condVal.toBool();

            // find matching endif considering nested ifs
            int j = idx + 1;
            int depth = 0;
            int elsePos = -1;
            int endifPos = -1;
            // We need to collect else-if and else positions at this nesting level
            for (int k = idx + 1; k < stmts.size(); ++k) {
                QString ss = trim(stmts[k]);
                QString lowk = lower(ss);
                if (startsIf(ss)) { ++depth; continue; }
                if (lowk == "endif") {
                    if (depth == 0) { endifPos = k; break; }
                    --depth; continue;
                }
                // record first else/else if at this level
                if (depth == 0 && (lowk.startsWith("else if") || lowk.startsWith("elseif") || lowk == "else")) {
                    // We will handle by iterating between idx and endif, picking the right branch by evaluating conditions sequentially.
                }
            }
            if (endifPos < 0) { err = "Missing endif"; return false; }

            // Now execute branch logic: iterate between idx+1 and endifPos and find segments
            // We'll walk k from idx+1 to endifPos, splitting into sequences separated by "else if(...)" or "else"
            int k = idx + 1;
            bool branchExecuted = false;
            while (k < endifPos) {
                // determine next separator
                int segStart = k;
                int segEnd = segStart;
                QString nextTokenLow;
                for (; segEnd < endifPos; ++segEnd) {
                    QString ss = trim(stmts[segEnd]);
                    QString lowk = lower(ss);
                    if (lowk.startsWith("else if") || lowk.startsWith("elseif") || lowk == "else") {
                        nextTokenLow = lowk;
                        break;
                    }
                }
                // segStart..segEnd-1 is the body
                // If first segment: use initial condition; otherwise evaluate the condition contained in separator (if else if)
                bool executeThis = false;
                if (segStart == idx + 1) {
                    if (condTrue) executeThis = true;
                } else {
                    // segStart corresponds to body after an else-if or else; the corresponding separator is stmts[segStart-1]
                    QString sep = trim(stmts[segStart-1]);
                    QString sepLow = lower(sep);
                    if (sepLow == "else") {
                        executeThis = true;
                    } else {
                        // else if (cond)
                        int lp2 = sep.indexOf('(');
                        int rp2 = sep.lastIndexOf(')');
                        if (lp2 < 0 || rp2 < 0 || rp2 <= lp2) { err = "Malformed else if condition"; return false; }
                        QString cond2 = sep.mid(lp2+1, rp2-lp2-1);
                        QString eErr2; QVariant v2 = evalExpression(cond2, vars, eErr2);
                        if (!eErr2.isEmpty()) { err = eErr2; return false; }
                        if (v2.toBool()) executeThis = true;
                    }
                }
                if (executeThis && !branchExecuted) {
                    // execute stmts segStart .. segEnd-1
                    int subIdx = segStart;
                    while (subIdx < segEnd) {
                        if (!executeStatements(stmts, subIdx, vars, parent, db, err)) return false;
                    }
                    branchExecuted = true;
                }
                // move to next segment (skip the separator if any)
                k = segEnd + 1;
            }
            // move idx past endif
            idx = endifPos + 1;
            continue;
        }

        if (startsWhile(s)) {
            // parse condition
            int lp = s.indexOf('(');
            int rp = s.lastIndexOf(')');
            if (lp < 0 || rp < 0 || rp <= lp) { err = "Malformed while condition"; return false; }
            QString condExpr = s.mid(lp+1, rp-lp-1);
            // find matching endwhile
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
            // loop
            while (true) {
                QString eErr;
                QVariant condVal = evalExpression(condExpr, vars, eErr);
                if (!eErr.isEmpty()) { err = eErr; return false; }
                if (!condVal.toBool()) break;
                int subIdx = idx + 1;
                while (subIdx < endwhilePos) {
                    if (!executeStatements(stmts, subIdx, vars, parent, db, err)) return false;
                }
            }
            idx = endwhilePos + 1;
            continue;
        }

        if (startsFor(s)) {
            // for(init; cond; after)
            int lp = s.indexOf('(');
            int rp = s.lastIndexOf(')');
            if (lp < 0 || rp < 0 || rp <= lp) { err = "Malformed for header"; return false; }
            QString inside = s.mid(lp+1, rp-lp-1);
            // split by top-level semicolons (no strings in for header expected)
            QStringList parts = inside.split(';');
            if (parts.size() < 3) { err = "for(...) must have init;cond;after"; return false; }
            QString init = parts[0].trimmed();
            QString cond = parts[1].trimmed();
            QString after = parts[2].trimmed();
            // find endfor
            int endforPos = -1; int depth = 0;
            for (int k = idx + 1; k < stmts.size(); ++k) {
                if (startsFor(stmts[k])) ++depth;
                if (isEndFor(trim(stmts[k]))) {
                    if (depth == 0) { endforPos = k; break; }
                    --depth;
                }
            }
            if (endforPos < 0) { err = "Missing endfor"; return false; }
            // execute init
            if (!init.isEmpty()) {
                QString eErr; QVariant v = evalExpression(init, vars, eErr);
                if (!eErr.isEmpty()) {
                    // However init could be assignment style like i=0, then evalExpression won't parse; attempt to execute statement
                    QString tmpErr;
                    if (!executeSingleStatement(init + ";", vars, parent, db, tmpErr)) { err = tmpErr; return false; }
                }
            }
            // loop
            while (true) {
                if (!cond.isEmpty()) {
                    QString eErr; QVariant v = evalExpression(cond, vars, eErr);
                    if (!eErr.isEmpty()) { err = eErr; return false; }
                    if (!v.toBool()) break;
                }
                // body
                int subIdx = idx + 1;
                while (subIdx < endforPos) {
                    if (!executeStatements(stmts, subIdx, vars, parent, db, err)) return false;
                }
                // after
                if (!after.isEmpty()) {
                    QString eErr; QVariant v = evalExpression(after, vars, eErr);
                    if (!eErr.isEmpty()) {
                        // if after is assignment, attempt executing as statement
                        QString tmpErr;
                        if (!executeSingleStatement(after + ";", vars, parent, db, tmpErr)) { err = tmpErr; return false; }
                    }
                }
            }
            idx = endforPos + 1;
            continue;
        }

        // end markers - should be handled by parent caller, return to allow outer to manage
        if (isEndIf(s) || isEndWhile(s) || isEndFor(s) || s.toLower().startsWith("else")) {
            return true;
        }

        // detect pattern: var = (SELECT ...) or var = (UPDATE ...) where SQL is inline -> handled in executeSingleStatement
        // Also detect the pattern var = (SELECT ...); followed by a SELECT; i.e. variable assignment indicating the next SELECT is interactive.
        QRegularExpression assignSqlNextRe("^([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*\\(\\s*SELECT\\b.*\\)$", QRegularExpression::CaseInsensitiveOption);
        if (assignSqlNextRe.match(s).hasMatch()) {
            // inline parenthesized SELECT handled by executeSingleStatement, but if user wrote var=(SELECT ...);SELECT ...;
            // We'll just call executeSingleStatement which handles parenthesized select inline too
            QString tmpErr;
            if (!executeSingleStatement(s, vars, parent, db, tmpErr)) { err = tmpErr; return false; }
            ++idx;
            continue;
        }

        // Check pattern: var=(SELECT ...) but without inner SQL (i.e. user wrote var=(SELECT) and next stmt is SELECT ...; )
        QRegularExpression assignNextRe("^([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*\\(\\s*SELECT\\s*\\)$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch m2 = assignNextRe.match(s);
        if (m2.hasMatch()) {
            QString varname = m2.captured(1);
            // next statement must be the SELECT
            if (idx + 1 >= stmts.size()) { err = "Expected SELECT after assignment marker"; return false; }
            QString next = trim(stmts[idx+1]);
            if (!lower(next).startsWith("select")) { err = "Expected SELECT after assignment marker"; return false; }
            if (!executeSqlSelectAndShow(db, next, parent, vars, varname, err)) return false;
            idx += 2; continue;
        }

        // Check pattern: var=(UPDATE) then next stmt UPDATE ...
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

        // Finally, simple statement execution
        QString outAssigned;
        if (!executeSingleStatement(s, vars, parent, db, err, &outAssigned)) return false;
        ++idx;
    }
    return true;
}

} // anonymous namespace

bool runScript(const QString &script, QWidget *parent, QSqlDatabase db) {
    QString noComments = removeComments(script);
    QVector<QString> stmts = splitStatements(noComments);

    QMap<QString,QVariant> vars; // untyped variables

    int idx = 0;
    QString err;
    bool ok = executeStatements(stmts, idx, vars, parent, db, err);
    if (!ok) {
        qWarning() << "Script error:" << err;
        // show message box about error (optional)
        if (parent) {
            QMetaObject::invokeMethod(qApp, [parent, err](){
                QMessageBox::critical(parent, "Script error", err);
            }, Qt::QueuedConnection);
        }
    }
    return ok;
}
