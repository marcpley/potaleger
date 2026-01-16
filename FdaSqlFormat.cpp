#include "FdaSqlFormat.h"
#include "FdaUtils.h"
#include <QStringList>

QString fdaSqlFormat::formatSql(const QString sql) {
    if (sql.contains("[")) return sql;
    QString result=formatSqlInit(sql);
    result=formatSqlExplore(result);
    result=formatSqlFormatAll(result);
    return result;
};

QString fdaSqlFormat::formatSqlInit(const QString sql) {
    subSQLdata.clear();
    QString result=sql;
    //Return to dense single line query.
    result=StrReplace(result,"\n"," ");
    result=StrReplaceAll(result,"  "," ");
    result=StrReplaceAll(result,", ",",");
    return result;
};

QString fdaSqlFormat::formatSqlExplore(const QString sql) {
    QString result=sql;
    int level=0;
    //result=
    result.replace("()","[]"); //count() make an infinite loop.
    bool bStop=false;
    while (!bStop) { //Loop on deep levels.
        int pos = 0;
        if (!result.contains("("))
            bStop=true;
        while (!bStop) { //Loop on current level.
            QString subSql=extractSubContent(result,pos);
            // if (subSql.startsWith("SELECT sum"))
            //     qDebug() << "stop";
            if (subSql.isEmpty()) {
                level++;
                break;
            }
            if (subSql=="[Unbalanced]")
                bStop=true;
            int posOpen = subSql.indexOf("(");
            if (posOpen == -1) {
                result=result.first(pos-1)+"["+str(subSQLdata.count())+"]"+result.mid(pos+subSql.length()+1);
                subSQLdata.append({result.first(pos-1), //keyword before subSql.
                                   subSql.trimmed(),
                                   level,
                                   0}); //Place for parent indent size
            } else {
                pos+=posOpen;
            }
        }
    }
    //result=
    result.replace("[]","()");
    return result;
}

QString fdaSqlFormat::formatSqlFormatAll(const QString sql) {
    QString result=formatSqlFormat(-1,sql.trimmed());
    for (int i=subSQLdata.size()-1;i>=0;i--) {
        //qDebug() << "i" << i << " - levels[i]" << subSQLdata[i][2].toString() <<  " - subSQLs[i]" << subSQLdata[i][1].toString().replace("  "," ") <<  " - keyWords[i]" << subSQLdata[i][0].toString() <<  " - ParentIdent[i]" << subSQLdata[i][3].toString();
        result.replace("["+str(i)+"]","("+formatSqlFormat(i)+")");
    }
    return result;
};

