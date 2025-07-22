#ifndef DIALOGS_H
#define DIALOGS_H

#include "qcontainerfwd.h"
#include "qsqldatabase.h"
#include <QStyle>

void MessageDlg(const QString &titre, const QString &message, const QString &message2 = "", QStyle::StandardPixmap iconType = QStyle::SP_CustomBase, const int MinWidth=350);
QString QueryDialog(const QString &titre, const QString &message, QSqlDatabase db);
bool OkCancelDialog(const QString &titre, const QString &message, QStyle::StandardPixmap iconType = QStyle::SP_CustomBase, const int MinWidth=350);
int RadiobuttonDialog(const QString &titre, const QString &message, const QStringList &options, const int iDef, QStyle::StandardPixmap iconType = QStyle::SP_CustomBase, const int MinWidth=350);
bool YesNoDialog(const QString &titre, const QString &message, QStyle::StandardPixmap iconType = QStyle::SP_CustomBase, const int MinWidth=350);

#endif // DIALOGS_H

