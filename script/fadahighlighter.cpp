#include "fadahighlighter.h"
#include "FdaUtils.h"

FadaHighlighter::FadaHighlighter(QTextDocument *parent,QStringList dbNames,QStringList varNames)
    : QSyntaxHighlighter{parent}
{
    for (int i=0;i<dbNames.count();i++)
        dbNamePaterns.append("\\b"+dbNames[i]+"\\b");

    setRules(varNames);
}

void FadaHighlighter::highlightAt(QPlainTextEdit* SQLEdit, int position, QList<QTextEdit::ExtraSelection>& extraSelections) {
    QTextEdit::ExtraSelection selection;
    if (isDarkTheme())
        selection.format.setBackground(QColor("#1D545C"));
    else
        selection.format.setBackground(QColor("#6EBBC6"));
    QTextCursor selCursor=SQLEdit->textCursor();
    selCursor.setPosition(position);
    selCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor=selCursor;
    extraSelections.append(selection);
}

void FadaHighlighter::setRules(QStringList varNames) {
    QTextCharFormat SqlFormat,SqlModFormat,SqlDefFormat,CFormat,FadaFormat,CommentFormat,NumFormat,StringFormat,TableAndFieldFormat,varFormat;//,OperatorsFormat
    //Color
    if (isDarkTheme()) { //Prefered
        SqlFormat.setForeground(QColor("#B19DF4"));
        SqlModFormat.setForeground(QColor("#F49DEB"));
        SqlDefFormat.setForeground(QColor("#E61818"));
        CFormat.setForeground(QColor("#25BDD0"));
        //OperatorsFormat.setForeground(QColor("#25BDD0"));
        FadaFormat.setForeground(QColor("#F07979"));
        CommentFormat.setForeground(QColor("#9DA0A5"));
        NumFormat.setForeground(QColor("#CE9043"));
        StringFormat.setForeground(QColor("#CE9043"));
        TableAndFieldFormat.setForeground(QColor("#7ECC40"));
        varFormat.setForeground(QColor("#E0D36B"));
    } else {
        SqlFormat.setForeground(QColor("#5F0A0A"));
        SqlModFormat.setForeground(QColor("#593956"));
        SqlDefFormat.setForeground(QColor("#4C3D61"));
        CFormat.setForeground(QColor("#0B393F"));
        //OperatorsFormat.setForeground(QColor("#0B393F"));
        FadaFormat.setForeground(QColor("#663232"));
        CommentFormat.setForeground(QColor("#505254"));
        NumFormat.setForeground(QColor("#51381A"));
        StringFormat.setForeground(QColor("#51381A"));
        TableAndFieldFormat.setForeground(QColor("#3A5E1E"));
        varFormat.setForeground(QColor("#5C5313"));
    }

    //Bold
    SqlFormat.setFontWeight(QFont::Bold);
    SqlModFormat.setFontWeight(QFont::Bold);
    SqlDefFormat.setFontWeight(QFont::Bold);

    //Italic
    SqlFormat.setFontItalic(true);
    SqlModFormat.setFontItalic(true);
    SqlDefFormat.setFontItalic(true);
    CFormat.setFontItalic(true);
    //OperatorsFormat.setFontItalic(true);
    CommentFormat.setFontItalic(true);

    //Underline
    //TableFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    QStringList SqlPatterns={
        "\\bSELECT\\b", "\\bFROM\\b", "\\bWHERE\\b", "\\bGROUP\\s+BY\\b", "\\bORDER\\s+BY\\b", "\\bJOIN\\b", "\\bLEFT\\b", "\\bRIGHT\\b", "\\bUSING\\b"
    };
    QStringList SqlModPatterns={
        "\\bUPDATE\\b", "\\bINSERT\\s+INTO\\b", "\\bDELETE\\s+FROM\\b", "\\bSET\\b", "\\bBEGIN TRANSACTION\\b", "\\bNEW\\b", "\\bOLD\\b", "\\bVALUES\\b",
        "\\bCOMMIT\\b", "\\bROLLBACK\\b"
    };
    QStringList SqlDefPatterns={
        "\\bCREATE\\b", "\\bTABLE\\b", "\\bVIEW\\b", "\\bTRIGGER\\b", "\\bDROP\\b", "\\bAFTER\\b", "\\bBEFORE\\b", "\\bALTER\\b",
        "\\bINSTEAD\\s+OF\\b", "\\bINSERT\\s+ON\\b", "\\bUPDATE\\s+ON\\b", "\\bDELETE\\s+ON\\b"
    };
    QStringList CPatterns={
        "\\bIF\\b", "\\bELSE\\b", "\\bWHILE\\b", "\\bRETURN\\b", "\\bBREAK\\b", "\\bCONTINUE\\b" //, "\\bFOR\\b"
    };
    // QStringList OperatorPatterns={
    //     "\\bAND\\b", "\\bOR\\b", "\\bNULL\\b", "\\bISNULL\\b", "\\bNOTNULL\\b", "\\bNOT\\b", "\\bBETWEEN\\b", "\\|\\|"
    // };
    QStringList FadaPatterns={
        "\\bINPUTDIALOG\\b", "\\bINPUTSDIALOG\\b", "\\bMESSAGEDIALOG\\b", "\\bOKCANCELDIALOG\\b", "\\bRADIOBUTTONDIALOG\\b",
        "\\bSELECTDIALOG\\b", "\\bTABLEDIALOG\\b", "\\bYESNODIALOG\\b",
        "\\bSYSCMD\\b", "\\bSYSOPEN\\b", "\\bSYSRUN\\b"
    };
    QStringList CommentPatterns={
        "--[^\n]*"
    };
    QStringList NumPatterns={
        "\\b\\d+\\.?\\d*\\b"
    };
    QStringList StringPatterns={
        "'([^']*)'"
    };

    varNamePaterns.clear();
    for (int i=0;i<varNames.count();i++)
        varNamePaterns.append("\\b"+varNames[i]+"\\b");

    rules.clear();

    for (const QString &pattern : NumPatterns)
        rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), NumFormat});

    for (const QString &pattern : SqlPatterns)
        rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), SqlFormat});

    for (const QString &pattern : dbNamePaterns)
        rules.append({QRegularExpression(pattern), TableAndFieldFormat});

    for (const QString &pattern : SqlModPatterns)
        rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), SqlModFormat});

    for (const QString &pattern : SqlDefPatterns)
        rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), SqlDefFormat});

    for (const QString &pattern : CPatterns)
        rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), CFormat});

    // for (const QString &pattern : OperatorPatterns)
    //     rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), OperatorsFormat});

    for (const QString &pattern : FadaPatterns)
        rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), FadaFormat});

    for (const QString &pattern : varNamePaterns)
        rules.append({QRegularExpression(pattern), varFormat});

    for (const QString &pattern : StringPatterns)
        rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), StringFormat});

    for (const QString &pattern : CommentPatterns)
        rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), CommentFormat});
}

