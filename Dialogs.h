#ifndef DIALOGS_H
#define DIALOGS_H

#include "qcombobox.h"
#include "qcontainerfwd.h"
#include "qsqldatabase.h"
#include <QStyle>
#include <QFrame>
#include <QMouseEvent>

void MessageDlg(const QString &titre, const QString &message, const QString &message2="", QStyle::StandardPixmap iconType=QStyle::SP_CustomBase, const int MinWidth=350);

const int xAxisGroupNo=0;
const int xAxisGroupSame=1;

const int xAxisGroupFirstWord=2;
const int xAxisGroupfirstChar=3;
const int xAxisGroupfirstChar2=4;
const int xAxisGroupfirstChar3=5;
const int xAxisGroupfirstChar4=6;
const int xAxisGroupfirstChar5=7;
const int xAxisGroupfirstChar6=8;
const int xAxisGroupfirstChar7=9;
const int xAxisGroupfirstChar8=10;
const int xAxisGroupfirstChar9=11;
const int xAxisGroupfirstChar10=12;

const int xAxisGroupYear=2;
const int xAxisGroupMonth=3;
const int xAxisGroupWeek=4;
const int xAxisGroupDay=5;
const int xAxisGroup1000=2;
const int xAxisGroup100=3;
const int xAxisGroup10=4;
const int xAxisGroup1=5;
const int xAxisGroup1Decimal=6;
const int xAxisGroup2Decimals=7;
const int xAxisGroup3Decimals=8;
const int xAxisGroup4Decimals=9;
const int xAxisGroup5Decimals=10;
const int xAxisGroup6Decimals=11;

const int calcSeriesNotNull=0;
const int calcSeriesDistinct=1;
const int calcSeriesFirst=2;
const int calcSeriesLast=3;
const int calcSeriesAverage=4;
const int calcSeriesMin=5;
const int calcSeriesMax=6;
const int calcSeriesSum=7;

const int typeSeriesLine=0;
const int typeSeriesScatter=1;
const int typeSeriesScatNotNull=2;
const int typeSeriesBar=3;

QStringList GraphDialog(const QString &titre, const QString &message, QStringList columns, QStringList dataTypes);
void setXAxisGroup(QComboBox *cb, QString dataType);
void setYAxis(QComboBox *cb, bool grouping, QStringList columns, QStringList dataTypes, int xIndex, bool nullValue);
void setYAxisCalc(QComboBox *cb, QString dataType);
// void setYAxisType(QComboBox *cb, int axisType);

QString QueryDialog(const QString &titre, const QString &message, QSqlDatabase db);
bool OkCancelDialog(const QString &titre, const QString &message, QStyle::StandardPixmap iconType=QStyle::SP_CustomBase, const int MinWidth=350);
int RadiobuttonDialog(const QString &titre, const QString &message, const QStringList &options, const int iDef, const QSet<int> disabledOptions={}, const bool bNext=false, QStyle::StandardPixmap iconType=QStyle::SP_CustomBase, const int MinWidth=350);
bool YesNoDialog(const QString &titre, const QString &message, QStyle::StandardPixmap iconType=QStyle::SP_CustomBase, const int MinWidth=350);



#endif // DIALOGS_H

