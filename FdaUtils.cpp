#include "muParser/mpParser.h"
// #include "muParser/muParserDef.h"
#include "qapplication.h"
#include "qcolor.h"
#include "qdir.h"
#include "qlabel.h"
//#include "qregularexpression.h"
#include "qsqlerror.h"
#include "FdaUtils.h"
#include <QFont>

bool PotaQuery::ExecShowErr(QString query)
{
    if (query.trimmed().isEmpty()) return true;

    clear();

    if (!query.startsWith("PRAGMA ") and !query.startsWith("SELECT "))
        logMessage("sql.log",query.trimmed());

    // if (query.contains("ADD COLUMN Réc_ter"))
    //     qDebug() << "stop";

    exec(query);

    if (lastError().type() != QSqlError::NoError) {
        qWarning() << query;
        qWarning() << lastError().text();
        if (lErr)
            SetColoredText(lErr,lastError().text(),"Err");
        if (query.startsWith("PRAGMA "))
            logMessage("sql.log",query);
        logMessage("sql.log",lastError().text());
        return false;

    }
    return true;
}

bool PotaQuery::ExecMultiShowErr(const QString querys, const QString spliter, QProgressBar *progressBar, bool stopIfError, bool FDAInsert) //,bool keepReturns
{
    QStringList QueryList=querys.split(spliter);
    if (lErr) {
        QString s;
        s=lErr->text();
        if ((s.length()>11)and(s.last(11)==" statements"))
            s=str(s.first(s.length()-11).toInt()+QueryList.count())+" statements";
        else
            s=str(QueryList.count())+" statements";
        SetColoredText(lErr,s,"Info");
    }
    if (progressBar) {
        progressBar->setValue(0);
        progressBar->setMaximum(QueryList.count());
    }
    for (int i=0;i<QueryList.count();i++) {
        if (QueryList[i].trimmed().isEmpty())
            continue;
        QString fda_cmd="";
        QString sQuery;
        if (FDAInsert)
            sQuery=RemoveSQLcomment(QueryList[i].trimmed(),false,&fda_cmd); //keepReturns
        else
            sQuery=RemoveSQLcomment(QueryList[i].trimmed(),false);
        clear();
        ExecShowErr(sQuery);
        if (progressBar) progressBar->setValue(progressBar->value()+1);
        if (lastError().type()!=QSqlError::NoError and stopIfError and !lastError().text().startsWith("UNIQUE constraint failed")) {
            //qWarning() << sQuery;
            //qWarning() << lastError().text();
            if (lErr)
                SetColoredText(lErr,QueryList[i].trimmed()+"\n"+lastError().text(),//+"\n"+DBInfo(db),
                               "Err");
            return false;
        } else if (!fda_cmd.isEmpty()) {
            clear();
            ExecMultiShowErr(fda_cmd,";",nullptr);
        }
    }
    return true;
}

QVariant PotaQuery::Select0ShowErr(QString query)
{
    QVariant vNull;
    if (ExecShowErr(query) and next())
        return value(0);
    else
        return vNull;
}

void AppBusy(bool busy,QProgressBar *pb,int max,int pos,QString text) {
    if (busy) {
        if(pb) {
            if (max>-1) pb->setMaximum(max);
            if (pos>-1) pb->setValue(pos);
            pb->setFormat(text);
            pb->setVisible(true);
            QCoreApplication::processEvents();
        }
        QApplication::setOverrideCursor(Qt::WaitCursor);
        //QCoreApplication::processEvents(); pire
    } else {
        if(pb) {
            pb->setVisible(false);
        }
        QApplication::restoreOverrideCursor();
    }
}

QColor blendColors(const QColor& baseColor, const QColor& overlayColor) {
    qreal alpha=overlayColor.alphaF();
    qreal invAlpha=1.0 - alpha;

    int red=(baseColor.red() * invAlpha + overlayColor.red() * alpha);
    int green=(baseColor.green() * invAlpha + overlayColor.green() * alpha);
    int blue=(baseColor.blue() * invAlpha + overlayColor.blue() * alpha);

    return QColor(red, green, blue);
}

