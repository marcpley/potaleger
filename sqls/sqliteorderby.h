#ifndef SQLITEORDERBY_H
#define SQLITEORDERBY_H

#include "sqls/sqlitestatement.h"
#include "sqls/sqlitesortorder.h"
#include "sqls/sqlitenulls.h"
#include "sqls/sqliteextendedindexedcolumn.h"

class SqliteExpr;

class API_EXPORT SqliteOrderBy : public SqliteStatement, public SqliteExtendedIndexedColumn
{
    public:
        SqliteOrderBy();
        SqliteOrderBy(const SqliteOrderBy& other);
        SqliteOrderBy(SqliteExpr* expr, SqliteSortOrder order, SqliteNulls nulls);
        ~SqliteOrderBy();

        SqliteStatement* clone();
        bool isSimpleColumn() const;
        QString getColumnName() const;
        QString getCollation() const;
        QString getColumnString() const;
        void setColumnName(const QString& name);
        void setCollation(const QString& name);
        void clearCollation();

        SqliteExpr* expr = nullptr;
        SqliteSortOrder order = SqliteSortOrder::null;
        SqliteNulls nulls = SqliteNulls::null;

    protected:
        TokenList rebuildTokensFromContents();
        void evaluatePostParsing();

    private:
        void pullLastCollationAsOuterExpr();
};

typedef QSharedPointer<SqliteOrderBy> SqliteOrderByPtr;

#endif // SQLITEORDERBY_H
