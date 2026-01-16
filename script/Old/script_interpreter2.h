#pragma once

#include <QtCore>
#include <QtSql>
#include <QtWidgets>

/*
  Simple script interpreter embedded in a Qt application.

  Function:
    bool runScript(const QString &script,
                   QWidget *parent = nullptr,
                   QSqlDatabase db = QSqlDatabase::database(),
                   int *errorLine = nullptr,
                   int *errorColumn = nullptr);

  - Returns true on success.
  - On parsing/runtime error, returns false and (if provided) sets errorLine/errorColumn (1-based)
    pointing to the location in the input script where the error occurred.
*/

bool runScript(const QString &script,
               QWidget *parent = nullptr,
               QSqlDatabase db = QSqlDatabase::database(),
               int *errorLine = nullptr,
               int *errorColumn = nullptr);