QString DataType(QString sValue){
    if (sValue.isEmpty())
        return "";
    else if (sValue.length()==10 and sValue[4]=='-' and sValue[7]=='-')
        return "DATE";
    else if (QString::number(sValue.toInt())==sValue)
        return "INT";
    else {
        bool ok = false;
        //double d=
        sValue.toDouble(&ok);
        if (ok)
            return "REAL";
        else
            return "TEXT";
    }
}

QString DataType(QSqlDatabase *db, QString TableName, QString FieldName){
    QString result="";
    PotaQuery query(*db);
    // query.ExecShowErr("PRAGMA table_xinfo("+TableName+")");
    // while (query.next()){
    //     if (query.value(1).toString()==FieldName){
    //         result=query.value(2).toString();
    //         break;
    //     }
    // }
    result=query.Select0ShowErr("SELECT type FROM pragma_table_xinfo('"+TableName+"') WHERE name='"+FieldName+"'").toString();

    if (result=="") {//Unknow view field.
        //ViewFieldIsDate(FieldName,Data)
        QString sData=query.Select0ShowErr("SELECT "+FieldName+" FROM "+TableName+" WHERE "+FieldName+" NOTNULL").toString();
        QString result="";
        if(!sData.isEmpty()) {
            result=DataType(sData);
        }
        if (result!="DATE") //CAST(... AS DATE) don't WORK fine: only year is displayed.
            qWarning() << "Unknow field type for "+TableName+"."+FieldName+" : use CAST in view definition.";
        return result;
    } else if (result.startsWith("NUM"))
        return "REAL";
    else
        return result;
}

// QString SQLiteDate() {
//     return QDate::currentDate().toString("yyyy-MM-dd");
// }

// QString DBInfo(QSqlDatabase *db)
// {
//     //QSqlDatabase db=QSqlDatabase::database();
//     QString sResult;
//     sResult=db->databaseName()+" - "+db->driverName()+" - "+db->connectOptions()+" : ";
//     sResult.append(iif(db->isValid(),"VALID DRIVER, ","").toString());
//     //sResult.append(iif(db.isOpen(),"DB IS OPEN, ","").toString()); isOpen always return true
//     sResult.append(iif(db->isOpenError(),"OPENERROR, ","").toString());
//     sResult.truncate(sResult.length()-2);
//     return sResult;
// }

