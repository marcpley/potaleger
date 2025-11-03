#include "qapplication.h"
#include "qcolor.h"
#include "qdir.h"
#include "qlabel.h"
#include "qregularexpression.h"
#include "qsqlerror.h"
#include "PotaUtils.h"
#include <QFont>

bool PotaQuery::ExecShowErr(QString query)
{
    if (query.trimmed().isEmpty()) return true;

    clear();

    if (!query.startsWith("PRAGMA ") and !query.startsWith("SELECT "))
        logMessage("sql.log",query.trimmed());

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
    if (ExecShowErr(query)) {
        next();
        //qDebug() << query+"->"+value(0).toString();
        return value(0);
    }
    else
        return vNull;
}

void AppBusy(bool busy,QProgressBar *pb,int max,QString text) {
    if (busy) {
        if(pb) {
            pb->setValue(0);
            pb->setMaximum(max);
            pb->setFormat(text);
            pb->setVisible(true);
        }
        QApplication::setOverrideCursor(Qt::WaitCursor);
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

QString DataType(QSqlDatabase *db, QString TableName, QString FieldName){
    QString result="";
    PotaQuery query(*db);
    query.ExecShowErr("PRAGMA table_xinfo("+TableName+")");
    while (query.next()){
        if (query.value(1).toString()==FieldName){
            result=query.value(2).toString();
            break;
        }
    }

    if (result=="") {//Unknow view field.
        //ViewFieldIsDate(FieldName,Data)
        QString sData=query.Select0ShowErr("SELECT "+FieldName+" FROM "+TableName+" WHERE "+FieldName+" NOTNULL").toString();
        if(!sData.isEmpty() and sData.length()==10 and sData[4]=='-' and sData[7]=='-')
            return "DATE";
        else if (QString::number(sData.toInt())==sData)
            return "INT";
        else if (QString::number(sData.toDouble())==sData)
            return "REAL";
        else
            return "TEXT";
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

QString EscapeCSV(QString s,QString sep) {
    s=StrReplace(s,"\"","\"\"");
    if (s.contains("\n") or s.contains(sep) or s.contains("\""))
        s="\""+s+"\"";
    return s;
}

QString EscapeSQL(QString s) {
    s=StrReplace(s,"'","''");
    s=StrReplace(s,"\\n\\n","'||x'0a0a'||'"); //For literal line returns.
    s=StrReplace(s,"\\n","'||x'0a'||'"); //For literal line returns.
    s=StrReplace(s,"\n\n","'||x'0a0a'||'"); //For real line returns.
    s=StrReplace(s,"\n","'||x'0a'||'"); //For real line returns.
    s="'"+s+"'";
    return s;
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
    QStringList LinesList=sCde.split("\n");
    QString s;
    QString sResult="";
    QString tablename="";
    QString fda_cmd="";
    int comment_index;
    //bool indent4;
    bool bTable;
    for (int i=0;i<LinesList.count();i++) {
        s=LinesList[i];
        //indent4=(s.startsWith("    ") and !s.startsWith("     "));//Line starts with 4 spaces.
        s=s.trimmed();
        QString comment="";
        comment_index=s.indexOf("--");
        if (comment_index!=-1) {
            sResult += iif(i>0 and keepReturns,"\n","").toString()+s.first(comment_index).trimmed()+iif(s.first(comment_index).trimmed().isEmpty(),""," ").toString();
            comment=SubString(s,comment_index);
        } else {
            sResult += iif(i>0 and keepReturns,"\n","").toString()+s.trimmed()+iif(s.trimmed().isEmpty(),""," ").toString();
        }
        if (fda_cmd_from_comments and comment.startsWith("---")) {
            if (comment.length()>3)
                comment=EscapeSQL(SubString(comment,4));
            else
                comment="NULL";
            if (s.startsWith("CREATE TABLE ")) {
                bTable=true;
                tablename=SubString(s,13);
                int space_index=tablename.indexOf(" ");
                tablename=tablename.first(space_index);
                fda_cmd="INSERT INTO fda_schema (name,description,tbl_type) VALUES ('"+tablename+"',"+comment+",'Table');";
            } else if (s.startsWith("CREATE VIEW ")) {
                bTable=false;
                tablename=SubString(s,12);
                int space_index=tablename.indexOf(" ");
                tablename=tablename.first(space_index);
                fda_cmd="INSERT INTO fda_schema (name,description,tbl_type) VALUES ('"+tablename+"',"+comment+",'View');";
            } else if (fda_cmd!="" and !s.startsWith("--")) {
                if (bTable) { //Parse fields of a CREATE TABLE
                if (s.startsWith("Notes "))
                qDebug() << "s";
                    int space_index=s.indexOf(" ");
                    if (space_index>0) {
                        QString fieldname=s.first(space_index);
                        s=SubString(s,space_index).trimmed();
                        int type_index1=s.indexOf(" ");
                        int type_index2=s.indexOf(",");
                        int type_index3=s.indexOf(")");
                        if (type_index1==-1) type_index1=1000000;
                        if (type_index2==-1) type_index2=1000000;
                        if (type_index3==-1) type_index3=1000000;
                        type_index1=fmin(fmin(type_index1,type_index2),type_index3);
                        if (type_index1<1000000) {
                            QString type=s.first(type_index1);
                            QString readOnly=iif(s.toUpper().contains(" AS ("),"'Calculated'","NULL").toString();
                            int ref_index=s.indexOf("REFERENCES ");
                            QString masterTable="NULL";
                            QString masterField="NULL";
                            if (ref_index>0) {
                                s=SubString(s,ref_index+11).trimmed();
                                ref_index=s.indexOf(")");
                                if (ref_index>1) {
                                    masterTable=s.first(ref_index);
                                    ref_index=masterTable.indexOf(" (");
                                    masterField="'"+SubString(masterTable,ref_index+2)+"'";
                                    masterTable="'"+masterTable.first(ref_index)+"'";
                                }
                            }
                            fda_cmd+="INSERT INTO fda_schema (name,field_name,type,description,tbl_type,master_table,master_field,readOnly) VALUES ('"+tablename+"','"+fieldname+"','"+type+"',"+comment+",'Table',"+masterTable+","+masterField+","+readOnly+");";
                        }
                    } else {
                        qWarning() << "SQL parse error 1: "+s;
                    }
                } else { //Parse fields of a CREATE VIEW
                    int space_index=s.indexOf("---");
                    if (space_index>0) {
                        s=s.first(space_index).trimmed();
                        int fieldIndex=s.lastIndexOf(" ");
                        if (fieldIndex==-1) fieldIndex=s.lastIndexOf(".");
                        QString fieldname="";
                        if (fieldIndex>0)
                            fieldname=SubString(s,fieldIndex+1);
                        else if (fieldIndex==-1)
                            fieldname=s;
                        if (!fieldname.isEmpty()) {
                            if (fieldname.endsWith(",")) fieldname.removeLast();
                            fda_cmd+="INSERT INTO fda_schema (name,field_name,description,tbl_type) "
                                     "VALUES ('"+tablename+"','"+fieldname+"',"+comment+",'View');";
                        } else {
                            qWarning() << "SQL parse error 2: "+s;
                        }
                    } else {
                        qWarning() << "SQL parse error 3: "+s;
                    }
                }
            }
        }
    }
    if (fda_cmd_from_comments)
        *fda_cmd_from_comments=fda_cmd;

    return sResult;
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

QString SubString(QString s, int iDeb, int iFin) {
    QString result="";

    if (iFin==-1) iFin=s.length();

    if (iDeb>iFin)
        return "";

    for (int i=iDeb;i<min(s.length(),iFin+1);i++)
        result+=s[i];

    return result;
}

