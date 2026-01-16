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
                   QPlainTextEdit *SQLEdit = nullptr);

  - Returns true on success.
  - On parsing/runtime error, returns false and (if SQLEdit provided) selects in the editor
    the token region that caused the error.
  - Control structures use C-style braces:
        if (cond) { ... } else if (cond2) { ... } else { ... }
        while (cond) { ... }
        for (init; cond; after) { ... }
*/

bool runScript(const QString &script,
               QWidget *parent = nullptr,
               QSqlDatabase db = QSqlDatabase::database(),
               QPlainTextEdit *SQLEdit = nullptr);
