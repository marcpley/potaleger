#include "fadascriptengine2.h"
#include "Dialogs.h"
#include "FdaUtils.h"

FadaScriptEngine2::FadaScriptEngine2(QObject *parent)
    : QObject{parent}
{}

int FadaScriptEngine2::runScript(QString script, QSqlDatabase db, bool rollBack, QPlainTextEdit *SQLEdit, FadaHighlighter *hl) {

    scriptError.clear();
    lastStatementStart=0;
    bool result;

    stmt level1Statement;
    level1Statement.statement="START TRANSACTION";
    level1Statement.fromScript=false;
    result=executeSimpleStatement(level1Statement);
    resetStmt(level1Statement);

    if (result) {
        level1Statement.statement=script;
        result=execute(level1Statement);
        resetStmt(level1Statement);
    }


    if (rollBack or !result or !scriptError.isEmpty()) {
        level1Statement.statement="ROLLBACK";
        level1Statement.fromScript=false;
        executeSimpleStatement(level1Statement);
    }

    if (scriptError.isEmpty()) {
        //Add vars in hl.
    } else {
        //Select last executed line in SQLEdit.
        QTextCursor cursor = SQLEdit->textCursor();
        cursor.setPosition(lastStatementStart);
        cursor.setPosition(lastStatementStart+lastStatementHintSize, QTextCursor::KeepAnchor);
        SQLEdit->setTextCursor(cursor);
        SQLEdit->setFocus();
        qWarning() << scriptError;

        MessageDlg(scriptTitle,tr("Script error"),"Position "+QString::number(lastStatementStart)+scriptError,QStyle::SP_MessageBoxCritical);
    }

    return 0;
}

void FadaScriptEngine2::resetStmt(stmt &stmt, bool fromScript) {
    stmt.statement="";
    stmt.stmtType=st_unknown;
    stmt.bracedType=br_unknown;
    stmt.fromScript=fromScript;
    stmt.implicitTerminator=false;
    stmt.originalPos=0;
    stmt.originalSize=0;
}

bool FadaScriptEngine2::execute(stmt statement) {
    if (statement.statement.isEmpty()) return true;

    if (statement.stmtType==st_unknown) setStatementType(statement);

    if (statement.stmtType==st_single) return executeSimpleStatement(statement);
    if (statement.stmtType==st_braced) return executeBracedStatement(statement);
    if (statement.stmtType==st_multiple) {
        QList<stmt> statements=splitStatements(statement.statement,statement.originalPos);
        bool result=true;
        for (int i=0;i<statements.count();i++) {
            result=execute(statements[i]);
            if (!result) break;
        }
        return result;
    }
    scriptError+="\nUnknown statement type";
    // lastStatementStart=statement.originalPos;
    // lastStatementHintSize=statement.originalSize;
    return false;
}

bool FadaScriptEngine2::executeSimpleStatement(stmt statement) {
    //Execution (todo)
    bool result=true;

    //Report
    if (result) { //Sucess
        qDebug() << statement.originalPos << statement.statement;
    } else { //Error
        qWarning() << statement.originalPos << statement.statement;
        scriptError+="\nStatement error: "+statement.statement;
    }
    if (statement.fromScript) { //User statement
        lastStatementStart=statement.originalPos;
        lastStatementHintSize=statement.originalSize;
    }

    return result;
}

bool FadaScriptEngine2::executeBracedStatement(stmt statement) {
    if (statement.bracedType==br_if) {
        return executeIfStatement(statement);
    } else if (statement.bracedType==br_for) {
        return executeForStatement(statement);
    } else if (statement.bracedType==br_while) {
        return executeWhileStatement(statement);
    } else {
        if (statement.statement.size()>9)
            scriptError+="\nUnknown braced statement type: "+statement.statement.first(10);
        else
            scriptError+="\nUnknown braced statement type: "+statement.statement;
        lastStatementStart=statement.originalPos+1;
        int pos1=statement.statement.indexOf("(");
        int pos2=statement.statement.indexOf("{");
        if (pos1>-1 and(pos2==-1 or pos2>pos1))
            lastStatementHintSize=pos1;
        else if (pos2>-1)
            lastStatementHintSize=pos2;
        else
            lastStatementHintSize=std::min(statement.originalSize,10);
        return false;
    }
}

