#include "Dialogs.h"
#include <QApplication>
#include <QMessageBox>
#include "FdaSqlFormat.h"
#include "FdaWidget.h"
//#include "data/FdaCalls.h"
#include "qcheckbox.h"
#include "qcombobox.h"
#include "qdatetimeedit.h"
#include "qlineedit.h"
#include "qmenu.h"
#include "qscrollarea.h"
#include "qspinbox.h"
#include "qsqlerror.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QLEInteger>
#include "FdaUtils.h"
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QScreen>
#include <QSettings>
#include <QStyle>
#include <QGuiApplication>
#include <QFileDialog>
#include <QColorDialog>
#include <QToolTip>


class SqlHighlighter : public QSyntaxHighlighter {
public:
    SqlHighlighter(QTextDocument *parent=nullptr) : QSyntaxHighlighter(parent) {
        QTextCharFormat keywordFormat, keywordFormat2, keywordFormat3;
        if (isDarkTheme()) {
            keywordFormat.setForeground(QColor("#0085c4"));
            keywordFormat2.setForeground(Qt::darkYellow);
            keywordFormat3.setForeground(Qt::red);
        } else {
            keywordFormat.setForeground(Qt::blue);
            keywordFormat2.setForeground(Qt::darkYellow);
            keywordFormat3.setForeground(Qt::red);
        }
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywordPatterns={
            "\\bSELECT\\b", "\\bFROM\\b", "\\bWHERE\\b", "\\bGROUP BY\\b", "\\bORDER BY\\b", "\\bJOIN\\b", "\\bLEFT\\b", "\\bRIGHT\\b", "\\bUSING\\b"
        };
        QStringList keywordPatterns2={
            "\\bAND\\b", "\\bOR\\b", "\\bNULL\\b", "\\bISNULL\\b", "\\bNOTNULL\\b", "\\bNOT\\b", "\\bBETWEEN\\b"
        };
        QStringList keywordPatterns3={
            "\\bUPDATE\\b", "\\bINSERT\\b", "\\bDELETE\\b", "\\bSET\\b", "\\bCREATE\\b", "\\bDROP\\b", "\\bALTER\\b"
        };
        for (const QString &pattern : keywordPatterns)
            rules.append({QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), keywordFormat});
        for (const QString &pattern2 : keywordPatterns2)
            rules.append({QRegularExpression(pattern2, QRegularExpression::CaseInsensitiveOption), keywordFormat2});
        for (const QString &pattern3 : keywordPatterns3)
            rules.append({QRegularExpression(pattern3, QRegularExpression::CaseInsensitiveOption), keywordFormat3});
    }
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
};