void drawFromFdaText(QPainter *painter, const QRect &rect, QString FdaText) {
    QStringList drawCmds=FdaText.split(")");
    int xOffSet=0;
    int yOffSet=0;
    QRect cellRect = rect;
    QColor initialColor=painter->pen().color();
    painter->save();
    painter->setClipRegion(cellRect);
    for (int i=0;i<drawCmds.count();i++) {
        int pos=drawCmds[i].indexOf("(");
        if (pos>0) {
            QString cmd=drawCmds[i].first(pos).toUpper().trimmed();
            QStringList params=drawCmds[i].mid(pos+1).split(",");

            auto setColor = [&] (int colorIndex) {
                if (params.count()==1 and params[0].isEmpty()) { // Reset color and alpha.
                    painter->setPen(initialColor);
                    painter->setBrush(initialColor);
                } else { // Set color and alpha
                    QColor c;
                    if (params.count()>colorIndex and QColor(params[colorIndex]).isValid())
                        c=QColor(params[colorIndex]);
                    else
                        c=painter->pen().color();
                    if (params.count()>(colorIndex+1) and params[colorIndex+1].toInt()>0 and params[colorIndex+1].toInt()<=255)
                        c.setAlpha(params[colorIndex+1].toInt());
                    else
                        c.setAlpha(painter->pen().color().alpha());
                    QBrush b(c);
                    painter->setPen(c);
                    painter->setBrush(b);
                }
            };

            auto defParam = [&] (int paramIndex, int defValue) {
                if (params.count()>paramIndex and params[paramIndex].toInt()!=0)
                    return params[paramIndex].toInt();
                else
                    return defValue;
            };

            if (cmd=="COLOR") { //COLOR,alpha
                setColor(0);

            } else if (cmd=="LINE") { //X1,y1,x2,y2,color,alpha
                if (params.count()>0) {
                    // if (params[0].toInt()==10)
                    //     qDebug() << "stop";
                    int x1=params[0].toInt();
                    int y1=defParam(1,0); //y1 def = bottom
                    int x2=defParam(2,params[0].toInt()); //x2 def = x1
                    int y2=defParam(3,iif(y1==0 and x2==params[0].toInt(),rect.height(),y1).toInt()); //y2 def = bottom or y1
                    if (x1<=x2 and y1<=y2) {
                        painter->save();
                        setColor(4);
                        painter->drawLine(xOffSet+rect.left()+x1,
                                          yOffSet+rect.top()+rect.height()-y1,
                                          xOffSet+rect.left()+x2,
                                          yOffSet+rect.top()+rect.height()-y2);
                        painter->restore();

                    }
                }
            } else if (cmd=="OFFSET") { //x,y
                if (params.count()>0) {
                    if (params[0].startsWith("+"))
                        xOffSet=xOffSet+params[0].mid(1).toInt();
                    else if (params[0].startsWith("-"))
                        xOffSet=xOffSet-params[0].mid(1).toInt();
                    else
                        xOffSet=params[0].toInt();
                }
                if (params.count()>1) {
                    if (params[1].startsWith("+"))
                        yOffSet=yOffSet-params[1].mid(1).toInt();
                    else if (params[1].startsWith("-"))
                        yOffSet=yOffSet+params[1].mid(1).toInt();
                    else
                        yOffSet=-params[1].toInt();
                }

            } else if (cmd=="OVERFLOW") { //top,right,bottom,left
                cellRect = rect;
                cellRect.setTop(rect.top()-defParam(0,0));
                cellRect.setLeft(rect.left()-defParam(3,0));
                cellRect.setWidth(rect.width()+defParam(1,0)+defParam(3,0));
                cellRect.setHeight(rect.height()+defParam(0,0)+defParam(2,0));
                painter->setClipRegion(cellRect);

            } else if (cmd=="RECT") { //X,Y,W,H,color,alpha,gradient
                if (params.count()>3) {
                    int x=xOffSet+rect.left()+params[0].toInt();
                    int y=yOffSet+rect.top()+rect.height()-params[1].toInt();
                    int w=params[2].toInt();
                    int h=params[3].toInt();
                    if (w>0 and h>0) {
                        painter->save();
                        setColor(4);
                        if (params.count()>6 and !params[6].isEmpty()) {
                            QLinearGradient gradient;
                            QColor c=painter->pen().color();
                            if (params[6].toUpper()=="H+") {
                                gradient.setStart(x,y);
                                gradient.setFinalStop(x+w,y);
                            } else if (params[6].toUpper()=="H-") {
                                gradient.setStart(x+w,y);
                                gradient.setFinalStop(x,y);
                            } else if (params[6].toUpper()=="V+") {
                                gradient.setStart(x,y);
                                gradient.setFinalStop(x,y-h);
                            } else if (params[6].toUpper()=="V-") {
                                gradient.setStart(x,y-h);
                                gradient.setFinalStop(x,y);
                            }
                            gradient.setColorAt(0,c);
                            if (params.count()>7 and QColor(params[7]).isValid())
                                c=QColor(params[7]);
                            else
                                c=painter->pen().color();
                            c.setAlpha(defParam(8,0));
                            gradient.setColorAt(1,c);
                            painter->fillRect(x,y,w,-h,gradient);
                        } else
                            painter->fillRect(x,y,w,-h,painter->brush());
                        painter->restore();
                    }
                }
            } else if (cmd=="TEXT") { //TEXT,X,y,color,alpha,fontattributes
                if (params.count()>1) {
                    painter->save();
                    setColor(3);
                    if (params.count()>5) { // Font attributes.
                        QFont font=painter->font();
                        if (params[5].contains("b"))
                            font.setBold(true);
                        if (params[5].contains("u"))
                            font.setUnderline(true);
                        if (params[5].contains("s"))
                            font.setStrikeOut(true);
                        if (params[5].toInt()>=50)
                            font.setStretch(min(params[5].toInt(),200));
                        painter->setFont(font);
                    }
                    painter->drawText(xOffSet+rect.left()+params[1].toInt(),
                                      yOffSet+rect.top()+rect.height()-defParam(2,5), //y def = 5 = base line for text
                                      params[0]);
                    painter->restore();
                }
            }
        }
    }
    painter->restore();
};