void FadaScriptEngine2::setStatementType(stmt &statement) {
    //Must be runned after statement construction, when statement is complete.
    int statementSize=statement.statement.size();
    int pos=0; //Position in statement.
    bool inString=false; //at pos.
    bool statementEnd=false;
    bool braced=false;
    int braceLevel=0;
    int braceErrPos=-1;
    int braceErrPosDef=0;
    QString curStatement;
    QChar c;
    while (pos<statementSize) {
        c=statement.statement[pos];

        if (c==';' and !inString and braceLevel==0) statementEnd=true;
        else if (c=='{' and !inString) {
            if (braceLevel==0) braceErrPosDef=pos;
            braceLevel++;
            braced=true;
            //Search for brace error.
            QString beforeBrace=statement.statement.first(pos).trimmed().toLower();
            if (!beforeBrace.endsWith(")") and !beforeBrace.endsWith("else"))
                braceErrPos=pos;
        } else if (c=='}' and !inString) {
            //Search for brace error.
            braceLevel--;
            if (braceLevel==0) braceErrPosDef=pos;
            if (braceLevel<0) {
                braceErrPos=pos;
                break;
            }
        }

        if (c=="'") inString=!inString; // Start or end of literal string.
        else if (c=='-' and pos+1<statementSize and statement.statement[pos+1]=='-') { // Start of comment
            while (pos+1<statementSize) {
                pos++;
                c=statement.statement[pos];
                if (c=='\n') break;
            }
        }
        pos++;
    }

    if (braceLevel!=0) {
        scriptError+="\nUnbalanced braces {}";
        statement.stmtType=st_error;
        lastStatementStart=statement.originalPos+iif(braceErrPos==-1,braceErrPosDef,braceErrPos).toInt();
        lastStatementHintSize=1;
    } else if (statementEnd) {
        statement.stmtType=st_multiple;
    } else if (braced) {
        statement.stmtType=st_braced;
    } else {
        statement.stmtType=st_single;
    }
}

void FadaScriptEngine2::setStatementBracedType(stmt &statement) {
    //Must be runned at begining of statement construction.
    if (statement.bracedType==br_unknown) {
        if (statement.statement.size()<8) {
            if (statement.statement.startsWith("if (",Qt::CaseInsensitive) or statement.statement.startsWith("if(",Qt::CaseInsensitive))
                statement.bracedType=br_if;
            else if (statement.statement.startsWith("for (",Qt::CaseInsensitive) or statement.statement.startsWith("for(",Qt::CaseInsensitive))
                statement.bracedType=br_for;
            else if (statement.statement.startsWith("while (",Qt::CaseInsensitive) or statement.statement.startsWith("while(",Qt::CaseInsensitive))
                statement.bracedType=br_while;
            else
                statement.bracedType=br_unknown;
        } else {
            statement.bracedType=br_no;
        }
    }
}

QList<FadaScriptEngine2::stmt> FadaScriptEngine2::splitStatements(QString statements, int offSet) {
    QList<stmt> result;
    int scriptSize=statements.size();
    int pos=0; //Position in script.
    int statementStartPos=0;
    bool inString=false; //at pos.
    int braceLevel=0;
    //QString statementType="";
    stmt curStatement;
    QChar c;
    while (pos<scriptSize) {
        c=statements[pos];

        if (c=="'") inString=!inString; // Start or end of literal string.
        else if (c=='{' and !inString) braceLevel++;
        else if (c=='}' and !inString) braceLevel--;
        else if (c=='\n' or c==' ') {
            if (curStatement.statement.isEmpty()) statementStartPos++;
        } else if (c=='-' and pos+1<scriptSize and statements[pos+1]=='-') { // Start of comment
            while (pos+1<scriptSize) {
                pos++;
                c=statements[pos];
                if (curStatement.statement.isEmpty()) statementStartPos++;
                if (c=='\n') break;
            }
        }

        if (endOfStatement(c,pos,scriptSize,inString,braceLevel,curStatement,statements)) {
            setStatementType(curStatement);
            curStatement.originalPos=statementStartPos+offSet;
            curStatement.originalSize=pos-statementStartPos;
            statementStartPos=pos+1;
            if (curStatement.implicitTerminator) statementStartPos--;
            result.append(curStatement);
            resetStmt(curStatement);
            //inString=false;
        } else {
            addToCurStatement(c,curStatement.statement);
        }
        pos++;
    }

    return result;
};

bool FadaScriptEngine2::endOfStatement(const QChar c, const int pos, const int scriptSize, const bool inString,
                                       const int braceLevel, stmt &curStatement, QString statements) {
    if (curStatement.bracedType==br_unknown)
        setStatementBracedType(curStatement);

    curStatement.implicitTerminator=true;

    if (c==';' and !inString and braceLevel==0) { //End of statement.
        curStatement.implicitTerminator=false;
        return true;
    } else if (pos==scriptSize-1) { //End of script.
        addToCurStatement(c,curStatement.statement);
        return true;
    } else if (c=='}' and !inString and braceLevel==0) {
        bool endOfBracedStatement=false;
        if  (pos>scriptSize-2) // '}' is last char
            endOfBracedStatement=true;
        else if (curStatement.bracedType!=br_if)
            endOfBracedStatement=true;
        else {
            QString trailing=statements.mid(pos+1).trimmed();
            QRegularExpression ifif("^if[ \\{\n]", QRegularExpression::CaseInsensitiveOption);
            QRegularExpression ifelse("^else[ \\{\n]", QRegularExpression::CaseInsensitiveOption);
            if (!ifif.match(trailing).hasMatch() and !ifelse.match(trailing).hasMatch()) //End of braced statement.
                endOfBracedStatement=true;
        }

        if (endOfBracedStatement) {
            addToCurStatement(c,curStatement.statement);
            return true;
        }
    }
    return false;
}