QString fdaSqlFormat::formatSqlFormat(const int iSubSql,QString sql1) {
    QString sql;
    QString keyWord;
    int curIndent;
    if (iSubSql>=0) {
        sql=subSQLdata[iSubSql][1].toString();
        keyWord=subSQLdata[iSubSql][0].toString();
        curIndent=subSQLdata[iSubSql][3].toInt();
    } else {
        sql=sql1;
        keyWord="";
        curIndent=0;
    }
    QString sIndent=" ";
    QString result=sql;
    if (keyWord.endsWith("CAST",Qt::CaseInsensitive))
        curIndent+=5;
    else if (keyWord.endsWith("COALESCE",Qt::CaseInsensitive))
        curIndent+=9;
    else if(keyWord.endsWith("IIF",Qt::CaseInsensitive))
        curIndent+=4;
    else if(keyWord.endsWith("IN",Qt::CaseInsensitive))
        curIndent+=3;
    else if(keyWord.endsWith("ROUND",Qt::CaseInsensitive))
        curIndent+=6;
    else if(keyWord.endsWith("THEN ",Qt::CaseInsensitive))
        curIndent+=6;
    else if(keyWord.endsWith("(",Qt::CaseInsensitive))
        curIndent+=1;

    if (sql.startsWith("CASE WHEN ",Qt::CaseInsensitive)) {
        result=sql.first(5);
        curIndent+=5;
        setParentIdentOnChilds(sql,curIndent);
        QString sRest=sql.mid(5);
        QString sep1="";
        QString sep2="\n"+sIndent.repeated(curIndent);
        int whenLength;
        while (!sRest.isEmpty()) {
            whenLength=sRest.toUpper().indexOf(" WHEN ");
            if (whenLength==-1)
                whenLength=sRest.toUpper().indexOf(" ELSE ");
            if (whenLength==-1)
                whenLength=sRest.toUpper().indexOf(" END");
            if (whenLength>-1) {
                int posThen=sRest.toUpper().indexOf(" THEN ");
                if (posThen>-1) // WHEN ... THEN
                    result+=sep1+sRest.first(posThen)+sep2+sRest.mid(posThen+1,whenLength-posThen);
                else // ELSE
                    result+=sep1+sRest.first(whenLength);
                sRest=sRest.mid(whenLength+1);
            } else { // END
                result+=sep1+sRest;
                sRest="";
            }
            sep1=sep2;
        }
    } else if (sql.startsWith("SELECT ",Qt::CaseInsensitive)) {
        result=sql.first(7);
        curIndent+=7;
        setParentIdentOnChilds(sql,curIndent);
        int posFrom=sql.toUpper().indexOf(" FROM ");
        if (posFrom==-1) posFrom=sql.length();
        //Loop on fields
        QStringList fields=sql.mid(7,posFrom-7).split(",");
        for (int i=0;i<fields.count();i++) {
            result=result+iif(i>0,sIndent.repeated(curIndent),"").toString()+fields[i].trimmed()+",\n";
        }
        result=result.removeLast().removeLast();
        curIndent-=7;
        QString sRest=sql.mid(posFrom+1);
        QString sFrom="";
        QString sJoin="";
        QString sWhere="";
        QString sOrderBy="";
        if (!sRest.isEmpty()) {
            int posJoin=posJoinF(sRest);
            int posWhere=sRest.toUpper().indexOf(" WHERE ");
            int posOrderBy=sRest.toUpper().indexOf(" ORDER BY ");

            // FROM
            if (posJoin>-1) {
                sFrom=sRest.first(posJoin);
            } else if (posWhere>-1) {
                sFrom=sRest.first(posWhere);
            } else if (posOrderBy>-1) {
                sFrom=sRest.first(posOrderBy);
            } else {
                sFrom=sRest;
            }
            sFrom="\n"+sIndent.repeated(curIndent)+sFrom;

            // JOIN
            if (posJoin>-1) {
                QString sJoinRest;
                if (posWhere>-1) {
                    sJoinRest=sRest.mid(posJoin+1,posWhere-posJoin);
                } else if (posOrderBy>-1) {
                    sJoinRest=sRest.mid(posJoin+1,posOrderBy-posJoin);
                } else {
                    sJoinRest=sRest.mid(posJoin+1);
                }
                while (!sJoinRest.isEmpty()) {
                    posJoin=posJoinF(sJoinRest.mid(10));
                    if (posJoin>-1) {
                        posJoin+=10;
                        sJoin+="\n"+sIndent.repeated(curIndent)+sJoinRest.first(posJoin);
                        sJoinRest=sJoinRest.mid(posJoin+1);
                    } else {
                        sJoin+="\n"+sIndent.repeated(curIndent)+sJoinRest;
                        sJoinRest="";
                    }
                }
            }

            // WHERE
            if (posWhere>-1) {
                if (posOrderBy>-1) {
                    sWhere=sRest.mid(posWhere+1,posOrderBy-posWhere);
                } else {
                    sWhere=sRest.mid(posWhere+1);
                }
                sWhere="\n"+sIndent.repeated(curIndent)+sWhere.first(6)+splitOperands(sWhere.mid(6),curIndent+6);
                setParentIdentOnChilds(sWhere,curIndent-1); //Diff between "SELECT" and "WHERE".
            }

            // ORDER BY
            if (posOrderBy>-1) {
                sOrderBy="\n"+sIndent.repeated(curIndent)+sRest.mid(posOrderBy+1);
                setParentIdentOnChilds(sOrderBy,curIndent+2); //Diff between "SELECT" and "ORDER BY".
            }

            result+=sFrom+sJoin+sWhere+sOrderBy;
        }
    } else {
        if (originalLength(sql)>80-curIndent) { //Split expression.
            result="";
            QStringList exprs=sql.split(",");
            for (int i=0;i<exprs.count();i++) {
                result=result+iif(i>0,sIndent.repeated(curIndent),"").toString()+splitOperands(exprs[i].trimmed(),curIndent)+",\n";
            }
            result=result.removeLast().removeLast();
        }
        setParentIdentOnChilds(sql,curIndent);
    }


    return result;
}

