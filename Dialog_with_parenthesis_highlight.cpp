#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"
#include "qsqlerror.h"
#include "ui_mainwindow.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QLEInteger>
#include "PotaUtils.h"
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QScreen>
#include <QSettings>
#include <QStyle>
#include <QGuiApplication>

// --- Highlighter, inchangé ---
class SqlHighlighter : public QSyntaxHighlighter {
public:
    SqlHighlighter(QTextDocument *parent = nullptr) : QSyntaxHighlighter(parent) {
        QTextCharFormat keywordFormat, keywordFormat2;
        if (isDarkTheme()) {
            keywordFormat.setForeground(QColor("#0085c4"));
            keywordFormat2.setForeground(Qt::darkYellow);
        } else {
            keywordFormat.setForeground(Qt::blue);
            keywordFormat2.setForeground(Qt::darkYellow);
        }
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywordPatterns = {
            "\\bSELECT\\b", "\\bFROM\\b", "\\bWHERE\\b", "\\bGROUP BY\\b", "\\bORDER BY\\b", "\\bJOIN\\b", "\\bLEFT\\b", "\\bRIGHT\\b", "\\bUSING\\b"
        };
        QStringList keywordPatterns2 = {
            "\\bAND\\b", "\\bOR\\b", "\\bNULL\\b", "\\bISNULL\\b", "\\bNOTNULL\\b", "\\bNOT\\b", "\\bBETWEEN\\b"
        };
        for (const QString &pattern : keywordPatterns)
            rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), keywordFormat});
        for (const QString &pattern2 : keywordPatterns2)
            rules.append({QRegularExpression(pattern2, QRegularExpression::CaseInsensitiveOption), keywordFormat2});
    }
protected:
    void highlightBlock(const QString &text) override {
        for (const auto &rule : rules) {
            auto matchIter = rule.pattern.globalMatch(text);
            while (matchIter.hasNext()) {
                auto match = matchIter.next();
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
};

// --- Ajout : Highlight dynamique des parenthèses ---
namespace {
void highlightParentheses(QPlainTextEdit* SQLEdit) {
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextCursor cursor = SQLEdit->textCursor();
    int pos = cursor.position();

    QString text = SQLEdit->toPlainText();
    if (text.isEmpty())
        return;

    QChar charUnder, charBefore;
    if (pos < text.length())
        charUnder = text.at(pos);
    if (pos > 0)
        charBefore = text.at(pos-1);

    int matchPos = -1;
    QChar open, close;

    // Cherche si curseur sur une parenthèse ouvrante
    if (charUnder == '(' || charUnder == '[' || charUnder == '{') {
        open = charUnder;
        close = (open == '(') ? ')' : (open == '[') ? ']' : '}';
        matchPos = findMatchingParenthesis(text, pos, open, close, true);
        highlightAt(SQLEdit, pos, extraSelections);
        if (matchPos != -1)
            highlightAt(SQLEdit, matchPos, extraSelections);
    }
    // Cherche si curseur juste après une parenthèse fermante
    else if (charBefore == ')' || charBefore == ']' || charBefore == '}') {
        close = charBefore;
        open = (close == ')') ? '(' : (close == ']') ? '[' : '{';
        matchPos = findMatchingParenthesis(text, pos-1, open, close, false);
        highlightAt(SQLEdit, pos-1, extraSelections);
        if (matchPos != -1)
            highlightAt(SQLEdit, matchPos, extraSelections);
    }

    SQLEdit->setExtraSelections(extraSelections);
}

// Recherche la parenthèse associée
int findMatchingParenthesis(const QString& text, int pos, QChar open, QChar close, bool forward) {
    int depth = 0;
    if (forward) {
        for (int i = pos+1; i < text.length(); ++i) {
            if (text[i] == open) depth++;
            else if (text[i] == close) {
                if (depth == 0) return i;
                depth--;
            }
        }
    } else {
        for (int i = pos-1; i >= 0; --i) {
            if (text[i] == close) depth++;
            else if (text[i] == open) {
                if (depth == 0) return i;
                depth--;
            }
        }
    }
    return -1;
}

// Ajoute la sélection visuelle
void highlightAt(QPlainTextEdit* SQLEdit, int position, QList<QTextEdit::ExtraSelection>& extraSelections) {
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor(255, 255, 0, 128)); // Jaune semi-transparent
    QTextCursor selCursor = SQLEdit->textCursor();
    selCursor.setPosition(position);
    selCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = selCursor;
    extraSelections.append(selection);
}
} // namespace

// --- Méthode QueryDialog ---
QString MainWindow::QueryDialog(const QString &titre, const QString &message)
{
    QDialog dialog(this);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout = new QHBoxLayout();

    QLabel *messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QPlainTextEdit *SQLEdit = new QPlainTextEdit(&dialog);
    new SqlHighlighter(SQLEdit->document());
    QFont monospaceFont;
    monospaceFont.setStyleHint(QFont::Monospace);
    monospaceFont.setFamily("Monospace");
    SQLEdit->setFont(monospaceFont);

    QSettings settings("greli.net", "Potaléger");
    SQLEdit->setPlainText(settings.value("SQL").toString());
    layout->addWidget(SQLEdit);

    QSize screenSize = QGuiApplication::primaryScreen()->size();
    int maxHeight = screenSize.height()-200;
    SQLEdit->setMaximumHeight(maxHeight);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Annuler"));
    okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
    cancelButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogCancelButton));
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    int result = false;
    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        PotaQuery pQuery(db);
        if (!SQLEdit->toPlainText().toUpper().startsWith("SELECT ")) {
            messageLabel->setText(tr("La requête doit commencer par %1.").arg("SELECT"));
        } else if (pQuery.exec(SQLEdit->toPlainText())) {
            result = true;
            dialog.accept();
        } else {
            messageLabel->setText(pQuery.lastError().text());
        }
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        result = false;
        dialog.reject();
    });

    // --- Connexion pour le highlight des parenthèses ---
    QObject::connect(SQLEdit, &QPlainTextEdit::cursorPositionChanged, [&]() {
        highlightParentheses(SQLEdit);
    });

    // Highlight initial si texte déjà présent
    highlightParentheses(SQLEdit);

    settings.beginGroup(titre);
    const auto geometry = settings.value("geometry").toByteArray();
    if (geometry.isEmpty())
        dialog.setGeometry(50, 50, 600, 400);
    else
        dialog.restoreGeometry(geometry);
    settings.endGroup();

    dialog.exec();

    settings.beginGroup(titre);
    settings.setValue("geometry", dialog.saveGeometry());
    settings.endGroup();

    if (result) {
        settings.setValue("SQL", SQLEdit->toPlainText());
        return SQLEdit->toPlainText();
    } else {
        return "";
    }
}