namespace {// Highlight dynamique des parenthèses
// Recherche la parenthèse associée
int findMatchingParenthesis(const QString& text, int pos, QChar open, QChar close, bool forward) {
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

// Ajoute la sélection visuelle
void highlightAt(QPlainTextEdit* SQLEdit, int position, QList<QTextEdit::ExtraSelection>& extraSelections) {
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor(255, 255, 0, 90)); // Jaune semi-transparent
    QTextCursor selCursor=SQLEdit->textCursor();
    selCursor.setPosition(position);
    selCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor=selCursor;
    extraSelections.append(selection);
}

void highlightParentheses(QPlainTextEdit* SQLEdit) {
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

QString selectionInfo(QPlainTextEdit* SQLEdit) {
    QString result="";
    result="Caracters "+str(SQLEdit->toPlainText().length())+" - "+
           "Lines "+str(SQLEdit->blockCount())+" - "+
           "Cursor "+str(SQLEdit->textCursor().blockNumber()+1)+","+str(SQLEdit->textCursor().positionInBlock())+" - "+
           "Selection "+str(SQLEdit->textCursor().selectedText().length());
    return result;
}

} // namespace

QStringList GraphDialog(const QString &titre, const QString &message, QStringList columns, QStringList dataTypes)
{
    QDialog dialog(QApplication::activeWindow());
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout=new QHBoxLayout();
    if (false) {
        QLabel *iconLabel=new QLabel();
        QIcon icon;
        icon=QIcon(":/images/potaleger.svg");
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }
    QLabel *messageLabel=new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QLabel *lxAxis=new QLabel();
    QComboBox *xAxis=new QComboBox();
    QComboBox *xAxisGroup=new QComboBox();
    QCheckBox *xAxisYearsSeries=new QCheckBox(QObject::tr("Une série par an"));

    QLabel *ly1Axis=new QLabel(); ly1Axis->setObjectName("ly1Axis");
    QComboBox *y1Axis=new QComboBox();
    QComboBox *y1AxisCalc=new QComboBox();
    QComboBox *y1AxisType=new QComboBox();
    QToolButton *y1Color=new QToolButton();
    QCheckBox *y1RightAxis=new QCheckBox("");
    QHBoxLayout *y1AxisLayout=new QHBoxLayout();

    QLabel *ly2Axis=new QLabel();
    QComboBox *y2Axis=new QComboBox();
    QComboBox *y2AxisCalc=new QComboBox();
    QComboBox *y2AxisType=new QComboBox();
    QToolButton *y2Color=new QToolButton();
    QCheckBox *y2RightAxis=new QCheckBox("");
    QHBoxLayout *y2AxisLayout=new QHBoxLayout();

    QLabel *ly3Axis=new QLabel();
    QComboBox *y3Axis=new QComboBox();
    QComboBox *y3AxisCalc=new QComboBox();
    QComboBox *y3AxisType=new QComboBox();
    QToolButton *y3Color=new QToolButton();
    QCheckBox *y3RightAxis=new QCheckBox("");
    QHBoxLayout *y3AxisLayout=new QHBoxLayout();

    QLabel *ly4Axis=new QLabel();
    QComboBox *y4Axis=new QComboBox();
    QComboBox *y4AxisCalc=new QComboBox();
    QComboBox *y4AxisType=new QComboBox();
    QToolButton *y4Color=new QToolButton();
    QCheckBox *y4RightAxis=new QCheckBox("");
    QHBoxLayout *y4AxisLayout=new QHBoxLayout();

    bool bComboSetting=false;
    QList<QColor> seriesColors={y2Color->palette().base().color(),QColor(),QColor(),QColor(),QColor()};

    xAxis->setToolTip(QObject::tr("Champ de la table (ou vue) qui contient les \n"
                                  "valeurs d'abscisse (axe horizontal) des points du graphique."));
    xAxisGroup->setToolTip(QObject::tr("S'il y a plusieurs lignes avec une même valeur dans le champ abscisse, \n"
                                       "les valeurs d'ordonnées de ces lignes peuvent être groupées."));
    xAxisYearsSeries->setToolTip(QObject::tr("Si les données couvrent plusieurs années, une série sera créé pour chaque année.\n"
                                             "Ceci permet de comparer les données d'une année sur l'autre."));
    y1Axis->setToolTip(QObject::tr("Champ de la table (ou vue) qui contient les \n"
                                   "valeurs d'ordonnée (axe vertical) des points \n"
                                   "de la 1ère série de données.\n\n"
                                   "Sélectionner 'Nombre de lignes' pour compter \n"
                                   "les lignes ayant une même valeur d'abscisse."));
    y1AxisType->setToolTip(QObject::tr("Type de représentation graphique de la série.\n"
                                       "'Points > 0' : seulement les points dont l'ordonnée est supérieure à zéro."));
    //y1Color->setToolTip(QObject::tr("Couleur de la série.")); prend la couleur du bouton
    y1RightAxis->setToolTip(QObject::tr("Attacher la série à l'axe vertical droit."));

    lxAxis->setText(QObject::tr("Abscisse"));
    xAxis->addItems(columns);
    QObject::connect(xAxis, &QComboBox::currentIndexChanged, [&]() {
        if (bComboSetting) return;
        bComboSetting=true;
        setXAxisGroup(xAxisGroup,dataTypes[columns.indexOf(xAxis->currentText())]);
        xAxisYearsSeries->setVisible(dataTypes[columns.indexOf(xAxis->currentText())]=="DATE");
        if (!xAxisYearsSeries->isVisible()) xAxisYearsSeries->setChecked(false);
        bComboSetting=false;
    });

    QObject::connect(xAxisGroup, &QComboBox::currentIndexChanged, [&]() {
        if (bComboSetting) return;
        bComboSetting=true;
        setYAxis(y1Axis,xAxisGroup->currentIndex()!=xAxisGroupNo, columns, dataTypes,xAxis->currentIndex(),false);
        setYAxis(y2Axis,xAxisGroup->currentIndex()!=xAxisGroupNo, columns, dataTypes,xAxis->currentIndex(),true);
        setYAxis(y3Axis,xAxisGroup->currentIndex()!=xAxisGroupNo, columns, dataTypes,xAxis->currentIndex(),true);
        setYAxis(y4Axis,xAxisGroup->currentIndex()!=xAxisGroupNo, columns, dataTypes,xAxis->currentIndex(),true);
        y1AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y1Axis->currentText()!=QObject::tr("Nombre de lignes"));
        if (!xAxisYearsSeries->isChecked()) {
            y2AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y2Axis->currentText()!=QObject::tr("Nombre de lignes"));
            y3AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y3Axis->currentText()!=QObject::tr("Nombre de lignes"));
            y4AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y4Axis->currentText()!=QObject::tr("Nombre de lignes"));
        } else {
            y2AxisCalc->setVisible(false);
            y3AxisCalc->setVisible(false);
            y4AxisCalc->setVisible(false);
        }
        bComboSetting=false;
    });

    xAxisYearsSeries->setVisible(false);
    QObject::connect(xAxisYearsSeries, &QCheckBox::checkStateChanged, [&]() {
        if (bComboSetting) return;
        bComboSetting=true;
        y2Axis->setVisible(!xAxisYearsSeries->isChecked());
        y2AxisCalc->setVisible(!xAxisYearsSeries->isChecked());
        y2AxisType->setVisible(!xAxisYearsSeries->isChecked());
        y2Color->setVisible(!xAxisYearsSeries->isChecked());
        y2RightAxis->setVisible(!xAxisYearsSeries->isChecked());
        y3Axis->setVisible(!xAxisYearsSeries->isChecked());
        y3AxisCalc->setVisible(!xAxisYearsSeries->isChecked());
        y3AxisType->setVisible(!xAxisYearsSeries->isChecked());
        y3Color->setVisible(!xAxisYearsSeries->isChecked());
        y3RightAxis->setVisible(!xAxisYearsSeries->isChecked());
        y4Axis->setVisible(!xAxisYearsSeries->isChecked());
        y4AxisCalc->setVisible(!xAxisYearsSeries->isChecked());
        y4AxisType->setVisible(!xAxisYearsSeries->isChecked());
        y4Color->setVisible(!xAxisYearsSeries->isChecked());
        y4RightAxis->setVisible(!xAxisYearsSeries->isChecked());
        if (xAxisYearsSeries->isChecked()) {
            ly1Axis->setText(QObject::tr("Séries"));
            ly2Axis->setText("");
            ly3Axis->setText("");
            ly4Axis->setText("");
        } else {
            ly1Axis->setText(QObject::tr("Série %1").arg(1));
            ly2Axis->setText(QObject::tr("Série %1").arg(2));
            ly3Axis->setText(QObject::tr("Série %1").arg(3));
            ly4Axis->setText(QObject::tr("Série %1").arg(4));
        }

        bComboSetting=false;
    });

    QHBoxLayout *xAxisLayout=new QHBoxLayout();
    xAxisLayout->addWidget(lxAxis);
    xAxisLayout->addWidget(xAxis);
    xAxisLayout->addWidget(xAxisGroup);
    xAxisLayout->addWidget(xAxisYearsSeries);
    layout->addLayout(xAxisLayout);

    ly1Axis->setText(QObject::tr("Série %1").arg(1));
    ly2Axis->setText(QObject::tr("Série %1").arg(2));
    ly3Axis->setText(QObject::tr("Série %1").arg(3));
    ly4Axis->setText(QObject::tr("Série %1").arg(4));

    y1AxisCalc->setFixedWidth(150);
    y2AxisCalc->setFixedWidth(150);
    y3AxisCalc->setFixedWidth(150);
    y4AxisCalc->setFixedWidth(150);

    QStringList typeItems={QObject::tr("Courbe"),QObject::tr("Points"),QObject::tr("Points > 0"),QObject::tr("Barres")};
    y1AxisType->addItems(typeItems);
    y2AxisType->addItems(typeItems);
    y3AxisType->addItems(typeItems);
    y4AxisType->addItems(typeItems);

    y1AxisType->setFixedWidth(100);
    y2AxisType->setFixedWidth(100);
    y3AxisType->setFixedWidth(100);
    y4AxisType->setFixedWidth(100);


    y1RightAxis->setEnabled(false);
    y1RightAxis->setFixedWidth(40);
    y2RightAxis->setFixedWidth(40);
    y3RightAxis->setFixedWidth(40);
    y4RightAxis->setFixedWidth(40);

    y1AxisLayout->addWidget(ly1Axis);
    y1AxisLayout->addWidget(y1Axis);
    y1AxisLayout->addWidget(y1AxisCalc);
    y1AxisLayout->addWidget(y1AxisType);
    y1AxisLayout->addWidget(y1Color);
    y1AxisLayout->addWidget(y1RightAxis);
    layout->addLayout(y1AxisLayout);

    y2AxisLayout->addWidget(ly2Axis);
    y2AxisLayout->addWidget(y2Axis);
    y2AxisLayout->addWidget(y2AxisCalc);
    y2AxisLayout->addWidget(y2AxisType);
    y2AxisLayout->addWidget(y2Color);
    y2AxisLayout->addWidget(y2RightAxis);
    layout->addLayout(y2AxisLayout);

    y3AxisLayout->addWidget(ly3Axis);
    y3AxisLayout->addWidget(y3Axis);
    y3AxisLayout->addWidget(y3AxisCalc);
    y3AxisLayout->addWidget(y3AxisType);
    y3AxisLayout->addWidget(y3Color);
    y3AxisLayout->addWidget(y3RightAxis);
    layout->addLayout(y3AxisLayout);

    y4AxisLayout->addWidget(ly4Axis);
    y4AxisLayout->addWidget(y4Axis);
    y4AxisLayout->addWidget(y4AxisCalc);
    y4AxisLayout->addWidget(y4AxisType);
    y4AxisLayout->addWidget(y4Color);
    y4AxisLayout->addWidget(y4RightAxis);
    layout->addLayout(y4AxisLayout);

    QObject::connect(y1Axis, &QComboBox::currentIndexChanged, [&]() {
        if (bComboSetting) return;
        bComboSetting=true;
        y1AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y1Axis->currentText()!=QObject::tr("Nombre de lignes"));
        if (columns.indexOf(y1Axis->currentText())>-1)
            setYAxisCalc(y1AxisCalc,dataTypes[columns.indexOf(y1Axis->currentText())]);
        bComboSetting=false;
    });
    QObject::connect(y2Axis, &QComboBox::currentIndexChanged, [&]() {
        if (bComboSetting) return;
        bComboSetting=true;
        y2AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y2Axis->currentText()!=QObject::tr("Nombre de lignes"));
        if (columns.indexOf(y2Axis->currentText())>-1)
            setYAxisCalc(y2AxisCalc,dataTypes[columns.indexOf(y2Axis->currentText())]);
        y2AxisCalc->setEnabled(y2Axis->currentText()!="");
        y2AxisType->setEnabled(y2AxisCalc->isEnabled());
        y2Color->setEnabled(y2AxisCalc->isEnabled());
        if (y2Color->isEnabled() and seriesColors[2].isValid())
            y2Color->setStyleSheet("background-color: " + seriesColors[2].name() + ";border: none;width: 21px;");
        else
            y2Color->setStyleSheet("background-color: " + seriesColors[0].name() + ";width: 16px;");
        y2RightAxis->setEnabled(y2AxisCalc->isEnabled());
        bComboSetting=false;
    });
    QObject::connect(y3Axis, &QComboBox::currentIndexChanged, [&]() {
        if (bComboSetting) return;
        bComboSetting=true;
        y3AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y3Axis->currentText()!=QObject::tr("Nombre de lignes"));
        if (columns.indexOf(y3Axis->currentText())>-1)
            setYAxisCalc(y3AxisCalc,dataTypes[columns.indexOf(y3Axis->currentText())]);
        y3AxisCalc->setEnabled(y3Axis->currentText()!="");
        y3AxisType->setEnabled(y3AxisCalc->isEnabled());
        y3Color->setEnabled(y3AxisCalc->isEnabled());
        if (y3Color->isEnabled() and seriesColors[3].isValid())
            y3Color->setStyleSheet("background-color: " + seriesColors[3].name() + ";border: none;width: 21px;");
        else
            y3Color->setStyleSheet("background-color: " + seriesColors[0].name() + ";width: 16px;");
        y3RightAxis->setEnabled(y3AxisCalc->isEnabled());
        bComboSetting=false;
    });
    QObject::connect(y4Axis, &QComboBox::currentIndexChanged, [&]() {
        if (bComboSetting) return;
        bComboSetting=true;
        y4AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y4Axis->currentText()!=QObject::tr("Nombre de lignes"));
        if (columns.indexOf(y4Axis->currentText())>-1)
            setYAxisCalc(y4AxisCalc,dataTypes[columns.indexOf(y4Axis->currentText())]);
        y4AxisCalc->setEnabled(y4Axis->currentText()!="");
        y4AxisType->setEnabled(y4AxisCalc->isEnabled());
        y4Color->setEnabled(y4AxisCalc->isEnabled());
        if (y4Color->isEnabled() and seriesColors[4].isValid())
            y4Color->setStyleSheet("background-color: " + seriesColors[4].name() + ";border: none;width: 21px;");
        else
            y4Color->setStyleSheet("background-color: " + seriesColors[0].name() + ";width: 16px;");
        y4RightAxis->setEnabled(y4AxisCalc->isEnabled());
        bComboSetting=false;
    });

    // QObject::connect(y1AxisType, &QComboBox::currentIndexChanged, [&]() {
    //     if (bComboSetting) return;
    //     bComboSetting=true;
    //     setYAxisType(y2AxisType,y1AxisType->currentIndex());
    //     setYAxisType(y3AxisType,y1AxisType->currentIndex());
    //     setYAxisType(y3AxisType,y1AxisType->currentIndex());
    //     bComboSetting=false;
    // });

    QObject::connect(y1Color, &QPushButton::clicked, [&]() {
        seriesColors[1]=QColorDialog::getColor(seriesColors[1], QApplication::activeWindow(), QObject::tr("Couleur de la série"),
                                                 {QColorDialog::ShowAlphaChannel,QColorDialog::DontUseNativeDialog});
        if (seriesColors[1].isValid())
            y1Color->setStyleSheet("background-color: " + seriesColors[1].name() + ";border: none;width: 21px;");
        else
            y1Color->setStyleSheet("background-color: " + seriesColors[0].name() + ";width: 16px;");
    });
    QObject::connect(y2Color, &QPushButton::clicked, [&]() {
        seriesColors[2]=QColorDialog::getColor(seriesColors[2], QApplication::activeWindow(), QObject::tr("Couleur de la série"),
                                                 {QColorDialog::ShowAlphaChannel,QColorDialog::DontUseNativeDialog});
        if (seriesColors[2].isValid())
            y2Color->setStyleSheet("background-color: " + seriesColors[2].name() + ";border: none;width: 21px;");
        else
            y2Color->setStyleSheet("background-color: " + seriesColors[0].name() + ";width: 16px;");
    });
    QObject::connect(y3Color, &QPushButton::clicked, [&]() {
        seriesColors[3]=QColorDialog::getColor(seriesColors[3], QApplication::activeWindow(), QObject::tr("Couleur de la série"),
                                                 {QColorDialog::ShowAlphaChannel,QColorDialog::DontUseNativeDialog});
        if (seriesColors[3].isValid())
            y3Color->setStyleSheet("background-color: " + seriesColors[3].name() + ";border: none;width: 21px;");
        else
            y3Color->setStyleSheet("background-color: " + seriesColors[0].name() + ";width: 16px;");
    });
    QObject::connect(y4Color, &QPushButton::clicked, [&]() {
        seriesColors[4]=QColorDialog::getColor(seriesColors[4], QApplication::activeWindow(), QObject::tr("Couleur de la série"),
                                                 {QColorDialog::ShowAlphaChannel,QColorDialog::DontUseNativeDialog});
        if (seriesColors[4].isValid())
            y4Color->setStyleSheet("background-color: " + seriesColors[4].name() + ";border: none;width: 21px;");
        else
            y4Color->setStyleSheet("background-color: " + seriesColors[0].name() + ";width: 16px;");
    });


    QHBoxLayout *buttonLayout=new QHBoxLayout();
    QPushButton *okButton=new QPushButton(QObject::tr("OK"));
    QPushButton *cancelButton=new QPushButton(QObject::tr("Annuler"));
    okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
    cancelButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogCancelButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    QStringList result;
    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        if (xAxisGroup->currentIndex()==xAxisGroupNo and dataTypes[columns.indexOf(xAxis->currentText())]=="DATE") {
            MessageDlg(titre,QObject::tr("Le champ de l'abscisse  est de type DATE,\n"
                                         "il faut choisir une méthode de regroupement."),
                                         "",QStyle::SP_MessageBoxWarning);
            return;
        }
        if (y1AxisType->currentIndex()==typeSeriesBar and xAxisYearsSeries->isVisible() and xAxisYearsSeries->isChecked()) {
            MessageDlg(titre,QObject::tr("L'option 'Une série par an' ne fonctionne pas avec la présentation 'Barres'."),
                                         "",QStyle::SP_MessageBoxWarning);
            return;
        }
        result.append(str(xAxis->currentIndex()));
        result.append(str(xAxisGroup->currentIndex()));
        result.append(iif(xAxisYearsSeries->isVisible() and xAxisYearsSeries->isChecked(),1,0).toString());
        //Series 1
        result.append(str(columns.indexOf(y1Axis->currentText())));
        result.append(str(y1AxisCalc->currentIndex()));
        result.append(str(y1AxisType->currentIndex()));
        if (seriesColors[1].isValid()) result.append(seriesColors[1].name());
        else result.append("");
        result.append("0");
        //Series 2
        result.append(str(columns.indexOf(y2Axis->currentText())));
        result.append(str(y2AxisCalc->currentIndex()));
        result.append(str(y2AxisType->currentIndex()));
        if (seriesColors[2].isValid()) result.append(seriesColors[2].name());
        else result.append("");
        result.append(iif(y2RightAxis->checkState(),1,0).toString());
        //Series 3
        result.append(str(columns.indexOf(y3Axis->currentText())));
        result.append(str(y3AxisCalc->currentIndex()));
        result.append(str(y3AxisType->currentIndex()));
        if (seriesColors[3].isValid()) result.append(seriesColors[3].name());
        else result.append("");
        result.append(iif(y3RightAxis->checkState(),1,0).toString());
        //Series 4
        result.append(str(columns.indexOf(y4Axis->currentText())));
        result.append(str(y4AxisCalc->currentIndex()));
        result.append(str(y4AxisType->currentIndex()));
        if (seriesColors[4].isValid()) result.append(seriesColors[4].name());
        else result.append("");
        result.append(iif(y4RightAxis->checkState(),1,0).toString());

        QSettings settings;
        settings.beginGroup(titre);
        settings.setValue("xAxis",xAxis->currentText());
        settings.setValue("xAxisGroup",xAxisGroup->currentText());
        settings.setValue("xAxisYearsSeries",iif(xAxisYearsSeries->isChecked(),1,0).toString());
        settings.setValue("y1Axis",y1Axis->currentText());
        settings.setValue("y1AxisCalc",y1AxisCalc->currentText());
        settings.setValue("y1AxisType",y1AxisType->currentText());
        if (seriesColors[1].isValid()) settings.setValue("y1Color",seriesColors[1].name());
        else settings.setValue("y1Color","");
        settings.setValue("y2Axis",y2Axis->currentText());
        settings.setValue("y2AxisCalc",y2AxisCalc->currentText());
        settings.setValue("y2AxisType",y2AxisType->currentText());
        if (seriesColors[2].isValid()) settings.setValue("y2Color",seriesColors[2].name());
        else settings.setValue("y2Color","");
        settings.setValue("y2RightAxis",iif(y2RightAxis->checkState(),1,0).toString());
        settings.setValue("y3Axis",y3Axis->currentText());
        settings.setValue("y3AxisCalc",y3AxisCalc->currentText());
        settings.setValue("y3AxisType",y3AxisType->currentText());
        if (seriesColors[3].isValid()) settings.setValue("y3Color",seriesColors[3].name());
        else settings.setValue("y3Color","");
        settings.setValue("y3RightAxis",iif(y3RightAxis->checkState(),1,0).toString());
        settings.setValue("y4Axis",y4Axis->currentText());
        settings.setValue("y4AxisCalc",y4AxisCalc->currentText());
        settings.setValue("y4AxisType",y4AxisType->currentText());
        if (seriesColors[4].isValid()) settings.setValue("y4Color",seriesColors[4].name());
        else settings.setValue("y4Color","");
        settings.setValue("y4RightAxis",iif(y4RightAxis->checkState(),1,0).toString());
        settings.endGroup();

        dialog.accept();
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        dialog.reject();
    });

    int w,h;
    w=fmax(dialog.sizeHint().width(),600);
    h=fmax(dialog.sizeHint().height(),150);
    dialog.setFixedSize(w,h);//User can't resize the window.

    QSettings settings;
    settings.beginGroup(titre);

    xAxis->setCurrentText(settings.value("xAxis").toString());
    setXAxisGroup(xAxisGroup,dataTypes[columns.indexOf(xAxis->currentText())]);
    xAxisGroup->setCurrentText(settings.value("xAxisGroup").toString());
    setYAxis(y1Axis,xAxisGroup->currentIndex()!=xAxisGroupNo, columns, dataTypes,xAxis->currentIndex(),false);
    setYAxis(y2Axis,xAxisGroup->currentIndex()!=xAxisGroupNo, columns, dataTypes,xAxis->currentIndex(),true);
    setYAxis(y3Axis,xAxisGroup->currentIndex()!=xAxisGroupNo, columns, dataTypes,xAxis->currentIndex(),true);
    setYAxis(y4Axis,xAxisGroup->currentIndex()!=xAxisGroupNo, columns, dataTypes,xAxis->currentIndex(),true);

    y1Axis->setCurrentText(settings.value("y1Axis").toString());
    y1AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y1Axis->currentText()!=QObject::tr("Nombre de lignes"));
    if (columns.indexOf(y1Axis->currentText())>-1)
        setYAxisCalc(y1AxisCalc,dataTypes[columns.indexOf(y1Axis->currentText())]);
    y1AxisCalc->setCurrentText(settings.value("y1AxisCalc").toString());
    y1AxisType->setCurrentText(settings.value("y1AxisType").toString());
    // setYAxisType(y2AxisType,y1AxisType->currentIndex());
    // setYAxisType(y3AxisType,y1AxisType->currentIndex());
    // setYAxisType(y4AxisType,y1AxisType->currentIndex());
    if (QColor(settings.value("y1Color").toString()).isValid()) {
        seriesColors[1]=QColor(settings.value("y1Color").toString());
        y1Color->setStyleSheet("background-color: " + seriesColors[1].name() + ";border: none;width: 21px;");
    } else {
        y1Color->setStyleSheet("background-color: " + seriesColors[0].name() + ";width: 16px;");
    }

    y2Axis->setCurrentText(settings.value("y2Axis").toString());
    y2AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y2Axis->currentText()!=QObject::tr("Nombre de lignes"));
    if (columns.indexOf(y2Axis->currentText())>-1)
        setYAxisCalc(y2AxisCalc,dataTypes[columns.indexOf(y2Axis->currentText())]);
    y2AxisCalc->setCurrentText(settings.value("y2AxisCalc").toString());
    y2AxisType->setCurrentText(settings.value("y2AxisType").toString());
    if (QColor(settings.value("y2Color").toString()).isValid() and y2Color->isEnabled()) {
        seriesColors[2]=QColor(settings.value("y2Color").toString());
        y2Color->setStyleSheet("background-color: " + seriesColors[2].name() + ";border: none;width: 21px;");
    } else {
        y2Color->setStyleSheet("background-color: " + seriesColors[0].name() + ";width: 16px;");
    }
    if (settings.value("y2RightAxis").toInt()==1) y2RightAxis->setCheckState(Qt::Checked);

    y3Axis->setCurrentText(settings.value("y3Axis").toString());
    y3AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y3Axis->currentText()!=QObject::tr("Nombre de lignes"));
    if (columns.indexOf(y3Axis->currentText())>-1)
        setYAxisCalc(y3AxisCalc,dataTypes[columns.indexOf(y3Axis->currentText())]);
    y3AxisCalc->setCurrentText(settings.value("y3AxisCalc").toString());
    y3AxisType->setCurrentText(settings.value("y3AxisType").toString());
    if (QColor(settings.value("y3Color").toString()).isValid() and y3Color->isEnabled()) {
        seriesColors[3]=QColor(settings.value("y3Color").toString());
        y3Color->setStyleSheet("background-color: " + seriesColors[3].name() + ";border: none;width: 21px;");
    } else {
        y3Color->setStyleSheet("background-color: " + seriesColors[0].name() + ";width: 16px;");
    }
    if (settings.value("y3RightAxis").toInt()==1) y3RightAxis->setCheckState(Qt::Checked);

    y4Axis->setCurrentText(settings.value("y4Axis").toString());
    y4AxisCalc->setVisible(xAxisGroup->currentIndex()!=xAxisGroupNo and y4Axis->currentText()!=QObject::tr("Nombre de lignes"));
    if (columns.indexOf(y4Axis->currentText())>-1)
        setYAxisCalc(y4AxisCalc,dataTypes[columns.indexOf(y4Axis->currentText())]);
    y4AxisCalc->setCurrentText(settings.value("y4AxisCalc").toString());
    y4AxisType->setCurrentText(settings.value("y4AxisType").toString());
    if (QColor(settings.value("y4Color").toString()).isValid() and y4Color->isEnabled()) {
        seriesColors[4]=QColor(settings.value("y4Color").toString());
        y4Color->setStyleSheet("background-color: " + seriesColors[4].name() + ";border: none;width: 21px;");
    } else {
        y4Color->setStyleSheet("background-color: " + seriesColors[0].name() + ";width: 16px;");
    }
    if (settings.value("y4RightAxis").toInt()==1) y4RightAxis->setCheckState(Qt::Checked);

    xAxisYearsSeries->setVisible(dataTypes[columns.indexOf(xAxis->currentText())]=="DATE");
    if (settings.value("xAxisYearsSeries").toInt()==1)
        xAxisYearsSeries->setCheckState(Qt::Checked);

    settings.endGroup();

    dialog.exec();

    return result;
}