QString EscapeCSV(QString s,QString sep) {
    s=StrReplace(s,"\"","\"\"");
    if (s.contains("\n") or s.contains(sep) or s.contains("\""))
        s="\""+s+"\"";
    return s;
}

QString EscapeSQL(QString s) {
    if (s.isEmpty()) {
        return "NULL";
    } else {
        s=StrReplace(s,"'","''");
        s=StrReplace(s,"\\n\\n","'||x'0a0a'||'"); //For literal line returns.
        s=StrReplace(s,"\\n","'||x'0a'||'"); //For literal line returns.
        s=StrReplace(s,"\n\n","'||x'0a0a'||'"); //For real line returns.
        s=StrReplace(s,"\n","'||x'0a'||'"); //For real line returns.
        s="'"+s+"'";
        return s;
    }
}

QDate firstDayOffWeek(int year, int week) {
    QDate date(year, 1, 1);
    QDate firstMonday;
    if (date.dayOfWeek() <= 1)
        firstMonday=date;
    else
        firstMonday=date.addDays(8 - date.dayOfWeek());
    QDate result=firstMonday.addDays((week - 1) * 7);
    return result;
}

QDate firstDayOffWeek(QDate date) {
    return date.addDays(1 - date.dayOfWeek());;
}

QVariant iif(bool bCond,QVariant Var1,QVariant Var2)
{
    if (bCond)
        return Var1;
    else
        return Var2;
}

bool isDarkTheme() {
    QColor backgroundColor=QApplication::palette().color(QPalette::Window);
    return backgroundColor.lightness() < 128;
}

QString loadSQLFromResource(QString fileName) {
    QFile file(":/sql/"+fileName+".sql");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Fail open SQL ressource "+fileName;
        return "";
    }

    QByteArray bytes = file.readAll();
    QString queries = QString::fromUtf8(bytes);

    return queries;
}

void logMessage(const QString fileName, const QString message)
{
    QFile logFile(QCoreApplication::applicationDirPath() + QDir::toNativeSeparators("/"+fileName));
    if (message=="Start")
        logFile.remove();
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " - " << message << "\n";
        logFile.close();
    } else {
        qWarning() << "fail to open log file.";
    }
}

float min(float a,float b) {
    if (a>b)
        return b;
    else
        return a;
}

int min(int a,int b) {
    if (a>b)
        return b;
    else
        return a;
}

