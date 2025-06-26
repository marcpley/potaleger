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
#include <QFileDialog>

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

namespace {// Highlight dynamique des parenthèses
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

} // namespace

void MainWindow::MessageDialog(const QString &titre, const QString &message, const QString &message2, QStyle::StandardPixmap iconType, const int MinWidth)
{
    QDialog dialog(this);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout = new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel = new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon = QIcon(":/images/potaleger.svg");
        else
            icon = QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QString sMess=message;
    QString sMess2=message2;
    if (isDarkTheme()){
        sMess=StrReplace(sMess,"<a href","<a style=\"color: #7785ff\" href");
        sMess2=StrReplace(sMess2,"<a href","<a style=\"color: #7785ff\" href");
    }
    QLabel *messageLabel = new QLabel(sMess);
    //messageLabel->setWordWrap(true);
    messageLabel->setOpenExternalLinks(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    messageLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    headerLayout->addWidget(messageLabel);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    if (message2!=""){
        QScrollArea *scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        //QHBoxLayout *textLayout = new QHBoxLayout();
        QLabel *messageLabel2 = new QLabel(sMess2);
        messageLabel2->setWordWrap(true);
        messageLabel2->setOpenExternalLinks(true);
        messageLabel2->setTextInteractionFlags(Qt::TextSelectableByMouse);
        messageLabel2->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        SetFontWeight(messageLabel2,QFont::Light);
        SetFontWeight(messageLabel,QFont::DemiBold);
        scrollArea->setWidget(messageLabel2);
        layout->addWidget(scrollArea);
        //textLayout->addWidget(messageLabel2);
        //layout->addLayout(textLayout);
        QSize screenSize = QGuiApplication::primaryScreen()->size();
        int maxHeight = screenSize.height()-200; // Par exemple, la moitié de la hauteur de l'écran
        scrollArea->setMaximumHeight(maxHeight);
    }

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("OK"));
    okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    layout->addLayout(buttonLayout);

    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        dialog.accept();
    });

    int w,h;
    w=fmax(dialog.sizeHint().width(),MinWidth);
    h=fmax(dialog.sizeHint().height(),150);
    dialog.setFixedSize(w,h);//User can't resize the window.
    //dialog.setMinimumWidth(w);


    dialog.exec();
}

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
            qDebug() << SQLEdit->toPlainText();
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
    QObject::connect(SQLEdit, &QPlainTextEdit::cursorPositionChanged, [&]() {
        highlightParentheses(SQLEdit);
    });

    // Highlight initial si texte déjà présent
    highlightParentheses(SQLEdit);

    //Menu contextuel
    SQLEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(SQLEdit, &QPlainTextEdit::customContextMenuRequested, SQLEdit, [SQLEdit, &dialog]() {
        QMenu menu;
        QAction* openAction = menu.addAction(QObject::tr("Ouvrir un fichier SQL"));
        QAction* saveAction = menu.addAction(QObject::tr("Enregistrer dans un fichier SQL"));
        menu.addSeparator();
        // Ajoute le menu par défaut de l'éditeur
        menu.addActions(SQLEdit->createStandardContextMenu()->actions());
        QSettings settings("greli.net", "Potaléger");

        QAction* chosen = menu.exec(QCursor::pos());
        if (chosen == openAction) {
            QString fileName = QFileDialog::getOpenFileName(SQLEdit, QObject::tr("Ouvrir un fichier SQL"),settings.value("SQLdir").toString(),"*.sql");
            if (!fileName.isEmpty()) {
                QFile file(fileName);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QFileInfo fileInfo(fileName);
                    settings.setValue("SQLdir",fileInfo.absoluteFilePath());
                    QTextStream in(&file);
                    SQLEdit->setPlainText(in.readAll());
                    file.close();
                } else {
                    QMessageBox::warning(&dialog, QObject::tr("Erreur"), QObject::tr("Impossible d'ouvrir le fichier"));
                }
            }
        } else if (chosen == saveAction) {
            QString fileName = QFileDialog::getSaveFileName( SQLEdit, QObject::tr("Enregistrer dans un fichier SQL"),settings.value("SQLdir").toString(),"*.sql");
            if (!fileName.isEmpty()) {
                QFile file(fileName);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QFileInfo fileInfo(fileName);
                    settings.setValue("SQLdir",fileInfo.absoluteFilePath());
                    QTextStream out(&file);
                    out << SQLEdit->toPlainText();
                    file.close();
                } else {
                    QMessageBox::warning(&dialog, QObject::tr("Erreur"), QObject::tr("Impossible d'enregistrer le fichier."));
                }
            }
        }
    });

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

bool MainWindow::OkCancelDialog(const QString &titre, const QString &message, QStyle::StandardPixmap iconType, const int MinWidth)
{
    QDialog dialog(this);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout = new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel = new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon = QIcon(":/images/potaleger.svg");
        else
            icon = QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QLabel *messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

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
        result = true;
        dialog.accept();
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        result = false;
        dialog.reject();
    });

    int w,h;
    w=fmax(dialog.sizeHint().width(),MinWidth);
    h=fmax(dialog.sizeHint().height(),150);
    dialog.setFixedSize(w,h);//User can't resize the window.

    dialog.exec();

    return result;
}

int MainWindow::RadiobuttonDialog(const QString &titre, const QString &message, const QStringList &options, const int iDef, QStyle::StandardPixmap iconType, const int MinWidth) {
    QDialog dialog(this);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout = new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel = new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon = QIcon(":/images/potaleger.svg");
        else
            icon = QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QLabel *messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QButtonGroup *buttonGroup = new QButtonGroup(&dialog);
    buttonGroup->setExclusive(true);

    QList<QRadioButton *> radioButtons;
    for (int i = 0; i < options.size(); ++i) {
        QRadioButton *radioButton = new QRadioButton(options[i]);
        buttonGroup->addButton(radioButton, i);
        layout->addWidget(radioButton);
        radioButtons.append(radioButton);
        if (i==iDef)
            radioButton->setChecked(true);
    }

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Annuler"));
    okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
    cancelButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogCancelButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    int result = -1; // Valeur par défaut si annulé
    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        result = buttonGroup->checkedId(); // Récupère l'ID du bouton sélectionné
        dialog.accept();
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        result = -1;
        dialog.reject();
    });

    int w,h;
    w=fmax(dialog.sizeHint().width(),MinWidth);
    h=fmax(dialog.sizeHint().height(),150);
    dialog.setFixedSize(w,h);//User can't resize the window.

    dialog.exec();

    return result;
}

bool MainWindow::YesNoDialog(const QString &titre, const QString &message, QStyle::StandardPixmap iconType, const int MinWidth)
{
    QDialog dialog(this);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout = new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel = new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon = QIcon(":/images/potaleger.svg");
        else
            icon = QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QLabel *messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *yesButton = new QPushButton(tr("Oui"));
    QPushButton *noButton = new QPushButton(tr("Non"));
    yesButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogYesButton));
    noButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogNoButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(yesButton);
    buttonLayout->addWidget(noButton);
    layout->addLayout(buttonLayout);

    int result = false;
    QObject::connect(yesButton, &QPushButton::clicked, [&]() {
        result = true;
        dialog.accept();
    });
    QObject::connect(noButton, &QPushButton::clicked, [&]() {
        result = false;
        dialog.reject();
    });

    int w,h;
    w=fmax(dialog.sizeHint().width(),MinWidth);
    h=fmax(dialog.sizeHint().height(),150);
    dialog.setFixedSize(w,h);//User can't resize the window.

    dialog.exec();

    return result;
}