void setXAxisGroup(QComboBox *cb, QString dataType) {
    QString text=cb->currentText();
    cb->clear();
    cb->addItem(QObject::tr("Toutes les lignes")); //xAxisGroup
    cb->addItem(QObject::tr("Grouper si identique"));
    if (dataType=="TEXT") {
        cb->addItem(QObject::tr("Grouper par 1er mot"));
        cb->addItem(QObject::tr("Grouper par 1er caractère"));
        cb->addItem(QObject::tr("Grouper par %1 1ers caractères").arg(2));
        cb->addItem(QObject::tr("Grouper par %1 1ers caractères").arg(3));
        cb->addItem(QObject::tr("Grouper par %1 1ers caractères").arg(4));
        cb->addItem(QObject::tr("Grouper par %1 1ers caractères").arg(5));
        cb->addItem(QObject::tr("Grouper par %1 1ers caractères").arg(6));
        cb->addItem(QObject::tr("Grouper par %1 1ers caractères").arg(7));
        cb->addItem(QObject::tr("Grouper par %1 1ers caractères").arg(8));
        cb->addItem(QObject::tr("Grouper par %1 1ers caractères").arg(9));
        cb->addItem(QObject::tr("Grouper par %1 1ers caractères").arg(10));
    } else if (dataType=="DATE") {
        cb->addItem(QObject::tr("Grouper par année"));
        cb->addItem(QObject::tr("Grouper par mois"));
        cb->addItem(QObject::tr("Grouper par semaine"));
        cb->addItem(QObject::tr("Grouper par jour"));
    } else if (dataType=="REAL" or dataType.startsWith("INT")) {
        cb->addItem(QObject::tr("Grouper par millier"));
        cb->addItem(QObject::tr("Grouper par centaine"));
        cb->addItem(QObject::tr("Grouper par dizaine"));
        if (dataType=="REAL") {
            cb->addItem(QObject::tr("Grouper par entier"));
            cb->addItem(QObject::tr("Grouper par arrondi à 1 décimale"));
            cb->addItem(QObject::tr("Grouper par arrondi à %1 décimales").arg(2));
            cb->addItem(QObject::tr("Grouper par arrondi à %1 décimales").arg(3));
            cb->addItem(QObject::tr("Grouper par arrondi à %1 décimales").arg(4));
            cb->addItem(QObject::tr("Grouper par arrondi à %1 décimales").arg(5));
            cb->addItem(QObject::tr("Grouper par arrondi à %1 décimales").arg(6));
        }
    }
    cb->setCurrentText(text);
};

