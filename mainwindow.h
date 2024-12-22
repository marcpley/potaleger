#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlQueryModel>
#include <QSqlTableModel>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event) override;
    void ShowEvent(QShowEvent *event) ;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void RestaureParams();
    void SauvParams();

private slots:
    void on_mSelecDB_triggered();
    void on_mRafraichir_triggered();
    void on_mParam_triggered();
    void on_mFermerOnglet_triggered();
    void on_mFermerOnglets_triggered();
    void on_mAbandonnerModifs_triggered();

    void on_tabWidget_currentChanged(int index);

    void on_mCopyBDD_triggered();

private:
    Ui::MainWindow *ui;
    void ActiverMenusData(bool b);
    bool ExecSql(QSqlQueryModel &model,QString sSQL);
    bool ExecSql(QSqlTableModel &model);
    bool InfosBDD();
    bool MaJStruBDD(QString sVersionBDD);
    bool OkCancelDialog(QString sMessage);
    void MessageDialog(QString sMessage, QMessageBox::Icon Icon);
    void MessageDialog(QString sMessage);
    void FermerBDD();
    void OuvrirBDD(QString sFichier);
    void OuvrirOnglet(QString const sNames,QString const sTableName, QString const sTitre);
    void FermerOnglet(QWidget *Tab);
};
#endif // MAINWINDOW_H
