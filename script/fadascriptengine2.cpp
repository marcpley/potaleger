#include "fadascriptengine2.h"
#include "Dialogs.h"
#include "FdaUtils.h"
#include "qregularexpression.h"
#include "qsqlerror.h"
#include "script/fadascriptedit.h"

FadaScriptEngine2::FadaScriptEngine2(QObject *parent)
    : QObject{parent}
{
    resetVars();
}

int FadaScriptEngine2::runScript(QString script, QSqlDatabase *db, bool testScript, FadaScriptEdit *SQLEdit, FadaHighlighter *hl) {

    if (SQLEdit){
        SQLEdit->removeTraillingSpaces();
        script=SQLEdit->toPlainText();
    }

    scriptError.clear();
    lastStatementStart=0;
    this->db=db;
    returnScript=false;

    resetVars();

    QString mapedScript=map(script);

    if (db->transaction()) { //testScript and
        transactionOpen=true;
        qInfo() << "BEGIN TRANSACTION";
    }

    stmt level1Statement;
    level1Statement.statement=mapedScript;

    bool result;
    ////////////////////////////////
    result=execute(level1Statement);
    ////////////////////////////////

    //resetStmt(level1Statement);


    if (transactionOpen) {
        if(testScript or !result or !scriptError.isEmpty()) {
            transactionOpen=!db->rollback();
            if (!transactionOpen)
                qInfo() << "ROLLBACK";
            else
                qWarning() << "ROLLBACK failed";
        } else {
            transactionOpen=!db->commit();
            if (!transactionOpen)
                qInfo() << "COMMIT";
            else
                qWarning() << "COMMIT failed";
        }
    }

    if (scriptError.isEmpty()) {
        if (hl) {
            //Add vars in hl.
            hl->setRules(vars.keys());
            hl->rehighlight();
        }
        if (SQLEdit)
            SQLEdit->setCompleterKeywords(vars.keys());

    } else {
        if (SQLEdit) {
            //Select last executed line in SQLEdit.
            QTextCursor cursor = SQLEdit->textCursor();
            cursor.setPosition(unMap(lastStatementStart));
            cursor.setPosition(unMap(lastStatementStart+lastStatementHintSize), QTextCursor::KeepAnchor);
            SQLEdit->setTextCursor(cursor);
            SQLEdit->setFocus();
        }
        qWarning() << scriptError;

        MessageDlg(scriptTitle,tr("Script error"),"Position "+QString::number(unMap(lastStatementStart))+scriptError,QStyle::SP_MessageBoxCritical);
    }

    return 0;
}

QString FadaScriptEngine2::map(QString script) {
    scriptMap.clear();
    QString result;
    int scriptSize=script.size();
    int pos=0;
    bool inString=false;
    bool lineStart=true;
    bool lineEnd=false;
    QChar c;
    while (pos<scriptSize) {
        c=script[pos];
        bool regularChar=true;
        if (!inString) {
            if (c=='\n') { //Line return.
                regularChar=false;
                lineStart=true;
                lineEnd=false;
                result+=" "; //xxx
                scriptMap.append(pos);
            } else if (c==' ' and lineStart) {//Indent space
                regularChar=false;
            } else if (c==' ' and lineEnd) {//Trailing space
                regularChar=false;
            } else if ((c==' ' and script.mid(pos).trimmed().startsWith("--"))or //Space before comment
                       (c=='-' and pos+1<scriptSize and script[pos+1]=='-')){ // Start of comment
                while (pos+1<scriptSize) { //Skip comment
                    pos++;
                    c=script[pos];
                    if (c=='\n') break;
                }
                regularChar=false;
                lineStart=true;
                lineEnd=false;
            } else {
                lineStart=false;
                if (c=='}' or c==';')
                    lineEnd=true;
            }
        }
        if (c=="'") inString=!inString; // Start or end of literal string.

        if (regularChar) {
            result+=c;
            scriptMap.append(pos);
        }

        pos++;
    }
    return result;
}

int FadaScriptEngine2::unMap(int posInNoCommentScript) {
    if (posInNoCommentScript>-1 and posInNoCommentScript<scriptMap.size())
        return scriptMap[posInNoCommentScript];
    else {
        scriptError+="\nMap error ("+QString::number(posInNoCommentScript)+")";
        return -1;
    }
}

void FadaScriptEngine2::resetStmt(stmt &stmt, bool fromScript) {
    stmt.statement="";
    stmt.stmtType=st_unknown;
    stmt.bracedType=br_unknown;
    stmt.fromScript=fromScript;
    // stmt.implicitTerminator=false;
    stmt.originalPos=0;
    stmt.originalSize=0;
}

bool FadaScriptEngine2::execute(stmt statement) {
    if (statement.statement.isEmpty() or returnScript) return true;

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

    //Unknown statement type error.
    if (scriptError.isEmpty()) {
        lastStatementStart=statement.originalPos;
        lastStatementHintSize=statement.originalSize;
    }
    scriptError+="\nUnknown statement type";
    return false;
}