void setYAxis(QComboBox *cb, bool grouping, QStringList columns, QStringList dataTypes, int xIndex, bool nullValue) {
    QString text=cb->currentText();
    cb->clear();
    if (!grouping) { //All lines, only numerical fields
        for (int i=0;i<columns.count();i++) {
            if (i!=xIndex and(dataTypes[i]=="REAL" or dataTypes[i].startsWith("INT")))
                cb->addItem(columns[i]);
        }
    } else { //Grouping
        cb->addItem(QObject::tr("Nombre de lignes"));
        for (int i=0;i<columns.count();i++) {
            if (i!=xIndex)
                cb->addItem(columns[i]);
        }
    }
    if (nullValue)
        cb->addItem("");
    cb->setCurrentText(text);
}

void setYAxisCalc(QComboBox *cb, QString dataType) {
    QString text=cb->currentText();
    cb->clear();
    cb->addItem(QObject::tr("Nb valeurs non vides")); //calcSeries
    cb->addItem(QObject::tr("Nb valeurs distinctes"));
    if (dataType=="DATE" or dataType=="REAL" or dataType.startsWith("INT")) {
        cb->addItem(QObject::tr("1ère valeur"));
        cb->addItem(QObject::tr("Dernière valeur"));
        cb->addItem(QObject::tr("Moyenne"));
        cb->addItem(QObject::tr("Minimun"));
        cb->addItem(QObject::tr("Maximum"));
        if (dataType=="REAL" or dataType.startsWith("INT"))
            cb->addItem(QObject::tr("Somme"));
    }
    cb->setCurrentText(text);
}

