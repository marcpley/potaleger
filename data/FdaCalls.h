#ifndef FDACALLS_H
#define FDACALLS_H

#include "FdaWidget.h"
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

QString FdaBaseData(QSqlDatabase *db, const QString sTableName,const QString sFieldName);
bool FdaMultiline(QSqlDatabase *db, const QString sTableName, const QString sFieldName);
QString FdaCombo(QSqlDatabase *db, const QString sTableName, const QString sFieldName);
int FdaColWidth(QSqlDatabase *db, const QString sTableName, const QString sFieldName);
QString FdaCondFormats(QSqlDatabase *db, const QString sTableName,const QString sFieldName);
QString FdaDraw(QSqlDatabase *db, const QString sTableName,const QString sFieldName);
QString FdaDynHeader(QSqlDatabase *db, const QString sTableName,const QString sFieldName);
QString DynDDL(QString sQuery);
bool FdaMoney(QSqlDatabase *db, const QString sTableName, const QString sFieldName);
bool FdaCanOpenTab(QSqlDatabase *db, const QString sTableName);
QString FdaFkFilter(QSqlDatabase *db, const QString sTableName, const QString sFieldName, const QModelIndex &index);
QString FdaFkSortField(QSqlDatabase *db, const QString sTableName, const QString sFieldName);
//QString GeneratedFielnameForDummyFilter(const QString sTableName);
bool FdaHidden(QSqlDatabase *db, QString sTableName, QString sFieldName);
bool FdaGotoLast(QSqlDatabase *db, const QString sTableName);
QString FdaNaturalSortFields(QSqlDatabase *db, const QString sTableName);
QString FdaNoDataText(QSqlDatabase *db, const QString sTableName);
bool FdaReadonly(QSqlDatabase *db, const QString sTableName,const QString sFieldName="");
//QColor RowColor(QString sValue, QString sTableName);
QString FdaRowSummary(QSqlDatabase *db, const QString sTableName, const QString rowSummaryModel, const QSqlTableModel *model, const int row);
QString FdaRowSummaryModel(QSqlDatabase *db, QString sTableName);
QColor FdaColor(QSqlDatabase *db, QString sTableName, QString sFieldName);
QPixmap FdaMenuPixmap(QSqlDatabase *db, QString sTableName, QString text="");
QString FdaToolTip(QSqlDatabase *db, const QString sTableName, const QString sFieldName="", const QString sDataType="", const QString sBaseData="");
QString FdaUnit(QSqlDatabase *db, const QString sTableName, const QString sFieldName, const bool bSpaceBefore=false);
//bool ViewFieldIsDate(const QString sFieldName, QString sData="");

#endif // FDACALLS_H
