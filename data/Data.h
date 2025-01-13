#ifndef DATA_H
#define DATA_H

#include "potawidget.h"
#include "qabstractitemmodel.h"
#include "qcolor.h"

const QColor cPrevue=QColor();
const QColor cSousAbris=QColor("#ff6000");//Rouge
const QColor cEnPlace=QColor("#76c801");//Vert
const QColor cATerminer=QColor("#007aff");//Bleu
const QColor cTerminee=QColor("#808080");//Gris

QString DynDDL(QString sQuery);
QString GeneratedFielnameForDummyFilter(const QString sTableName);
bool ReadOnly(const QString sTableName,const QString sFieldName);
QColor RowColor(QString sValue);
QString RowSummary(QString TableName, const QModelIndex &index);
QColor TableColor(QString sTName,QString sFName);
QString ToolTip(const QString sTableName,const QString sFieldName);

#endif // DATA_H