// void setYAxisType(QComboBox *cb, int axisType) {
//     cb->clear();
//     if (axisType==typeSeriesBar) {
//         cb->addItem(QObject::tr("Barres"));
//     } else {
//         cb->addItem(QObject::tr("Courbe"));
//         cb->addItem(QObject::tr("Points"));
//         cb->addItem(QObject::tr("Points > 0"));
//     }
// }

QString QueryDialog(const QString &titre, const QString &message,QSqlDatabase db)
{
    QDialog dialog(QApplication::activeWindow());
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout=new QHBoxLayout();

    QLabel *messageLabel=new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QPlainTextEdit *SQLEdit=new QPlainTextEdit(&dialog);
    new SqlHighlighter(SQLEdit->document());
    QFont font;
    //font.setStyleHint(QFont::Monospace);
    font.setFamily("Monospace");
    SQLEdit->setFont(font);
    SQLEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

    QSettings settings;//("greli.net", "Potaléger");
    SQLEdit->setPlainText(settings.value("SQL").toString());

    layout->addWidget(SQLEdit);
    QSize screenSize=QGuiApplication::primaryScreen()->size();
    int maxHeight=screenSize.height()-200;
    SQLEdit->setMaximumHeight(maxHeight);

    QHBoxLayout *buttonLayout=new QHBoxLayout();
    QLabel *lInfo=new QLabel();
    //lInfo->setText("Loading...");
    QPushButton *okButton=new QPushButton(QObject::tr("OK"));
    QPushButton *cancelButton=new QPushButton(QObject::tr("Annuler"));
    okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
    cancelButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogCancelButton));

    buttonLayout->addWidget(lInfo);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    bool result=false;
    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        PotaQuery pQuery(db);
        QStringList values;
        if (SQLEdit->toPlainText().toUpper().startsWith("SELECT ") or
            //SQLEdit->toPlainText().toUpper().startsWith("PRAGMA TABLE_") or
            SQLEdit->toPlainText().toUpper().startsWith("WITH ")) {
            values=SQLEdit->toPlainText().split(";\n");
            if (pQuery.exec(values[0])) {
                //qDebug() << values[0];
                result=true;
                dialog.accept();
            } else {
                messageLabel->setText(StrElipsis(pQuery.lastError().text(),200));
            }
        } else if (SQLEdit->toPlainText().toUpper().startsWith("INSERT ")or
                   SQLEdit->toPlainText().toUpper().startsWith("UPDATE ")or
                   SQLEdit->toPlainText().toUpper().startsWith("DELETE ")or
                   SQLEdit->toPlainText().toUpper().startsWith("PRAGMA ")) {
            if(pQuery.Select0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='SQL_données'")=="Oui!") {
                QString QueryError="";
                values=SQLEdit->toPlainText().split(";\n");
                for(int i=0;i<values.count();i++){
                    if (!values[i].trimmed().isEmpty() and
                        !values[i].toUpper().startsWith("INSERT ")and
                        !values[i].toUpper().startsWith("UPDATE ")and
                        !values[i].toUpper().startsWith("DELETE ")and
                        !values[i].toUpper().startsWith("PRAGMA ")){
                        QueryError=values[i];
                        break;
                    }
                }
                if (QueryError.isEmpty()) {
                    if (values.count()==1) {
                        messageLabel->setText(StrElipsis(SQLEdit->toPlainText(),200)+" : "+
                                              pQuery.Select0ShowErr(SQLEdit->toPlainText()).toString()+"\n"
                                              "Rows affected: "+str(pQuery.numRowsAffected())+
                                              pQuery.lastError().text());
                    } else {
                        if(!pQuery.ExecMultiShowErr(SQLEdit->toPlainText(),";\n",nullptr))
                            messageLabel->setText(StrElipsis(pQuery.lastError().text(),200));
                        else
                            messageLabel->setText(StrElipsis(pQuery.lastQuery(),200)+"\n"
                                                  "Rows affected: "+str(pQuery.numRowsAffected()));
                    }
                } else {
                    messageLabel->setText(QObject::tr("Requête de modification de données incorrecte :")+"\n"+StrElipsis(QueryError,200));
                }
            } else {
                messageLabel->setText(QObject::tr("Requêtes de modification de données non autorisées dans le paramétrage."));
            }
        } else {
            if(pQuery.Select0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='SQL_schéma'")=="Oui!") {
                QString QueryError="";
                values=SQLEdit->toPlainText().split(";;\n");
                for(int i=0;i<values.count();i++){
                    if (!values[i].toUpper().startsWith("CREATE ")and
                        !values[i].toUpper().startsWith("ALTER ")and
                        !values[i].toUpper().startsWith("DROP ")){
                        QueryError=values[i];
                        break;
                    }
                }
                if (QueryError.isEmpty()) {
                    if(!pQuery.ExecMultiShowErr(SQLEdit->toPlainText(),";;\n",nullptr))
                        messageLabel->setText(StrElipsis(pQuery.lastError().text(),200));
                    else
                        messageLabel->setText(StrElipsis(pQuery.lastQuery(),200)+"\n"
                                              "Rows affected: "+str(pQuery.numRowsAffected()));
                } else {
                    messageLabel->setText(QObject::tr("Requête de modification du schéma de base de données incorrecte :")+"\n"+StrElipsis(QueryError,200));
                }
            } else {
                messageLabel->setText(QObject::tr("Requêtes de modification de données non autorisées dans le paramétrage."));
            }
        }
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        result=false;
        dialog.reject();
    });
    QObject::connect(SQLEdit, &QPlainTextEdit::cursorPositionChanged, [&]() {
        highlightParentheses(SQLEdit);
        lInfo->setText(selectionInfo(SQLEdit));
    });

    // Highlight initial si texte déjà présent
    highlightParentheses(SQLEdit);

    //Menu contextuel
    SQLEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(SQLEdit, &QPlainTextEdit::customContextMenuRequested, SQLEdit, [SQLEdit]() {
        QMenu menu;
        QAction* formatAction=menu.addAction(QObject::tr("Formater la requête SQL"));
        formatAction->setToolTip(QObject::tr("Des retours à la ligne sont injectés pour que les lignes ne dépassent pas 80 caractères.\n"
                                             "Mettez des espaces autour des opérateurs pour forcer un retour à la ligne:\n"
                                             "'Val1 + Val2' est scindé en 2 lignes, 'Val1+Val2' non.\n"
                                             "Mettez les opérateurs booléens en majuscules pour forcer un retour à la ligne:\n"
                                             "'Val1 AND Val2' est scindé en 2 lignes, 'Val1 and Val2' non."));
        QAbstractEventDispatcher::connect(formatAction, &QAction::hovered, &menu, [formatAction, &menu]() {
                QToolTip::showText(QCursor::pos(), formatAction->toolTip(), &menu);
        });
        QAction* openAction=menu.addAction(QObject::tr("Ouvrir un fichier SQL"));
        QAction* saveAction=menu.addAction(QObject::tr("Enregistrer dans un fichier SQL"));
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

    settings.beginGroup(titre);
    const auto geometry=settings.value("geometry").toByteArray();
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


QList<inputResult> inputDialog(const QString &titre, const QString &message,QList<inputStructure> inputs,
                               QString &buttons, QStyle::StandardPixmap iconType, const QString geometry) {
    QDialog dialog(nullptr);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout=new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel=new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon=QIcon(":/images/potaleger.svg");
        else
            icon=QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QLabel *messageLabel=new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QVBoxLayout *inputsLayout=new QVBoxLayout();
    QList<QHBoxLayout*> HInputLayoutList;
    QHBoxLayout *curHBoxLayout;
    int lastInputRigth=0;

    for (int i=0;i<inputs.count();i++) {
        if (HInputLayoutList.count()==0 or lastInputRigth>inputs[i].left) {
            if (HInputLayoutList.count()>0) curHBoxLayout->addStretch();
            QHBoxLayout *inputLayout=new QHBoxLayout();
            HInputLayoutList.append(inputLayout);
            inputsLayout->addLayout(inputLayout);
            lastInputRigth=0;
        }
        curHBoxLayout=HInputLayoutList[HInputLayoutList.count()-1]; //Last QHBoxLayout.

        if(inputs[i].type!="BOOL") {
            QLabel *inputLabel=new QLabel(inputs[i].label);
            if (inputs[i].labelWidth>-1)
                inputLabel->setFixedWidth(inputs[i].labelWidth);
            if (inputs[i].left>0)
                curHBoxLayout->addSpacing(inputs[i].left-lastInputRigth);
            curHBoxLayout->addWidget(inputLabel);
        }
        QWidget *inputWidget=new QWidget();

        QString autoToolTip;
        if(inputs[i].type=="INTEGER") {
            QSpinBox *input=new QSpinBox();
            QStringList sl=inputs[i].valDef.split('|');
            if (sl.count()>2) {
                input->setValue(sl[0].toInt());
                input->setMinimum(sl[1].toInt());
                input->setMaximum(sl[2].toInt());
                autoToolTip="\n\n"+QObject::tr("Min : ")+sl[1]+
                            "\n"+QObject::tr("Max : ")+sl[2];
            } else {
                input->setValue(inputs[i].valDef.toInt());
                input->setMinimum(-100000);
                input->setMaximum(100000);
            }
            inputWidget=input;
        } else if(inputs[i].type=="REAL") {
            QDoubleSpinBox *input=new QDoubleSpinBox();
            QStringList sl=inputs[i].valDef.split('|');
            if (sl.count()>2) {
                input->setValue(sl[0].toDouble());
                input->setMinimum(sl[1].toDouble());
                input->setMaximum(sl[2].toDouble());
                autoToolTip="\n\n"+QObject::tr("Min : ")+sl[1]+
                            "\n"+QObject::tr("Max : ")+sl[2];
            } else {
                input->setValue(inputs[i].valDef.toDouble());
                input->setMinimum(-100000);
                input->setMaximum(100000);
            }
            // QLocale l=input->locale();
            // l.decimalPoint()=".";
            // input->setLocale(l);
            inputWidget=input;
        } else if(inputs[i].type=="DATE") {
            QDateEdit *input=new QDateEdit();
            QStringList sl=inputs[i].valDef.split('|');
            if (sl.count()>2) {
                input->setDate(QDate::fromString(sl[0],"yyyy-MM-dd"));
                input->setMinimumDate(QDate::fromString(sl[1],"yyyy-MM-dd"));
                input->setMaximumDate(QDate::fromString(sl[2],"yyyy-MM-dd"));
                autoToolTip="\n\n"+QObject::tr("Min : ")+QDate::fromString(sl[1],"yyyy-MM-dd").toString("dd/MM/yyyy")+
                            "\n"+QObject::tr("Max : ")+QDate::fromString(sl[2],"yyyy-MM-dd").toString("dd/MM/yyyy");
            } else
                input->setDate(QDate::fromString(inputs[i].valDef,"yyyy-MM-dd"));
            inputWidget=input;
        } else if(inputs[i].type=="BOOL") {
            if (inputs[i].left>0)
                curHBoxLayout->addSpacing(inputs[i].left-lastInputRigth);
            QCheckBox *input=new QCheckBox(inputs[i].label);
            input->setChecked(inputs[i].valDef=="1" or inputs[i].valDef.toLower()=="true");
            inputWidget=input;
        } else {
            QStringList sl=inputs[i].valDef.split('|');
            if (sl.count()>1) {
                QComboBox *input=new QComboBox();
                input->addItems(sl);
                inputWidget=input;
            } else {
                QLineEdit *input=new QLineEdit();
                input->setText(inputs[i].valDef);
                inputWidget=input;
            }
        }
        inputWidget->setObjectName("vn_"+inputs[i].varName);
        inputWidget->setFixedWidth(inputs[i].inputWidth);
        inputWidget->setToolTip(inputs[i].toolTip+autoToolTip);
        curHBoxLayout->addWidget(inputWidget);
        lastInputRigth=inputs[i].left+inputs[i].labelWidth+inputs[i].inputWidth;
    }
    if (HInputLayoutList.count()>0) curHBoxLayout->addStretch();

    layout->addLayout(inputsLayout);

    QHBoxLayout *buttonLayout=new QHBoxLayout();
    QPushButton *prevButton=new QPushButton("<< "+QObject::tr("Précédent"));
    QPushButton *okButton=new QPushButton(QObject::tr("OK"));
    QPushButton *cancelButton=new QPushButton(QObject::tr("Annuler"));
    if (buttons=="NextCancel") {
        okButton->setText(QObject::tr("Suivant")+" >>");
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogYesButton));
        prevButton->setVisible(false);
    } else if (buttons=="PrevNextCancel") {
        okButton->setText(QObject::tr("Suivant")+" >>");
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogYesButton));
    } else if (buttons=="PrevFinishCancel") {
        okButton->setText(QObject::tr("Terminer"));
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
    } else {
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
        prevButton->setVisible(false);
    }
    cancelButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogCancelButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(prevButton);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addStretch();
    layout->addLayout(buttonLayout);

    QList<inputResult> result; // Valeur par défaut si annulé
    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        for (int i=0;i<HInputLayoutList.count();i++) {
            curHBoxLayout=HInputLayoutList[i];
            for (int j=0;j<curHBoxLayout->count();j++) {
                QLayoutItem *item = curHBoxLayout->itemAt(j);
                if (QWidget *widget = item->widget()) {
                    if (widget->objectName().startsWith("vn_")) {
                        if (QSpinBox *widget2 = qobject_cast<QSpinBox*>(widget))
                            result.append({widget2->objectName().mid(3),
                                           widget2->value()});
                        else if (QSpinBox *widget2 = qobject_cast<QSpinBox*>(widget))
                            result.append({widget2->objectName().mid(3),
                                           widget2->value()});
                        else if (QDoubleSpinBox *widget2 = qobject_cast<QDoubleSpinBox*>(widget))
                            result.append({widget2->objectName().mid(3),
                                           widget2->value()});
                        else if (QDateEdit *widget2 = qobject_cast<QDateEdit*>(widget))
                            result.append({widget2->objectName().mid(3),
                                           widget2->date()});
                        else if (QCheckBox *widget2 = qobject_cast<QCheckBox*>(widget))
                            result.append({widget2->objectName().mid(3),
                                           widget2->isChecked()});
                        else if (QComboBox *widget2 = qobject_cast<QComboBox*>(widget))
                            result.append({widget2->objectName().mid(3),
                                           widget2->currentText()});
                        else if (QLineEdit *widget2 = qobject_cast<QLineEdit*>(widget))
                            result.append({widget2->objectName().mid(3),
                                           widget2->text()});
                    }
                }
            }
        }
        buttons="ok";
        dialog.accept();
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        buttons="cancel";
        dialog.reject();
    });
    QObject::connect(prevButton, &QPushButton::clicked, [&]() {
        buttons="previous";
        dialog.reject();
    });

    setGeometry(&dialog,geometry);

    dialog.exec();

    dialog.deleteLater();

    return result;
}

