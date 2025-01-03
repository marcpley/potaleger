#ifndef DATA_H
#define DATA_H

QString DynDDL(QString sQuery);
bool ReadOnly(const QString sTableName,const QString sFieldName);
QColor TableColor(QString sTName,QString sFName);
QString ToolTip(const QString sTableName,const QString sFieldName);

#endif // DATA_H
