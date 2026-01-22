#ifndef FADASCRIPTENGINE2_H
#define FADASCRIPTENGINE2_H

#include "FdaUtils.h"
#include "qlabel.h"
#include "qplaintextedit.h"
#include "qprogressbar.h"
#include "qsqldatabase.h"
#include "qstyle.h"
#include "script/fadahighlighter.h"
#include "script/fadascriptedit.h"
#include <QObject>

class FadaScriptEngine2 : public QObject
{
    Q_OBJECT
public:
    explicit FadaScriptEngine2(QObject *parent = nullptr);

    QString scriptTitle,scriptError;
    int lastStatementStart=0;
    int lastStatementHintSize=1;
    QProgressBar *feProgressBar;
    QLabel *feLErr;
    QMap<QString,QVariant> vars;

    int runScript(QString script, QSqlDatabase *db, bool testScript, FadaScriptEdit *SQLEdit=nullptr, FadaHighlighter *hl=nullptr);

private:
    enum execResult {er_error,er_success,er_continue,er_break,er_return};
    enum stType { st_single, st_braced, st_multiple, st_error, st_unknown };
    enum brType { br_if, br_while, br_no, br_unknown}; //, br_for
    struct stmt {
        QString statement="";
        stType stmtType=st_unknown;
        brType bracedType=br_unknown;
        bool fromScript=true;
        // bool implicitTerminator=false;
        int originalPos=0;
        int originalSize=0;
    };
    void resetStmt(FadaScriptEngine2::stmt &stmt, bool fromScript=true);

    QList<int> scriptMap;
    QString map(QString script);
    int unMap(int posInNoCommentScript);

    //bool returnScript;
    // QMap<QString,int> gotoLabels;
    // int labelPos=-1;

    QSqlDatabase *db;
    bool transactionOpen=false;

    bool endOfStatement(const QChar c, const int pos, const int scriptSize, const bool inString,
                        const int braceLevel, stmt &curStatement, QString statements);
    void addToCurStatement(QChar c, QString &statement);
    execResult execute(stmt statement);
    execResult executeSimpleStatement(stmt statement);
    execResult executeBracedStatement(stmt statement);
    void setStatementType(stmt &statement);
    QList<stmt> splitStatements(QString statements, int offSet);
    void setStatementBracedType(stmt &statement, bool onlyAtParsingStart);
    execResult executeIfStatement(stmt statement);
    //bool executeForStatement(stmt statement);
    execResult executeWhileStatement(stmt statement);
    QString extractSubExpr(QString expr, QChar startDelimiter, QChar endDelimiter, int &offSet);
    QList<QVariant> extractArgs(QString expr);
    QVariant evalExpr(QString expr, QString &error);
    void setVar(QString varName, QVariant value, int originalPos);
    void resetVars();
    void fadaFunction(QString funcName, QString &expr, QString &error);

    //h define short functions
    QStyle::StandardPixmap standardPixmap(QString sp) {
        // if (sp.toInt()>0 and sp.toInt())
        //     return static_cast<QStyle::StandardPixmap>(sp.toInt());
        // else
        //     return QStyle::SP_CustomBase;
        if (sp=="Critical")
             return QStyle::SP_MessageBoxCritical;
        else if (sp=="Warning")
            return QStyle::SP_MessageBoxWarning;
        else if (sp=="Question")
            return QStyle::SP_MessageBoxQuestion;
        else if (sp=="Information")
            return QStyle::SP_MessageBoxInformation;
        else
            return QStyle::SP_CustomBase;
    }
    // QMetaType::Type metaType(QVariant ty) {
    //     if (ty.canConvert<int>())
    //         return static_cast<QMetaType::Type>(ty.toInt());
    //     else
    //         return QMetaType::QString;
    // }
    QString variantToExpr(QVariant vExpr, bool toString=false) {
        if (vExpr.isNull())
            return "null";
        else if (vExpr.typeId()==QMetaType::QDate)
            return "'"+vExpr.toDate().toString("yyyy-MM-dd")+"'";
        else if (vExpr.typeId()==QMetaType::QString or toString)
            return "'"+vExpr.toString().replace("'","''")+"'";
        else if (vExpr.typeId()==QMetaType::Bool)
            return iif(vExpr.toBool(),"1","0").toString();
        else
            return vExpr.toString();
    }

    int findMatchingDelimiter(QString expr,QChar startDelimiter,QChar endDelimiter) {
        int exprSize=expr.size();
        int pos=0;
        bool inString=false;
        int level=0;
        QChar c;
        while (pos<exprSize) {
            c=expr[pos];

            if (c==startDelimiter and !inString) {
                level++;
            } else if (c==endDelimiter and !inString) {
                level--;
                if (level==0)
                    return pos;
            }

            if (c=="'") inString=!inString; // Start or end of literal string.

            pos++;
        }
        return -1;
    }
    QString extractWord1(QString expr) {
        QString varChars="abcdefghijklmnopqrstuvwxyz_1234567890";
        int pos=0;
        int exprSize=expr.size();
        while (pos<exprSize) {
            if (!varChars.contains(expr[pos],Qt::CaseInsensitive)) break;
            pos++;
        }
        return expr.first(pos);
    }

};

#endif // FADASCRIPTENGINE2_H