void setGeometry(QDialog *dialog, QString sGeometry) {
    QStringList iGeometry=QString(sGeometry+"||||").split("|");
    int w=std::max(std::max(dialog->sizeHint().width(),iGeometry[0].toInt()),350);
    int h=std::max(std::max(dialog->sizeHint().height(),iGeometry[1].toInt()),150);
    int l=iGeometry[2].toInt();
    int t=iGeometry[3].toInt();
    dialog->setFixedSize(w,h);//User can't resize the window.
    if (l!=0 or t!=0) {
        dialog->setGeometry(l,t,w,h);
    }
}

void MessageDlg(const QString &titre, const QString &message, const QString &message2, QStyle::StandardPixmap iconType, const QString geometry)
{
    QDialog dialog(nullptr);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout=new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel=new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon=QIcon(":/images/potaleger.svg");
        else
            icon=QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap( 150, 64));
        iconLabel->setFixedSize(150,64);
        headerLayout->addWidget(iconLabel);
    }

    QString sMess=message;
    QString sMess2=message2;
    if (isDarkTheme()){
        sMess=StrReplace(sMess,"<a href","<a style=\"color: #7785ff\" href");
        sMess2=StrReplace(sMess2,"<a href","<a style=\"color: #7785ff\" href");
    }
    QLabel *messageLabel=new QLabel(sMess);
    //messageLabel->setWordWrap(true);
    messageLabel->setOpenExternalLinks(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    messageLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    headerLayout->addWidget(messageLabel);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    if (message2!=""){
        QScrollArea *scrollArea=new QScrollArea();
        //scrollArea->setWidgetResizable(true);

        QLabel *messageLabel2=new QLabel(sMess2);
        messageLabel2->setWordWrap(true);
        messageLabel2->setOpenExternalLinks(true);
        messageLabel2->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
        SetFontWeight(messageLabel2,QFont::Light);
        SetFontWeight(messageLabel,QFont::DemiBold);
        //messageLabel2->setFixedWidth(fmax(dialog.sizeHint().width(),MinWidth)-45);
        QSize screenSize=QGuiApplication::primaryScreen()->size();
        int maxHeight=screenSize.height()-200;
        scrollArea->setMaximumHeight(maxHeight);
        scrollArea->setWidget(messageLabel2);

        layout->addWidget(scrollArea);
    }

    QHBoxLayout *buttonLayout=new QHBoxLayout();
    QPushButton *okButton=new QPushButton(QObject::tr("OK"));
    okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    layout->addLayout(buttonLayout);

    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        dialog.accept();
    });

    setGeometry(&dialog,geometry);

    dialog.exec();

    dialog.deleteLater();
}