bool FadaScriptEngine2::executeSimpleStatement(stmt statement) {
    if (statement.fromScript and !scriptError.isEmpty()) { //Not normal.
        qDebug() << "Script interpreter possible bug 1";
        return false;
    }

    if (returnScript) return true;

    QString error;
    bool done=false;
    bool evalExprAllreadyUpdateScriptError=false;

    ///////////////////
    //Variables setting
    ///////////////////

    //varName=expression;
    if (!done) {
        QString expr=statement.statement;
        int pos=0;
        int exprSize=expr.size();
        QChar c;
        QString varName;
        QString varChars="abcdefghijklmnopqrstuvwxyz_1234567890";
        while (pos<exprSize-1) {
            c=expr[pos];

            if (c=='=') {
                if (pos>0)
                  varName=expr.first(pos);
                break;
            } else if (!varChars.contains(c,Qt::CaseInsensitive)) {
                break;
            }
            pos++;
        }
        if (!varName.isEmpty()) {
            expr=expr.mid(pos+1);

            ///////////////////////////////////
            QVariant eval=evalExpr(expr,error);
            ///////////////////////////////////

            if (error.isEmpty()) {
                setVar(varName,eval,statement.originalPos);
            }
            evalExprAllreadyUpdateScriptError=true;
            done=true;
        }
    }

    /////////
    //Dialogs
    /////////

    if (!done) {
        //inputsDialog(text
        //             ,varName,label,type,left,labelWidth,inpuWidth,valDef,toolTip
        //             [,varName,label,type,left,labelWidth,inpuWidth,valDef,toolTip]
        //             ...
        //             [,messageType[,dialogWidth[,NextButton]]]]])
        if (statement.statement.startsWith("inputsdialog(",Qt::CaseInsensitive)) {
            QList<QVariant> args=extractArgs(statement.statement);
            if (args.count()>8) {
                QList<inputStructure> inputs;
                int i;
                for (i=1;i<args.count();i++) {
                    if (args.count()<i+8) break;
                    inputStructure input;
                    input.varName=args[i].toString();
                    input.label=args[i+1].toString();
                    // QString type=args[i+2].toString().toLower();
                    // if (type.startsWith("int")) input.type=QMetaType::Int;
                    // else if (type=="real") input.type=QMetaType::Double;
                    // else if (type=="date") input.type=QMetaType::QDate;
                    // else if (type=="bool") input.type=QMetaType::Bool;
                    input.type=args[i+2].toString();
                    input.left=args[i+3].toInt();
                    input.labelWidth=args[i+4].toInt();
                    input.inputWidth=args[i+5].toInt();
                    input.valDef=args[i+6].toString();
                    input.toolTip=args[i+7].toString();
                    inputs.append(input);
                    i+=7;
                }
                QString messageType; if (args.count()>i) messageType=args[i].toString();
                int dialogWidth=350; if (args.count()>i+1) dialogWidth=args[i+1].toInt();
                bool bNext=false; if (args.count()>i+2) bNext=(args[i+2].toBool());

                QList<inputResult> result=inputDialog(scriptTitle,args[0].toString(),inputs,bNext,standardPixmap(messageType),dialogWidth);

                if (result.count()==0) {
                    returnScript=true;
                    qInfo() << unMap(statement.originalPos) << "cancel";
                } else {
                    for (int i=0;i<result.count();i++) {
                        setVar(result[i].varName,result[i].value,statement.originalPos);
                    }
                }
            } else {
                error="inputsdialog() without arguments.";
            }
            done=true;
        }

        //messageDialog(shortText[,longText[,messageType]])
        else if (statement.statement.startsWith("messagedialog(",Qt::CaseInsensitive)) {
            QList<QVariant> args=extractArgs(statement.statement);
            if (args.count()>0) {
                QString longText; if (args.count()>1) longText=args[1].toString();
                QString messageType; if (args.count()>2) messageType=args[2].toString();
                int messageWidth=350; if (args.count()>3) messageWidth=args[3].toInt();

                MessageDlg(scriptTitle,args[0].toString(),longText,standardPixmap(messageType),messageWidth);
            } else {
                error="messageDialog() without arguments.";
            }
            done=true;
        }

        //selectDialog(text,varName,selectStatement[,toolTip[,messageType[NextButton]]])
        else if (statement.statement.startsWith("selectdialog(",Qt::CaseInsensitive)) {
            QList<QVariant> args=extractArgs(statement.statement);
            if (args.count()>2) {
                QString toolTip; if (args.count()>3) toolTip=args[3].toString();
                QString messageType; if (args.count()>4) messageType=args[4].toString();
                bool bNext=false; if (args.count()>5) bNext=(args[5].toBool());

                PotaQuery pQuery(*db);
                pQuery.ExecShowErr("DROP VIEW IF EXISTS Temp_"+scriptTitle.replace("'","")+";");
                pQuery.ExecShowErr("CREATE TEMP VIEW Temp_"+scriptTitle.replace("'","")+" AS "+args[2].toString());

                QList<inputResult> result=selectDialog( scriptTitle,args[0].toString(),*db,args[1].toString(),"Temp_"+scriptTitle.replace("'",""),"", feProgressBar,feLErr, bNext, standardPixmap(messageType),toolTip);

                if (result.count()==0) {
                    returnScript=true;
                    qInfo() << unMap(statement.originalPos) << "cancel";
                } else {
                    for (int i=0;i<result.count();i++) {
                        setVar(result[i].varName,result[i].value,statement.originalPos);
                    }
                }

            } else {
                error="selectDialog() without arguments.";
            }
            done=true;
        }

        //tableDialog(text,varName,tableName[,whereClose[,toolTip[,messageType[,NextButton]]]])
        else if (statement.statement.startsWith("tabledialog(",Qt::CaseInsensitive)) {
            QList<QVariant> args=extractArgs(statement.statement);
            if (args.count()>2) {
                QString whereClose; if (args.count()>3) whereClose=args[3].toString();
                QString toolTip; if (args.count()>4) toolTip=args[4].toString();
                QString messageType; if (args.count()>5) messageType=args[5].toString();
                //int dialogWidth=350; if (args.count()>6) dialogWidth=args[6].toInt();
                bool bNext=false; if (args.count()>6) bNext=(args[6].toBool());

                QList<inputResult> result=selectDialog(scriptTitle,args[0].toString(),*db,args[1].toString(),args[2].toString(),whereClose,feProgressBar,feLErr, bNext, standardPixmap(messageType),toolTip);

                if (result.count()==0) {
                    returnScript=true;
                    qInfo() << unMap(statement.originalPos) << "cancel";
                } else if (result.count()==1 and result[0].varName=="error") {
                    returnScript=true;
                    qWarning() << unMap(statement.originalPos) << result[0].value;
                    error=result[0].value.toString();
                } else {
                    for (int i=0;i<result.count();i++) {
                        setVar(result[i].varName,result[i].value,statement.originalPos);
                    }
                }

            } else {
                error="tableDialog() without arguments.";
            }
            done=true;
        }
    }

    ////////
    //C-like
    ////////

    //return;
    if (!done and statement.statement.toLower()=="return") {
        returnScript=true;
        qInfo() << unMap(statement.originalPos) << "return";
        done=true;
    }

    ////////////////
    //SQL statements
    ////////////////
    if (!done) {
        if (statement.statement.toLower()=="begin transaction") {
            transactionOpen=db->transaction();
            if (transactionOpen)
                qInfo() << unMap(statement.originalPos) << "BEGIN TRANSACTION";
            else
                qWarning() << unMap(statement.originalPos) << "BEGIN TRANSACTION failed";
            done=true;

        } else if (statement.statement.toLower()=="commit") {
            transactionOpen=!db->commit();
            if (!transactionOpen)
                qInfo() << unMap(statement.originalPos) << "COMMIT";
            else
                qWarning() << unMap(statement.originalPos) << "COMMIT failed";
            done=true;

        } else if (statement.statement.toLower()=="rollback") {
            transactionOpen=!db->rollback();
            if (!transactionOpen)
                qInfo() << unMap(statement.originalPos) << "ROLLBACK";
            else
                qWarning() << unMap(statement.originalPos) << "ROLLBACK failed";
            done=true;

        } else if (statement.statement.startsWith("INSERT INTO ",Qt::CaseInsensitive)) {
            PotaQuery query(*db);
            ///////////////////////////////////////
            query.ExecShowErr(statement.statement);
            ///////////////////////////////////////
            if (query.lastError().type() == QSqlError::NoError) {
                QString varName="insert_"+extractWord1(statement.statement.mid(12));
                if (vars.contains(varName)) {//Var exists
                    vars.find(varName)->setValue(query.numRowsAffected()+vars.find(varName).value().toInt());
                } else {
                    vars.insert(varName, query.numRowsAffected());
                }
                qInfo() << unMap(statement.originalPos) << QString::number(query.numRowsAffected())+" rows inserted" << statement.statement;
            } else {
                error=query.lastError().text();
            }
            done=true;

        } else if (statement.statement.startsWith("UPDATE ",Qt::CaseInsensitive)) {
            PotaQuery query(*db);
            query.ExecShowErr(statement.statement);
            if (query.lastError().type() == QSqlError::NoError) {
                QString varName="update_"+extractWord1(statement.statement.mid(7));
                if (vars.contains(varName)) {//Var exists
                    vars.find(varName)->setValue(query.numRowsAffected()+vars.find(varName).value().toInt());
                } else {
                    vars.insert(varName, query.numRowsAffected());
                }
                qInfo() << unMap(statement.originalPos) << QString::number(query.numRowsAffected())+" rows updated" << statement.statement;
            } else {
                error=query.lastError().text();
            }
            done=true;

        } else if (statement.statement.startsWith("DELETE FROM ",Qt::CaseInsensitive)) {
            PotaQuery query(*db);
            query.ExecShowErr(statement.statement);
            if (query.lastError().type() == QSqlError::NoError) {
                QString varName="delete_"+extractWord1(statement.statement.mid(12));
                if (vars.contains(varName)) {//Var exists
                    vars.find(varName)->setValue(query.numRowsAffected()+vars.find(varName).value().toInt());
                } else {
                    vars.insert(varName, query.numRowsAffected());
                }
                qInfo() << unMap(statement.originalPos) << QString::number(query.numRowsAffected())+" rows deleted" << statement.statement;
            } else {
                error=query.lastError().text();
            }
            done=true;
        }
    }

    //Unsupported statement.
    if (!done) {
        if (statement.fromScript)
            error="Unsupported statement :"+statement.statement;
        else
            qWarning() << unMap(statement.originalPos) << "?" << statement.statement;
    }

    if (!error.isEmpty()) {
        if (!evalExprAllreadyUpdateScriptError)
            scriptError+="\n"+error;
        qWarning() << unMap(statement.originalPos) << error;
        lastStatementStart=statement.originalPos;
        lastStatementHintSize=statement.originalSize;
    }
    return error.isEmpty();
}

