#ifndef POTAWIDGET_H
#define POTAWIDGET_H

#include "qlabel.h"
#include "qsqlrelationaltablemodel.h"
#include <QTableView>

class PotaTableModel: public QSqlRelationalTableModel
{
public:
    PotaTableModel(QWidget *parent = 0) {}

    QString FieldName(int index);
    bool SelectShowErr();
    bool SubmitAllShowErr();
    bool RevertAllShowErr();
};

class PotaQueryModel: public QSqlQueryModel
{
public:
    PotaQueryModel(QWidget *parent = 0) {}

    QString TableName;
    QLabel *lErr;
    QString FieldName(int index);
    bool setQueryShowErr(QString query);
    bool setMultiQueryShowErr(QString querys);
};

class PotaWidget: public QWidget
{
    Q_OBJECT

public:
    PotaWidget(QWidget *parent = 0);

    PotaTableModel *model;
    QTableView *tv;
    QLabel *lErr;
    bool ModifsEnCours=false;
};

#endif // POTAWIDGET_H
