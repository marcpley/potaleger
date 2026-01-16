#ifndef DIALOGS_H
#define DIALOGS_H

#include "qcombobox.h"
#include "qcontainerfwd.h"
#include "qlabel.h"
#include "qprogressbar.h"
#include "qsqldatabase.h"
#include <QStyle>
#include <QFrame>
#include <QMouseEvent>

void MessageDlg(const QString &titre, const QString &message, const QString &message2="", QStyle::StandardPixmap iconType=QStyle::SP_CustomBase, const int MinWidth=350);

const int xAxisGroupNo=0;
const int xAxisGroupSame=1;

const int xAxisGroupFirstWord=2;
const int xAxisGroupFirstChar=3;
const int xAxisGroupFirstChar2=4;
const int xAxisGroupFirstChar3=5;
const int xAxisGroupFirstChar4=6;
const int xAxisGroupFirstChar5=7;
const int xAxisGroupFirstChar6=8;
const int xAxisGroupFirstChar7=9;
const int xAxisGroupFirstChar8=10;
const int xAxisGroupFirstChar9=11;
const int xAxisGroupFirstChar10=12;

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

    struct inputStructure{
        QString varName="";
        QString label="";
        QString type="TEXT";
        int left=0;
        int labelWidth=250;
        int inputWidth=100;
        QString valDef="";
        QString toolTip="";
    };
    struct inputResult{
        QString varName;
        QVariant value;
    };

    QList<inputResult> inputDialog(const QString &titre, const QString &message, QList<inputStructure> inputs,
                                   const bool bNext=false, QStyle::StandardPixmap iconType=QStyle::SP_CustomBase, const int MinWidth=350);
bool OkCancelDialog(const QString &titre, const QString &message, const bool bNext=false, QStyle::StandardPixmap iconType=QStyle::SP_CustomBase, const int MinWidth=350);
int RadiobuttonDialog(const QString &titre, const QString &message, const QStringList &options, const int iDef, const QSet<int> disabledOptions={}, const bool bNext=false, QStyle::StandardPixmap iconType=QStyle::SP_CustomBase, const int MinWidth=350);
QList<inputResult> selectDialog(const QString &titre, const QString &message, QSqlDatabase db, QString varName, QString tableName, QString whereClose,
                                QProgressBar *progressBar, QLabel *lErr,const bool bNext=false, QStyle::StandardPixmap iconType=QStyle::SP_CustomBase, QString toolTip="");
bool YesNoDialog(const QString &titre, const QString &message, QStyle::StandardPixmap iconType=QStyle::SP_CustomBase, const int MinWidth=350);



#endif // DIALOGS_H

