#include "fadascriptengine.h"
#include "qboxlayout.h"
#include "qdialog.h"
#include "qdialogbuttonbox.h"
#include "qheaderview.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "qsqlrecord.h"
#include "qstandarditemmodel.h"
#include "qtableview.h"
//#include <QMessageBox>
#include "Dialogs.h"

FadaScriptEngine::FadaScriptEngine(QObject *parent)
    : QObject(parent)
{}

bool FadaScriptEngine::runScript(QString script, QSqlDatabase db, bool rollBack, QPlainTextEdit *SQLEdit, FadaHighlighter *hl) {
    // Build scriptWithoutComments and mapping to original script positions
    scriptError.clear();
    int statementId = 0;
    exitScript=false;


    QVector<int> origMap; // origMap[i] = index in original script corresponding to scriptWithoutComments[i]
    QString scriptWithoutComments = prepareScript(script, origMap);

    QVector<int> stmtStarts;
    QVector<QString> stmts = splitStatements(scriptWithoutComments, stmtStarts);

    QMap<QString,QVariant> vars;
    int errorPosInNoComments = -1;

    bool ok = executeStatements(stmts, stmtStarts, statementId, vars, db, &errorPosInNoComments);

    if (!ok) {
        qWarning() << "Script error:" << scriptError;
        showMessageBox("Script error", scriptError,QStyle::SP_MessageBoxCritical);
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

// Remove comments starting with -- until EOL.
// Produces the filtered script (scriptWithoutComments) and a mapping vector origPosMap where
// origPosMap[i] is the index in original originalScript that corresponds to scriptWithoutComments[i].
QString FadaScriptEngine::prepareScript(const QString &originalScript, QVector<int> &origPosMap) {
    QString out;
    origPosMap.clear();
    const int n = originalScript.size();
    int i = 0;
    while (i < n) {
        // if we find "--", skip to end of line
        if (i + 1 < n && originalScript[i] == '-' && originalScript[i+1] == '-') {
            // skip until end of line or end of input
            while (i < n && originalScript[i] != '\n') ++i;
            // if newline present, include it (to preserve line numbers)
            if (i < n && originalScript[i] == '\n') {
                out += '\n';
                origPosMap.append(i);
                ++i;
            }
            continue;
        }
        // regular char -> append and map position
        out += originalScript[i];
        origPosMap.append(i);
        ++i;
    }
    return out;
}

// Split into top-level statements. Semicolon ';' separates statements when not inside string and not inside a top-level brace group.
// For control blocks (if/else/while/for) we include the whole if(...) { ... } [else if(...) { ... }] [else { ... }] sequence as one statement.
QVector<QString> FadaScriptEngine::splitStatements(const QString &script, QVector<int> &starts) {
    QVector<QString> result;
    starts.clear();
    QString curStatement;
    bool inSingle = false;
    int i = 0;
    int scriptSize = script.size();
    int curStart = 0;
    while (i < scriptSize) {
        QChar c = script[i];
        if (curStatement.isEmpty()) curStart = i;
        // string literal handling
        if (c == '\'') {
            curStatement += c;
            ++i;
            while (i < scriptSize) {
                curStatement += script[i];
                if (script[i] == '\'') {
                    if (i+1 < scriptSize && script[i+1] == '\'') { // escape
                        curStatement += script[i+1];
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
            if (curStatement.trimmed().isEmpty()) {
                int j = i;
                // skip whitespace
                while (j < scriptSize && script[j].isSpace()) { curStatement += script[j]; ++j; }
                // if "if" or "while" or "for" starts at j
                QString remaining = script.mid(j).toLower();
                if (remaining.startsWith("if") || remaining.startsWith("while") || remaining.startsWith("for")) {
                    // ensure keyword is followed by space or '('
                    int kwLen = remaining.startsWith("if") ? 2 : remaining.startsWith("while") ? 5 : 3;
                    if (j+kwLen < scriptSize && (script[j+kwLen].isSpace() || script[j+kwLen] == '(')) {
                        // copy until we find the opening '{' of the first block
                        int k = j;
                        // move until we find the first '{' that starts the block (handle strings)
                        bool foundBrace = false;
                        for (; k < scriptSize; ++k) {
                            QChar cc = script[k];
                            if (cc == '\'') {
                                // copy string
                                curStatement += script.mid(i, k - i); // copy chunk until string
                                // append string properly
                                int sPos = k;
                                curStatement += script[sPos];
                                ++sPos;
                                while (sPos < scriptSize) {
                                    curStatement += script[sPos];
                                    if (script[sPos] == '\'') {
                                        if (sPos+1 < scriptSize && script[sPos+1] == '\'') { curStatement += script[sPos+1]; sPos += 2; continue; }
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
                                    curStatement += script.mid(i);
                                    i = scriptSize;
                                    break;
                                }
                                // include up to match
                                curStatement += script.mid(i, match - i + 1);
                                i = match + 1;
                                // Now check for trailing else/else if clauses, include them if present
                                while (true) {
                                    // skip spaces
                                    int p = i;
                                    while (p < scriptSize && script[p].isSpace()) ++p;
                                    // check if 'else' starts at p
                                    if (p + 4 <= scriptSize && script.mid(p,4).toLower() == "else") {
                                        // include 'else' token and possible 'if' header and its block
                                        // find next '{' after p
                                        int nextBrace = -1;
                                        int q = p;
                                        while (q < scriptSize) {
                                            if (script[q] == '{') { nextBrace = q; break; }
                                            if (script[q] == '\'') {
                                                // skip string
                                                ++q;
                                                while (q < scriptSize) {
                                                    if (script[q] == '\'') {
                                                        if (q+1 < scriptSize && script[q+1] == '\'') { q += 2; continue; }
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
                                            curStatement += script.mid(i);
                                            i = scriptSize;
                                            break;
                                        }
                                        int match2 = findMatchingBrace(script, nextBrace);
                                        if (match2 < 0) { curStatement += script.mid(i); i = scriptSize; break; }
                                        // include everything up to match2
                                        curStatement += script.mid(i, match2 - i + 1);
                                        i = match2 + 1;
                                        continue; // check for further else
                                    }
                                    break;
                                }
                                foundBrace = true;
                                break;
                            }
                        }
                        if (!foundBrace && i < scriptSize) {
                            // didn't find a brace, fallback to accumulate char and continue
                            curStatement += script[i];
                            ++i;
                            continue;
                        }
                        // we've already advanced i to after the block(s); next loop iteration will set new curStart
                        // finalize statement
                        QString t = trim(curStatement);
                        if (!t.isEmpty()) { result.append(t); starts.append(curStart); }
                        curStatement.clear();
                        continue;
                    }
                }
                // if not control keyword, fall through to normal char append
            }
        }

        // normal char handling
        if (!inSingle && c == ';') {
            QString t = trim(curStatement);
            if (!t.isEmpty()) { result.append(t); starts.append(curStart); }
            curStatement.clear();
            ++i;
            continue;
        } else {
            curStatement += c;
            ++i;
        }
    }
    QString t = trim(curStatement);
    if (!t.isEmpty()) { result.append(t); starts.append(curStart); }
    return result;
}

// Main executor for a list of statements where stmtStarts are absolute offsets in scriptWithoutComments text.
bool FadaScriptEngine::executeStatements(const QVector<QString> &stmts, const QVector<int> &stmtStarts, int &statementId, QMap<QString,QVariant> &vars, QSqlDatabase &db, int *outErrorPos) {
    while (statementId < stmts.size()) {
        QString statement = trim(stmts[statementId]);
        if (statement.isEmpty()) { ++statementId; continue; }

        QString lowStmt = lower(statement);
        if (lowStmt.startsWith("if") && !statement.contains('{')) {
            scriptError = "Missing '{' after if statement";
            if (outErrorPos) {
                int parenR = statement.indexOf(')');
                int pos = (parenR >= 0) ? stmtStarts[statementId] + parenR + 1 : stmtStarts[statementId];
                *outErrorPos = pos;
            }
            return false;
        }
        if (lowStmt.startsWith("while") && !statement.contains('{')) {
            scriptError = "Missing '{' after while statement";
            if (outErrorPos) {
                int parenR = statement.indexOf(')');
                int pos = (parenR >= 0) ? stmtStarts[statementId] + parenR + 1 : stmtStarts[statementId];
                *outErrorPos = pos;
            }
            return false;
        }
        if (lowStmt.startsWith("for") && !statement.contains('{')) {
            scriptError = "Missing '{' after for statement";
            if (outErrorPos) {
                int parenR = statement.indexOf(')');
                int pos = (parenR >= 0) ? stmtStarts[statementId] + parenR + 1 : stmtStarts[statementId];
                *outErrorPos = pos;
            }
            return false;
        }


        // IF (...) { ... } [ else if(...) { ... } ] [ else { ... } ]
        if (lower(statement).startsWith("if") && statement.contains('{')) {
            // parse first condition
            int parenL = statement.indexOf('(');
            int parenR = statement.indexOf(')', parenL);
            if (parenL < 0 || parenR < 0) { scriptError = "Malformed if condition"; return false; }
            QString condExpr = statement.mid(parenL+1, parenR-parenL-1).trimmed();
            int condPos = stmtStarts[statementId] + statement.indexOf(condExpr);
            int condErrPos = -1;
            QVariant condVal = evalExpression(condExpr, vars, condPos, &condErrPos);
            if (!scriptError.isEmpty()) {
                if (outErrorPos && condErrPos>=0) *outErrorPos = condErrPos;
                return false;
            }

            // find bodies for if / else if / else inside the same statement string s
            // We'll iterate over s to extract sequences of (cond, bodySource, bodyStartOffset)
            struct Branch { QString cond; int condAbsPos; QString body; int bodyAbsStart; bool isElse; };
            QVector<Branch> branches;

            int pos = 0;
            // parse initial if
            pos = statement.indexOf('{', parenR);
            if (pos < 0) { scriptError = "Malformed if block"; return false; }
            int match = findMatchingBrace(statement, pos);
            if (match < 0) { scriptError = "Malformed if block braces"; return false; }
            QString body = statement.mid(pos+1, match-pos-1);
            int bodyAbsStart2 = stmtStarts[statementId] + pos + 1;
            branches.append({condExpr, static_cast<int>(stmtStarts[statementId] + statement.indexOf(condExpr)), body, bodyAbsStart2, false});
            pos = match + 1;
            // handle chained else if / else
            while (true) {
                // skip whitespace
                while (pos < statement.size() && statement[pos].isSpace()) ++pos;
                if (pos >= statement.size()) break;
                if (statement.mid(pos,4).toLower() == "else") {
                    pos += 4;
                    // skip whitespace
                    while (pos < statement.size() && statement[pos].isSpace()) ++pos;
                    if (pos < statement.size() && statement.mid(pos,2).toLower() == "if") {
                        // else if
                        int pIfStart = pos;
                        int pParenL = statement.indexOf('(', pos);
                        int pParenR = statement.indexOf(')', pParenL);
                        if (pParenL < 0 || pParenR < 0) { scriptError = "Malformed else if"; return false; }
                        QString cond2 = statement.mid(pParenL+1, pParenR-pParenL-1).trimmed();
                        int pBodyL = statement.indexOf('{', pParenR);
                        if (pBodyL < 0) { scriptError = "Malformed else if block"; return false; }
                        int pMatch = findMatchingBrace(statement, pBodyL);
                        if (pMatch < 0) { scriptError = "Malformed else if braces"; return false; }
                        QString body2 = statement.mid(pBodyL+1, pMatch-pBodyL-1);
                        int body2Abs = stmtStarts[statementId] + pBodyL + 1;
                        branches.append({cond2, static_cast<int>(stmtStarts[statementId] + statement.indexOf(cond2, pParenL)), body2, body2Abs, false});
                        pos = pMatch + 1;
                        continue;
                    } else {
                        // else { ... }
                        int pBodyL = statement.indexOf('{', pos);
                        if (pBodyL < 0) { scriptError = "Malformed else block"; return false; }
                        int pMatch = findMatchingBrace(statement, pBodyL);
                        if (pMatch < 0) { scriptError = "Malformed else braces"; return false; }
                        QString body2 = statement.mid(pBodyL+1, pMatch-pBodyL-1);
                        int body2Abs = stmtStarts[statementId] + pBodyL + 1;
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
                        if (!executeFromSource(b.body, b.bodyAbsStart, vars, db, stmtStarts, outErrorPos))
                            return false;
                        executed = true;
                        break;
                    }
                } else {
                    int localErr = -1;
                    QVariant v = evalExpression(b.cond, vars, b.condAbsPos, &localErr);
                    if (!scriptError.isEmpty()) {
                        if (outErrorPos && localErr>=0) *outErrorPos = localErr;
                        return false;
                    }
                    if (v.toBool()) {
                        if (!executeFromSource(b.body, b.bodyAbsStart, vars, db, stmtStarts, outErrorPos))
                            return false;
                        executed = true;
                        break;
                    }
                }
            }
            ++statementId;
            continue;
        }

        // WHILE (...) { ... }
        if (lower(statement).startsWith("while") && statement.contains('{')) {
            int parenL = statement.indexOf('(');
            int parenR = statement.indexOf(')', parenL);
            if (parenL < 0 || parenR < 0) { scriptError = "Malformed while condition"; return false; }
            QString condExpr = statement.mid(parenL+1, parenR-parenL-1).trimmed();
            int bodyL = statement.indexOf('{', parenR);
            if (bodyL < 0) { scriptError = "Malformed while body"; return false; }
            int match = findMatchingBrace(statement, bodyL);
            if (match < 0) { scriptError = "Malformed while braces"; return false; }
            QString body = statement.mid(bodyL+1, match-bodyL-1);
            int bodyAbs = stmtStarts[statementId] + bodyL + 1;
            while (true) {
                int condPos = stmtStarts[statementId] + statement.indexOf(condExpr);
                int condErrPos = -1;
                QVariant condVal = evalExpression(condExpr, vars, condPos, &condErrPos);
                if (!scriptError.isEmpty()) {
                    if (outErrorPos && condErrPos>=0) *outErrorPos = condErrPos;
                    return false;
                }
                if (!condVal.toBool()) break;
                if (!executeFromSource(body, bodyAbs, vars, db, stmtStarts, outErrorPos)) return false;
                if (exitScript) return true;
            }
            ++statementId;
            continue;
        }

        // FOR(init; cond; after) { ... }
        if (lower(statement).startsWith("for") && statement.contains('{')) {
            int parenL = statement.indexOf('(');
            int parenR = statement.indexOf(')', parenL);
            if (parenL < 0 || parenR < 0) { scriptError = "Malformed for header"; return false; }
            QString inside = statement.mid(parenL+1, parenR-parenL-1);
            QStringList parts = inside.split(';');
            if (parts.size() < 3) { scriptError = "for(...) must have init;cond;after"; return false; }
            QString init = parts[0].trimmed();
            QString cond = parts[1].trimmed();
            QString after = parts[2].trimmed();
            int bodyL = statement.indexOf('{', parenR);
            if (bodyL < 0) { scriptError = "Malformed for body"; return false; }
            int match = findMatchingBrace(statement, bodyL);
            if (match < 0) { scriptError = "Malformed for braces"; return false; }
            QString body = statement.mid(bodyL+1, match-bodyL-1);
            int bodyAbs = stmtStarts[statementId] + bodyL + 1;
            // init
            if (!init.isEmpty()) {
                int posInit = stmtStarts[statementId] + statement.indexOf(init);
                int initErrPos = -1;
                QVariant v = evalExpression(init, vars, posInit, &initErrPos);
                if (!scriptError.isEmpty()) {
                    // try as statement (assignment)
                    if (!executeSingleStatement(init + ";", stmtStarts[statementId] + statement.indexOf(init), vars, db, nullptr, outErrorPos))
                        return false;
                    else
                        scriptError.clear();
                }
            }
            // loop
            while (true) {
                if (!cond.isEmpty()) {
                    int posCond = stmtStarts[statementId] + statement.indexOf(cond);
                    int condErrPos = -1;
                    QVariant vcond = evalExpression(cond, vars, posCond, &condErrPos);
                    if (!scriptError.isEmpty()) {
                        if (outErrorPos && condErrPos>=0) *outErrorPos = condErrPos;
                        return false;
                    }
                    if (!vcond.toBool()) break;
                }
                if (!executeFromSource(body, bodyAbs, vars, db, stmtStarts, outErrorPos)) return false;
                if (!after.isEmpty()) {
                    int posAfter = stmtStarts[statementId] + statement.indexOf(after);
                    int afterErrPos = -1;
                    QVariant va = evalExpression(after, vars, posAfter, &afterErrPos);
                    if (!scriptError.isEmpty()) {
                        // try as statement
                        QString tmpErr;
                        if (!executeSingleStatement(after + ";", stmtStarts[statementId] + statement.indexOf(after), vars, db, nullptr, outErrorPos))
                            return false;
                        else
                            scriptError.clear();
                        if (exitScript) return true;
                    }
                }
            }
            ++statementId;
            continue;
        }

        // Patterns like var = (SELECT) followed by SELECT statement - we keep same behavior as before:
        QRegularExpression assignNextRe("^([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*\\(\\s*SELECT\\s*\\)$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch m2 = assignNextRe.match(statement);
        if (m2.hasMatch()) {
            QString varname = m2.captured(1);
            if (statementId + 1 >= stmts.size()) { scriptError = "Expected SELECT after assignment marker"; return false; }
            QString next = trim(stmts[statementId+1]);
            if (!lower(next).startsWith("select")) { scriptError = "Expected SELECT after assignment marker"; return false; }
            if (!executeSqlSelectAndShow(db, next, vars, varname)) return false;
            statementId += 2; continue;
        }
        QRegularExpression assignNextUpdRe("^([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*\\(\\s*UPDATE\\s*\\)$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch m3 = assignNextUpdRe.match(statement);
        if (m3.hasMatch()) {
            QString varname = m3.captured(1);
            if (statementId + 1 >= stmts.size()) { scriptError = "Expected UPDATE after assignment marker"; return false; }
            QString next = trim(stmts[statementId+1]);
            if (!lower(next).startsWith("update")) { scriptError = "Expected UPDATE after assignment marker"; return false; }
            int affected = 0;
            if (!executeSqlMod(db, next, affected)) return false;
            vars.insert(varname, affected);
            statementId += 2; continue;
        }

        // Finally execute a simple statement
        int stmtAbsStart = (statementId < stmtStarts.size() ? stmtStarts[statementId] : 0);
        if (!executeSingleStatement(statement, stmtAbsStart, vars, db, nullptr, outErrorPos))
            return false;
        if (exitScript) return true;
        ++statementId;
    }
    return true;
}

// Execute script source (a chunk) given its base offset in scriptWithoutComments; this helper splits and transforms starts to absolute offsets.
bool FadaScriptEngine::executeFromSource(const QString &source, int baseOffset, QMap<QString,QVariant> &vars, QSqlDatabase &db, const QVector<int> &origStmtStarts, int *outErrorPos) {
    Q_UNUSED(origStmtStarts);
    QVector<int> localStarts;
    QVector<QString> localStmts = splitStatements(source, localStarts);
    // convert localStarts to absolute offsets
    QVector<int> absStarts;
    absStarts.reserve(localStarts.size());
    for (int s : localStarts) absStarts.append(baseOffset + s);
    int statementId = 0;
    return [&] (const QVector<QString> &stmts, const QVector<int> &stmtStarts, int &statementId, QMap<QString,QVariant> &vars, QSqlDatabase &db, int *outErr) -> bool {
        // forward to main executor (defined below). We'll use a function pointer call by name.
        return executeStatements(stmts, stmtStarts, statementId, vars, db, outErr);
    } (localStmts, absStarts, statementId, vars, db, outErrorPos);
}

// Execute a single statement. rawStmtStart is the statement's start offset in the scriptWithoutComments script.
bool FadaScriptEngine::executeSingleStatement(const QString &rawStmt, int rawStmtStart, QMap<QString,QVariant> &vars, QSqlDatabase &db, QString *outAssignedVar, int *outErrorPos) {
    QString singleStatement = trim(rawStmt);
    if (singleStatement.isEmpty()) return true;

    // assignment: name = expr
    QRegularExpression assignRe("^([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*(.+)$");
    QRegularExpressionMatch m = assignRe.match(singleStatement);
    if (m.hasMatch()) {
        QString name = m.captured(1);
        QString rhs = trim(m.captured(2));
        // inline parenthesized SQL: name = (UPDATE ...); name = (SELECT ...)
        if (rhs.startsWith('(') && rhs.endsWith(')')) {
            QString inner = rhs.mid(1, rhs.size()-2).trimmed();
            QString low = lower(inner);
            if (low.startsWith("update") || low.startsWith("insert") || low.startsWith("delete")) {
                int affected = 0;
                if (!executeSqlMod(db, inner, affected)) return false;
                vars.insert(name, affected);
                if (outAssignedVar) *outAssignedVar = name;
                return true;
            } else if (low.startsWith("select")) {
                QVariant value00;
                if (!executeSqlSelect(db, inner, value00)) return false;
                vars.insert(name, value00);
                if (outAssignedVar) *outAssignedVar = name;
                return true;
            }
        }
        if (rhs.startsWith("selectRow(") && rhs.endsWith(')')) {
            QString inner = rhs.mid(10, rhs.size()-11).trimmed();
            QString low = lower(inner);
            if (low.startsWith("select")) {
                if (!executeSqlSelectAndShow(db, inner, vars, name)) return false;
                if (outAssignedVar) *outAssignedVar = name;
                return true;
            }
        }
        // normal expression; compute position of rhs in rawStmt to compute absolute base
        int localPos = rawStmt.indexOf(rhs);
        int base = rawStmtStart + (localPos >= 0 ? localPos : 0);
        int exprErrPos = -1;
        QVariant val = evalExpression(rhs, vars, base, &exprErrPos);
        if (!scriptError.isEmpty()) {
            scriptError = QString("Expression error: %1").arg(scriptError);
            if (outErrorPos && exprErrPos >= 0) *outErrorPos = exprErrPos;
            return false;
        }
        vars.insert(name, val);
        return true;
    }

    // messageDialog(title, text)
    if (lower(singleStatement).startsWith("messagedialog")) {
        int lp = singleStatement.indexOf('('), rp = singleStatement.lastIndexOf(')');
        if (lp < 0 || rp < 0 || rp <= lp) { scriptError = "Invalid messageDialog syntax"; return false; }
        QString args = singleStatement.mid(lp+1, rp-lp-1);
        QStringList parts;
        QString cur; bool inS=false;
        for (int i=0;i<args.size();++i){
            QChar c = args[i];
            if (c == '\'') { inS = !inS; cur += c; continue; }
            if (c == ',' && !inS) { parts << cur.trimmed(); cur.clear(); continue; }
            cur += c;
        }
        if (!cur.isEmpty()) parts << cur.trimmed();
        if (parts.size() < 2) { scriptError = "messageDialog requires two arguments"; return false; }
        int pos0 = rawStmt.indexOf(parts[0]);
        int pos1 = rawStmt.indexOf(parts[1], pos0>=0?pos0:0);
        int errPos = -1;
        QVariant a = evalExpression(parts[0], vars, rawStmtStart + (pos0>=0?pos0:0), &errPos);
        if (!scriptError.isEmpty()) {
            if (outErrorPos && errPos>=0) *outErrorPos = errPos;
            return false;
        }
        QVariant b = evalExpression(parts[1], vars, rawStmtStart + (pos1>=0?pos1:0), &errPos);
        if (!scriptError.isEmpty()) {
            if (outErrorPos && errPos>=0) *outErrorPos = errPos;
            return false;
        }
        showMessageBox(a.toString(), b.toString());
        return true;
    }

    // okCancelDialog(title, text) -> returns boolean if assigned
    if (lower(singleStatement).startsWith("okCancelDialog")) {
        int lp = singleStatement.indexOf('('), rp = singleStatement.lastIndexOf(')');
        if (lp < 0 || rp < 0 || rp <= lp) { scriptError = "Invalid okCancelDialog syntax"; return false; }
        QString args = singleStatement.mid(lp+1, rp-lp-1);
        QStringList parts;
        QString cur; bool inS=false;
        for (int i=0;i<args.size();++i){
            QChar c = args[i];
            if (c == '\'') { inS = !inS; cur += c; continue; }
            if (c == ',' && !inS) { parts << cur.trimmed(); cur.clear(); continue; }
            cur += c;
        }
        if (!cur.isEmpty()) parts << cur.trimmed();
        if (parts.size() < 2) { scriptError = "okCancelDialog requires two arguments"; return false; }
        int pos0 = rawStmt.indexOf(parts[0]);
        int pos1 = rawStmt.indexOf(parts[1], pos0>=0?pos0:0);
        int errPos = -1;
        QVariant a = evalExpression(parts[0], vars, rawStmtStart + (pos0>=0?pos0:0), &errPos);
        if (!scriptError.isEmpty()) {
            if (outErrorPos && errPos>=0) *outErrorPos = errPos;
            return false;
        }
        QVariant b = evalExpression(parts[1], vars, rawStmtStart + (pos1>=0?pos1:0), &errPos);
        if (!scriptError.isEmpty()) {
            if (outErrorPos && errPos>=0) *outErrorPos = errPos;
            return false;
        }

        bool res = showOkCancel(a.toString(), b.toString());

        Q_UNUSED(res);
        return true;
    }

    // system('command') or run('command')
    if (lower(singleStatement).startsWith("system(") || lower(singleStatement).startsWith("run(")) {
        int lp = singleStatement.indexOf('('), rp = singleStatement.lastIndexOf(')');
        if (lp < 0 || rp < 0 || rp <= lp) { scriptError = "Invalid system(...) syntax"; return false; }
        QString inner = singleStatement.mid(lp+1, rp-lp-1).trimmed();
        int posInner = rawStmt.indexOf(inner);
        int errPos = -1;
        QVariant cmd = evalExpression(inner, vars, rawStmtStart + (posInner>=0?posInner:0), &errPos);
        if (!scriptError.isEmpty()) {
            if (outErrorPos && errPos>=0) *outErrorPos = errPos;
            return false;
        }
        int exitCode = executeSystemCommand(cmd.toString());
        Q_UNUSED(exitCode);
        return true;
    }

    // SQL: SELECT -> show, UPDATE/INSERT/DELETE -> execute
    QString low = lower(singleStatement);
    if (low.startsWith("select")) { //todo
        if (!executeSqlSelectAndShow(db, singleStatement, vars, QString())) return false;
        return true;
    }
    if (low.startsWith("update") || low.startsWith("insert") || low.startsWith("delete")) {
        int affected = 0;
        if (!executeSqlMod(db, singleStatement, affected)) return false;
        return true;
    }
    if (low=="return") {
        exitScript=true;
        return true;
    }
    scriptError = QString("Unknown statement: %1").arg(singleStatement);
    return false;
}

// Evaluate an expression with error position reporting.
// baseOffset is absolute offset in scriptWithoutComments string where expr starts.
// outErrorPos receives absolute offset in scriptWithoutComments if a tokenization error occurs.
QVariant FadaScriptEngine::evalExpression(const QString &expr, const QMap<QString,QVariant> &vars, int baseOffset, int *outErrorPos) {
    QString e = expr.trimmed();
    if (e.isEmpty()) return QVariant();
    int localErrPos = -1;
    QVector<Token> tokens = tokenizeExpr(e, &localErrPos);
    if (!scriptError.isEmpty()) {
        if (outErrorPos && localErrPos >= 0) *outErrorPos = baseOffset + localErrPos;
        return {};
    }
    QVector<Token> rpn;
    if (!toRPN(tokens, rpn)) {
        if (outErrorPos) *outErrorPos = baseOffset;
        return {};
    }
    QVariant res = evaluateRPN(rpn, vars);
    if (!scriptError.isEmpty()) {
        if (outErrorPos && localErrPos >= 0) *outErrorPos = baseOffset + localErrPos;
    }
    return res;
}

// GUI helpers (thread-safe)
void FadaScriptEngine::showMessageBox(const QString &shortText, const QString &text, QStyle::StandardPixmap sp) {
    if (QThread::currentThread() == qApp->thread()) {
        MessageDlg(scriptTitle,shortText,text,sp);
        //QMessageBox::information(parentWidget(),title, text);
    } else {
        QMetaObject::invokeMethod(qApp, [shortText,text, this, sp](){
            //QMessageBox::information(parentWidget(), shortText, text);
            MessageDlg(scriptTitle,shortText,text,sp);
        }, Qt::BlockingQueuedConnection);
    }
}

bool FadaScriptEngine::showOkCancel(const QString &shortTitle, const QString &text, QStyle::StandardPixmap sp) {
    bool result = false;
    if (QThread::currentThread() == qApp->thread()) {
        result = OkCancelDialog(scriptTitle,text,sp);
        // QMessageBox::StandardButton b = QMessageBox::question(parentWidget(), shortTitle, text, QMessageBox::Ok|QMessageBox::Cancel);
        // result = (b == QMessageBox::Ok);
    } else {
        QMetaObject::invokeMethod(qApp, [&result,shortTitle,text, this, sp](){
            result = OkCancelDialog(scriptTitle,text,sp);
            // QMessageBox::StandardButton b = QMessageBox::question(parentWidget(), shortTitle, text, QMessageBox::Ok|QMessageBox::Cancel);
            // result = (b == QMessageBox::Ok);
        }, Qt::BlockingQueuedConnection);
    }
    return result;
}

// SQL helpers
bool FadaScriptEngine::executeSqlMod(QSqlDatabase &db, const QString &sql, int &affectedRows) {
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        scriptError = q.lastError().text();
        return false;
    }
    affectedRows = q.numRowsAffected();
    return true;
}

bool FadaScriptEngine::executeSqlSelect(QSqlDatabase &db, const QString &sql, QVariant &value00) {
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        scriptError = q.lastError().text();
        return false;
    }
    if (!q.next())
        value00 = QVariant();
    else
        value00 = q.value(0);
    return true;
}

bool FadaScriptEngine::executeSqlSelectAndShow(QSqlDatabase &db, const QString &sql, QMap<QString,QVariant> &vars, const QString &assignVar) {
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        scriptError = q.lastError().text();
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

    QDialog dlg(parentWidget());
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
int FadaScriptEngine::executeSystemCommand(const QString &cmd) {
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

// Tokenize expression. If an unexpected character is found, set errorMsg and (if errorPos non-null) *errorPos = position in expr.
QVector<FadaScriptEngine::Token> FadaScriptEngine::tokenizeExpr(const QString &expr, int *errorPos) {
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
        scriptError = QString("Unexpected character in expression: '%1'").arg(c);
        return {};
    }
    return out;
}

int FadaScriptEngine::opPrecedence(const QString &op) {
    if (op == "||") return 1;
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") return 2;
    if (op == "+" || op == "-") return 3;
    if (op == "*" || op == "/") return 4;
    return 0;
}


bool FadaScriptEngine::toRPN(const QVector<Token> &tokens, QVector<Token> &outRPN) {
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
            if (!found) { scriptError = "Mismatched parentheses"; return false; }
        }
    }
    while (!stack.isEmpty()) {
        if (stack.last().type == T_LP || stack.last().type == T_RP) { scriptError = "Mismatched parentheses"; return false; }
        outRPN.append(stack.takeLast());
    }
    return true;
}

// Helper to get variable value with simple dot/bracket support (one level)
QVariant FadaScriptEngine::getVariableValue(const QMap<QString,QVariant> &vars, const QString &ident) {
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

// Evaluate RPN stack and return QVariant. error in scriptError.
QVariant FadaScriptEngine::evaluateRPN(const QVector<Token> &rpn, const QMap<QString,QVariant> &vars) {
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
            if (stack.size() < 1) { scriptError = "Malformed expression"; return {}; }
            if (op == "||") {
                if (stack.size() < 2) { scriptError = "Operator || needs two operands"; return {}; }
                QVariant b = stack.takeLast(); QVariant a = stack.takeLast();
                stack.append(a.toString() + b.toString());
                continue;
            }
            if (stack.size() < 2) { scriptError = QString("Operator %1 requires two operands").arg(op); return {}; }
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
                if (denom == 0) { scriptError = "Division by zero"; return {}; }
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
                scriptError = QString("Unsupported operator: %1").arg(op);
                return {};
            }
        }
    }
    if (stack.isEmpty()) return QVariant();
    return stack.last();
}

// Find matching brace index starting at pos of '{'.
// Returns index of matching '}' or -1 on error. Handles strings '...' and nested braces.
int FadaScriptEngine::findMatchingBrace(const QString &s, int startPos) {
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

