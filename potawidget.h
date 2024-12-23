#ifndef POTAWIDGET_H
#define POTAWIDGET_H

#include <QSqlRelationalTableModel>
#include <QTableView>

class PotaModel: public QSqlRelationalTableModel
{
public:
    PotaModel(QWidget *parent = 0);
    QString FieldName(int index);
    bool SelectShowErr();
};

class PotaQueryModel: public QSqlQueryModel
{
public:
    PotaQueryModel(QWidget *parent = 0);
    QString FieldName(int index);
    bool setQueryShowErr(QString query);
};

class PotaWidget: public QWidget
{
    Q_OBJECT

public:
    PotaWidget(QWidget *parent = 0);
    PotaModel *model = new PotaModel(this);
    QTableView *tv = new QTableView(this);
    bool ModifsEnCours=false;
};

#endif // POTAWIDGET_H
