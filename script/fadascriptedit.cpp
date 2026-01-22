#include "fadascriptedit.h"
#include "FdaUtils.h"
#include "qlistview.h"
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTextCursor>

FadaScriptEdit::FadaScriptEdit(QWidget *parent,QStringList dbNames,QStringList varNames)
    : QPlainTextEdit(parent) {

    // Autocompletion
    model = new QStringListModel(this);
    completer = new QCompleter(this);
    completer->setModel(model);
    completer->setWidget(this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    QListView *popup = new QListView();
    // popup->setUniformItemSizes(true); // Pour des tailles d'éléments uniformes
    //popup->setEditTriggers(QAbstractItemView::NoEditTriggers); // Désactive l'édition
    // popup->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); // Barre de défilement si nécessaire
    popup->setMinimumWidth(200);
    popup->setMaximumHeight(200);
    completer->setPopup(popup);

    // connect(completer->popup(), QOverload<const QString &>::of(&QAbstractItemView::SelectedClicked),
    //         this, &FadaScriptEdit::insertCompletion);

    completerDbName=dbNames;
    setCompleterKeywords(varNames);
}

void FadaScriptEdit::setCompleterKeywords(QStringList varNames) {
    completerKeywords.clear();

    //SQL
    completerKeywords.append("SELECT ");
    completerKeywords.append("SELECT 🔸 FROM ");
    completerKeywords.append("SELECT 🔸 FROM  WHERE ");
    completerKeywords.append("SELECT 🔸 FROM  WHERE  ORDER BY ");

    completerKeywords.append("INSERT INTO ");
    completerKeywords.append("INSERT INTO 🔸 () VALUES ()");
    completerKeywords.append("INSERT INTO 🔸 () SELECT ");

    completerKeywords.append("UPDATE ");
    completerKeywords.append("UPDATE 🔸 SET ");
    completerKeywords.append("UPDATE 🔸 SET WHERE ");

    completerKeywords.append("DELETE FROM ");
    completerKeywords.append("DELETE FROM 🔸 WHERE ");

    completerKeywords.append("BEGIN TRANSACTION;");
    completerKeywords.append("COMMIT;");
    completerKeywords.append("ROLLBACK;");

    //C-like
    completerKeywords.append("if (🔸) {}");
    completerKeywords.append("if (🔸) {} else {}");
    completerKeywords.append("if (🔸) {} else if () {}");
    completerKeywords.append("if (🔸) {} else if () {} else {}");

    completerKeywords.append("while (🔸) {}");

    completerKeywords.append("return;");
    completerKeywords.append("break;");
    completerKeywords.append("continue;");

    //Fada dialogs.
    completerKeywords.append("inputDialog");
    completerKeywords.append("inputDialog('🔸','Valeur',it_Text)");
    completerKeywords.append("inputDialog('🔸','Valeur',it_Text,'0|300|200','val1|val2',sp_,'600|350||',bs_)");

    completerKeywords.append("inputsDialog");
    completerKeywords.append("inputsDialog('🔸','var1','Valeur1',it_Text,'0|300|200','def1','toolTip1',sp_,'600|350||',bs_);");

    completerKeywords.append("messageDialog");
    completerKeywords.append("messageDialog('🔸');");
    completerKeywords.append("messageDialog('🔸','',sp_);");

    completerKeywords.append("okCancelDialog");
    completerKeywords.append("okCancelDialog('🔸')");
    completerKeywords.append("okCancelDialog('🔸',sp_)");
    completerKeywords.append("okCancelDialog('🔸',sp_None,'600|350||',bs_)");

    completerKeywords.append("radioButtonDialog");
    completerKeywords.append("radioButtonDialog('🔸')");
    completerKeywords.append("radioButtonDialog('🔸','op1|op2',0,'',sp_,'600|350||',bs_)");

    completerKeywords.append("selectDialog");
    completerKeywords.append("selectDialog('🔸','var1','SELECT * FROM ... WHERE ...','toolTip',sp_,'600|350||',bs_);");

    completerKeywords.append("tableDialog");
    completerKeywords.append("tableDialog('🔸','var1','tableName','fieldName LIKE ''A%''','toolTip',sp_,'600|350||',bs_);");

    completerKeywords.append("yesNoDialog");
    completerKeywords.append("yesNoDialog('🔸')");
    completerKeywords.append("yesNoDialog('🔸','sp_)");

    completerKeywords.append("sysCmd");
    completerKeywords.append("sysCmd('🔸')");
    completerKeywords.append("sysOpen");
    completerKeywords.append("sysOpen('🔸')");
    completerKeywords.append("sysRun");
    completerKeywords.append("sysRun('🔸')");


    completerKeywords.append(varNames);
    completerKeywords.append(completerDbName);


}

void FadaScriptEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return or event->key() == Qt::Key_Enter) {
        if (completer->popup()->isVisible()) {
            insertCompletion(completer->popup()->currentIndex().data().toString());
            completer->popup()->hide();
            event->accept();
            return;

        } else if (textCursor().atBlockEnd()) {
            // Insert return + indent as current line.
            int indent = getIndent(textCursor());
            textCursor().insertText("\n" + QString(" ").repeated(indent));
            event->accept();
            return;
        }

    } else if (event->key() == Qt::Key_Tab) {
        // Adjust indent to previous line.
        QTextCursor cursor=textCursor();
        cursor.beginEditBlock();
        if (cursor.hasSelection()) {
            int endSel=cursor.selectionEnd();
            int startSel=cursor.selectionStart();
            cursor.setPosition(startSel);
            cursor.movePosition(QTextCursor::StartOfLine);
            while (cursor.position()<endSel) {
                if (event->modifiers() & Qt::ControlModifier)
                    addineIndent(cursor,-4);
                else
                    addineIndent(cursor,4);
                endSel+=4;
                cursor.movePosition(QTextCursor::StartOfLine);
                if (!cursor.movePosition(QTextCursor::Down)) break;
            }
        } else {
            int indentPrev=getIndent(textCursor(),-1);
            int indentCurr=getIndent(textCursor());
            int addIndent;
            if (event->modifiers() & Qt::ControlModifier) {
                //Remove spaces to have 4 mulitple indent.
                int newIndent=(indentCurr-4)/4*4;
                addIndent=newIndent-indentCurr;
            } else if (indentCurr<indentPrev) {
                //Increase indent to previous line indent.
                addIndent=indentPrev-indentCurr;
            } else {
                //Add spaces to have 4 mulitple indent.
                int newIndent=(indentCurr+4)/4*4;
                addIndent=newIndent-indentCurr;
            }
            addineIndent(cursor,addIndent);
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor,getIndent(cursor));
            setTextCursor(cursor);
        }
        cursor.endEditBlock();
        event->accept();
        return;

    } else if (event->key()==Qt::Key_Slash && (event->modifiers() & Qt::ControlModifier)) {
        QTextCursor cursor=textCursor();
        cursor.beginEditBlock();
        if (cursor.hasSelection()) {
            int endSel=cursor.selectionEnd();
            int startSel=cursor.selectionStart();
            cursor.setPosition(startSel);
            cursor.movePosition(QTextCursor::StartOfLine);
            bool comment=lineCommented(cursor);
            while (cursor.position()<endSel) {
                if (setLineCommented(cursor,!comment))
                    endSel+=2;
                else
                    endSel-=2;
                cursor.movePosition(QTextCursor::StartOfLine);
                if (!cursor.movePosition(QTextCursor::Down)) break;
            }
        } else
            setLineCommented(cursor,!lineCommented(cursor));
        cursor.endEditBlock();
        event->accept();
        return;
    }

    QPlainTextEdit::keyPressEvent(event);

    //Autocompletion.
    QString varChars="abcdefghijklmnopqrstuvwxyz_1234567890";
    if (!event->text().isEmpty() and
        (varChars.contains(event->text().at(0),Qt::CaseInsensitive) or event->key()==Qt::Key_Backspace))   {
        QTextCursor cursor = textCursor();
        //cursor.select(QTextCursor::WordUnderCursor);
        cursor.movePosition(QTextCursor::PreviousWord,QTextCursor::KeepAnchor);
        QString currentWord = cursor.selectedText().trimmed();
        while (!currentWord.isEmpty() and !varChars.contains(currentWord[0],Qt::CaseInsensitive))
            currentWord.removeFirst();

        int startCompletion;
        if (currentWord.startsWith("if",Qt::CaseInsensitive))
            startCompletion=2;
        else
            startCompletion=3;

        if (currentWord.length()>=startCompletion) {
            // Select keywords begining by currentWord
            QStringList matches;
            int maxSize=0;
            for (const QString &keyword : completerKeywords) {
                if (keyword.startsWith(currentWord, Qt::CaseInsensitive)) {
                    matches << keyword;
                    maxSize=std::max(maxSize,int(keyword.size()));
                }
            }

            model->setStringList(matches);

            if (!matches.isEmpty()) {

                QFontMetrics fontMetrics(completer->popup()->font());
                int itemHeight = fontMetrics.height() + 1;
                int maxVisibleItems = 10;
                int popupHeight = qMin(matches.count(), maxVisibleItems) * itemHeight + 4;
                completer->popup()->setFixedHeight(popupHeight);
                completer->popup()->setFixedWidth(maxSize*fontMetrics.averageCharWidth()+20);

                // QPoint globalPos = mapToGlobal(QPlainTextEdit::cursorRect().topLeft());
                // QRect cursorRect = QRect(globalPos,QPlainTextEdit::cursorRect().size());
                completer->setCompletionPrefix(currentWord);
                completer->complete(QPlainTextEdit::cursorRect());
                completer->popup()->setCurrentIndex(completer->model()->index(0,0));
            } else {
                completer->popup()->hide();
            }
        } else {
            completer->popup()->hide();
        }

    //} else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    // } else if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Up ||
    //            event->key() == Qt::Key_Down) {
    } else {
        completer->popup()->hide();
    }

}