//void FadaHighlighter::setVarRules(QStringList varNames) {
    // QTextCharFormat varFormat;
    // //Color
    // if (isDarkTheme()) {
    //     varFormat.setForeground(QColor("#E0D36B"));
    // } else {
    //     varFormat.setForeground(QColor("#5C5313"));
    // }

    //Bold
    //varFormat.setFontWeight(QFont::Bold);

    // varNamePaterns.clear();
    // for (int i=0;i<varNames.count();i++)
    //     varNamePaterns.append("\\b"+varNames[i]+"\\b");

    // //Delete existing var rules
    // for (int i=0;i<rules.count();i++) {
    //     if (rules[i].format==varFormat and
    //         varNamePaterns.contains(rules[i].pattern)) {
    //         rules.remove(i);
    //         i--;
    //     }
    // }

    // for (const QString &pattern : varNamePaterns)
    //     rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), varFormat});
//}

void FadaHighlighter::highlightParentheses(QPlainTextEdit* SQLEdit) {
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextCursor cursor=SQLEdit->textCursor();
    int pos=cursor.position();

    QString text=SQLEdit->toPlainText();
    if (text.isEmpty())
        return;

    QChar charUnder, charBefore;
    if (pos < text.length())
        charUnder=text.at(pos);
    if (pos > 0)
        charBefore=text.at(pos-1);

    int matchPos=-1;
    QChar open, close;

    // Cherche si curseur sur une parenthèse ouvrante
    if (charUnder=='(' || charUnder=='[' || charUnder=='{') {
        open=charUnder;
        close=(open=='(') ? ')' : (open=='[') ? ']' : '}';
        matchPos=findMatchingParenthesis(text, pos, open, close, true);
        highlightAt(SQLEdit, pos, extraSelections);
        if (matchPos != -1)
            highlightAt(SQLEdit, matchPos, extraSelections);
    }
    // Cherche si curseur juste après une parenthèse fermante
    else if (charBefore==')' || charBefore==']' || charBefore=='}') {
        close=charBefore;
        open=(close==')') ? '(' : (close==']') ? '[' : '{';
        matchPos=findMatchingParenthesis(text, pos-1, open, close, false);
        highlightAt(SQLEdit, pos-1, extraSelections);
        if (matchPos != -1)
            highlightAt(SQLEdit, matchPos, extraSelections);
    }

    SQLEdit->setExtraSelections(extraSelections);
}

int FadaHighlighter::findMatchingParenthesis(const QString& text, int pos, QChar open, QChar close, bool forward) {
    int depth=0;
    if (forward) {
        for (int i=pos+1; i < text.length(); ++i) {
            if (text[i]==open) depth++;
            else if (text[i]==close) {
                if (depth==0) return i;
                depth--;
            }
        }
    } else {
        for (int i=pos-1; i >= 0; --i) {
            if (text[i]==close) depth++;
            else if (text[i]==open) {
                if (depth==0) return i;
                depth--;
            }
        }
    }
    return -1;
}
