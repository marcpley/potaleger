#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "PotaUtils.h"
#include <QMainWindow>
#include <QtSql/QSqlQueryModel>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QtSvg>
#include "sqlite/sqlite3.h"
//#include "sqlean/sqlite3.h"
#include <QStyle>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

QString const Version="1.0RC3";
QString const DbVersion="2025-03-05";

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Ui::MainWindow *ui;

    //global variables.
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    // bool userDataEditing=false;//todo
    // bool dbIsOpen=false;
    QString PathExport="";
    QString PathImport="";
    int TypeImport=0;

    void RestaureParams();
    void SauvParams();
    void SetUi();
    void showIfDdOpen();

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

    void on_mAnaCultures_triggered();

    void on_mITPTempo_triggered();

    void on_mCreerCultures_triggered();

    void on_tabWidget_currentChanged(int index);

    void on_mCulturesParplante_triggered();

    void on_mLargeurs_triggered();

    void on_mEditNotes_triggered();

    void on_mAPropos_triggered();

    void on_mFilterFind_triggered();

    void on_mUpdateSchema_triggered();

    void on_mExporter_triggered();

    void on_mImporter_triggered();

    void on_mIncDatesCultures_triggered();

    void on_mDestinations_triggered();

    void on_mInventaire_triggered();

    void on_mEsSaisieSorties_triggered();

private:
    void SetEnabledDataMenuEntries(bool b);
    // bool PotaBDDInfo();
    bool UpdateDBShema(QString sDBVersion);
    void MessageDialog(const QString &message, const QString &message2 = "", QStyle::StandardPixmap iconType = QStyle::SP_CustomBase);
    bool OkCancelDialog(const QString &message, QStyle::StandardPixmap iconType = QStyle::SP_CustomBase);
    int RadiobuttonDialog(const QString &message, const QStringList &options, const int iDef, QStyle::StandardPixmap iconType = QStyle::SP_CustomBase);
    bool YesNoDialog(const QString &message, QStyle::StandardPixmap iconType = QStyle::SP_CustomBase);
    bool dbOpen(QString sFichier, bool bNew, bool bResetSQLean, bool SetFkOn);
    void dbClose();
    bool PotaDbOpen(QString sFichier, QString sNew, bool bUpdate);
    void PotaDbClose();
    bool OpenPotaTab(QString const sObjName, QString sTableName, QString const sTitre);
    void ClosePotaTab(QWidget *Tab);
    void ClosePotaTabs();
    void CreateNewDB(bool bEmpty);
    // static void sommeFunc(sqlite3_context *context, int argc, sqlite3_value **argv);
    // void define_manage_init2(sqlite3 *db_handle);
    // static void logCallback(void *pArg, int iErrCode, const char *zMsg);
};
#endif // MAINWINDOW_H
