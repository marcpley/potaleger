#ifndef POTAWIDGET_H
#define POTAWIDGET_H

#include "qboxlayout.h"
#include "qlabel.h"
#include "qspinbox.h"
#include "qsqlrelationaltablemodel.h"
#include "qtoolbutton.h"
#include <QSqlRelationalDelegate>
#include <QTableView>
#include <QCheckBox>

class PotaTableModel: public QSqlRelationalTableModel
{
    Q_OBJECT

public:
    PotaTableModel() {}

    QString FieldName(int index);
    bool SelectShowErr();
    bool SubmitAllShowErr();
    bool RevertAllShowErr();
    bool InsertRowShowErr();
    bool DeleteRowShowErr();
};

class PotaTableView: public QTableView
{
    Q_OBJECT

public:
    PotaTableView() {}

};


class PotaQueryModel: public QSqlQueryModel
{
    Q_OBJECT

public:
    PotaQueryModel() {}

    QString TableName = "";
    QLabel *lErr;
    QString FieldName(int index);
    bool setQueryShowErr(QString query);
    bool setMultiQueryShowErr(QString querys);
};

class PotaItemDelegate : public QSqlRelationalDelegate
{
    Q_OBJECT

public:
    PotaItemDelegate() {}//QObject *parent

    QColor cTableColor;
    QColor cColColors[50];

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const    override;
    //SqlQueryItem *getItem(const QModelIndex &index) const;
};


class PotaWidget: public QWidget
{
    Q_OBJECT

public:
    PotaWidget(QWidget *parent = 0);

    void Init(QString TableName);
    PotaTableModel *model;
    QTableView *tv;
    PotaItemDelegate *delegate;
    QTabWidget *twParent;
    int iSortCol = -1;
    bool bSortDes = false;
    bool isCommittingError=false;
    bool isView = false;

    QWidget *toolbar;
    QToolButton *pbRefresh;
    QToolButton *pbCommit;
    QToolButton *pbRollback;
    QSpinBox *sbInsertRows;
    QToolButton *pbInsertRow;
    QToolButton *pbDeleteRow;
    QFrame * fFilter;
    QCheckBox *cbFilter;
    QLineEdit *leFilter;
    QLabel *lFilter;
    QSpinBox *sbFilter;
    QHBoxLayout *lf;
    QHBoxLayout *ltb;
    QVBoxLayout *lw;

    QLabel *lErr;

private slots:
    void curChanged(const QModelIndex cur, const QModelIndex pre);
    void dataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight,const QList<int> &roles);
    void headerColClicked(int logicalIndex);
    void headerRowClicked();//int logicalIndex
    void pbRefreshClick();
    void pbCommitClick();
    void pbRollbackClick();
    void pbInsertRowClick();
    void pbDeleteRowClick();
    void cbFilterClick(Qt::CheckState state);
    void leFilterReturnPressed();
};

#endif // POTAWIDGET_H
