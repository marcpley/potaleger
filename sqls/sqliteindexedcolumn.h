#ifndef SQLITEINDEXEDCOLUMN_H
#define SQLITEINDEXEDCOLUMN_H

#include "sqls/sqlitestatement.h"
#include "sqls/sqlitesortorder.h"
#include "sqls/sqliteextendedindexedcolumn.h"
#include <QString>

class API_EXPORT SqliteIndexedColumn : public SqliteStatement, public SqliteExtendedIndexedColumn
{
    public:
        SqliteIndexedColumn();
        SqliteIndexedColumn(const SqliteIndexedColumn& other);
        SqliteIndexedColumn(const QString& name, const QString& collate, SqliteSortOrder sortOrder);
        explicit SqliteIndexedColumn(const QString& name);

        SqliteStatement* clone();
        QString getColumnName() const;
        void setColumnName(const QString& name);
        void setCollation(const QString& name);
        QString getCollation() const;
        void clearCollation();

        QString name = QString();
        SqliteSortOrder sortOrder = SqliteSortOrder::null;
        QString collate = QString();

    protected:
        QStringList getColumnsInStatement();
        TokenList getColumnTokensInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteIndexedColumn> SqliteIndexedColumnPtr;

#endif // SQLITEINDEXEDCOLUMN_H
