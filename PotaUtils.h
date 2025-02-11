#ifndef POTAUTILS_H
#define POTAUTILS_H

#include "qlabel.h"
#include "qprogressbar.h"
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
    bool ExecMultiShowErr(const QString querys, const QString spliter, QProgressBar *progressBar);
    QVariant Selec0ShowErr(QString query);
};

QString DataType(QString TableName, QString FieldName);
//QString SQLiteDate();
QString DBInfo();
QVariant iif(bool bCond,QVariant Var1,QVariant Var2);
bool isDarkTheme();
float min(float a,float b);
int min(int a,int b);
QString RemoveComment(QString sCde, QString sCommentMarker);
void SetButtonSize(QToolButton *b);
void SetFontColor(QWidget* widget, QColor color);
void SetFontWeight(QWidget* widget, QFont::Weight weight);
void SetColoredText(QLabel *l, QString text, QString type);
QString str(int i);
QString str(float i);
QString str(qsizetype i);
QString StrFirst(QString s, int i);
QString StrLast(QString s, int i);
QString StrReplace(QString s, const QString sTarg, const QString sRepl);
QString SubString(QString s, int iDeb, int iFin);

#endif // POTAUTILS_H
