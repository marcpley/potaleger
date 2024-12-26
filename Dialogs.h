#ifndef DIALOGS_H
#define DIALOGS_H
#include <QString>
#include <QMessageBox>

bool OkCancelDialog(QString sMessage);
void MessageDialog(QString sMessage, QMessageBox::Icon Icon);
void MessageDialog(QString sMessage);

#endif // DIALOGS_H
