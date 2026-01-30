#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include "qprogressbar.h"
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

QString const Version="1.5.0"; //Update in what's new
QString const DbVersion="2025-11-28"; //Update CREATE VIEW Info_Potaléger

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
public:
    MainWindow(QWidget *parent=nullptr);
    ~MainWindow();
    Ui::MainWindow *ui;

    //global variables.
    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");
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
    void on_mCopyDB_triggered();
    void on_mCreateDB_triggered();
    void on_mCreateEmptyDB_triggered();

    void on_FdaMenu(const QString &sActionName, const QString &sTitre, const QString &sDesc, const QString &sFilters, const QString &sGraph);

    //void on_mCreerCultures_triggered();
    void on_tabWidget_currentChanged(int index);
    void on_mAbout_triggered();
    void on_mUpdateSchema_triggered();
    void on_cbTheme_currentIndexChanged(int index);
    void on_cbFont_currentTextChanged(const QString &arg1);
    void on_mNotes_triggered();
    void on_mWhatSNew_triggered();
    void on_mRequeteSQL_triggered();

    void on_mFKErrors_triggered();

    void on_mSQLiteSchema_triggered();

    void on_mFdaTSchema_triggered();

    void on_mFdaFSchema_triggered();

    void on_mLaunchers_triggered();

    void on_mCloseDB_triggered();

    void on_mScripts_triggered();

private:
    void SetLaunchers(bool b);
    void CreateLaunchers(QString parentName, QWidget *parent);
    void DeleteLaunchers(QWidget *parent);
    // bool PotaBDDInfo();
    bool UpdateDBShema(QString sDBVersion);
    bool dbOpen(QString sFichier, bool bNew, bool SetFkOn);
    void dbClose();
    bool PotaDbOpen(QString sFichier, QString sNew, bool bUpdate);
    void PotaDbClose();
    bool OpenPotaTab(QString sTableName, QString const sTitre, const QString sDesc="");
    void ClosePotaTab(QWidget *Tab);
    void ClosePotaTabs();
    void CreateNewDB(bool bEmpty);
    void fitSplittersToPixmap(QPixmap pixmap);
    // static void sommeFunc(sqlite3_context *context, int argc, sqlite3_value **argv);
    // void define_manage_init2(sqlite3 *db_handle);
    // static void logCallback(void *pArg, int iErrCode, const char *zMsg);
};
#endif // MAINWINDOW_H