QVariant muParse(QString formula) {//, QString returnType
    bool returnError=false;
    if (formula.endsWith("!")) {
        formula=formula.removeLast();
        returnError=true;
    }
    mup::string_type expr;
    #ifdef _WIN32
        expr=formula.trimmed().toStdWString();
    #else
        expr=formula.trimmed().toStdString();
    #endif
    mup::ParserX parser;

    parser.SetExpr(expr);

    QVariant result;
    try {
        mup::Value val = parser.Eval();
        // if (returnType.startsWith("BOOL"))
        //     result=val.GetBool();
        // else if (returnType.startsWith("INT"))
        //     result=std::round(val.GetFloat());
        // else
        //     result=val.GetFloat();

        if (val.IsInteger())
            result=std::round(val.GetFloat());
        else if (val.IsString())
            #ifdef _WIN32
            result=QString::fromStdWString(val.GetString());
            #else
            result=QString::fromStdString(val.GetString());
            #endif
        else
            result=val.GetFloat();

        return result;
    } catch (const mup::ParserError &e) {
        QString err;
        #ifdef _WIN32
            err=QString::fromStdWString(e.GetMsg());
        #else
            err=QString::fromStdString(e.GetMsg());
        #endif
        //qDebug() << e.GetMsg();
        if (returnError)
            return QVariant(err);
        else
            return QVariant("="+formula+"!");
    };
}

void parseCSV(QString entry, QString sep, QStringList &list) {
    QStringList values;
    QString parsedValue;
    values=entry.split(sep);

    list.clear();
    parsedValue=values[0];
    for(int i=0;i<values.count();i++) {
        if(parsedValue.count("\"") % 2==0) {//Even number of ".
            if (StrFirst(parsedValue,1)=="\"" and StrLast(parsedValue,1)=="\"")
                parsedValue=SubString(parsedValue,1,parsedValue.length()-2);
            list.append(parsedValue);
            //qDebug() << parsedValue;
            if(i==values.count()-1) break;
            parsedValue=values[i+1];
        } else {
            if(i==values.count()-1) break;
            parsedValue+=sep+values[i+1];
        }
    }
}

// QString PrimaryKeyFieldName(QSqlDatabase *db, QString TableName)  {
//     PotaQuery query(*db);
//     query.ExecShowErr("PRAGMA table_xinfo("+TableName+")");
//     while (query.next()){
//         if (query.value(5).toInt()==1)
//             return query.value(1).toString();
//     }
//     //PK not found
//     return "";
// }

QString RemoveAccents(QString input) {
    return input;
    // QString normalized=input.normalized(QString::NormalizationForm_D).toLower();
    // normalized.remove(QRegularExpression("[̀-̈]"));  // Supprime les accents combinés
    // return normalized;
}