bool OkCancelDialog(const QString &titre, const QString &message, QString &buttons, QStyle::StandardPixmap iconType, const QString geometry)
{
    QDialog dialog(nullptr);//QApplication::activeWindow()
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout=new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel=new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon=QIcon(":/images/potaleger.svg");
        else
            icon=QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QLabel *messageLabel=new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QHBoxLayout *buttonLayout=new QHBoxLayout();

    QPushButton *prevButton=new QPushButton("<< "+QObject::tr("Précédent"));
    QPushButton *okButton=new QPushButton(QObject::tr("OK"));
    QPushButton *cancelButton=new QPushButton(QObject::tr("Annuler"));
    if (buttons=="NextCancel") {
        okButton->setText(QObject::tr("Suivant")+" >>");
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogYesButton));
        prevButton->setVisible(false);
    } else if (buttons=="PrevNextCancel") {
        okButton->setText(QObject::tr("Suivant")+" >>");
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogYesButton));
    } else if (buttons=="PrevFinishCancel") {
        okButton->setText(QObject::tr("Terminer"));
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
    } else {
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
        prevButton->setVisible(false);
    }
    cancelButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogCancelButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(prevButton);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    int result=false;
    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        result=true;
        buttons="ok";
        dialog.accept();
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        buttons="cancel";
        dialog.reject();
    });
    QObject::connect(prevButton, &QPushButton::clicked, [&]() {
        buttons="previous";
        dialog.reject();
    });

    setGeometry(&dialog,geometry);

    dialog.exec();

    dialog.deleteLater();

    return result;
}

