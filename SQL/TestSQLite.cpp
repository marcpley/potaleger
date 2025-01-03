#include "qsqldriver.h"
#include <sqlite3.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>

// Fonction SQLite personnalisée
static void testFunction(sqlite3_context* context, int argc, sqlite3_value** argv) {
    sqlite3_result_text(context, "Test Function Called", -1, SQLITE_TRANSIENT);
}

// Enregistrement de la fonction
void registerTestFunction(QSqlDatabase& db) {
    auto handle = db.driver()->handle();
    if (handle.isNull()) {
        qWarning() << "Failed to retrieve SQLite handle.";
        return;
    }

    sqlite3* sqliteHandle = static_cast<sqlite3*>(handle.data());
    int rc = sqlite3_create_function(
        sqliteHandle,
        "TestFunction",
        0,
        SQLITE_UTF8,
        nullptr,
        &testFunction,
        nullptr,
        nullptr
        );

    if (rc != SQLITE_OK) {
        qWarning() << "Failed to register function:" << sqlite3_errmsg(sqliteHandle);
    }
}
