#ifndef POTAUTILS_H
#define POTAUTILS_H

#include "qlabel.h"
#include "qtoolbutton.h"
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QPainter>

class PotaQuery: public QSqlQuery
{

public:
    PotaQuery() {}

    QLabel *lErr;
    bool ExecShowErr(QString query);
    bool ExecMultiShowErr(QString querys, QString spliter);
    QVariant Selec0ShowErr(QString query);
};

QString DBInfo();
QVariant iif(bool bCond,QVariant Var1,QVariant Var2);
bool isDarkTheme();
QString RemoveComment(QString sCde, QString sCommentMarker);
void SetButtonSize(QToolButton *b);
void SetColoredText(QLabel *l, QString text, QString type);
QString str(int i);
QString str(float i);
QString str(qsizetype i);
QString StrReplace(QString s, const QString sTarg, const QString sRepl);

#endif // POTAUTILS_H