QString RemoveSQLcomment(QString sCde, bool keepReturns, QString *fda_cmd_from_comments)
{
    QStringList cdeLines=sCde.split("\n");
    QString cdeLine;
    QString resultWithoutComment="";
    QString tablename="";
    QString fda_cmd="";
    int comment_index;
    int field_index=1;
    bool bTable;
    for (int i=0;i<cdeLines.count();i++) {
        cdeLine=cdeLines[i];
        cdeLine=cdeLine.trimmed();
        QString comment="";
        comment_index=cdeLine.indexOf("--");
        if (comment_index!=-1) {
            resultWithoutComment+=iif(i>0 and keepReturns,"\n","").toString()+
                     cdeLine.first(comment_index).trimmed()+
                     iif(cdeLine.first(comment_index).trimmed().isEmpty(),""," ").toString();
            comment=cdeLine.mid(comment_index);
        } else {
            resultWithoutComment+=iif(i>0 and keepReturns,"\n","").toString()+
                     cdeLine.trimmed()+
                     iif(cdeLine.trimmed().isEmpty(),""," ").toString();
        }

        if (fda_cmd_from_comments and comment.startsWith("---")) { //FDA comment
            comment=comment.trimmed().mid(3);
            cdeLine=cdeLine.first(comment_index).trimmed();

            if (comment.contains("--")) // Description comment (---) ends with dév comment (--).
                comment=comment.first(comment.indexOf("--"));

            //Read multiline FDA comment.
            for (int j=i+1;j<cdeLines.count();j++) {
                if (cdeLines[j].trimmed().startsWith("--")) {
                    if (cdeLines[j].trimmed().startsWith("---"))
                        comment+="\n"+cdeLines[j].trimmed().mid(3);
                    i++;//don't read again those comment lines next loop.
                } else {
                    break;
                }
            }

            //extract properties from multiline FDA comment.
            QStringList commentLines=comment.split("\n");
            QString description="";
            QList<QStringList> properties={{},{}};
            for (int j=0;j<commentLines.count();j++) {
                // if (commentLines[j]=="goto_last")
                //     qDebug() << "stop";
                if (commentLines[j].contains("--")) // FDA comment (---) ends with dév comment (--).
                    commentLines[j]=commentLines[j].first(commentLines[j].indexOf("--"));
                if (commentLines[j].startsWith(" ") or commentLines[j].isEmpty()) { //No keyword
                    description+=iif(!description.isEmpty(),"\n\n","").toString()+commentLines[j].trimmed();
                } else {
                    int keyWordPos=commentLines[j].indexOf(" ");
                    QString keyWord,sValue;
                    if (keyWordPos==-1) {
                        keyWord=commentLines[j];
                        sValue="x";
                    } else {
                        keyWord=commentLines[j].first(keyWordPos);
                        sValue=commentLines[j].mid(keyWordPos+1);
                    }
                    if (properties[0].contains(keyWord)) {
                        properties[1][properties[0].indexOf(keyWord)]+="\n\n"+iif(sValue=="x","",sValue).toString();
                    } else {
                        properties[0].append(keyWord);
                        properties[1].append(sValue);
                    }
                }
            }

            if (cdeLine.startsWith("CREATE TABLE ")) {
                bTable=true;
                tablename=SubString(cdeLine,13);
                int space_index=tablename.indexOf(" ");
                tablename=tablename.first(space_index);
                fda_cmd="INSERT INTO fda_t_schema (name,tbl_type,description) "
                        "VALUES ('"+tablename+"','Table',"+EscapeSQL(description)+");";
                for (int j=0;j<properties[0].count();j++) {
                    fda_cmd+="UPDATE fda_t_schema SET "+properties[0][j]+"="+EscapeSQL(properties[1][j])+" "
                             "WHERE name='"+tablename+"';";
                }

            } else if (cdeLine.startsWith("CREATE VIEW ")) {
                bTable=false;
                tablename=SubString(cdeLine,12);
                int space_index=tablename.indexOf(" ");
                tablename=tablename.first(space_index);

                fda_cmd="INSERT INTO fda_t_schema (name,tbl_type,description) "
                        "VALUES ('"+tablename+"','View',"+EscapeSQL(description)+");";
                for (int j=0;j<properties[0].count();j++) {
                    fda_cmd+="UPDATE fda_t_schema SET "+properties[0][j]+"="+EscapeSQL(properties[1][j])+" "
                             "WHERE name='"+tablename+"';";
                }

            } else if (fda_cmd!="" and !cdeLine.startsWith("--")) {  //Parse fields.
                if (bTable) { //Parse fields of a CREATE TABLE
                    int space_index=cdeLine.indexOf(" ");
                    if (space_index>0) {
                        QString fieldname=cdeLine.first(space_index);
                        cdeLine=SubString(cdeLine,space_index).trimmed();
                        int type_index1=cdeLine.indexOf(" ");
                        int type_index2=cdeLine.indexOf(",");
                        int type_index3=cdeLine.indexOf(")");
                        if (type_index1==-1) type_index1=1000000;
                        if (type_index2==-1) type_index2=1000000;
                        if (type_index3==-1) type_index3=1000000;
                        type_index1=fmin(fmin(type_index1,type_index2),type_index3);
                        if (type_index1<1000000) {
                            QString fieldtype=cdeLine.first(type_index1);
                            bool pk=cdeLine.toUpper().contains("PRIMARY KEY");
                            QString readOnly=iif(cdeLine.toUpper().contains(" AS "),"'Calculated'","NULL").toString();
                            if (cdeLine.contains(" AUTOINCREMENT")) {
                                fieldtype="AUTOINCREMENT";
                                readOnly="'x'";
                            }
                            int ref_index=cdeLine.indexOf("REFERENCES ");
                            QString masterTable="NULL";
                            QString masterField="NULL";
                            if (ref_index>0) {
                                cdeLine=SubString(cdeLine,ref_index+11).trimmed();
                                ref_index=cdeLine.indexOf(")");
                                if (ref_index>1) {
                                    masterTable=cdeLine.first(ref_index);
                                    ref_index=masterTable.indexOf(" (");
                                    masterField="'"+SubString(masterTable,ref_index+2)+"'";
                                    masterTable="'"+masterTable.first(ref_index)+"'";
                                }
                            }
                            fda_cmd+="INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,"
                                                               "natural_sort,master_table,master_field,readonly) "
                                     "VALUES ('"+tablename+"',"+str(field_index)+",'"+fieldname+"','"+fieldtype+"',"+EscapeSQL(description)+","+
                                              iif(pk,"0","NULL").toString()+","+masterTable+","+masterField+","+readOnly+");";
                            field_index++;
                            for (int j=0;j<properties[0].count();j++) {
                                fda_cmd+="UPDATE fda_f_schema SET "+properties[0][j]+"="+EscapeSQL(properties[1][j])+" "
                                         "WHERE (name='"+tablename+"')AND(field_name='"+fieldname+"');";
                            }
                        } else {
                            qWarning() << "SQL parse error 0: "+cdeLine;
                            qWarning() << tablename+"."+fieldname;
                        }
                    } else {
                        qWarning() << "SQL parse error 1: "+cdeLine;
                    }
                } else { //Parse CREATE VIEW
                    if (cdeLine.toUpper().startsWith("ORDER BY ")) {
                        QStringList orderBy=cdeLine.mid(9).split(",");
                        QString sOrderBy="";
                        for (int j=0;j<orderBy.count();j++) {
                            if (orderBy[j].endsWith(")")) orderBy[j].removeLast();
                            if (orderBy[j].contains("."))
                                orderBy[j]=orderBy[j].split(".")[1];
                            fda_cmd+="UPDATE fda_f_schema SET natural_sort="+str(j)+" "
                                     "WHERE (name='"+tablename+"')AND(field_name='"+orderBy[j]+"');";
                        }
                    } else { //Parse fields of a CREATE VIEW
                        int fieldIndex=cdeLine.lastIndexOf(" ");
                        if (fieldIndex==-1) fieldIndex=cdeLine.lastIndexOf(".");
                        QString fieldname="";
                        if (fieldIndex>0)
                            fieldname=SubString(cdeLine,fieldIndex+1);
                        else if (fieldIndex==-1)
                            fieldname=cdeLine;
                        if (!fieldname.isEmpty()) {
                            if (fieldname.endsWith(",")) fieldname.removeLast();
                            QString readOnly=iif(cdeLine.contains("(") or cdeLine.contains(")"),"'Calculated'","'x'").toString();
                            fda_cmd+="INSERT INTO fda_f_schema (name,field_index,field_name,description,readonly) "
                                     "VALUES ('"+tablename+"',"+str(field_index)+",'"+fieldname+"',"+EscapeSQL(description)+","+readOnly+");";
                            field_index++;
                            for (int j=0;j<properties[0].count();j++) {
                                fda_cmd+="UPDATE fda_f_schema SET "+properties[0][j]+"="+EscapeSQL(properties[1][j])+" "
                                         "WHERE (name='"+tablename+"')AND(field_name='"+fieldname+"');";
                            }
                        } else {
                            qWarning() << "SQL parse error 2: "+cdeLine;
                        }
                    }

                }
            }
        }
    }
    if (fda_cmd_from_comments)
        *fda_cmd_from_comments=fda_cmd;

    //qDebug() << fda_cmd;

    return resultWithoutComment;
}

