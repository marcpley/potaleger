#include "qapplication.h"
#include "qcolor.h"
#include "qlabel.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "PotaUtils.h"
#include <QFont>
#include "data/Data.h"

bool PotaQuery::ExecShowErr(QString query)
{
    clear();
    exec(query);
    if (lastError().type() != QSqlError::NoError)
    {
        if (lErr!=nullptr)
            SetColoredText(lErr,lastError().text(),"Err");
        return false;

    }
    return true;
}

bool PotaQuery::ExecMultiShowErr(QString querys, QString spliter)
{
    QStringList QueryList = querys.split(spliter);
    QString s;
    s=lErr->text();
    if ((s.length()>11)and(s.last(11)==" statements"))
        s=s.first(s.length()-11)+"+"+str(QueryList.count())+" statements";
    else
        s=str(QueryList.count())+" statements";
    SetColoredText(lErr,s,"Info");
    for (int i=0;i<QueryList.count();i++)
    {
        if (QueryList[i].trimmed().isEmpty())
            continue;
        QString sQuery=RemoveComment(QueryList[i].trimmed(),"--");
        clear();
        ExecShowErr(sQuery);
        if (lastError().type() != QSqlError::NoError)
        {
            if (lErr!=nullptr)
                SetColoredText(lErr,QueryList[i].trimmed()+"\n"+lastError().text()+"\n"+
                                    DBInfo(),"Err");
            return false;

        }
    }
    return true;
}

QVariant PotaQuery::Selec0ShowErr(QString query)
{
    clear();
    QVariant vNull;
    if (ExecShowErr(query))
    {
        next();
        return value(0);
    }
    else
        return vNull;
}

QString DataType(QString TableName, QString FieldName){
    QString result="";
    QSqlQuery query("PRAGMA table_xinfo("+TableName+")");
    while (query.next()){
        if (query.value(1).toString()==FieldName){
            result=query.value(2).toString();
            break;
        }
    }

    if (result=="" and//Unknow view field.
        ViewFieldIsDate(FieldName))
        return "DATE";
    else
        return result;
}

// QString SQLiteDate() {
//     return QDate::currentDate().toString("yyyy-MM-dd");
// }

QString DBInfo()
{
    QSqlDatabase db = QSqlDatabase::database();
    QString sResult;
    sResult=db.databaseName()+" - "+db.driverName()+" - "+db.connectOptions()+" : ";
    sResult.append(iif(db.isValid(),"VALID DRIVER, ","").toString());
    sResult.append(iif(db.isOpen(),"DB IS OPEN, ","").toString());
    sResult.append(iif(db.isOpenError(),"OPENERROR, ","").toString());
    sResult.truncate(sResult.length()-2);
    return sResult;
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

QString RemoveComment(QString sCde, QString sCommentMarker)
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
            sResult += s.first(index).trimmed()+iif(s.first(index).trimmed().isEmpty(),""," ").toString();
        else
            sResult += s.trimmed()+iif(s.trimmed().isEmpty(),""," ").toString();
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
        s="Valeur incorrecte pour "+s;
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
        p.setColor(QPalette::WindowText, "white");
    l->setPalette(p);
}

void SetButtonSize(QToolButton *b)
{
    b->setFixedSize(24,24);
    b->setIconSize(QSize(24,24));
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
    // int index = 0;
    // while ((index = s.indexOf(sTarg, index)) != -1) {
    //     s.replace(index, sTarg.length(), sRepl);
    //     index += sRepl.length();  // To avoid infinite loop
    // }
    // return s;

    while (s.indexOf(sTarg)!= -1)
        s.replace(s.indexOf(sTarg),sTarg.length(),sRepl);
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