void FadaScriptEngine2::addToCurStatement(QChar c,QString &statement) {
    if ((c!="\n")and // no line returns.
        (c!=" " or !statement.isEmpty())) // no indent spaces.
        statement+=c;
}

bool FadaScriptEngine2::executeIfStatement(stmt statement) {
    // if (...) { ... } [ else if(...) { ... } ] [ else { ... } ]
    bool result=true;
    int pos=0; //Position in statement.
    int offSet=0;
    QString toParse=statement.statement;
    QString condition;
    QString subStatement;
    while (true) {
        pos=toParse.indexOf('(');
        if (pos>-1 and pos<toParse.indexOf('{')) {
            //Parse condition.
            toParse=toParse.mid(pos);
            offSet+=pos;
            condition=extractSubExpr(toParse,'(',')',offSet);
            result=!condition.isEmpty();
            toParse=toParse.mid(condition.size()+2);
            offSet+=condition.size()+1;
        } else {
            condition="true";
        }

        if (result) {
            //Parse substatement.
            subStatement=extractSubExpr(toParse,'{','}',offSet);
            result=!subStatement.isEmpty();
        }

        if (result) {
            if (evalExpr(condition)) {
                qDebug() << "if condition: " << condition << "true";
                stmt st;
                st.statement=subStatement;
                st.originalPos=statement.originalPos+offSet;
                result=execute(st);
                break;
            } else {
                qDebug() << "if condition: " << condition << "false";
                //Parse next condition and substatement.
                toParse=toParse.mid(subStatement.size()+2);
                offSet+=subStatement.size()+2;
                if (!toParse.contains('{'))
                    break;
            }
        } else {
            break;
        }
    }

    if (!result) {
        qWarning() << statement.originalPos << statement.statement;
        scriptError+="\nStatement error: "+statement.statement;
    }
    lastStatementStart=statement.originalPos;
    lastStatementHintSize=statement.originalSize;

    return result;
}

bool FadaScriptEngine2::executeForStatement(stmt statement) {
    //Execution (todo)
    bool result=true;

    //Report
    if (result) { //Sucess
        qDebug() << statement.originalPos << statement.statement;
    } else { //Error
        qWarning() << statement.originalPos << statement.statement;
        scriptError+="\nStatement error: "+statement.statement;
    }
    lastStatementStart=statement.originalPos;
    lastStatementHintSize=statement.originalSize;

    return result;
}

bool FadaScriptEngine2::executeWhileStatement(stmt statement) {
    //Execution (todo)
    bool result=true;

    //Report
    if (result) { //Sucess
        qDebug() << statement.originalPos << statement.statement;
    } else { //Error
        qWarning() << statement.originalPos << statement.statement;
        scriptError+="\nStatement error: "+statement.statement;
    }
    lastStatementStart=statement.originalPos;
    lastStatementHintSize=statement.originalSize;

    return result;
}

QString FadaScriptEngine2::extractSubExpr(QString expr, QChar startDelimiter, QChar endDelimiter, int &offSet) {
    int level=0;
    int pos=0;
    int exprSize=expr.size();
    bool inString=false;
    QString result;
    QChar c;
    bool error=true;
    while (pos<exprSize) {
        c=expr[pos];

        //if (c==';' and !inString) break;
        //else
        if (c==startDelimiter and !inString) {
            level++;
        } else if (c==endDelimiter and !inString) {
            level--;
            if (level==0) {
                error=false;
                break;
            }
        }

        if (c=="'") inString=!inString; // Start or end of literal string.
        else if (c=='-' and pos+1<exprSize and expr[pos+1]=='-') { // Start of comment
            while (pos+1<exprSize) {
                pos++;
                c=expr[pos];
                offSet++;
                if (c=='\n') break;
            }
        }

        if (level>0 and (!result.isEmpty() or c!=startDelimiter)) result+=c;
        else offSet++;
        pos++;
    }
    if (!error)
        return result;
    else
        return QString(startDelimiter)+"error"+QString(endDelimiter);
}

bool FadaScriptEngine2::evalExpr(QString expr) {
    return (expr=="true");
}
