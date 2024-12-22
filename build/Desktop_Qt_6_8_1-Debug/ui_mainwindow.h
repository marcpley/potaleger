/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *mSelecDB;
    QAction *mFamilles;
    QAction *mEspeces;
    QAction *mFournisseurs;
    QAction *mApports;
    QAction *mPlanches;
    QAction *mITP;
    QAction *mRotations;
    QAction *mDetailsRotations;
    QAction *mParam;
    QAction *mIlots;
    QAction *mCulturesParIlots;
    QAction *mCulturesParEspeces;
    QAction *mCulturesParPlanche;
    QAction *mCreerCultures;
    QAction *mSuccessionParPlanche;
    QAction *mSemences;
    QAction *mSemencesNecessaires;
    QAction *mVarietes;
    QAction *mCuSemisAFaire;
    QAction *mCuPlantationsAFaire;
    QAction *mCuRecoltesAFaire;
    QAction *mCuSaisieRecoltes;
    QAction *mCuATerminer;
    QAction *mCuToutes;
    QAction *mAnaITP;
    QAction *mAnaEspeces;
    QAction *mCuNonTer;
    QAction *mRafraichir;
    QAction *mFermerOnglet;
    QAction *mFermerOnglets;
    QAction *mValiderModifs;
    QAction *mAbandonnerModifs;
    QAction *mCreerBDD;
    QAction *mCreerBDDVide;
    QAction *mCopyBDD;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_3;
    QTabWidget *tabWidget;
    QWidget *Info;
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QLabel *lDB;
    QTextBrowser *tbInfoDB;
    QPlainTextEdit *pteNotes;
    QWidget *wData;
    QVBoxLayout *verticalLayout_2;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *toolButton;
    QToolButton *toolButton_2;
    QSpacerItem *horizontalSpacer;
    QTableView *tableView;
    QHBoxLayout *horizontalLayout;
    QLabel *lDBErr;
    QLabel *lVer;
    QLabel *lVerBDDAttendue;
    QMenuBar *menubar;
    QMenu *mFichiers;
    QMenu *mBaseData;
    QMenu *mAssolement;
    QMenu *mPlanif;
    QMenu *mCultures;
    QMenu *mAnalyses;
    QMenu *mEdition;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(862, 552);
        mSelecDB = new QAction(MainWindow);
        mSelecDB->setObjectName("mSelecDB");
        QIcon icon(QIcon::fromTheme(QIcon::ThemeIcon::FolderOpen));
        mSelecDB->setIcon(icon);
        mFamilles = new QAction(MainWindow);
        mFamilles->setObjectName("mFamilles");
        mFamilles->setEnabled(false);
        mEspeces = new QAction(MainWindow);
        mEspeces->setObjectName("mEspeces");
        mEspeces->setEnabled(false);
        mFournisseurs = new QAction(MainWindow);
        mFournisseurs->setObjectName("mFournisseurs");
        mFournisseurs->setEnabled(false);
        mApports = new QAction(MainWindow);
        mApports->setObjectName("mApports");
        mApports->setEnabled(false);
        mPlanches = new QAction(MainWindow);
        mPlanches->setObjectName("mPlanches");
        mITP = new QAction(MainWindow);
        mITP->setObjectName("mITP");
        mITP->setEnabled(false);
        mRotations = new QAction(MainWindow);
        mRotations->setObjectName("mRotations");
        mDetailsRotations = new QAction(MainWindow);
        mDetailsRotations->setObjectName("mDetailsRotations");
        mParam = new QAction(MainWindow);
        mParam->setObjectName("mParam");
        mParam->setChecked(false);
        mParam->setEnabled(false);
        QIcon icon1(QIcon::fromTheme(QString::fromUtf8("emblem-system")));
        mParam->setIcon(icon1);
        mIlots = new QAction(MainWindow);
        mIlots->setObjectName("mIlots");
        mCulturesParIlots = new QAction(MainWindow);
        mCulturesParIlots->setObjectName("mCulturesParIlots");
        mCulturesParEspeces = new QAction(MainWindow);
        mCulturesParEspeces->setObjectName("mCulturesParEspeces");
        mCulturesParPlanche = new QAction(MainWindow);
        mCulturesParPlanche->setObjectName("mCulturesParPlanche");
        mCreerCultures = new QAction(MainWindow);
        mCreerCultures->setObjectName("mCreerCultures");
        mSuccessionParPlanche = new QAction(MainWindow);
        mSuccessionParPlanche->setObjectName("mSuccessionParPlanche");
        mSemences = new QAction(MainWindow);
        mSemences->setObjectName("mSemences");
        mSemencesNecessaires = new QAction(MainWindow);
        mSemencesNecessaires->setObjectName("mSemencesNecessaires");
        mVarietes = new QAction(MainWindow);
        mVarietes->setObjectName("mVarietes");
        mVarietes->setEnabled(false);
        mCuSemisAFaire = new QAction(MainWindow);
        mCuSemisAFaire->setObjectName("mCuSemisAFaire");
        mCuPlantationsAFaire = new QAction(MainWindow);
        mCuPlantationsAFaire->setObjectName("mCuPlantationsAFaire");
        mCuRecoltesAFaire = new QAction(MainWindow);
        mCuRecoltesAFaire->setObjectName("mCuRecoltesAFaire");
        mCuSaisieRecoltes = new QAction(MainWindow);
        mCuSaisieRecoltes->setObjectName("mCuSaisieRecoltes");
        mCuATerminer = new QAction(MainWindow);
        mCuATerminer->setObjectName("mCuATerminer");
        mCuToutes = new QAction(MainWindow);
        mCuToutes->setObjectName("mCuToutes");
        mAnaITP = new QAction(MainWindow);
        mAnaITP->setObjectName("mAnaITP");
        mAnaEspeces = new QAction(MainWindow);
        mAnaEspeces->setObjectName("mAnaEspeces");
        mCuNonTer = new QAction(MainWindow);
        mCuNonTer->setObjectName("mCuNonTer");
        mRafraichir = new QAction(MainWindow);
        mRafraichir->setObjectName("mRafraichir");
        mRafraichir->setEnabled(false);
        QIcon icon2(QIcon::fromTheme(QIcon::ThemeIcon::ViewRefresh));
        mRafraichir->setIcon(icon2);
        mFermerOnglet = new QAction(MainWindow);
        mFermerOnglet->setObjectName("mFermerOnglet");
        mFermerOnglet->setEnabled(false);
        QIcon icon3(QIcon::fromTheme(QIcon::ThemeIcon::MailReplySender));
        mFermerOnglet->setIcon(icon3);
        mFermerOnglets = new QAction(MainWindow);
        mFermerOnglets->setObjectName("mFermerOnglets");
        mFermerOnglets->setEnabled(false);
        QIcon icon4(QIcon::fromTheme(QIcon::ThemeIcon::MailReplyAll));
        mFermerOnglets->setIcon(icon4);
        mValiderModifs = new QAction(MainWindow);
        mValiderModifs->setObjectName("mValiderModifs");
        mValiderModifs->setEnabled(false);
        QIcon icon5(QIcon::fromTheme(QString::fromUtf8("emblem-default")));
        mValiderModifs->setIcon(icon5);
        mAbandonnerModifs = new QAction(MainWindow);
        mAbandonnerModifs->setObjectName("mAbandonnerModifs");
        mAbandonnerModifs->setEnabled(false);
        QIcon icon6(QIcon::fromTheme(QString::fromUtf8("face-smirk")));
        mAbandonnerModifs->setIcon(icon6);
        mCreerBDD = new QAction(MainWindow);
        mCreerBDD->setObjectName("mCreerBDD");
        QIcon icon7(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew));
        mCreerBDD->setIcon(icon7);
        mCreerBDDVide = new QAction(MainWindow);
        mCreerBDDVide->setObjectName("mCreerBDDVide");
        mCreerBDDVide->setIcon(icon7);
        mCopyBDD = new QAction(MainWindow);
        mCopyBDD->setObjectName("mCopyBDD");
        mCopyBDD->setEnabled(false);
        QIcon icon8(QIcon::fromTheme(QIcon::ThemeIcon::EditCopy));
        mCopyBDD->setIcon(icon8);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_3 = new QVBoxLayout(centralwidget);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy);
        Info = new QWidget();
        Info->setObjectName("Info");
        verticalLayout = new QVBoxLayout(Info);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(2, 2, 2, 2);
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        label = new QLabel(Info);
        label->setObjectName("label");
        QPalette palette;
        QBrush brush(QColor(154, 153, 150, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        label->setPalette(palette);

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        lDB = new QLabel(Info);
        lDB->setObjectName("lDB");

        formLayout->setWidget(0, QFormLayout::FieldRole, lDB);

        tbInfoDB = new QTextBrowser(Info);
        tbInfoDB->setObjectName("tbInfoDB");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tbInfoDB->sizePolicy().hasHeightForWidth());
        tbInfoDB->setSizePolicy(sizePolicy1);
        QPalette palette1;
        QBrush brush1(QColor(255, 190, 111, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Text, brush1);
        palette1.setBrush(QPalette::Inactive, QPalette::Text, brush1);
        tbInfoDB->setPalette(palette1);
        QFont font;
        font.setItalic(true);
        tbInfoDB->setFont(font);

        formLayout->setWidget(1, QFormLayout::SpanningRole, tbInfoDB);

        pteNotes = new QPlainTextEdit(Info);
        pteNotes->setObjectName("pteNotes");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(pteNotes->sizePolicy().hasHeightForWidth());
        pteNotes->setSizePolicy(sizePolicy2);

        formLayout->setWidget(2, QFormLayout::SpanningRole, pteNotes);


        verticalLayout->addLayout(formLayout);

        tabWidget->addTab(Info, QString());
        wData = new QWidget();
        wData->setObjectName("wData");
        wData->setEnabled(true);
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(wData->sizePolicy().hasHeightForWidth());
        wData->setSizePolicy(sizePolicy3);
        verticalLayout_2 = new QVBoxLayout(wData);
        verticalLayout_2->setSpacing(2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(2, 2, 2, 2);
        widget = new QWidget(wData);
        widget->setObjectName("widget");
        sizePolicy3.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy3);
        horizontalLayout_2 = new QHBoxLayout(widget);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(2, 0, 2, 0);
        toolButton = new QToolButton(widget);
        toolButton->setObjectName("toolButton");
        toolButton->setIconSize(QSize(32, 32));

        horizontalLayout_2->addWidget(toolButton);

        toolButton_2 = new QToolButton(widget);
        toolButton_2->setObjectName("toolButton_2");

        horizontalLayout_2->addWidget(toolButton_2);

        horizontalSpacer = new QSpacerItem(741, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout_2->addWidget(widget);

        tableView = new QTableView(wData);
        tableView->setObjectName("tableView");
        sizePolicy2.setHeightForWidth(tableView->sizePolicy().hasHeightForWidth());
        tableView->setSizePolicy(sizePolicy2);

        verticalLayout_2->addWidget(tableView);

        tabWidget->addTab(wData, QString());

        verticalLayout_3->addWidget(tabWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        lDBErr = new QLabel(centralwidget);
        lDBErr->setObjectName("lDBErr");
        QPalette palette2;
        QBrush brush2(QColor(246, 97, 81, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette2.setBrush(QPalette::Active, QPalette::WindowText, brush2);
        palette2.setBrush(QPalette::Inactive, QPalette::WindowText, brush2);
        lDBErr->setPalette(palette2);
        lDBErr->setMargin(2);

        horizontalLayout->addWidget(lDBErr);

        lVer = new QLabel(centralwidget);
        lVer->setObjectName("lVer");
        QSizePolicy sizePolicy4(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(lVer->sizePolicy().hasHeightForWidth());
        lVer->setSizePolicy(sizePolicy4);
        lVer->setMargin(2);

        horizontalLayout->addWidget(lVer);

        lVerBDDAttendue = new QLabel(centralwidget);
        lVerBDDAttendue->setObjectName("lVerBDDAttendue");
        sizePolicy4.setHeightForWidth(lVerBDDAttendue->sizePolicy().hasHeightForWidth());
        lVerBDDAttendue->setSizePolicy(sizePolicy4);
        lVerBDDAttendue->setMargin(2);

        horizontalLayout->addWidget(lVerBDDAttendue);


        verticalLayout_3->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 862, 20));
        mFichiers = new QMenu(menubar);
        mFichiers->setObjectName("mFichiers");
        mBaseData = new QMenu(menubar);
        mBaseData->setObjectName("mBaseData");
        mAssolement = new QMenu(menubar);
        mAssolement->setObjectName("mAssolement");
        mPlanif = new QMenu(menubar);
        mPlanif->setObjectName("mPlanif");
        mCultures = new QMenu(menubar);
        mCultures->setObjectName("mCultures");
        mAnalyses = new QMenu(menubar);
        mAnalyses->setObjectName("mAnalyses");
        mEdition = new QMenu(menubar);
        mEdition->setObjectName("mEdition");
        MainWindow->setMenuBar(menubar);

        menubar->addAction(mFichiers->menuAction());
        menubar->addAction(mEdition->menuAction());
        menubar->addAction(mBaseData->menuAction());
        menubar->addAction(mAssolement->menuAction());
        menubar->addAction(mPlanif->menuAction());
        menubar->addAction(mCultures->menuAction());
        menubar->addAction(mAnalyses->menuAction());
        mFichiers->addAction(mSelecDB);
        mFichiers->addAction(mCopyBDD);
        mFichiers->addAction(mCreerBDD);
        mFichiers->addAction(mCreerBDDVide);
        mBaseData->addAction(mFamilles);
        mBaseData->addAction(mEspeces);
        mBaseData->addAction(mVarietes);
        mBaseData->addAction(mApports);
        mBaseData->addAction(mFournisseurs);
        mBaseData->addAction(mITP);
        mAssolement->addAction(mRotations);
        mAssolement->addAction(mDetailsRotations);
        mAssolement->addAction(mPlanches);
        mAssolement->addAction(mIlots);
        mAssolement->addAction(mSuccessionParPlanche);
        mPlanif->addAction(mCulturesParIlots);
        mPlanif->addAction(mCulturesParEspeces);
        mPlanif->addAction(mCulturesParPlanche);
        mPlanif->addAction(mCreerCultures);
        mPlanif->addAction(mSemencesNecessaires);
        mPlanif->addAction(mSemences);
        mCultures->addAction(mCuNonTer);
        mCultures->addSeparator();
        mCultures->addAction(mCuSemisAFaire);
        mCultures->addAction(mCuPlantationsAFaire);
        mCultures->addAction(mCuRecoltesAFaire);
        mCultures->addAction(mCuSaisieRecoltes);
        mCultures->addAction(mCuATerminer);
        mCultures->addSeparator();
        mCultures->addAction(mCuToutes);
        mAnalyses->addAction(mAnaITP);
        mAnalyses->addAction(mAnaEspeces);
        mEdition->addAction(mRafraichir);
        mEdition->addAction(mValiderModifs);
        mEdition->addAction(mAbandonnerModifs);
        mEdition->addAction(mFermerOnglet);
        mEdition->addAction(mFermerOnglets);
        mEdition->addAction(mParam);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Potal\303\251ger", nullptr));
        mSelecDB->setText(QCoreApplication::translate("MainWindow", "S\303\251lectionner une base de donn\303\251es", nullptr));
        mFamilles->setText(QCoreApplication::translate("MainWindow", "Familles botaniques", nullptr));
        mEspeces->setText(QCoreApplication::translate("MainWindow", "Esp\303\250ces", nullptr));
        mFournisseurs->setText(QCoreApplication::translate("MainWindow", "Fournisseurs", nullptr));
        mApports->setText(QCoreApplication::translate("MainWindow", "Apports", nullptr));
        mPlanches->setText(QCoreApplication::translate("MainWindow", "Planches", nullptr));
        mITP->setText(QCoreApplication::translate("MainWindow", "Itin\303\251raires techniques", nullptr));
        mRotations->setText(QCoreApplication::translate("MainWindow", "Rotations", nullptr));
        mDetailsRotations->setText(QCoreApplication::translate("MainWindow", "D\303\251tails des rotations", nullptr));
        mParam->setText(QCoreApplication::translate("MainWindow", "Param\303\250tres", nullptr));
        mIlots->setText(QCoreApplication::translate("MainWindow", "Ilots", nullptr));
        mCulturesParIlots->setText(QCoreApplication::translate("MainWindow", "Cultures pr\303\251vues par ilots", nullptr));
        mCulturesParEspeces->setText(QCoreApplication::translate("MainWindow", "Cultures pr\303\251vues par esp\303\250ces", nullptr));
        mCulturesParPlanche->setText(QCoreApplication::translate("MainWindow", "Cultures pr\303\251vues par planche", nullptr));
        mCreerCultures->setText(QCoreApplication::translate("MainWindow", "Cr\303\251er les cultures", nullptr));
        mSuccessionParPlanche->setText(QCoreApplication::translate("MainWindow", "Successions de cultures par planche", nullptr));
        mSemences->setText(QCoreApplication::translate("MainWindow", "Inventaire et commandes semences", nullptr));
        mSemencesNecessaires->setText(QCoreApplication::translate("MainWindow", "Semences n\303\251cessaires", nullptr));
        mVarietes->setText(QCoreApplication::translate("MainWindow", "Vari\303\251t\303\251s", nullptr));
        mCuSemisAFaire->setText(QCoreApplication::translate("MainWindow", "Semis \303\240 faire", nullptr));
        mCuPlantationsAFaire->setText(QCoreApplication::translate("MainWindow", "Plantations \303\240 faire", nullptr));
        mCuRecoltesAFaire->setText(QCoreApplication::translate("MainWindow", "R\303\251coltes \303\240 faire", nullptr));
        mCuSaisieRecoltes->setText(QCoreApplication::translate("MainWindow", "Saisie des r\303\251coltes", nullptr));
        mCuATerminer->setText(QCoreApplication::translate("MainWindow", "A terminer", nullptr));
        mCuToutes->setText(QCoreApplication::translate("MainWindow", "Toutes les cultures", nullptr));
        mAnaITP->setText(QCoreApplication::translate("MainWindow", "Itin\303\251raires techniques", nullptr));
        mAnaEspeces->setText(QCoreApplication::translate("MainWindow", "Esp\303\250ces", nullptr));
        mCuNonTer->setText(QCoreApplication::translate("MainWindow", "Toutes les cultures non termin\303\251es", nullptr));
        mRafraichir->setText(QCoreApplication::translate("MainWindow", "Rafraichir", nullptr));
#if QT_CONFIG(tooltip)
        mRafraichir->setToolTip(QCoreApplication::translate("MainWindow", "Rafraichir l'onglet courant", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        mRafraichir->setShortcut(QCoreApplication::translate("MainWindow", "F5", nullptr));
#endif // QT_CONFIG(shortcut)
        mFermerOnglet->setText(QCoreApplication::translate("MainWindow", "Fermer l'onglet courant", nullptr));
#if QT_CONFIG(shortcut)
        mFermerOnglet->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+W", nullptr));
#endif // QT_CONFIG(shortcut)
        mFermerOnglets->setText(QCoreApplication::translate("MainWindow", "Fermer tous les onglets", nullptr));
#if QT_CONFIG(shortcut)
        mFermerOnglets->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+W", nullptr));
#endif // QT_CONFIG(shortcut)
        mValiderModifs->setText(QCoreApplication::translate("MainWindow", "Valider les modifications", nullptr));
        mAbandonnerModifs->setText(QCoreApplication::translate("MainWindow", "Abandonner les modifications", nullptr));
        mCreerBDD->setText(QCoreApplication::translate("MainWindow", "Cr\303\251er une base de donn\303\251es avec les donn\303\251es de base ", nullptr));
        mCreerBDDVide->setText(QCoreApplication::translate("MainWindow", "Cr\303\251er une base de donn\303\251es vide", nullptr));
        mCopyBDD->setText(QCoreApplication::translate("MainWindow", "Faire une copie de  la base de donn\303\251es", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Base de donn\303\251e :", nullptr));
        lDB->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        tbInfoDB->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:10pt; font-weight:400; font-style:italic;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p></body></html>", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(Info), QCoreApplication::translate("MainWindow", "Info", nullptr));
        toolButton->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        toolButton_2->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(wData), QCoreApplication::translate("MainWindow", "Tab 2", nullptr));
        lVer->setText(QCoreApplication::translate("MainWindow", "lVer", nullptr));
        lVerBDDAttendue->setText(QCoreApplication::translate("MainWindow", "lVerBDDAttendue", nullptr));
        mFichiers->setTitle(QCoreApplication::translate("MainWindow", "Fichiers", nullptr));
        mBaseData->setTitle(QCoreApplication::translate("MainWindow", "Donn\303\251es de base", nullptr));
        mAssolement->setTitle(QCoreApplication::translate("MainWindow", "Assolement", nullptr));
        mPlanif->setTitle(QCoreApplication::translate("MainWindow", "Planification", nullptr));
        mCultures->setTitle(QCoreApplication::translate("MainWindow", "Cultures", nullptr));
        mAnalyses->setTitle(QCoreApplication::translate("MainWindow", "Analyses", nullptr));
        mEdition->setTitle(QCoreApplication::translate("MainWindow", "\303\211dition", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