int fdaSqlFormat::originalLength(const QString s) {
    QString rebuild=s;
    for (int i=subSQLdata.size()-1;i>=0;i--) {
        if (rebuild.contains("["+str(i)+"]")) {
            //rebuild=
            rebuild.replace("["+str(i)+"]","("+subSQLdata[i][1].toString()+")");
        }
    }
    return rebuild.length();
}

QString fdaSqlFormat::splitOperands(const QString s, const int indent) {
    QString sIndent=" ";
    QString result=s;
    result=StrReplace(result," AND NOT"," AND\n"+sIndent.repeated(indent)+"NOT");
    result=StrReplace(result," AND "," AND\n"+sIndent.repeated(indent));
    result=StrReplace(result," OR NOT"," OR\n"+sIndent.repeated(indent)+"NOT");
    result=StrReplace(result," OR "," OR\n"+sIndent.repeated(indent));
    result=StrReplace(result," + ","\n"+sIndent.repeated(indent)+"+ ");
    result=StrReplace(result," - ","\n"+sIndent.repeated(indent)+"- ");
    result=StrReplace(result," / ","\n"+sIndent.repeated(indent)+"/ ");
    result=StrReplace(result," * ","\n"+sIndent.repeated(indent)+"* ");
    result=StrReplace(result," || "," ||\n"+sIndent.repeated(indent));
    return result;
}

void fdaSqlFormat::setParentIdentOnChilds(const QString s, const int parentIndent) {
    for (int i=0;i<subSQLdata.size();i++) {
        if (s.contains("["+str(i)+"]")) {
            subSQLdata[i][3]=parentIndent;
        }
    }
}

int fdaSqlFormat::posJoinF(const QString s) {
    int posJoin=s.toUpper().indexOf(" JOIN ");
    if (posJoin>-1) {
        int posJoin1=s.toUpper().indexOf(" NATURAL ");
        int posJoin2=s.toUpper().indexOf(" LEFT ");
        int posJoin3=s.toUpper().indexOf(" RIGTH ");
        int posJoin4=s.toUpper().indexOf(" FULL ");
        int posJoin5=s.toUpper().indexOf(" INNER ");
        int posJoin6=s.toUpper().indexOf(" CROSS ");
        if (posJoin1>-1 and posJoin1<posJoin) posJoin=posJoin1;
        if (posJoin2>-1 and posJoin2<posJoin) posJoin=posJoin2;
        if (posJoin3>-1 and posJoin3<posJoin) posJoin=posJoin3;
        if (posJoin4>-1 and posJoin4<posJoin) posJoin=posJoin4;
        if (posJoin5>-1 and posJoin5<posJoin) posJoin=posJoin5;
        if (posJoin6>-1 and posJoin6<posJoin) posJoin=posJoin6;
    }
    return posJoin;
}

QString fdaSqlFormat::extractSubContent(const QString s, int &pos)
{
    int posOpen = s.indexOf("(",pos);
    if (posOpen == -1)
        return QString();

    int level = 1;
    int posClose = posOpen + 1;
    while (posClose < s.length() && level > 0) {
        if (s[posClose] == "(") {
            level++;
        } else if (s[posClose] == ")") {
            level--;
        }
        posClose++;
    }

    if (level != 0) {
        pos=s.length();
        return "[Unbalanced]"; // Parentheses non équilibrées
    }

    pos=posOpen+1;
    return s.mid(posOpen + 1, posClose - posOpen - 2);
}