int RadiobuttonDialog(const QString &titre, const QString &message, QString &buttons,
                      const QStringList &options, const int iDef, const QSet<int> disabledOptions,
                      QStyle::StandardPixmap iconType, const QString geometry) {
    QDialog dialog(nullptr);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout=new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel=new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon=QIcon(":/images/potaleger.svg");
        else
            icon=QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QLabel *messageLabel=new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QButtonGroup *buttonGroup=new QButtonGroup(&dialog);
    buttonGroup->setExclusive(true);

    QList<QRadioButton *> radioButtons;
    for (int i=0; i < options.size(); ++i) {
        QRadioButton *radioButton=new QRadioButton(options[i]);
        buttonGroup->addButton(radioButton, i);
        layout->addWidget(radioButton);
        radioButtons.append(radioButton);
        if (disabledOptions.contains(i))
            radioButton->setEnabled(false);
        else if (i==0 or i==iDef)
            radioButton->setChecked(true);
    }

    QHBoxLayout *buttonLayout=new QHBoxLayout();
    QPushButton *prevButton=new QPushButton("<< "+QObject::tr("Précédent"));
    QPushButton *okButton=new QPushButton(QObject::tr("OK"));
    QPushButton *cancelButton=new QPushButton(QObject::tr("Annuler"));
    if (buttons=="NextCancel") {
        okButton->setText(QObject::tr("Suivant")+" >>");
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogYesButton));
        prevButton->setVisible(false);
    } else if (buttons=="PrevNextCancel") {
        okButton->setText(QObject::tr("Suivant")+" >>");
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogYesButton));
    } else if (buttons=="PrevFinishCancel") {
        okButton->setText(QObject::tr("Terminer"));
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
    } else {
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
        prevButton->setVisible(false);
    }
    cancelButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogCancelButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(prevButton);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    int result=-1;
    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        result=buttonGroup->checkedId(); // Récupère l'ID du bouton sélectionné
        buttons="ok";
        dialog.accept();
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        buttons="cancel";
        dialog.reject();
    });
    QObject::connect(prevButton, &QPushButton::clicked, [&]() {
        buttons="previous";
        dialog.reject();
    });

    setGeometry(&dialog,geometry);

    dialog.exec();

    dialog.deleteLater();

    return result;
}

QList<inputResult> selectDialog(const QString &titre, const QString &message, QSqlDatabase db, QString varName, QString tableName, QString whereClose,
                                QProgressBar *progressBar, QLabel *lErr, QString &buttons, QStyle::StandardPixmap iconType, QString toolTip) {
    QList<inputResult> result;

    QDialog dialog(QApplication::activeWindow());
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout=new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel=new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon=QIcon(":/images/potaleger.svg");
        else
            icon=QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QLabel *messageLabel=new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QVBoxLayout *selectLayout=new QVBoxLayout();
    PotaWidget *w=new PotaWidget(&dialog);
    w->setObjectName("PW"+tableName);
    w->model->db=&db;

    if (!w->Init(titre,tableName,true,progressBar,lErr)) {
        result.append({"error",w->model->lastError().text()});
        return result;
    }
    if (!whereClose.isEmpty()) {
        w->scriptFilter=whereClose;
        w->pbFilterClick(false);
    }
    if (!toolTip.isEmpty()) {
        QToolButton *dialogToolTip=new QToolButton(w);
        dialogToolTip->setIcon(QIcon(":/images/help.svg"));
        dialogToolTip->setToolTip(toolTip);
        w->toolbar->layout()->addWidget(dialogToolTip);
    }

    selectLayout->addWidget(w);
    layout->addLayout(selectLayout);

    QHBoxLayout *buttonLayout=new QHBoxLayout();
    QPushButton *prevButton=new QPushButton("<< "+QObject::tr("Précédent"));
    QPushButton *okButton=new QPushButton(QObject::tr("OK"));
    QPushButton *cancelButton=new QPushButton(QObject::tr("Annuler"));
    if (buttons=="NextCancel") {
        okButton->setText(QObject::tr("Suivant")+" >>");
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogYesButton));
        prevButton->setVisible(false);
    } else if (buttons=="PrevNextCancel") {
        okButton->setText(QObject::tr("Suivant")+" >>");
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogYesButton));
    } else if (buttons=="PrevFinishCancel") {
        okButton->setText(QObject::tr("Terminer"));
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
    } else {
        okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
        prevButton->setVisible(false);
    }
    cancelButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogCancelButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(prevButton);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    QObject::connect(w->tv, &PotaTableView::doubleClicked, [&]() {
        okButton->clicked();
    });
    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        for (int i=0; i<w->model->columnCount();i++){
            result.append({varName+"_"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),
                           w->model->data(w->model->index(w->tv->currentIndex().row(),i),Qt::EditRole)});
        }
        result.append({varName+"_selectedColName",w->model->headerData(w->tv->currentIndex().column(),Qt::Horizontal,Qt::EditRole)});
        result.append({varName+"_selectedColValue",w->model->data(w->model->index(w->tv->currentIndex().row(),w->tv->currentIndex().column()),Qt::EditRole)});
        buttons="ok";
        dialog.accept();
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        buttons="cancel";
        dialog.reject();
    });
    QObject::connect(prevButton, &QPushButton::clicked, [&]() {
        buttons="previous";
        dialog.reject();
    });

    //dialog.setFixedSize(w,h);//User can't resize the window.
    //dialog.setMinimumWidth(fmax(dialog.sizeHint().width(),MinWidth));
    //dialog.setMinimumHeight(fmax(dialog.sizeHint().height(),150));
    QSettings settings;
    settings.beginGroup(titre);
    const auto geometry=settings.value("geometry").toByteArray();
    if (!geometry.isEmpty())
        dialog.restoreGeometry(geometry);
    settings.endGroup();

    //////////////
    dialog.exec();
    //////////////

    if (dialog.result()==QDialog::Accepted) {
        settings.beginGroup(titre);
        settings.setValue("geometry", dialog.saveGeometry());
        settings.endGroup();
    }

    return result;
}

bool YesNoDialog(const QString &titre, const QString &message, QStyle::StandardPixmap iconType, const QString geometry)
{
    QDialog dialog(nullptr);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout=new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel=new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon=QIcon(":/images/potaleger.svg");
        else
            icon=QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QLabel *messageLabel=new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QHBoxLayout *buttonLayout=new QHBoxLayout();
    QPushButton *yesButton=new QPushButton(QObject::tr("Oui"));
    QPushButton *noButton=new QPushButton(QObject::tr("Non"));
    yesButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogYesButton));
    noButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogNoButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(yesButton);
    buttonLayout->addWidget(noButton);
    layout->addLayout(buttonLayout);

    int result=false;
    QObject::connect(yesButton, &QPushButton::clicked, [&]() {
        result=true;
        dialog.accept();
    });
    QObject::connect(noButton, &QPushButton::clicked, [&]() {
        result=false;
        dialog.reject();
    });

    setGeometry(&dialog,geometry);

    dialog.exec();

    dialog.deleteLater();

    return result;
}

