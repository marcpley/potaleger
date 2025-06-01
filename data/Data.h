#ifndef DATA_H
#define DATA_H

#include "potawidget.h"
#include "qabstractitemmodel.h"
#include "qcolor.h"

const QColor cBase=QColor("#a17dc2");//Violet gris
const QColor cEspece=QColor("#002bff");//Bleu
const QColor cFamille=QColor("#0085c4");//Bleu gris
const QColor cFertilisant=QColor("#00A67A");//Bleu vert
const QColor cITP=QColor("#ff0000");//Rouge
const QColor cPlanche=QColor("#ff8100");//Orange
const QColor cRotation=QColor("#ce9462");//Orange gris
const QColor cVariete=QColor("#b7b202");//Jaune
const QColor cParam=QColor("#7f7f7f");//Gris

const QColor cCulture=QColor("#00ff00");//Vert
const QColor cPrevue=QColor();
const QColor cPepiniere=QColor("#ff6000");//Rouge
const QColor cEnPlace=QColor("#76c801");//Vert
const QColor cRecolte=QColor("#bf00ff");//Violet
const QColor cATerminer=QColor("#007aff");//Bleu
const QColor cTerminee=QColor("#808080");//Gris

bool AcceptReturns(const QString sFieldName);
QString ComboField(const QString sTableName, const QString sFieldName);
int DefColWidth(QSqlDatabase *db, const QString sTableName, const QString sFieldName);
QString DynDDL(QString sQuery);
bool FieldIsMoney(const QString sFieldName);
QString FkFilter(QSqlDatabase *db, const QString sTableName, const QString sFieldName, const QModelIndex &index, bool countFk=false);
QString GeneratedFielnameForDummyFilter(const QString sTableName);
int NaturalSortCol(const QString sTableName);
QString NoData(const QString sTableName);
bool ReadOnly(QSqlDatabase *db, const QString sTableName,const QString sFieldName);
QColor RowColor(QString sValue, QString sTableName);
QString RowSummary(QString TableName, const QSqlRecord &rec);
QColor TableColor(QString sTName,QString sFName);
QPixmap TablePixmap(QString sTName, QString text);
QString ToolTipField(const QString sTableName, const QString sFieldName, const QString sDataType);
QString ToolTipTable(const QString sTableName);
bool ViewFieldIsDate(const QString sFieldName);

#endif // DATA_H
