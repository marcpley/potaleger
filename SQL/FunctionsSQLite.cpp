#include "qsqldriver.h"
#include "mainwindow.h"
#include "qsqlerror.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QDebug>
//#include <sqlite3.h>
#include "sqlean/sqlite3.h"
#include "sqlean/define.h"
#include "SQL/FunctionsSQLite.sql"

bool MainWindow::initCustomFunctions() {
    qDebug() << "SQLite version:" << sqlite3_libversion();

    QSqlQuery q1;
    q1.exec("DELETE FROM sqlean_define");//"SELECT define_free('<function_name>')" don't work.

    QSqlDatabase db = QSqlDatabase::database();
    auto handle = db.driver()->handle();
    sqlite3 *db_handle = *static_cast<sqlite3 **>(handle.data());
    if (!db_handle or db_handle==0) {
        qCritical() << "Failed to retreive SQLite handle.";
        return false;
    }

    sqlite3_initialize();
    define_manage_init(db_handle);
    define_eval_init(db_handle);
    define_module_init(db_handle);
    sqlite3_close(db_handle);

    qDebug() << "Init functions ok.";

    return true;
}

bool MainWindow::registerCustomFunctions() {
    QSqlQuery q1;

    if (!q1.exec("SELECT define('SumTest', '? + ?')")){
        qCritical() << "Failed to register function 'Somme': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('PlanifCultureCalcDate','"+RemoveComment(sPlanifCultureCalcDate,"--")+"')")){
        qCritical() << "Failed to register function 'PlanifCultureCalcDate': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('ItpTempoNJPeriode','"+RemoveComment(sItpTempoNJPeriode,"--")+"')")){
        qCritical() << "Failed to register function 'ItpTempoNJPeriode': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('ItpTempoNJInterPe','"+RemoveComment(sItpTempoNJInterPe,"--")+"')")){
        qCritical() << "Failed to register function 'ItpTempoNJInterPe': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('ItpTempoNJ','"+RemoveComment(sItpTempoNJ,"--")+"')")){
        qCritical() << "Failed to register function 'ItpTempoNJ': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('ItpTempo','"+RemoveComment(sItpTempo,"--")+"')")){
        qCritical() << "Failed to register function 'ItpTempo': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('CulTempoNJPeriode','"+RemoveComment(sCulTempoNJPeriode,"--")+"')")){
        qCritical() << "Failed to register function 'CulTempoNJPeriode': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('CulTempo','"+RemoveComment(sCulTempo,"--")+"')")){
        qCritical() << "Failed to register function 'CulTempo': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }



    // q1.clear();
    // if (!q1.exec("SELECT define('Test','"+RemoveComment(sTest,"--")+"')")){
    //     qCritical() << "Failed to register function 'Test': "<< q1.lastError();
    //     qCritical() << q1.lastQuery();
    //     return false;
    // }

    qCritical() << q1.lastQuery();

    // q1.exec("SELECT undefine('PlanifCultureCalcDate')");
    // q1.clear();
    // if (!q1.exec("CREATE VIRTUAL TABLE PlanifCultureCalcDate USING define(("+sPlanifCultureCalcDate+"))")){
    //     qCritical() << "Failed to register function 'PlanifCultureCalcDate': "<< q1.lastError();
    //     return false;
    // }

    qDebug() << "Functions registered successfully.";

    return true;
}