void SetColoredText(QLabel *l, QString text, QString type)
{
    QString s;
    if (text.length()>600)
        s=text.first(200)+"...\n..."+text.last(400);

    else if (text.startsWith("NOT NULL constraint failed")) {
        int i=text.indexOf(".");
        QString field=SubString(text,i+1,1000);
        int j=field.indexOf(" ");
        field=SubString(field,0,j-1);
        s=QObject::tr("Saisir une valeur pour le champ %1").arg(field);
    } else if (text.startsWith("CHECK constraint failed: ")) {
        s=StrReplace(text,"CHECK constraint failed: ","");
        s=StrReplace(s," Unable to fetch row","");
        s=QObject::tr("Valeur incorrecte pour %1").arg(s);
    } else if (text=="FOREIGN KEY constraint failed Unable to fetch row") {
        s=QObject::tr("Suppression impossible, la donnée est utilisée ailleurs");
    } else
        s=text;

    l->setText(s);

    QPalette p=l->palette();
    if (type=="Err")
        p.setColor(QPalette::WindowText, "red");
    else if (type=="Ok")
        p.setColor(QPalette::WindowText, "green");
    else if (type=="Info")
        p.setColor(QPalette::WindowText, QColor("#9e5000"));
    else
        p.setColor(QPalette::WindowText, QApplication::palette().color(QPalette::ColorGroup::Disabled,QPalette::WindowText));//"white"
    l->setPalette(p);
}

