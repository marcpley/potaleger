#pragma once

#include "script/fadahighlighter.h"
#include <QtCore>
#include <QtSql>
#include <QtWidgets>

bool runScript(const QString &script, bool rollBack,
               QWidget *parent = nullptr,
               QSqlDatabase db = QSqlDatabase::database(),
               QPlainTextEdit *SQLEdit = nullptr, FadaHighlighter *hl = nullptr);
