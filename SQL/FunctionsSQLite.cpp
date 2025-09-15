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


// int PotaCollation(void* /*unused*/, int len1, const void* str1, int len2, const void* str2) {
//     QString s1 = QString::fromUtf8(static_cast<const char*>(str1), len1);
//     QString s2 = QString::fromUtf8(static_cast<const char*>(str2), len2);

//     QCollator collator(QLocale::French);

//     return collator.compare(s1, s2);
// }

// bool registerPotaCollation(QSqlDatabase& db) {
//     auto handle = db.driver()->handle();
//     sqlite3 *db_handle = *static_cast<sqlite3 **>(handle.data());
//     if (!db_handle or db_handle==0) {
//         qCritical() << "Failed to retreive SQLite handle.";
//         return false;
//     }

//     // Enregistrement de la collation
//     int result = sqlite3_create_collation(db_handle, "POTACOLLATION", SQLITE_UTF8, nullptr, PotaCollation);
//     if (result != SQLITE_OK) {
//         qCritical() << "Failed to register collation POTACOLLATION.";
//         return false;
//     }
//     qInfo() << "Init collation ok.";
//     return true;
// }

// static void RemoveAccents(sqlite3_context* context, int argc, sqlite3_value** argv) {
//     if (argc == 1 && sqlite3_value_type(argv[0]) == SQLITE_TEXT) {
//         QString input = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_value_text(argv[0]))).toLower();
//         QString normalized = input.normalized(QString::NormalizationForm_D);
//         normalized.remove(QRegularExpression("[̀-̈]"));  // Supprime les accents combinés
//         QByteArray utf8 = normalized.toUtf8();
//         sqlite3_result_text(context, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
//     } else {
//         sqlite3_result_null(context);
//     }
// }

// bool registerRemoveAccentsFunction(QSqlDatabase& db) {
//     auto handle = db.driver()->handle();
//     sqlite3 *db_handle = *static_cast<sqlite3 **>(handle.data());
//     if (!db_handle or db_handle==0) {
//         qCritical() << "Failed to retreive SQLite handle.";
//         return false;
//     }

//     int result = sqlite3_create_function(db_handle, "remove_accents", 1, SQLITE_UTF8, nullptr, RemoveAccents, nullptr, nullptr);
//     if (result != SQLITE_OK) {
//         qCritical() << "Failed to register function 'remove_accents'.";
//         return false;
//     }
//     qInfo() << "Init remove_accents ok.";
//     return true;
// }
