#include "qsqldriver.h"
#include "mainwindow.h"
#include "qsqlerror.h"
#include <QSqlDatabase>
#include <QString>
#include <QVariant>
#include <QDebug>
#include "sqlite/sqlite3.h"
#include "sqlean/define.h"
#include "SQL/FunctionsSQLite.sql"
#include <QCollator>
#include <QRegularExpression>
#include "PotaUtils.h"

bool initSQLean(QSqlDatabase *db) {

    //QSqlDatabase db = QSqlDatabase::database();
    auto handle = db->driver()->handle();
    sqlite3 *db_handle = *static_cast<sqlite3 **>(handle.data());
    if (!db_handle or db_handle==0) {
        qCritical() << "Failed to retreive SQLite handle.";
        return false;
    }

    sqlite3_initialize();

    define_manage_init(db_handle);
    define_eval_init(db_handle);
    define_module_init(db_handle);
    sqlite3_close( db_handle);


    // if (sqlite3_create_function(db_handle, "remove_accents", 1, SQLITE_UTF8, nullptr, RemoveAccents, nullptr, nullptr) != SQLITE_OK) {
    //     qCritical() << "Failed to register function 'remove_accents'";
    //     return false;
    // }
    // db.close();
    // db.open();

    qInfo() << "Init SQLean functions ok.";

    return true;
}

bool registerScalarFunctions(QSqlDatabase *db) {
    //return true;
    PotaQuery q1(*db);

    // Math (non testées)

    q1.clear();
    if (!q1.exec("SELECT define('ceil2','"+RemoveComment(sCeil,"--")+"')")){
        qCritical() << "Failed to register function 'ceil2': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('floor2','"+RemoveComment(sFloor,"--")+"')")){
        qCritical() << "Failed to register function 'floor2': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    //Potaléger

    q1.clear();
    if (!q1.exec("SELECT define('PlanifCultureCalcDate','"+RemoveComment(sPlanifCultureCalcDate,"--")+"')")){
        qCritical() << "Failed to register function 'PlanifCultureCalcDate': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('ItpCompleteDFPeriode','"+RemoveComment(sItpCompleteDFPeriode,"--")+"')")){
        qCritical() << "Failed to register function 'ItpCompleteDFPeriode': "<< q1.lastError();
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
    if (!q1.exec("SELECT define('CulNbRecoltesTheo','"+RemoveComment(sCulNbRecoltesTheo,"--")+"')")){
        qCritical() << "Failed to register function 'CulNbRecoltesTheo': "<< q1.lastError();
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

    q1.clear();
    if (!q1.exec("SELECT define('CulTer','"+RemoveComment(sCulTer,"--")+"')")){
        qCritical() << "Failed to register function 'CulTer': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('CulIncDate','"+RemoveComment(sCulIncDate,"--")+"')")){
        qCritical() << "Failed to register function 'CulIncDate': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('CulIncDates','"+RemoveComment(sCulIncDates,"--")+"')")){
        qCritical() << "Failed to register function 'CulIncDates': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('RotDecalDateMeP','"+RemoveComment(sRotDecalDateMeP,"--")+"')")){
        qCritical() << "Failed to register function 'RotDecalDateMeP': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('RotTempo','"+RemoveComment(sRotTempo,"--")+"')")){
        qCritical() << "Failed to register function 'RotTempo': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT define('ItpPlusN','"+RemoveComment(sItpPlusN,"--")+"')")){
        qCritical() << "Failed to register function 'ItpPlusN': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }


    //qCritical() << q1.lastQuery();

    qInfo() << "Scalar functions registered successfully.";

    return true;
}

