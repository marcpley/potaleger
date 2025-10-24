#ifndef POTAUTILS_H
#define POTAUTILS_H

#include "qdir.h"
#include "qlabel.h"
#include "qprogressbar.h"
#include "qtoolbutton.h"
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QPainter>

class PotaQuery: public QSqlQuery {

public:
    explicit PotaQuery(QSqlDatabase& db) : QSqlQuery(db), m_db(db) {} //demander à ChatGT ou virer ce constructeur compliqué et ajouter une variable *db, et l'affecter à chaque instance

    //QSqlDatabase *db;
    QLabel *lErr=nullptr;
    bool ExecShowErr(QString query);
    bool ExecMultiShowErr(const QString querys, const QString spliter, QProgressBar *progressBar, bool stopIfError=true); //, bool keepReturns=false
    QVariant Selec0ShowErr(QString query);

    QSqlDatabase& database() { return m_db; }

private:
    QSqlDatabase& m_db;
};

void AppBusy(bool busy, QProgressBar *pb=nullptr, int max=0, QString text="%p%");
QColor blendColors(const QColor& baseColor, const QColor& overlayColor);
QString DataType(QSqlDatabase *db, QString TableName, QString FieldName);
//QString SQLiteDate();
//QString DBInfo(QSqlDatabase *db);
QString EscapeCSV(QString s,QString sep);
QString EscapeSQL(QString s);
QDate firstDayOffWeek(int year, int week);
QDate firstDayOffWeek(QDate date);
QVariant iif(bool bCond,QVariant Var1,QVariant Var2);
bool isDarkTheme();
void logMessage(const QString fileName, const QString message);
float min(float a,float b);
int min(int a,int b);
void parseCSV(QString entry, QString sep, QStringList &list);
//QString PrimaryKeyFieldName(QSqlDatabase *db, QString TableName);
QString RemoveAccents(QString input);
QString RemoveComment(QString sCde, QString sCommentMarker, bool keepReturns=false);
void SetFontColor(QWidget* widget, QColor color);
void SetFontWeight(QWidget* widget, QFont::Weight weight);
void SetColoredText(QLabel *l, QString text, QString type);
QString str(int i);
QString str(float i);
QString str(qsizetype i);
QString StrElipsis(QString s, int i);
QString StrFirst(QString s, int i);
QString StrLast(QString s, int i);
QString StrRemoveLasts(QString s, int i);
QString StrReplace(QString s, const QString sTarg, const QString sRepl);
QString SubString(QString s, int iDeb, int iFin);
//bool dbSuspend(QSqlDatabase *db, bool bSuspend, bool bEditing, QLabel *ldbs);

#endif // POTAUTILS_H
