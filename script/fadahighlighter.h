#ifndef FADAHIGHLIGHTER_H
#define FADAHIGHLIGHTER_H

#include "qplaintextedit.h"
#include "qregularexpression.h"
//#include "qsqldatabase.h"
#include <QSyntaxHighlighter>

class FadaHighlighter : public QSyntaxHighlighter
{
public:
    explicit FadaHighlighter(QTextDocument *parent, QStringList dbNames={}, QStringList varNames={});
    QStringList dbNamePaterns,varNamePaterns;
    void setRules(QStringList varNames);
    //void setVarRules(QStringList varNames);
    void highlightParentheses(QPlainTextEdit* SQLEdit);
    //QSqlDatabase *db;
protected:
    void highlightBlock(const QString &text) override {
        for (const auto &rule : rules) {
            auto matchIter=rule.pattern.globalMatch(text);
            while (matchIter.hasNext()) {
                auto match=matchIter.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }
private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightRule> rules;
    int findMatchingParenthesis(const QString& text, int pos, QChar open, QChar close, bool forward);
    void highlightAt(QPlainTextEdit* SQLEdit, int position, QList<QTextEdit::ExtraSelection>& extraSelections);
};

#endif // FADAHIGHLIGHTER_H