bool FadaScriptEngine2::executeBracedStatement(stmt statement) {
    if (returnScript) return true;
    setStatementBracedType(statement,false);
    if (statement.bracedType==br_if) {
        return executeIfStatement(statement);
    // } else if (statement.bracedType==br_for) {
    //     return executeForStatement(statement);
    } else if (statement.bracedType==br_while) {
        return executeWhileStatement(statement);
    } else { //Error
        if (scriptError.isEmpty()) {
            lastStatementStart=statement.originalPos;
            int pos1=statement.statement.indexOf("(");
            int pos2=statement.statement.indexOf("{");
            if (pos1>-1 and(pos2==-1 or pos2>pos1))
                lastStatementHintSize=pos1;
            else if (pos2>-1)
                lastStatementHintSize=pos2;
            else
                lastStatementHintSize=std::min(statement.originalSize,10);
        }
        if (statement.statement.size()>9)
            scriptError+="\nUnknown braced statement type: "+QString::number(statement.bracedType)+" - "+statement.statement.first(10);
        else
            scriptError+="\nUnknown braced statement type: "+QString::number(statement.bracedType)+" - "+statement.statement;
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

        pos++;
    }

    if (braceLevel!=0) { //Error
        if (scriptError.isEmpty()) {
            lastStatementStart=statement.originalPos+iif(braceErrPos==-1,braceErrPosDef,braceErrPos).toInt();
            lastStatementHintSize=1;
        }
        scriptError+="\nUnbalanced braces {}";
        statement.stmtType=st_error;
    } else if (statementEnd) {
        statement.stmtType=st_multiple;
    } else if (braced) {
        statement.stmtType=st_braced;
    } else {
        statement.stmtType=st_single;
    }
}