QString MainWindow::testCustomFunctions() {
    //return "";


    QSqlQuery q1;
    if (!q1.exec("SELECT SumTest(1,2)") or !q1.next() or q1.value(0).toInt()!=3){
        qCritical() << "Function failed: SumTest(1,2) = " << q1.value(0).toInt();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "SumTest";
    }
    qDebug() << "Function ok : SumTest(1,2) = " << q1.value(0).toInt();

    q1.clear();
    if (!q1.exec("SELECT PlanifCultureCalcDate('2025-02-01','01-01')") or !q1.next() or q1.value(0).toString()!="2026-01-01"){
        qCritical() << "Function failed: PlanifCultureCalcDate('2025-02-01','01-01') = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "PlanifCultureCalcDate";
    }
    qDebug() << "Function ok : PlanifCultureCalcDate('2025-02-01','01-01') = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT ItpTempoNJPeriode(360,5,10)") or !q1.next() or q1.value(0).toInt()!=10){
        qCritical() << "Function failed: ItpTempoNJPeriode(360,5,10) = " << q1.value(0).toInt();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "ItpTempoNJPeriode";
    }
    qDebug() << "Function ok : ItpTempoNJPeriode(360,5,10) = " << q1.value(0).toInt();

    q1.clear();
    if (!q1.exec("SELECT ItpTempoNJInterPe(360,5,10)") or !q1.next() or q1.value(0).toInt()!=5){
        qCritical() << "Function failed: ItpTempoNJInterPe(360,5,10) = " << q1.value(0).toInt();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "ItpTempoNJInterPe";
    }
    qDebug() << "Function ok : ItpTempoNJInterPe(360,5,10) = " << q1.value(0).toInt();

    q1.clear();
    if (!q1.exec("SELECT ItpTempoNJ('02-01')") or !q1.next() or q1.value(0).toInt()!=32){
        qCritical() << "Function failed: ItpTempoNJ('02-01') = " << q1.value(0).toInt();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "ItpTempoNJ";
    }
    qDebug() << "Function ok : ItpTempoNJ('02-01') = " << q1.value(0).toInt();

    q1.clear();
    if (!q1.exec("SELECT ItpTempo('Semis sous abris',ItpTempoNJ('02-01'),ItpTempoNJ('02-15'),ItpTempoNJ('03-01'),ItpTempoNJ('03-15'),ItpTempoNJ('05-01'),ItpTempoNJ('06-01'))") or !q1.next() or q1.value(0).toString()!="31:14:15:14:47:31"){
        qCritical() << "Function failed: ItpTempo('Semis sous abris','02-01','02-15','03-01','03-15','05-01','06-01') = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "ItpTempo";
    }
    qDebug() << "Function ok : ItpTempo('Semis sous abris','02-01','02-15','03-01','03-15','05-01','06-01') = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT CulTempoNJPeriode('2000-01-01','2000-01-31')") or !q1.next() or q1.value(0).toInt()!=30){
        qCritical() << "Function failed: CulTempoNJPeriode(''2000-01-01','2000-01-31') = " << q1.value(0).toInt();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "CulTempoNJPeriode";
    }
    qDebug() << "Function ok : CulTempoNJPeriode('2000-01-01','2000-01-31') = " << q1.value(0).toInt();

    q1.clear();
    if (!q1.exec("SELECT CulTempo('Semis sous abris','2000-02-01','2000-02-15','2000-03-01','2000-03-15')") or !q1.next() or q1.value(0).toString()!="31:2:12:2:13:14"){
        qCritical() << "Function failed: CulTempo('Semis sous abris','2000-02-01','2000-02-15','2000-03-01','2000-03-15') = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "CulTempo";
    }
    qDebug() << "Function ok : CulTempo('Semis sous abris','2000-02-01','2000-02-15','2000-03-01','2000-03-15') = " << q1.value(0).toString();

    qDebug() << "Functions tested.";
    return "";
}

// void MainWindow::sommeFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
//     if (argc != 2) {
//         sqlite3_result_null(context);
//         return;
//     }
//     int a = sqlite3_value_int(argv[0]);
//     int b = sqlite3_value_int(argv[1]);
//     sqlite3_result_int(context, a + b);
// }

// void MainWindow::define_manage_init2(sqlite3 *db_handle) {
//     sqlite3_create_function(db_handle, "Somme", 2, SQLITE_UTF8, nullptr, sommeFunc, nullptr, nullptr);
// }

// void MainWindow::logCallback(void *pArg, int iErrCode, const char *zMsg) {
//     Q_UNUSED(pArg); // Pas utilisé ici
//     qDebug() << "SQLite Log [Error Code:" << iErrCode << "]:" << zMsg;
// }
