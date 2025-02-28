#ifndef POTAUTILS_H
#define POTAUTILS_H

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
    bool ExecMultiShowErr(const QString querys, const QString spliter, QProgressBar *progressBar, bool keepReturns=false);
    QVariant Selec0ShowErr(QString query);

    QSqlDatabase& database() { return m_db; }

private:
    QSqlDatabase& m_db;
};

QString DataType(QSqlDatabase *db, QString TableName, QString FieldName);
//QString SQLiteDate();
//QString DBInfo(QSqlDatabase *db);
QString EscapeCSV(QString line);
QVariant iif(bool bCond,QVariant Var1,QVariant Var2);
bool isDarkTheme();
float min(float a,float b);
int min(int a,int b);
void parseCSV(QString entry, QString sep, QStringList &list);
QString PrimaryKeyFieldName(QSqlDatabase *db, QString TableName);
QString RemoveComment(QString sCde, QString sCommentMarker, bool keepReturns=false);
void SetButtonSize(QToolButton *b);
void SetFontColor(QWidget* widget, QColor color);
void SetFontWeight(QWidget* widget, QFont::Weight weight);
void SetColoredText(QLabel *l, QString text, QString type);
QString str(int i);
QString str(float i);
QString str(qsizetype i);
QString StrFirst(QString s, int i);
QString StrLast(QString s, int i);
QString StrReplace(QString s, const QString sTarg, const QString sRepl);
QString SubString(QString s, int iDeb, int iFin);
//bool dbSuspend(QSqlDatabase *db, bool bSuspend, bool bEditing, QLabel *ldbs);

#endif // POTAUTILS_H
