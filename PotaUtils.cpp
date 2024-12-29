#include "qcolor.h"
#include "qlabel.h"
#include "qsqlerror.h"
#include "PotaUtils.h"

bool PotaQuery::ExecShowErr(QString query)
{
    exec(query);
    if (lastError().type() != QSqlError::NoError)
    {
        if (lErr!=nullptr)
            SetColoredText(lErr,lastError().text(),"Err");
        return false;

    }
    return true;
}

bool PotaQuery::ExecMultiShowErr(QString querys)
{
    QStringList QueryList = querys.split(";");
    for (int i=0;i<QueryList.count();i++)
    {
        if (QueryList[i].trimmed().isEmpty())
            continue;
        ExecMultiShowErr(QueryList[i]);
        if (lastError().type() != QSqlError::NoError)
        {
            if (lErr!=nullptr)
                SetColoredText(lErr,lastError().text(),"Err");
            return false;

        }
    }
    return true;
}

void SetButtonSize(QToolButton *b)
{
    b->setFixedSize(24,24);
    b->setIconSize(QSize(24,24));
}

void SetColoredText(QLabel *l, QString text, QString type)
{
    l->setText(text);
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