bool registerTableValuedFunctions(QSqlDatabase *db) {
    //return true;
    PotaQuery q1(*db);

    // q1.exec("DROP TABLE IF EXISTS Cul_Espece");
    // q1.clear();
    // if (!q1.exec("CREATE VIRTUAL TABLE Cul_Espece USING define(("+RemoveComment(sCul_Espece,"--")+"))")){
    //     qCritical() << "Failed to register function 'Cul_Espece': "<< q1.lastError();
    //     qCritical() << q1.lastQuery();
    //     return false;
    // }

    q1.exec("DROP TABLE IF EXISTS RF_trop_proches");
    q1.clear();
    if (!q1.exec("CREATE VIRTUAL TABLE RF_trop_proches USING define(("+RemoveComment(sRF_trop_proches,"--")+"))")){
        qCritical() << "Failed to register function 'RF_trop_proches': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.exec("DROP TABLE IF EXISTS R_ITP_CAnt");
    q1.clear();
    if (!q1.exec("CREATE VIRTUAL TABLE R_ITP_CAnt USING define(("+RemoveComment(sR_ITP_CAnt,"--")+"))")){
        qCritical() << "Failed to register function 'R_ITP_CAnt': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.exec("DROP TABLE IF EXISTS R_ITP_CDer");
    q1.clear();
    if (!q1.exec("CREATE VIRTUAL TABLE R_ITP_CDer USING define(("+RemoveComment(sR_ITP_CDer,"--")+"))")){
        qCritical() << "Failed to register function 'R_ITP_CDer': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.exec("DROP TABLE IF EXISTS Repartir_Recolte_sur");
    q1.clear();
    if (!q1.exec("CREATE VIRTUAL TABLE Repartir_Recolte_sur USING define(("+RemoveComment(sRepartir_Recolte_sur,"--")+"))")){
        qCritical() << "Failed to register function 'Repartir_Recolte_sur': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    // q1.exec("DROP TABLE IF EXISTS Analyse_de_sol_proche");
    // q1.clear();
    // if (!q1.exec("CREATE VIRTUAL TABLE Analyse_de_sol_proche USING define(("+RemoveComment(sAnalyse_de_sol_proche,"--")+"))")){
    //     qCritical() << "Failed to register function 'Analyse_de_sol_proche': "<< q1.lastError();
    //     qCritical() << q1.lastQuery();
    //     return false;
    // }

    q1.exec("DROP TABLE IF EXISTS Recoltes_cul");
    q1.clear();
    if (!q1.exec("CREATE VIRTUAL TABLE Recoltes_cul USING define(("+RemoveComment(sRecoltes_cul,"--")+"))")){
        qCritical() << "Failed to register function 'Recoltes_cul': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    q1.exec("DROP TABLE IF EXISTS Repartir_Fertilisation_sur");
    q1.clear();
    if (!q1.exec("CREATE VIRTUAL TABLE Repartir_Fertilisation_sur USING define(("+RemoveComment(sRepartir_Fertilisation_sur,"--")+"))")){
        qCritical() << "Failed to register function 'Repartir_Fertilisation_sur': "<< q1.lastError();
        qCritical() << q1.lastQuery();
        return false;
    }

    //qCritical() << q1.lastQuery();

    qInfo() << "Table valued functions registered successfully.";

    return true;
}

QString testCustomFunctions(QSqlDatabase *db) {
    //return "";

     PotaQuery q1(*db);
    // if (!q1.exec("SELECT SumTest(1,2)") or !q1.next() or q1.value(0).toInt()!=3){
    //     qCritical() << "Function failed: SumTest(1,2) = " << q1.value(0).toInt();
    //     qCritical() << q1.lastError();
    //     qCritical() << q1.lastQuery();
    //     return "SumTest";
    // }
    // qInfo() << "Function ok : SumTest(1,2) = " << q1.value(0).toInt();

    q1.clear();
    if (!q1.exec("SELECT PlanifCultureCalcDate('2025-02-01','03-01')") or !q1.next() or q1.value(0).toString()!="2025-03-01"){
        qCritical() << "Function failed: PlanifCultureCalcDate('2025-02-01','03-01') = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "PlanifCultureCalcDate";
    }
    qInfo() << "Function ok : PlanifCultureCalcDate('2025-02-01','03-01') = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT ItpCompleteDFPeriode(10)") or !q1.next() or q1.value(0).toString()!="10-01"){
        qCritical() << "Function failed: ItpCompleteDFPeriode(10) = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "ItpCompleteDFPeriode";
    }
    qInfo() << "Function ok : ItpCompleteDFPeriode(10) = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT ItpTempoNJPeriode(360,5,10)") or !q1.next() or q1.value(0).toInt()!=10){
        qCritical() << "Function failed: ItpTempoNJPeriode(360,5,10) = " << q1.value(0).toInt();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "ItpTempoNJPeriode";
    }
    qInfo() << "Function ok : ItpTempoNJPeriode(360,5,10) = " << q1.value(0).toInt();

    q1.clear();
    if (!q1.exec("SELECT ItpTempoNJInterPe(360,5,10)") or !q1.next() or q1.value(0).toInt()!=5){
        qCritical() << "Function failed: ItpTempoNJInterPe(360,5,10) = " << q1.value(0).toInt();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "ItpTempoNJInterPe";
    }
    qInfo() << "Function ok : ItpTempoNJInterPe(360,5,10) = " << q1.value(0).toInt();

    q1.clear();
    if (!q1.exec("SELECT ItpTempoNJ('02-01')") or !q1.next() or q1.value(0).toInt()!=32){
        qCritical() << "Function failed: ItpTempoNJ('02-01') = " << q1.value(0).toInt();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "ItpTempoNJ";
    }
    qInfo() << "Function ok : ItpTempoNJ('02-01') = " << q1.value(0).toInt();

    q1.clear();
    if (!q1.exec("SELECT ItpTempo('Semis pépinière',ItpTempoNJ('02-01'),ItpTempoNJ('02-15'),ItpTempoNJ('03-01'),ItpTempoNJ('03-15'),ItpTempoNJ('05-01'),ItpTempoNJ('06-01'))") or !q1.next() or q1.value(0).toString()!="31:14:14:14:47:31"){
        qCritical() << "Function failed: ItpTempo('Semis pépinière','02-01','02-15','03-01','03-15','05-01','06-01') = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "ItpTempo";
    }
    qInfo() << "Function ok : ItpTempo('Semis pépinière','02-01','02-15','03-01','03-15','05-01','06-01') = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT CulNbRecoltesTheo('v','2003-08-31',0,'2000-07-31')") or !q1.next() or q1.value(0).toInt()!=4){
        qCritical() << "Function failed: CulNbRecoltesTheo('v','2003-08-31',0,'2000-07-31') = " << q1.value(0).toInt();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "CulNbRecoltesTheo";
    }
    qInfo() << "Function ok : CulNbRecoltesTheo('v','2003-08-31',0,'2000-07-31') = " << q1.value(0).toInt();

    q1.clear();
    if (!q1.exec("SELECT CulTempoNJPeriode('2000-01-01','2000-01-31')") or !q1.next() or q1.value(0).toInt()!=30){
        qCritical() << "Function failed: CulTempoNJPeriode(''2000-01-01','2000-01-31') = " << q1.value(0).toInt();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "CulTempoNJPeriode";
    }
    qInfo() << "Function ok : CulTempoNJPeriode('2000-01-01','2000-01-31') = " << q1.value(0).toInt();

    q1.clear();
    if (!q1.exec("SELECT CulTempo('Semis pépinière','2000-02-01','2000-02-15','2000-03-01','2000-03-15')") or !q1.next() or q1.value(0).toString()!="31:4:10:4:11:14"){
        qCritical() << "Function failed: CulTempo('Semis pépinière','2000-02-01','2000-02-15','2000-03-01','2000-03-15') = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "CulTempo";
    }
    qInfo() << "Function ok : CulTempo('Semis pépinière','2000-02-01','2000-02-15','2000-03-01','2000-03-15') = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT CulTer('v')") or !q1.next() or q1.value(0).toBool()!=false){
        qCritical() << "Function failed: CulTer('v') = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "CulTer";
    }
    qInfo() << "Function ok : CulTer('v') = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT CulIncDate('2000','02-01','2000-01-30','02-01','D',1)") or !q1.next() or q1.value(0).toInt()!=1){
        qCritical() << "Function failed: CulIncDate(...) = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "CulIncDate";
    }
    qInfo() << "Function ok : CulIncDate(...) = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT CulIncDates('2000','02-01','2000-02-02','02-01','02-15','2000-03-02','03-01','03-15','2000-04-02','04-01','2000-04-14','04-15')") or !q1.next() or q1.value(0).toString()!=""){
        qCritical() << "Function failed: CulIncDates(...) = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "CulIncDates";
    }
    qInfo() << "Function ok : CulIncDates('2000-02-02','02-01','02-15','2000-03-02','03-01','03-15','2000-04-02','04-01','2000-04-14','04-15') = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT RotDecalDateMeP('2000-09-02')") or !q1.next() or q1.value(0).toString()!="2000-05-02"){
        qCritical() << "Function failed: RotDecalDateMeP('2000-09-02') = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "RotDecalDateMeP";
    }
    qInfo() << "Function ok : RotDecalDateMeP('2000-09-02') = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT RotTempo('Semis pépinière',ItpTempoNJ('02-01'),ItpTempoNJ('02-15'),ItpTempoNJ('03-01'),ItpTempoNJ('03-15'),ItpTempoNJ('05-01'),ItpTempoNJ('06-01'))") or !q1.next() or q1.value(0).toString()!="59:::14:47:31"){
        qCritical() << "Function failed: RotTempo('Semis pépinière','02-01','02-15','03-01','03-15','05-01','06-01') = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "RotTempo";
    }
    qInfo() << "Function ok : RotTempo('Semis pépinière','02-01','02-15','03-01','03-15','05-01','06-01') = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT * FROM RF_trop_proches('xxx')")){
        qCritical() << "Function failed: RF_trop_proches('xxx')";
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "RF_trop_proches";
    }
    qInfo() << "Function ok : RF_trop_proches('xxx')";

    q1.clear();
    if (!q1.exec("SELECT * FROM R_ITP_CAnt('xxx','2001-01-01')")){
        qCritical() << "Function failed: R_ITP_CAnt('xxx','2001-01-01')";
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "R_ITP_CAnt";
    }
    qInfo() << "Function ok : R_ITP_CAnt('xxx','2001-01-01')";

    q1.clear();
    if (!q1.exec("SELECT * FROM R_ITP_CDer(3,'xxx')")){
        qCritical() << "Function failed: R_ITP_CDer(3,'xxx')";
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "R_ITP_CDer";
    }
    qInfo() << "Function ok : R_ITP_CDer(3,'xxx')";

    q1.clear();
    if (!q1.exec("SELECT ItpPlusN('02-15','1 months')") or !q1.next() or q1.value(0).toString()!="03-15"){
        qCritical() << "Function failed: ItpPlusN('02-15','1 months') = " << q1.value(0).toString();
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "ItpPlusN";
    }
    qInfo() << "Function ok : ItpPlusN('02-15','1 months') = " << q1.value(0).toString();

    q1.clear();
    if (!q1.exec("SELECT * FROM Repartir_Recolte_sur('xxx','xxx','2000-01-01')")){
        qCritical() << "Function failed: Repartir_Recolte_sur('xxx','xxx','2000-01-01')";
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "Repartir_Recolte_sur";
    }
    qInfo() << "Function ok : Repartir_Recolte_sur('xxx','xxx','2000-01-01')";

    // q1.clear();
    // if (!q1.exec("SELECT * FROM Analyse_de_sol_proche('xxx')")){
    //     qCritical() << "Function failed: Analyse_de_sol_proche('xxx')";
    //     qCritical() << q1.lastError();
    //     qCritical() << q1.lastQuery();
    //     return "Analyse_de_sol_proche";
    // }
    // qInfo() << "Function ok : Analyse_de_sol_proche('xxx')";

    q1.clear();
    if (!q1.exec("SELECT * FROM Recoltes_cul(1,'v ','2000-05-01','2000-08-01')")){
        qCritical() << "Function failed: Recoltes_cul(1,'v ','2000-05-01','2000-08-01')";
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "Recoltes_cul";
    }
    qInfo() << "Function ok : Recoltes_cul(1,'v ','2000-05-01','2000-08-01')";

    q1.clear();
    if (!q1.exec("SELECT * FROM Repartir_Fertilisation_sur('xxx','xxx','2000-01-01')")){
        qCritical() << "Function failed: Repartir_Fertilisation_sur('xxx','xxx','2000-01-01')";
        qCritical() << q1.lastError();
        qCritical() << q1.lastQuery();
        return "Repartir_Fertilisation_sur";
    }
    qInfo() << "Function ok : Repartir_Fertilisation_sur('xxx','xxx','2000-01-01')";

    qInfo() << "Functions tested.";
    return "";
}

