#ifndef FADASCRIPTENGINE_H
#define FADASCRIPTENGINE_H

#include "qplaintextedit.h"
#include "qsqldatabase.h"
#include "qstyle.h"
#include "script/fadahighlighter.h"
#include <QObject>
#include <QtCore>

class FadaScriptEngine : public QObject
{
    Q_OBJECT
public:
    explicit FadaScriptEngine(QObject *parent = nullptr);

    bool exitScript=false;
    QString scriptTitle,scriptError;
    bool runScript(QString script, QSqlDatabase db, bool rollBack, QPlainTextEdit *SQLEdit=nullptr, FadaHighlighter *hl=nullptr);

private:
    // Expression tokenizer and evaluator (supports integers, strings, variables and operators)
    enum TokenType { T_NUMBER, T_STRING, T_IDENT, T_OP, T_LP, T_RP };

    struct Token {
        TokenType type;
        QString text;
    };

    // Helpers
    QWidget *parentWidget() const { return qobject_cast<QWidget*>(parent()); }
    static inline QString trim(const QString &s) { return s.trimmed(); }
    static inline QString lower(const QString &s) { return s.toLower(); }

    QString prepareScript(const QString &originalScript, QVector<int> &origPosMap);
    QVector<QString> splitStatements(const QString &script, QVector<int> &starts);
    bool executeStatements(const QVector<QString> &stmts, const QVector<int> &stmtStarts, int &statementId, QMap<QString,QVariant> &vars, QSqlDatabase &db, int *outErrorPos);
    bool executeFromSource(const QString &source, int baseOffset, QMap<QString,QVariant> &vars, QSqlDatabase &db, const QVector<int> &origStmtStarts, int *outErrorPos = nullptr);
    bool executeSingleStatement(const QString &rawStmt, int rawStmtStart, QMap<QString,QVariant> &vars, QSqlDatabase &db, QString *outAssignedVar = nullptr, int *outErrorPos = nullptr);

    QVariant evalExpression(const QString &expr, const QMap<QString,QVariant> &vars, int baseOffset = 0, int *outErrorPos = nullptr);
    void showMessageBox(const QString &shortText, const QString &text, QStyle::StandardPixmap sp=QStyle::SP_CustomBase);
    bool showOkCancel(const QString &shortText, const QString &text, QStyle::StandardPixmap sp=QStyle::SP_CustomBase);
    bool executeSqlMod(QSqlDatabase &db, const QString &sql, int &affectedRows);
    bool executeSqlSelect(QSqlDatabase &db, const QString &sql, QVariant &value00);
    bool executeSqlSelectAndShow(QSqlDatabase &db, const QString &sql, QMap<QString,QVariant> &vars, const QString &assignVar);
    int executeSystemCommand(const QString &cmd);

    QVector<Token> tokenizeExpr(const QString &expr, int *errorPos = nullptr);
    int opPrecedence(const QString &op);
    bool isRightAssoc(const QString &op) { Q_UNUSED(op); return false; };
    bool toRPN(const QVector<Token> &tokens, QVector<Token> &outRPN);
    QVariant getVariableValue(const QMap<QString,QVariant> &vars, const QString &ident);
    QVariant evaluateRPN(const QVector<Token> &rpn, const QMap<QString,QVariant> &vars);
    int findMatchingBrace(const QString &s, int startPos);

};

#endif // FADASCRIPTENGINE_H
