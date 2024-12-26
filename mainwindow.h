#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlQueryModel>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QtSvg>

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
    Ui::MainWindow *ui;
    void RestaureParams();
    void SauvParams();

private slots:
    void on_mSelecDB_triggered();
    void on_mParam_triggered();
    void on_mFermerOnglet_triggered();
    void on_mFermerOnglets_triggered();
    void on_mCopyBDD_triggered();
    void on_mCreerBDDVide_triggered();
    void on_mFamilles_triggered();
    void on_mEspeces_triggered();
    void on_mVarietes_triggered();
    void on_mApports_triggered();
    void on_mFournisseurs_triggered();
    void on_mITP_triggered();
    void on_mRotations_triggered();
    void on_mDetailsRotations_triggered();
    void on_mPlanches_triggered();
    void on_mIlots_triggered();

private:

    void ActiverMenusData(bool b);
    bool InfosBDD();
    bool MaJStruBDD(QString sVersionBDD);
    bool OkCancelDialog(QString sMessage);
    void MessageDialog(QString sMessage, QMessageBox::Icon Icon);
    void MessageDialog(QString sMessage);
    void FermerBDD();
    void OuvrirBDD(QString sFichier);
    bool OuvrirOnglet(QString const sObjName, QString sTableName, QString const sTitre);
    void FermerOnglet(QWidget *Tab);
    void FermerOnglets();
};
#endif // MAINWINDOW_H
