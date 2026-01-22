#include <QApplication>
#include <QMessageBox>
#include "FdaSqlFormat.h"
#include "qmenu.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QLEInteger>
#include "FdaUtils.h"
#include <QPlainTextEdit>
#include <QTextCharFormat>
#include <QScreen>
#include <QSettings>
#include <QStyle>
#include <QGuiApplication>
#include <QFileDialog>
#include <QColorDialog>
#include <QToolTip>
#include "script/fadahighlighter.h"
#include "script/fadascriptedit.h"
#include "script/fadascriptengine2.h"

QString selectionInfo(QPlainTextEdit* SQLEdit) {
    QString result="";
    result="Caracters "+str(SQLEdit->toPlainText().length())+" - "+
           "Lines "+str(SQLEdit->blockCount())+" - "+
           "Cursor "+str(SQLEdit->textCursor().blockNumber()+1)+","+str(SQLEdit->textCursor().positionInBlock())+" - "+
           "Selection "+str(SQLEdit->textCursor().selectedText().length());
    return result;
}

QString scriptEditor(const QString &title, const QString &message, const QString &script, QSqlDatabase db, QProgressBar *progressBar, QLabel *lErr)
{
    QDialog dialog(QApplication::activeWindow());
    dialog.setWindowTitle(title);

    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout=new QHBoxLayout();

    QString topMessage;
    if (message.isEmpty())
        topMessage=QObject::tr("%1 automatique à la fin du test.").arg("ROLLBACK");
    else
        topMessage=message;
    QLabel *messageLabel=new QLabel(topMessage);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    PotaQuery query(db);
    QList<QVariant> TableAndFieldNames=query.SelectCol0("SELECT DISTINCT tv_name FROM fada_t_schema "
                                                        "UNION "
                                                        "SELECT DISTINCT field_name FROM fada_f_schema ");
    QStringList dbNames;
    for (int i=0;i<TableAndFieldNames.count();i++)
        dbNames.append(TableAndFieldNames[i].toString());

    //Script engine
    FadaScriptEngine2 *se=new FadaScriptEngine2(QApplication::activeWindow());
    se->scriptTitle=title;
    se->feProgressBar=progressBar;
    se->feLErr=lErr;

    //SQL editor
    FadaScriptEdit *SQLEdit=new FadaScriptEdit(&dialog,dbNames,se->vars.keys());
    QPalette p=SQLEdit->palette();
    if (isDarkTheme())
        p.setColor(QPalette::Base, QColor("#1F1F1F"));
    else
        p.setColor(QPalette::Base, QColor("#E2E2E2"));
    SQLEdit->setPalette(p);
    QFont font;
    //font.setStyleHint(QFont::Monospace);
    font.setFamily("Monospace");
    SQLEdit->setFont(font);
    SQLEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    SQLEdit->setPlainText(script);
    layout->addWidget(SQLEdit);
    QSize screenSize=QGuiApplication::primaryScreen()->size();
    int maxHeight=screenSize.height()-200;
    SQLEdit->setMaximumHeight(maxHeight);

    //Highlighter
    FadaHighlighter *hl=new FadaHighlighter(SQLEdit->document(),dbNames,se->vars.keys());
    //hl->setVarRules({"NbCultPlanif","NbCultPlanifValid"}); //todo

    QHBoxLayout *buttonLayout=new QHBoxLayout();
    QLabel *lInfo=new QLabel();
    //lInfo->setText("Loading...");
    QPushButton *runButton=new QPushButton("▶️ "+QObject::tr("Tester"));
    QPushButton *saveButton=new QPushButton(QObject::tr("Enregistrer"));
    QPushButton *cancelButton=new QPushButton(QObject::tr("Annuler"));
    //runButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogRetryButton));
    saveButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogSaveButton));
    cancelButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogCancelButton));

    buttonLayout->addWidget(lInfo);
    buttonLayout->addStretch();
    buttonLayout->addWidget(runButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    bool result=false;
    QObject::connect(runButton, &QPushButton::clicked, [&]() {
        se->runScript(nullptr,&db,true,SQLEdit,hl);
    });
    QObject::connect(saveButton, &QPushButton::clicked, [&]() {
        result=true;
        dialog.accept();
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        if (script==SQLEdit->toPlainText() or
            QMessageBox::question(QApplication::activeWindow(), title, QObject::tr("Abandonner les modification ?"), QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes) {
            result=false;
            dialog.reject();
        }
    });
    QObject::connect(SQLEdit, &QPlainTextEdit::cursorPositionChanged, [&]() {
        hl->highlightParentheses(SQLEdit);
        lInfo->setText(selectionInfo(SQLEdit));
    });
    // QObject::connect(SQLEdit, &QPlainTextEdit::textChanged, [&]() {
    //     hl->rehighlight();
    // });

    // Highlight initial si texte déjà présent
    hl->highlightParentheses(SQLEdit);

    //Menu contextuel
    SQLEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(SQLEdit, &QPlainTextEdit::customContextMenuRequested, SQLEdit, [SQLEdit]() {
        QMenu menu;
        QAction* openAction=menu.addAction(QObject::tr("Ouvrir un fichier SQL"));
        QAction* saveAction=menu.addAction(QObject::tr("Enregistrer dans un fichier SQL"));
        QAction* formatAction=menu.addAction(QObject::tr("Formater la requête SQL"));
        formatAction->setToolTip(QObject::tr("Des retours à la ligne sont injectés pour que les lignes ne dépassent pas 80 caractères.\n"
                                             "Mettez des espaces autour des opérateurs pour forcer un retour à la ligne:\n"
                                             "'Val1 + Val2' est scindé en 2 lignes, 'Val1+Val2' non.\n"
                                             "Mettez les opérateurs booléens en majuscules pour forcer un retour à la ligne:\n"
                                             "'Val1 AND Val2' est scindé en 2 lignes, 'Val1 and Val2' non."));
        QAbstractEventDispatcher::connect(formatAction, &QAction::hovered, &menu, [formatAction, &menu]() {
                QToolTip::showText( QCursor::pos(), formatAction->toolTip(), &menu);
        });
        menu.addSeparator();
        // Ajoute le menu par défaut de l'éditeur
        menu.addActions(SQLEdit->createStandardContextMenu()->actions());
        QSettings settings;//("greli.net", "Potaléger");

        QAction* chosen=menu.exec(QCursor::pos());
        if (chosen==formatAction) {
            fdaSqlFormat formatter(&menu);
            SQLEdit->setPlainText(formatter.formatSql(SQLEdit->toPlainText()));//+"\n\n"+sql
            //formatter.deleteLater();
        } else if (chosen==openAction) {
            QString fileName=QFileDialog::getOpenFileName(SQLEdit, QObject::tr("Ouvrir un fichier SQL"),settings.value("SQLdir").toString(),"*.sql");
            if (!fileName.isEmpty()) {
                QFile file(fileName);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QFileInfo fileInfo(fileName);
                    settings.setValue("SQLdir",fileInfo.absoluteFilePath());
                    QTextStream in(&file);
                    SQLEdit->setPlainText(in.readAll());
                    file.close();
                } else {
                    QMessageBox::warning(&menu, QObject::tr("Erreur"), QObject::tr("Impossible d'ouvrir le fichier"));
                }
            }
        } else if (chosen==saveAction) {
            QString fileName=QFileDialog::getSaveFileName( SQLEdit, QObject::tr("Enregistrer dans un fichier SQL"),settings.value("SQLdir").toString(),"*.sql");
            if (!fileName.isEmpty()) {
                QFile file(fileName);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QFileInfo fileInfo(fileName);
                    settings.setValue("SQLdir",fileInfo.absoluteFilePath());
                    QTextStream out(&file);
                    out << SQLEdit->toPlainText();
                    file.close();
                } else {
                    QMessageBox::warning(&menu, QObject::tr("Erreur"), QObject::tr("Impossible d'enregistrer le fichier."));
                }
            }
        }
    });

    QSettings settings;//("greli.net", "Potaléger");
    settings.beginGroup(title);
    const auto geometry=settings.value("geometry").toByteArray();
    if (geometry.isEmpty())
        dialog.setGeometry(50, 50, 600, 400);
    else
        dialog.restoreGeometry(geometry);
    settings.endGroup();

    dialog.exec();

    settings.beginGroup(title);
    settings.setValue("geometry", dialog.saveGeometry());
    settings.endGroup();

    if (result) {
        //settings.setValue("SQL", SQLEdit->toPlainText());
        return SQLEdit->toPlainText();
    } else {
        return "";
    }
}