bool FadaScriptEdit::lineCommented(QTextCursor cursor) {
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
    QString currentLine=cursor.selectedText();
    cursor.movePosition(QTextCursor::StartOfLine);
    return currentLine.trimmed().startsWith("--");
}

bool FadaScriptEdit::setLineCommented(QTextCursor cursor, bool comment) {
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
    QString currentLine=cursor.selectedText();
    cursor.movePosition(QTextCursor::StartOfLine);
    if (!comment) {
        //Remove comment separator.
        int posSep=currentLine.indexOf("--");
        if (posSep>-1) {
            cursor.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,posSep);
            cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,2);
            cursor.removeSelectedText();
        }
        return false;
    } else {
        //Add comment separator.
        cursor.insertText("--");
        return true;
    }
}

int FadaScriptEdit::getIndent(QTextCursor cursor,int lineOffSet) {
    if (lineOffSet<0)
        cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, -lineOffSet);
    else
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineOffSet);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    QString currentLine = cursor.selectedText();
    int indent = 0;
    while (indent < currentLine.size() and currentLine[indent]==' ')
        indent++;
    return indent;
}

void FadaScriptEdit::addineIndent(QTextCursor cursor, int indent) {
    cursor.movePosition(QTextCursor::StartOfLine);
    if (indent>0) {
        cursor.insertText(QString(" ").repeated(indent));
    } else {
        cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,-indent);
        indent=-indent-cursor.selectedText().trimmed().size();
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,indent);
        cursor.removeSelectedText();
    }
}

