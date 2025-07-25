#include "qapplication.h"
#include "qcolor.h"
#include "qlabel.h"
#include "qsqlerror.h"
#include "PotaUtils.h"
#include <QFont>
#include "data/Data.h"
#include "SQL/FunctionsSQLite.h"

bool PotaQuery::ExecShowErr(QString query)
{
   //dbSuspend(&m_db,false,true,lErr);
    clear();

    exec(query);
    if (lastError().type() != QSqlError::NoError) {
        qWarning() << query;
        qWarning() << lastError().text();
        if (lErr)
            SetColoredText(lErr,lastError().text(),"Err");
        return false;

    }
    return true;
}

bool PotaQuery::ExecMultiShowErr(const QString querys, const QString spliter, QProgressBar *progressBar,bool keepReturns)
{
    QStringList QueryList = querys.split(spliter);
    QString s;
    s=lErr->text();
    if ((s.length()>11)and(s.last(11)==" statements"))
        s=s.first(s.length()-11)+"+"+str(QueryList.count())+" statements";
    else
        s=str(QueryList.count())+" statements";
    if (lErr)
        SetColoredText(lErr,s,"Info");
    if (progressBar) {
        progressBar->setValue(0);
        progressBar->setMaximum(QueryList.count());
    }
    for (int i=0;i<QueryList.count();i++) {
        if (QueryList[i].trimmed().isEmpty())
            continue;
        QString sQuery=RemoveComment(QueryList[i].trimmed(),"--",keepReturns);
        clear();
        ExecShowErr(sQuery);
        if (progressBar) progressBar->setValue(progressBar->value()+1);
        if (lastError().type() != QSqlError::NoError) {
            qWarning() << sQuery;
            qWarning() << lastError().text();
            if (lErr)
                SetColoredText(lErr,QueryList[i].trimmed()+"\n"+lastError().text(),//+"\n"+DBInfo(db),
                               "Err");
            return false;

        }
    }
    return true;
}

QVariant PotaQuery::Selec0ShowErr(QString query)
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
    qreal alpha = overlayColor.alphaF();
    qreal invAlpha = 1.0 - alpha;

    int red = (baseColor.red() * invAlpha + overlayColor.red() * alpha);
    int green = (baseColor.green() * invAlpha + overlayColor.green() * alpha);
    int blue = (baseColor.blue() * invAlpha + overlayColor.blue() * alpha);

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
        QString sData=query.Selec0ShowErr("SELECT "+FieldName+" FROM "+TableName+" WHERE "+FieldName+" NOTNULL").toString();
        if(!sData.isEmpty() and sData.length()==10 and sData[4]=='-' and sData[7]=='-')
            return "DATE";
        else
            return "";
    } else
        return result;
}

// QString SQLiteDate() {
//     return QDate::currentDate().toString("yyyy-MM-dd");
// }

// QString DBInfo(QSqlDatabase *db)
// {
//     //QSqlDatabase db = QSqlDatabase::database();
//     QString sResult;
//     sResult=db->databaseName()+" - "+db->driverName()+" - "+db->connectOptions()+" : ";
//     sResult.append(iif(db->isValid(),"VALID DRIVER, ","").toString());
//     //sResult.append(iif(db.isOpen(),"DB IS OPEN, ","").toString()); isOpen always return true
//     sResult.append(iif(db->isOpenError(),"OPENERROR, ","").toString());
//     sResult.truncate(sResult.length()-2);
//     return sResult;
// }

QString EscapeCSV(QString s) {
    //s=StrReplace(s,"\r","");
    //s=StrReplace(s,"\n","\\n");
    s=StrReplace(s,"\"","\"\"");
    if (s.contains("\n") or s.contains(";") or s.contains("\""))
        s="\""+s+"\"";
    return s;
}

QVariant iif(bool bCond,QVariant Var1,QVariant Var2)
{
    if (bCond)
        return Var1;
    else
        return Var2;
}

bool isDarkTheme() {
    QColor backgroundColor = QApplication::palette().color(QPalette::Window);
    return backgroundColor.lightness() < 128;
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
        if(parsedValue.count("\"") % 2 == 0) {//Even number of ".
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
    QString normalized = input.normalized(QString::NormalizationForm_D).toLower();
    normalized.remove(QRegularExpression("[̀-̈]"));  // Supprime les accents combinés
    return normalized;
}

QString RemoveComment(QString sCde, QString sCommentMarker, bool keepReturns)
{
    QStringList LinesList = sCde.split("\n");
    QString s;
    QString sResult="";
    int index;
    for (int i=0;i<LinesList.count();i++)
    {
        s = LinesList[i];
        index = s.indexOf(sCommentMarker);
        if (index!=-1)
            sResult += iif(i>0 and keepReturns,"\n","").toString()+s.first(index).trimmed()+iif(s.first(index).trimmed().isEmpty(),""," ").toString();
        else
            sResult += iif(i>0 and keepReturns,"\n","").toString()+s.trimmed()+iif(s.trimmed().isEmpty(),""," ").toString();
    }
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

    QPalette p = l->palette();
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
            QPalette palette = widget->palette();
            palette.setColor(QPalette::Text, color);
            widget->setPalette(palette);
        } else
            widget->setPalette(QApplication::palette());
    }
}

void SetFontWeight(QWidget* widget, QFont::Weight weight) {
    if (widget) {
        QFont font = widget->font();
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

QString StrReplace(QString s, const QString sTarg, const QString sRepl) {
    int index = 0;
    int index2 = 0;
    if (sTarg.isEmpty())
        return s;
    while ((index = s.indexOf(sTarg, index2)) != -1) {
        s.replace(index, sTarg.length(), sRepl);
        index2 = index+sRepl.length();  // To avoid infinite loop
    }
    return s;
}

QString SubString(QString s, int iDeb, int iFin) {
    QString result="";

    if (iDeb>iFin)
        return "";

    for (int i=iDeb;i<min(s.length(),iFin+1);i++)
        result+=s[i];

    return result;
}

//booldbSuspend(QSqlDatabase *db, bool bSuspend, bool bEditing, QLabel *ldbs) {
    //return true;
    // //QSqlDatabase db = QSqlDatabase::database();
    // if(bSuspend) {
    //     if (dbIsOpen and !bEditing){//db->isOpen() and
    //         // PotaQuery q1(*db);
    //         // q1.exec("SELECT define_free();");
    //         db->close();
    //         if (ldbs)
    //             SetColoredText(ldbs,"Database released","Ok");
    //         if (db->isOpen())
    //             qDebug() << "db open (fail to close)";
    //         else
    //             qDebug() << "db closed";
    //         dbIsOpen=db->isOpen();
    //     }
    //     dbIsOpen=false;
    //     return true;
    // } else {
    //     if (!dbIsOpen){
    //         if (db->open()){
    //             if(initSQLean(db)){
    //                 if (ldbs)
    //                     SetColoredText(ldbs,"Database exclusive access...","Err");
    //                 qDebug() << "db open";
    //                 dbIsOpen=true;
    //                 return true;
    //             } else {
    //                 db->close();
    //                 qDebug() << "db open (Err SQLean)";
    //                 dbIsOpen=true;
    //                 return false;
    //             }
    //         } else {
    //             if (db->isOpen())
    //                 qDebug() << "db open";
    //             else
    //                 qDebug() << "db close (fail to open)";
    //             dbIsOpen=db->isOpen();
    //             return false;
    //         }
    //     } else {
    //     //     qDebug() << "dbSuspend already OFF";
    //          return true;
    //     }

    // }
//}