void SetFontColor(QWidget* widget, QColor color) {
    if (widget) {
        if (color!=QColor()) {
            QPalette palette=widget->palette();
            palette.setColor(QPalette::Text, color);
            widget->setPalette(palette);
        } else
            widget->setPalette(QApplication::palette());
    }
}

void SetFontWeight(QWidget* widget, QFont::Weight weight) {
    if (widget) {
        QFont font=widget->font();
        font.setWeight(weight);
        widget->setFont(font);
    }
}

QString str(int i)
{
    QString s;
    s.setNum(i);
    return s;
}
QString str(float i)
{
    QString s;
    s.setNum(i);
    return s;
}
QString str(qsizetype i)
{
    QString s;
    s.setNum(i);
    return s;
}

QString StrElipsis(QString s, int i){
    if (i>0 and i<s.length())
        return s.first(i)+"...";
    else if (i==0)
        return "";
    else
        return s;
}

QString StrFirst(QString s, int i){
    if (i>0 and i<s.length())
        return s.first(i);
    else if (i==0)
        return "";
    else
        return s;
}

QString StrLast(QString s, int i){
    if (i>0 and i<s.length())
        return s.last(i);
    else if (i==0)
        return "";
    else
        return s;
}

QString StrRemoveLasts(QString s, int i){
    while (i>0 and s.length()>0) {
        s.removeLast();
        i-=1;
    }
    return s;
}

QString StrReplace(QString s, const QString sTarg, const QString sRepl) {
    int index=0;
    int index2=0;
    if (sTarg.isEmpty())
        return s;
    while ((index=s.indexOf(sTarg, index2)) != -1) {
        s.replace(index, sTarg.length(), sRepl);
        index2=index+sRepl.length();  // To avoid infinite loop
    }
    return s;
}

QString StrReplaceAll(QString s, const QString sTarg, const QString sRepl) {
    int i=0;
    while (s.contains(sTarg)) {
        s=s.replace(sTarg,sRepl);
        i++;
        if (i>1000) break;
    }
    return s;
}

QString SubString(QString s, int iDeb, int iFin) {
    QString result="";

    if (iFin==-1) iFin=s.length();

    if (iDeb>iFin)
        return "";

    for (int i=iDeb;i<min(s.length(),iFin+1);i++)
        result+=s[i];

    return result;
}