void FadaScriptEngine2::setStatementBracedType(stmt &statement,bool onlyAtStart) {
    //Must be runned at begining of statement construction.
    if (statement.bracedType==br_unknown) {
        if (!onlyAtStart or statement.statement.size()<7) {
            if (statement.statement.startsWith("if ",Qt::CaseInsensitive) or statement.statement.startsWith("if(",Qt::CaseInsensitive))// or
                //statement.statement.startsWith("else ",Qt::CaseInsensitive) or statement.statement.startsWith("else{",Qt::CaseInsensitive))
                statement.bracedType=br_if;
            // else if (statement.statement.startsWith("for ",Qt::CaseInsensitive) or statement.statement.startsWith("for(",Qt::CaseInsensitive))
            //     statement.bracedType=br_for;
            else if (statement.statement.startsWith("while ",Qt::CaseInsensitive) or statement.statement.startsWith("while(",Qt::CaseInsensitive))
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

        if (endOfStatement(c,pos,scriptSize,inString,braceLevel,curStatement,statements)) {
            if (curStatement.bracedType==br_unknown) curStatement.bracedType=br_no;
            setStatementType(curStatement);
            curStatement.originalPos=statementStartPos+offSet;
            curStatement.originalSize=curStatement.statement.size();// pos-statementStartPos-offSet;
            statementStartPos=pos+1;
            curStatement.statement=curStatement.statement.trimmed();

            if (!curStatement.statement.isEmpty()) //xxx
                ////////////////////////////
                result.append(curStatement);
                ////////////////////////////

            resetStmt(curStatement);
            //inString=false;
        } else if (c==' ' and curStatement.statement.isEmpty()) { //xxx
            statementStartPos++;
        } else {
            //addToCurStatement(c,curStatement.statement);
            curStatement.statement+=c;
        }
        pos++;
    }

    return result;
};

bool FadaScriptEngine2::endOfStatement(const QChar c, const int pos, const int scriptSize, const bool inString,
                                       const int braceLevel, stmt &curStatement, QString statements) {
    setStatementBracedType(curStatement,true);

    //curStatement.implicitTerminator=true;

    if (c==';' and !inString and braceLevel==0) { //End of statement.
        // curStatement.implicitTerminator=false;
        return true;
    } else if (pos==scriptSize-1) { //End of script.
        //addToCurStatement(c,curStatement.statement);
        curStatement.statement+=c;
        return true;
    } else if (c=='}' and !inString and braceLevel==0) {
        bool endOfBracedStatement=false;
        if  (pos>scriptSize-2) // '}' is last char
            endOfBracedStatement=true;
        else if (curStatement.bracedType!=br_if)
            endOfBracedStatement=true;
        else {
            QString trailing=statements.mid(pos+1).trimmed();
            // QRegularExpression ifif("^if[ \\{\n]", QRegularExpression::CaseInsensitiveOption);
            // QRegularExpression ifelse("^else[ \\{\n]", QRegularExpression::CaseInsensitiveOption);
            // if (!ifif.match(trailing).hasMatch() and !ifelse.match(trailing).hasMatch()) //End of braced statement.
            if (trailing.startsWith("else ",Qt::CaseInsensitive) or trailing.startsWith("else{",Qt::CaseInsensitive)) {
                if (!trailing.startsWith("else if",Qt::CaseInsensitive)) {
                    int posParenth=trailing.indexOf("(");
                    if (posParenth>-1 and posParenth<trailing.indexOf("{"))
                        //else () {} si incorrect.
                        endOfBracedStatement=true;
                }
            } else {
                endOfBracedStatement=true;
            }
        }

        if (endOfBracedStatement) {
            // addToCurStatement(c,curStatement.statement);
            curStatement.statement+=c;
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
    // if (...) {...} [else if (...) {...}] [else {...}]
    bool result=true;
    int pos=0; //Position in statement.
    int offSet=0;
    QString toParse=statement.statement;
    QString condition;
    int conditionPos;
    QString subStatement;
    //bool condError=false;
    while (true) {
        pos=toParse.indexOf('(');
        if (pos>-1 and pos<toParse.indexOf('{')) {
            //Parse condition.
            toParse=toParse.mid(pos);
            offSet+=pos;
            condition=extractSubExpr(toParse,'(',')',offSet);
            if (condition.isEmpty()) { //Error
                if (scriptError.isEmpty()) {
                    lastStatementStart=statement.originalPos+offSet;
                    lastStatementHintSize=1;
                }
                scriptError+="\nExtract condition fail.";
                return false;
            }
            conditionPos=pos+1;
            toParse=toParse.mid(condition.size()+2);
            offSet+=condition.size()+1;
        } else {
            condition="true";
        }

        //Parse substatement.
        subStatement=extractSubExpr(toParse,'{','}',offSet);
        if (subStatement.isEmpty() and toParse!="{}") { //Error
            if (scriptError.isEmpty()) {
                lastStatementStart=statement.originalPos+offSet;
                lastStatementHintSize=1;
            }
            scriptError+="\nExtract substatement fail.";
            return false;
        }

        QString error;
        bool eval=evalExpr(condition,error).toBool();
        if (!error.isEmpty()) {
            //if (scriptError.isEmpty()) {
                lastStatementStart=statement.originalPos+conditionPos;
                lastStatementHintSize=condition.size();
            //}
            //scriptError+="\n"+error;
            return false;
        } else if (eval) {
            qInfo() << unMap(statement.originalPos) << "if condition: " << condition << "true";
            stmt st;
            st.statement=subStatement;
            st.originalPos=statement.originalPos+offSet;
            result=execute(st);
            break;
        } else {
            qInfo() << unMap(statement.originalPos) << "if condition: " << condition << "false";
            //Parse next condition and substatement.
            toParse=toParse.mid(subStatement.size()+2);
            offSet+=subStatement.size()+1;
            int posBrace=toParse.indexOf("{");
            if (posBrace>-1) {
                toParse=toParse.mid(posBrace);
                offSet+=posBrace-1;
            } else {
                break;
            }
        }
    }

    return result;
}

// bool FadaScriptEngine2::executeForStatement(stmt statement) {
//     //Execution (todo)
//     bool result=true;

//     //Report
//     if (result) { //Sucess
//         qInfo() << unMap(statement.originalPos) << statement.statement;
//     } else { //Error
//         qWarning() << unMap(statement.originalPos) << statement.statement;
//         scriptError+="\nStatement error: "+statement.statement;
//     }
//     lastStatementStart=statement.originalPos;
//     lastStatementHintSize=statement.originalSize;

//     return result;
// }

bool FadaScriptEngine2::executeWhileStatement(stmt statement) {
    //while (...) {...}
    bool result=true;
    int pos=0; //Position in statement.
    int offSet=0;
    QString toParse=statement.statement;
    QString condition;
    int conditionPos;
    QString subStatement;

    pos=toParse.indexOf('(');
    if (pos>-1 and pos<toParse.indexOf('{')) {
        //Parse condition.
        toParse=toParse.mid(pos);
        offSet+=pos;
        condition=extractSubExpr(toParse,'(',')',offSet);
        if (condition.isEmpty()) { //No condition error.
            if (scriptError.isEmpty()) {
                lastStatementStart=statement.originalPos+offSet;
                lastStatementHintSize=1;
            }
            scriptError+="\nExtract condition fail.";
            return false;
        }
        conditionPos=pos+1;
        toParse=toParse.mid(condition.size()+2);
        offSet+=condition.size()+1;
    } else { //"While {}" without "()"
        if (scriptError.isEmpty()) {
            lastStatementStart=statement.originalPos+offSet;
            lastStatementHintSize=1;
        }
        scriptError+="\nNo condition.";
        return false;
    }

    //Parse substatement.
    subStatement=extractSubExpr(toParse,'{','}',offSet);
    if (subStatement.isEmpty()) {
        if (scriptError.isEmpty()) { //No substatement error.
            lastStatementStart=statement.originalPos+offSet;
            lastStatementHintSize=1;
        }
        scriptError+="\nExtract substatement fail.";
        return false;
    }

    int loop=0;
    while (result) {
        QString error;
        bool eval=evalExpr(condition,error).toBool();
        if (!error.isEmpty()) {
            //if (scriptError.isEmpty()) {
                lastStatementStart=statement.originalPos+conditionPos;
                lastStatementHintSize=condition.size();
            //}
            //scriptError+="\n"+error;
            return false;
        }
        if (!eval) break;

        qInfo() << unMap(statement.originalPos) << "while condition: " << condition << "true";
        stmt st;
        st.statement=subStatement;
        st.originalPos=statement.originalPos+offSet;
        result=execute(st);
        loop++;
        if (loop>10) { //Infinite loop error.
            if (scriptError.isEmpty()) {
                lastStatementStart=statement.originalPos+offSet;
                lastStatementHintSize=1;
            }
            scriptError+="\nLoop overflow (10).";
            return false;
        }
    }

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

        if (c==startDelimiter and !inString) {
            level++;
            if (level==1) {
                offSet++;
                pos++;
                continue;
            }
        } else if (c==endDelimiter and !inString) {
            level--;
            if (level==0) {
                error=false;
                break;
            }
        }

        if (c=="'") inString=!inString; // Start or end of literal string.

        if (level>0)// and (!result.isEmpty() or c!=startDelimiter))
            result+=c;
        else
            offSet++;
        pos++;
    }
    if (!error) {
        return result;
    } else {
        return "";
    }
}

QList<QVariant> FadaScriptEngine2::extractArgs(QString expr) {
    if (expr.isEmpty()) return QList<QVariant>();

    int level=0;
    int pos=0;
    int exprSize=expr.size();
    bool inString=false;
    QList<QVariant> result;
    QString arg="";
    QVariant vArg;
    QChar c;
    QString error;
    while (pos<exprSize) {
        c=expr[pos];

        if (c=='(' and !inString) {
            level++;
        } else if (c==')' and !inString) {
            level--;
            if (level==0) {
                if (!arg.isEmpty()) {

                    /////////////////////////
                    vArg=evalExpr(arg,error);
                    /////////////////////////

                    if (!error.isEmpty()) break;
                    //if (arg.size()>2 and arg.startsWith("'") and arg.endsWith("'"))
                    //    arg=arg.removeFirst().removeLast();
                    result.append(vArg);
                }
                break;
            }
        }

        if (c=="'") inString=!inString; // Start or end of literal string.

        if ((level==1 and c!='(') or level>1) {
            if (level==1 and c==',' and !inString) {

                ///////////////////////////////////
                arg=evalExpr(arg,error).toString();
                ///////////////////////////////////

                if (!error.isEmpty()) break;
                //if (rawStrings and arg.size()>2 and arg.startsWith("'") and arg.endsWith("'"))
                //    arg=arg.removeFirst().removeLast();
                result.append(arg);
                arg.clear();
            } else {
                arg+=c;
            }
        }
        pos++;
    }
    if (error.isEmpty() and level==0) {
        return result;
    } else {
        qWarning() << error;
        return QList<QVariant>();
    }
}

QVariant FadaScriptEngine2::evalExpr(QString expr, QString &error) {
    //Eval script expr.

    if (!error.isEmpty()) {//Not normal.
        qDebug() << "Script interpreter possible bug 2";
        return QVariant();
    }

    if (expr.isEmpty()) return QVariant();

    if (expr.endsWith("!")) {
        error="End of condition can't be '!'. ";
        return QVariant();
    }

    int pos,exprSize;
    bool inString;
    QChar c;
    QString newExpr="";

    // Replace vars by var values.
    if (error.isEmpty()) {
        for (int i=0;i<vars.count();i++) {
            QString varName=vars.keys()[i];
            QVariant varValue=vars.values()[i];

            pos=0;
            exprSize=expr.size();
            inString=false;
            newExpr="";
            bool lastCharIsPipe=false;
            QString varChars="abcdefghijklmnopqrstuvwxyz_1234567890";
            while (pos<exprSize) {
                c=expr[pos];
                if (c=="'") inString=!inString;
                if (!inString and expr.mid(pos).startsWith(varName)){
                    // QRegularExpression rx("\\b"+varName+"\\b\\.*");
                    // QRegularExpressionMatch rxm = rx.match(expr.mid(std::max(pos-1,0)));
                    //if (rxm.hasMatch()) {
                    if ((pos==0 or !varChars.contains(expr[pos-1],Qt::CaseInsensitive))and
                        (pos+varName.size()>=exprSize or !varChars.contains(expr[pos+varName.size()],Qt::CaseInsensitive))) {
                        bool nextCharIsPipe=(pos+varName.size()<exprSize and expr.mid(pos+varName.size()).trimmed().startsWith("|"));
                        newExpr+=variantToExpr(varValue,lastCharIsPipe or nextCharIsPipe);
                        pos+=varName.size();
                        continue;
                    }
                }

                if (c=='|') lastCharIsPipe=true;
                else if (c!=' ') lastCharIsPipe=false;

                newExpr+=c;
                pos++;
            }
            expr=newExpr;
        }
    }

    // Replace (SELECT ...) by row 0 col 0 field value.
    if (error.isEmpty() and expr.contains("SELECT",Qt::CaseInsensitive)) {
        pos=0;
        exprSize=expr.size();
        inString=false;
        bool inParenth=false;
        newExpr="";
        QString sQuery;
        QVariant selectResult;
        PotaQuery query(*db);
        bool lastCharIsPipe=false;
        while (pos<exprSize) { //search SELECT in expr.
            c=expr[pos];
            if (c=="'") inString=!inString;
            if (!inString and expr.mid(pos).startsWith("SELECT ",Qt::CaseInsensitive)){
                if (pos==0) {
                    sQuery=expr;
                } else {
                    QRegularExpression rx(R"((\(\s*SELECT\b.+\)))");
                    QRegularExpressionMatch rxm = rx.match(expr.mid(pos-1));
                    if (rxm.hasMatch()) {
                        int offSet;
                        sQuery=extractSubExpr( rxm.captured(1),'(',')',offSet);
                        inParenth=true;
                    } else {
                        sQuery="";
                    }
                }
                if (sQuery.startsWith("SELECT ",Qt::CaseInsensitive)) {
                    //////////////////////////////////////////
                    selectResult=query.Select0ShowErr(sQuery);
                    //////////////////////////////////////////
                    if (query.lastError().type() == QSqlError::NoError) {
                        if (inParenth) {
                            newExpr.removeLast(); //Remove "(" before SELECT.
                            pos+=1; //+1 to skip ")" after SELECT.
                        }
                        pos+=sQuery.size();
                        bool nextCharIsPipe=(pos<exprSize and expr.mid(pos).trimmed().startsWith("|"));
                        if (selectResult.isNull())
                            newExpr+="null";
                        else if (selectResult.typeId()==QMetaType::QString or lastCharIsPipe or nextCharIsPipe)
                            newExpr+="'"+selectResult.toString().replace("'","''")+"'";
                        else
                            newExpr+=selectResult.toString();
                        continue;
                    } else {
                        error="SELECT error.";
                        break;
                    }
                } else {
                    error="SELECT parse failed.";
                    break;
                }
            }

            if (c=='|') lastCharIsPipe=true;
            else if (c!=' ' and c!='(') lastCharIsPipe=false;

            newExpr+=c;
            pos++;
        }
        if (error.isEmpty())
            expr=newExpr;
    }

    // Replace function by function result.
    fadaFunction("format",expr,error);
    fadaFunction("iif",expr,error);

    // Replace dialogs by user response (1,0).
    fadaFunction("inputDialog",expr,error);
    fadaFunction("okCancelDialog",expr,error);
    fadaFunction("yesNoDialog",expr,error);
    fadaFunction("radioButtonDialog",expr,error);

    QVariant result;
    if (error.isEmpty()) {

        /////////////////////
        result=muParse(expr);
        /////////////////////

        if (result.toString().endsWith("!")) //muParse error
            error=muParse(expr+"!").toString();

    }

    if (error.isEmpty()) {
        return result;
    } else {
        scriptError+="\n"+error+"\n"+expr;
        qWarning() << error;
        qWarning() << expr;
        return QVariant();
    }
}

void FadaScriptEngine2::setVar(QString varName,QVariant value,int originalPos) {
    if (vars.contains(varName)) {//Var exists
        vars.find(varName)->setValue(value);
        if (originalPos>-1)
            qInfo() << unMap(originalPos) << varName+" <-- "+value.toString() << value.typeName();
    } else {
        vars.insert(varName, value);
        if (originalPos>-1)
            qInfo() << unMap(originalPos) << varName+" <- "+value.toString() << value.typeName();
    }
}

void FadaScriptEngine2::resetVars() {
    vars.clear();
    setVar("sp_Critical","Critical",-1); //QStyle::SP_MessageBoxCritical
    setVar("sp_Warning","Warning",-1);
    setVar("sp_Question","Question",-1);
    setVar("sp_Information","Information",-1);
    setVar("sp_None","",-1);

    setVar("it_Integer","INTEGER",-1); // QVariant::fromValue(static_cast<int>(QMetaType::Int))
    setVar("it_Real","REAL",-1);
    setVar("it_Date","DATE",-1);
    setVar("it_Bool","BOOL",-1);
    setVar("it_Text","TEXT",-1);
}

void FadaScriptEngine2::fadaFunction(QString funcName, QString &expr, QString &error) {
    if (error.isEmpty() and expr.contains(funcName,Qt::CaseInsensitive)) {
        int pos=0;
        int exprSize=expr.size();
        bool inString=false;
        QString newExpr="";
        QChar c;
        while (pos<exprSize) { //search function name in expr.
            c=expr[pos];
            if (c=="'") inString=!inString;
            if (!inString and expr.mid(pos).startsWith(funcName+"(",Qt::CaseInsensitive)){
                int offSet=findMatchingDelimiter(expr.mid(pos),'(',')');
                QList<QVariant> args=extractArgs(expr.mid(pos));
                if (args.count()>0) {
                    if (funcName=="format") {
                        QString result;
                        if (args[0].toDate().isValid()) //QDate::fromString( ,"yyyy-MM-dd")
                            result="'"+args[0].toDate().toString("dd/MM/yyyy")+"'";
                        newExpr+=result;
                    } else if (funcName=="iif") {
                        if (args.count()>2) {
                            if (args[0].toBool())
                                newExpr+=variantToExpr(args[1]);
                            else
                                newExpr+=variantToExpr(args[2]);
                        } else {
                            error=funcName+"() needs 3 arguments.";
                            break;
                        }
                        QString result;
                        if (args[0].toDate().isValid())
                            result="'"+args[0].toDate().toString("dd/MM/yyyy")+"'";
                        newExpr+=result;
                    } else if (funcName=="inputDialog") { //inputDialog(text
                                                                             //            ,label,type[,width[,valDef
                                                                             //            [,messageType[,dialogWidth[,NextButton]]]]])
                        if (args.count()>2) {
                            inputStructure input;
                            input.label=args[1].toString();
                            // QVariant varType;
                            // if (args[2].toLower().startsWith("int")) input.type=QMetaType::Int;
                            // else if (args[2].toLower()=="real") input.type=QMetaType::Double;
                            // else if (args[2].toLower()=="date") input.type=QMetaType::QDate;
                            // else if (args[2].toLower()=="bool") input.type=QMetaType::Bool;
                            input.type=args[2].toString();
                            if (args.count()>3) input.inputWidth=args[3].toInt();
                            if (args.count()>4) input.valDef=args[4].toString();

                            QString messageType; if (args.count()>5) messageType=args[5].toString();
                            int dialogWidth=350; if (args.count()>6) dialogWidth=args[6].toInt();
                            bool bNext=false; if (args.count()>7) bNext=(args[7].toBool());

                            QList<inputResult> result=inputDialog(scriptTitle,args[0].toString(),{input},bNext,standardPixmap(messageType),dialogWidth);

                            if (result.count()==0)
                                newExpr+="null";
                            else
                                newExpr+=variantToExpr(result[0].value);
                            // else if (args[2].toLower().startsWith("int")) newExpr+=result[0].value.toString();
                            // else if (args[2].toLower().startsWith("real")) newExpr+=result[0].value.toString();
                            // else if (args[2].toLower().startsWith("date")) newExpr+="'"+result[0].value.toDate().toString("yyyy-MM-dd")+"'";
                            // else if (args[2].toLower().startsWith("bool")) newExpr+=iif(result[0].value.toBool(),"1","0").toString();
                            // else newExpr+="'"+result[0].value.toString().replace("'","''")+"'";
                        } else {
                            error=funcName+"() needs 3 arguments minimum.";
                            break;
                        }
                    } else if (funcName=="okCancelDialog") { //okCancelDialog(text[,messageType[,messageWidth[,NextButton]]])
                        QString messageType; if (args.count()>1) messageType=args[1].toString();
                        int dialogWidth=350; if (args.count()>2) dialogWidth=args[2].toInt();
                        bool bNext=false; if (args.count()>3) bNext=(args[3].toBool());
                        newExpr+=iif(OkCancelDialog(scriptTitle,args[0].toString(),bNext,standardPixmap(messageType),dialogWidth),"1","0").toString();
                    } else if (funcName=="yesNoDialog") { //yesNoDialog(text[,messageType[,messageWidth]])
                        QString messageType; if (args.count()>1) messageType=args[1].toString();
                        int dialogWidth=350; if (args.count()>2) dialogWidth=args[2].toInt();
                        newExpr+=iif(YesNoDialog(scriptTitle,args[0].toString(),standardPixmap(messageType),dialogWidth),"1","0").toString();
                    } else if (funcName=="radioButtonDialog") { //radioButtonDialog(text,option1|option2...[,DefOptionIndex[,didabledOpIndexes[messageType[,messageWidth[,NextButton]]]]])
                        if (args.count()>1) {
                            QStringList options=args[1].toString().split("|");
                            int iDef=0; if (args.count()>2) iDef=args[2].toInt();
                            QStringList disabledOptionsSL; if (args.count()>3 and !args[3].toString().isEmpty()) disabledOptionsSL=args[3].toString().split("|");
                            QSet<int> disabledOptions; for (int i=0;i<disabledOptionsSL.count();i++) disabledOptions.insert(disabledOptionsSL[i].toInt());
                            QString messageType; if (args.count()>4) messageType=args[4].toString();
                            int dialogWidth=350; if (args.count()>5) dialogWidth=args[5].toInt();
                            bool bNext=false; if (args.count()>6) bNext=(args[6].toBool());
                            int result=RadiobuttonDialog(scriptTitle,args[0].toString(),options,iDef,disabledOptions,bNext,standardPixmap(messageType),dialogWidth);
                            if (result==-1)
                                newExpr+="null";
                            else
                                newExpr+=QString::number(result);
                        } else {
                            error=funcName+"() needs 2 arguments minimum.";
                            break;
                        }
                    } else {
                        error=funcName+"() unknown.";
                        break;
                    }
                    pos+=offSet+1;
                    continue;
                } else {
                    error=funcName+"() without arguments.";
                    break;
                }
            }
            newExpr+=c;
            pos++;
        }
        if (error.isEmpty())
            expr=newExpr;
    }
}
