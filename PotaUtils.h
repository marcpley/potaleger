#ifndef POTAUTILS_H
#define POTAUTILS_H

#include "qlabel.h"
#include "qtoolbutton.h"
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QPainter>
#include <QItemDelegate>

class PotaQuery: public QSqlQuery
{
public:
    PotaQuery() {}

    QLabel *lErr;
    bool ExecShowErr(QString query);
    bool ExecMultiShowErr(QString querys);
};

void SetColoredText(QLabel *l, QString text, QString type);
void SetButtonSize(QToolButton *b);
QString str(int i);
QString str(float i);
QString str(qsizetype i);

#endif // POTAUTILS_H
