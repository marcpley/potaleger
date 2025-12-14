#ifndef FDASQLFORMAT_H
#define FDASQLFORMAT_H

#include "qobject.h"
#include <QSet>

class fdaSqlFormat : public QObject
{
public:
    explicit fdaSqlFormat(QObject *parent = nullptr){
        setParent(parent);
    };

    QList<QList<QVariant>> subSQLdata;
    QString formatSql(const QString sql);
private:
    QString formatSqlInit(const QString sql);
    QString formatSqlExplore(const QString sql);
    QString formatSqlFormatAll(const QString sql);
    QString formatSqlFormat(const int iSubSql, QString sql1="");
    int originalLength(const QString s);
    QString splitOperands(const QString s, const int indent);
    void setParentIdentOnChilds(const QString s, const int parentIndent);
    int posJoinF(const QString s);
    QString extractSubContent(const QString s, int &pos);
};

#endif // FDASQLFORMAT_H
