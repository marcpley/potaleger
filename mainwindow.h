#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlQueryModel>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QtSvg>
#include <QStyle>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

QString const Version="1.3.0b2";
QString const DbVersion="2025-07-28";

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
    // bool userDataEditing=false;
    // bool dbIsOpen=false;
    QString PathExport="";
    QString PathImport="";
    int TypeImport=0;
    bool MarkdownFont=false;
    bool ReadOnlyDb=true;

    void RestaureParams();
    void SauvParams();
    void SetUi();
    void SetMenuIcons();
    void showIfDdOpen();

private slots:
    void on_mSelecDB_triggered();
    void on_mParam_triggered();
    void on_mCloseTab_triggered();
    void on_mCloseTabs_triggered();
    void on_mCopyBDD_triggered();
    void on_mCreateDB_triggered();
    void on_mCreateEmptyDB_triggered();

    void on_mFamilles_triggered();
    void on_mEspecesA_triggered();
    void on_mEspecesV_triggered();
    void on_mVarietes_triggered();
    //void on_mApports_triggered();
    void on_mFertilisants_triggered();
    void on_mFournisseurs_triggered();
    void on_mRotations_triggered();
    void on_mDetailsRotations_triggered();
    void on_mPlanches_triggered();
    void on_mIlots_triggered();
    void on_mSuccessionParPlanche_triggered();
    void on_mCulturesParIlots_triggered();
    void on_mRotationManquants_triggered();
    void on_mCulturesParPlanche_triggered();
    void on_mSemences_triggered();
    void on_mCuNonTer_triggered();
    void on_mCuASemerEP_triggered();
    void on_mCuAPlanter_triggered();
    void on_mCuARecolter_triggered();
    void on_mCuSaisieRecoltes_triggered();
    void on_mCuATerminer_triggered();
    void on_mCuToutes_triggered();
    void on_mAnaITPA_triggered();
    void on_mAnaCultures_triggered();
    void on_mITPTempo_triggered();
    void on_mCreerCultures_triggered();
    void on_tabWidget_currentChanged(int index);
    void on_mCulturesParplante_triggered();
    void on_mAbout_triggered();
    void on_mUpdateSchema_triggered();
    void on_mExport_triggered();
    void on_mImport_triggered();
    void on_mIncDatesCultures_triggered();
    void on_mDestinations_triggered();
    void on_mInventaire_triggered();
    void on_mEsSaisieConso_triggered();
    void on_cbTheme_currentIndexChanged(int index);
    void on_mCuASemerPep_triggered();
    void on_mCuASemer_triggered();
    void on_mCouverture_triggered();
    void on_cbFont_currentTextChanged(const QString &arg1);
    void on_mNotes_triggered();
    void on_mWhatSNew_triggered();
    void on_mAnalysesSol_triggered();
    void on_mRequeteSQL_triggered();
    void on_mCuAFertiliser_triggered();
    void on_mFertilisations_triggered();
    void on_mBilanPlanches_triggered();
    void on_mInventaireFert_triggered();
    void on_mCuAIrriguer_triggered();
    void on_mCuVivaces_triggered();
    void on_mPlanchesDeficit_triggered();
    void on_mAnaITPV_triggered();


    void on_mCuAFaire_triggered();

    void on_mPlants_triggered();

    void on_mRecoltesParSemaine_triggered();

private:
    void SetEnabledDataMenuEntries(bool b);
    // bool PotaBDDInfo();
    bool UpdateDBShema(QString sDBVersion);
    bool dbOpen(QString sFichier, bool bNew, bool bResetSQLean, bool SetFkOn);
    void dbClose();
    bool PotaDbOpen(QString sFichier, QString sNew, bool bUpdate);
    void PotaDbClose();
    bool OpenPotaTab(QString const sObjName, QString sTableName, QString const sTitre, const QString sDesc="");
    void ClosePotaTab(QWidget *Tab);
    void ClosePotaTabs();
    void CreateNewDB(bool bEmpty);
    // static void sommeFunc(sqlite3_context *context, int argc, sqlite3_value **argv);
    // void define_manage_init2(sqlite3 *db_handle);
    // static void logCallback(void *pArg, int iErrCode, const char *zMsg);
};
#endif // MAINWINDOW_H