int PotaCollation(void* /*unused*/, int len1, const void* str1, int len2, const void* str2) {
    QString s1 = QString::fromUtf8(static_cast<const char*>(str1), len1);
    QString s2 = QString::fromUtf8(static_cast<const char*>(str2), len2);

    QCollator collator(QLocale::French);
    //collator.setIgnorePunctuation(true);
    //collator.setStrength(QCollator::Primary);  // Ignore les différences d'accents

    return collator.compare(s1, s2);
}

bool registerPotaCollation(QSqlDatabase& db) {
    auto handle = db.driver()->handle();
    sqlite3 *db_handle = *static_cast<sqlite3 **>(handle.data());
    if (!db_handle or db_handle==0) {
        qCritical() << "Failed to retreive SQLite handle.";
        return false;
    }

    // Enregistrement de la collation
    int result = sqlite3_create_collation(db_handle, "POTACOLLATION", SQLITE_UTF8, nullptr, PotaCollation);
    if (result != SQLITE_OK) {
        qCritical() << "Failed to register collation POTACOLLATION.";
        return false;
    }
    qInfo() << "Init collation ok.";
    return true;
}

static void RemoveAccents(sqlite3_context* context, int argc, sqlite3_value** argv) { //todo: utile ?
    if (argc == 1 && sqlite3_value_type(argv[0]) == SQLITE_TEXT) {
        QString input = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_value_text(argv[0]))).toLower();
        QString normalized = input.normalized(QString::NormalizationForm_D);
        normalized.remove(QRegularExpression("[̀-̈]"));  // Supprime les accents combinés
        QByteArray utf8 = normalized.toUtf8();
        sqlite3_result_text(context, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
    } else {
        sqlite3_result_null(context);
    }
}

bool registerRemoveAccentsFunction(QSqlDatabase& db) {
    auto handle = db.driver()->handle();
    sqlite3 *db_handle = *static_cast<sqlite3 **>(handle.data());
    if (!db_handle or db_handle==0) {
        qCritical() << "Failed to retreive SQLite handle.";
        return false;
    }

    int result = sqlite3_create_function(db_handle, "remove_accents", 1, SQLITE_UTF8, nullptr, RemoveAccents, nullptr, nullptr);
    if (result != SQLITE_OK) {
        qCritical() << "Failed to register function 'remove_accents'.";
        return false;
    }
    qInfo() << "Init remove_accents ok.";
    return true;
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
