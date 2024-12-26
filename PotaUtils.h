#ifndef POTAUTILS_H
#define POTAUTILS_H

#include "qlabel.h"
#include "qpushbutton.h"
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
void SetButtonSize(QPushButton *b);
QString str(int i);
QString str(float i);

#endif // POTAUTILS_H
