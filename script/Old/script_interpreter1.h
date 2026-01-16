#pragma once

#include <QtCore>
#include <QtSql>
#include <QtWidgets>

/*
  Simple script interpreter embedded in a Qt application.

  Function:
    bool runScript(const QString &script, QWidget *parent = nullptr, QSqlDatabase db = QSqlDatabase::database());

  - The interpreter is intentionally small and pragmatic (not a full language).
  - Syntax inspirations:
      * statements end with ';'
      * comments: from -- to end of line
      * assignments:  myVar = 1;
      * expressions: numbers, strings in single quotes, variables, parentheses,
                     operators: ||  == != <= >= < >  + - * / 
      * if / else if / else / endif blocks:
          if(condition)
            stmt;
          else if(cond2)
            stmt;
          else
            stmt;
          endif;
      * while / endwhile:
          while(condition)
             ...
          endwhile;
      * for / endfor (C-style): for(init; cond; after) ... endfor;
      * SQL:
          - standalone SELECT/UPDATE/INSERT/DELETE can be written as SQL statements terminated by ';'
          - You can capture the affected-rows of an UPDATE/INSERT/DELETE by writing:
              result = (UPDATE ...);
            i.e. assignment to parentheses means "assign the number of affected rows of the upcoming SQL statement"
          - You can capture a selected row from a SELECT by preceding it with:
              rowValues = (SELECT ...);
            and then next SELECT ...; will be shown in a modal QTableView; the row chosen by the user will be stored as a QVariantMap in the variable.
      * messageBox(title, text);
      * okCancelBox(title, text) -> returns true/false assigned to a variable
      * system('command');  -> execute shell command (blocking)
  - The interpreter uses the Qt default database connection unless a QSqlDatabase is passed.
  - The interpreter shows dialogs as modal using the provided parent widget.
*/

bool runScript(const QString &script, QWidget *parent = nullptr, QSqlDatabase db = QSqlDatabase::database());