void FadaScriptEdit::insertCompletion(QString completion)
{
    // Get word under cursor.
    QTextCursor cursor = textCursor();
    //cursor.select(QTextCursor::WordUnderCursor);
    cursor.movePosition(QTextCursor::PreviousWord,QTextCursor::KeepAnchor);
    QString currentWord = cursor.selectedText();
    qDebug() << "Auto completion : "+currentWord+" -> "+completion;

    // replace with completion
    if (!completion.isEmpty()) {
        cursor.insertText(StrReplace(completion,"🔸",""));

        int anchor=completion.indexOf("🔸");
        if (anchor>-1)
            cursor.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor,completion.size()-anchor-2);
        setTextCursor(cursor);
    }
}

int FadaScriptEdit::removeTrailingSpaces(QTextCursor cursor, int keepIndent) {
    cursor.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
    QString line=cursor.selectedText();
    int i;
    for (i=line.size()-1;i>=0;i--) {
        if (line[i]!=' ') break;
    }
    line=line.mid(0,i+1);
    if (line.isEmpty()) {
        line=QString(" ").repeated(keepIndent);
        cursor.insertText(line);
        return keepIndent;
    } else {
        cursor.insertText(line);
        return line.size()-line.trimmed().size();
    }
}

void FadaScriptEdit::removeTrailingSpaces() {
    QTextCursor cursor=textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);
    int prevIndent=0;
    while (!cursor.atEnd()) {
        prevIndent=removeTrailingSpaces(cursor,prevIndent);
        if (!cursor.movePosition(QTextCursor::Down))
            break;
    }
    cursor.endEditBlock();
    return;
}

QString FadaScriptEdit::getTextAtCursor(int offSet, int length) {
    QTextCursor cursor=textCursor();
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, offSet);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, length);
    return cursor.selectedText();
}

QString FadaScriptEdit::getLineAtCursor(int lineOffSet) {
    QTextCursor cursor=textCursor();
    if (lineOffSet<0)
        cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, -lineOffSet);
    else
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineOffSet);
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    return cursor.selectedText();
}

