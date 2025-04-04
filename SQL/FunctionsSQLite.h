#ifndef FUNCTIONSSQLITE_H
#define FUNCTIONSSQLITE_H

#include "qobject.h"
#include "qsqldatabase.h"

bool initSQLean(QSqlDatabase *db);
bool registerScalarFunctions(QSqlDatabase *db);
bool registerTableValuedFunctions(QSqlDatabase *db);
QString testCustomFunctions(QSqlDatabase *db);

bool registerPotaCollation(QSqlDatabase& db);
bool registerRemoveAccentsFunction(QSqlDatabase& db);

#endif // FUNCTIONSSQLITE_H
