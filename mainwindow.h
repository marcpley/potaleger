#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "PotaUtils.h"
#include <QMainWindow>
#include <QtSql/QSqlQueryModel>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QtSvg>
//#include "sqlite/sqlite3.h"
#include "sqlean/sqlite3.h"

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
    void on_mCreerBDD_triggered();
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
    void on_mSuccessionParPlanche_triggered();
    void on_mTypes_de_planche_triggered();


    void on_mCulturesParIlots_triggered();

    void on_mRotationManquants_triggered();

    void on_mCulturesParPlanche_triggered();

    void on_mSemences_triggered();

    void on_mCuNonTer_triggered();

    void on_mCuSemisAFaire_triggered();

    void on_mCuPlantationsAFaire_triggered();

    void on_mCuRecoltesAFaire_triggered();

    void on_mCuSaisieRecoltes_triggered();

    void on_mCuATerminer_triggered();

    void on_mCuToutes_triggered();

    void on_mAnaITP_triggered();

    void on_mAnaEspeces_triggered();

    void on_mITPTempo_triggered();

    void on_mCreerCultures_triggered();

    void on_tabWidget_currentChanged(int index);

    void on_mCulturesParplante_triggered();

private:
    void ActiverMenusData(bool b);
    bool PotaBDDInfo();
    bool UpdateDBShema(QString sDBVersion);
    bool OkCancelDialog(QString sMessage);
    void MessageDialog(QString sMessage, QMessageBox::Icon Icon);
    void MessageDialog(QString sMessage);
    bool YesNoDialog(QString sMessage);
    bool dbOpen(QString sFichier, bool UpdateSQLean);
    bool initCustomFunctions();
    bool registerCustomFunctions();
    QString testCustomFunctions();
    void dbClose();
    void OuvrirBDD(QString sFichier);
    void FermerBDD();
    bool OuvrirOnglet(QString const sObjName, QString sTableName, QString const sTitre);
    void FermerOnglet(QWidget *Tab);
    void FermerOnglets();
    void CreateNewDB(bool bEmpty);
    static void sommeFunc(sqlite3_context *context, int argc, sqlite3_value **argv);
    void define_manage_init2(sqlite3 *db_handle);
    static void logCallback(void *pArg, int iErrCode, const char *zMsg);
};
#endif // MAINWINDOW_H
