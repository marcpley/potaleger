#ifndef POTAWIDGET_H
#define POTAWIDGET_H

#include "qboxlayout.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qspinbox.h"
#include "qsqlrelationaltablemodel.h"
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

    QString TableName;
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

    QColor Couleur;
    QColor CouleursColonnes[50];//Seules les 50 1ères colonnes d'une tables peuvent avoir une FK.

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
    int iSortCol = -1;
    bool bSortDes = false;
    //bool isPendingModifs=false;
    bool isCommittingError=false;

    QWidget *toolbar;
    QPushButton *pbRefresh;
    QPushButton *pbCommit;
    QPushButton *pbRollback;
    QSpinBox *sbInsertRows;
    QPushButton *pbInsertRow;
    QPushButton *pbDeleteRow;
    QFrame * fSort;
    QCheckBox *cbSort;
    QLineEdit *leSort;
    QSpinBox *sbSort;
    QHBoxLayout *ls;
    QHBoxLayout *ltb;
    QVBoxLayout *lw;

    QLabel *lErr;

private slots:
    void curChanged(const QModelIndex cur, const QModelIndex pre);
    void dataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight,const QList<int> &roles);
    void headerColClicked(int logicalIndex);
    void headerRowClicked(int logicalIndex);
    void pbRefreshClick();
    void pbCommitClick();
    void pbRollbackClick();
    void pbInsertRowClick();
    void pbDeleteRowClick();
    void cbSortClick(Qt::CheckState state);
};

#endif // POTAWIDGET_H
