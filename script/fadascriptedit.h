#ifndef FADASCRIPTEDIT_H
#define FADASCRIPTEDIT_H

#include "qcombobox.h"
#include <QPlainTextEdit>
#include <QCompleter>
#include <QStringListModel>

class FadaScriptEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit FadaScriptEdit(QWidget *parent = nullptr, QStringList dbNames={}, QStringList varNames={});

    void setCompleterKeywords(QStringList varNames);
    void removeTraillingSpaces();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    int getIndent(QTextCursor cursor,int lineOffSet=0);
    QString getTextAtCursor(int offSet=0, int length=1);
    QString getLineAtCursor(int lineOffSet=0);
    bool setLineCommented(QTextCursor cursor, bool comment);
    bool lineCommented(QTextCursor cursor);
    void addineIndent(QTextCursor cursor, int indent);
    int removeTraillingSpaces(QTextCursor cursor, int keepIndent);


private slots:
    void insertCompletion(QString completion);

private:
    QCompleter *completer;
    QStringListModel *model;
    QStringList completerKeywords;
    QStringList completerDbName;

};

#endif // FADASCRIPTEDIT_H
