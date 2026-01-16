#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "qcontainerfwd.h"
#include "qlabel.h"
#include "qprogressbar.h"
#include "qsqldatabase.h"
#include <QStyle>
#include <QFrame>
#include <QMouseEvent>

QString scriptEditor(const QString &titre, const QString &message, const QString &script, QSqlDatabase db, QProgressBar *progressBar, QLabel *lErr);


#endif // SCRIPTEDITOR_H

