#include "Dialogs.h"
#include "data/Data.h"
#include "mainwindow.h"
#include "qdir.h"
#include "qsqldriver.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlError>
#include "QDebug"
#include "PotaUtils.h"
#include <QWindow>
#include <QStyleFactory>
//#include <QLocale>
//#include "qtimer.h"
#include "SQL/FunctionsSQLite.h"
#include "sqlite/sqlite3.h"
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QImage>

void MainWindow::SetEnabledDataMenuEntries(bool b)
{
    ui->mCopyBDD->setEnabled(b);
    ui->mUpdateSchema->setEnabled(b);
    ui->mParam->setEnabled(b);
    ui->mNotes->setEnabled(b);
    for (int i = 0; i < ui->mBaseData->actions().count(); i++)
        ui->mBaseData->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mAssolement->actions().count(); i++)
        ui->mAssolement->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mPlanif->actions().count(); i++)
        ui->mPlanif->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mCultures->actions().count(); i++)
        ui->mCultures->actions().at(i)->setEnabled(b);

    ui->mASemer->setEnabled(b);
    for (int i = 0; i < ui->mASemer->actions().count(); i++)
        ui->mASemer->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mFertilisation->actions().count(); i++)
        ui->mFertilisation->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mStock->actions().count(); i++)
        ui->mStock->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mAnalyses->actions().count(); i++)
        ui->mAnalyses->actions().at(i)->setEnabled(b);
}

bool MainWindow::dbOpen(QString sFichier, bool bNew, bool bResetSQLean, bool SetFkOn)
{
    qInfo() << "SQLite version:" << sqlite3_libversion();
    qInfo() << "DB file:" << sFichier;
    dbClose();

    if (!bNew) {
        QFile fBDD(sFichier);
        if (!fBDD.exists()) {
            SetColoredText(ui->lDBErr,tr("Le fichier de BDD n'existe pas.")+"\n"+
                                          sFichier,"Err");
            return false;
        }
    }

    //QSqlDatabase db = QSqlDatabase::database();
    db.setDatabaseName(sFichier);

    // qDebug() << "Transactions: " << db.driver()->hasFeature(QSqlDriver::Transactions);
    // qDebug() << "QuerySize: " << db.driver()->hasFeature(QSqlDriver::QuerySize);
    // qDebug() << "BLOB: " << db.driver()->hasFeature(QSqlDriver::BLOB);
    // qDebug() << "Unicode: " << db.driver()->hasFeature(QSqlDriver::Unicode);
    // qDebug() << "PreparedQueries: " << db.driver()->hasFeature(QSqlDriver::PreparedQueries);
    // qDebug() << "NamedPlaceholders: " << db.driver()->hasFeature(QSqlDriver::NamedPlaceholders);
    // qDebug() << "PositionalPlaceholders: " << db.driver()->hasFeature(QSqlDriver::PositionalPlaceholders);
    // qDebug() << "LastInsertId: " << db.driver()->hasFeature(QSqlDriver::LastInsertId);
    // qDebug() << "BatchOperations: " << db.driver()->hasFeature(QSqlDriver::BatchOperations);
    // qDebug() << "SimpleLocking: " << db.driver()->hasFeature(QSqlDriver::SimpleLocking);
    // qDebug() << "LowPrecisionNumbers: " << db.driver()->hasFeature(QSqlDriver::LowPrecisionNumbers);
    // qDebug() << "EventNotifications: " << db.driver()->hasFeature(QSqlDriver::EventNotifications);
    // qDebug() << "FinishQuery: " << db.driver()->hasFeature(QSqlDriver::FinishQuery);
    // qDebug() << "MultipleResultSets: " << db.driver()->hasFeature(QSqlDriver::MultipleResultSets);
    // qDebug() << "CancelQuery: " << db.driver()->hasFeature(QSqlDriver::CancelQuery);

    if (!db.open()) {
        dbClose();
        SetColoredText(ui->lDBErr, tr("Impossible d'ouvrir la base de données.")+"\n"+
                                       sFichier, "Err");
        return false;
    }

    PotaQuery query(db);
    if (!query.exec("PRAGMA journal_mode = WAL;")) {
        dbClose();
        SetColoredText(ui->lDBErr, tr("Impossible d'initialiser %1.").arg("SQLite journal_mode = WAL"), "Err");
        return false;
    }

    if (!query.exec("PRAGMA locking_mode = NORMAL;")) {
        dbClose();
        SetColoredText(ui->lDBErr, tr("Impossible d'initialiser %1.").arg("SQLite locking_mode = NORMAL"), "Err");
        return false;
    }

    // if (!query.exec("PRAGMA quick_check;") or //return false with POTACOLLATION.
    //     !query.next()) {
    //     dbClose();
    //     SetColoredText(ui->lDBErr, tr("Impossible d'initialiser %1.").arg("SQLite quick_check"), "Err");
    //     return false;
    // }

    if (SetFkOn and !query.exec("PRAGMA foreign_keys = ON")) {
        dbClose();
        SetColoredText(ui->lDBErr, "Foreign keys : Err", "Err");
        return false;
    }

    if (true) {
        if (bResetSQLean and !query.exec("DELETE FROM sqlean_define")) {//"SELECT define_free('<function_name>')" don't work.
            dbClose();
            SetColoredText(ui->lDBErr, tr("Impossible d'effacer les fonctions %1.").arg("SQLean"), "Err");
            return false;
        }
        if (!initSQLean(&db)) {
            dbClose();
            SetColoredText(ui->lDBErr, tr("Impossible d'initialiser %1.").arg("SQLean"), "Err");
            return false;
        }

        // QString s=testCustomFunctions();
        // if (!s.isEmpty()){
        //     dbClose();
        //     SetColoredText(ui->lDBErr, tr("La fonction %1 ne fonctionne pas.").arg(s), "Err");
        //     return false;
        // }
    }

    if (!registerPotaCollation(db)) {
        dbClose();
        SetColoredText(ui->lDBErr, "Collation : Err", "Err");
        return false;
    }

    if (!registerRemoveAccentsFunction(db)) {
        dbClose();
        SetColoredText(ui->lDBErr, "RemoveAccents : Err", "Err");
        return false;
    }


    return true;
}

void MainWindow::dbClose()
{
    //QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()){
        PotaQuery q1(db);
        q1.exec("SELECT define_free();");

        db.close();
    }
}

bool MainWindow::PotaDbOpen(QString sFichier, QString sNew,bool bUpdate)
{
    //userDataEditing=false;
    ReadOnlyDb=true;

    if (!dbOpen(sFichier,(sNew!=""),false,true))
        return false;

    PotaQuery pQuery(db);
    pQuery.lErr = ui->lDBErr;
    ui->lDBErr->clear();
    ui->lDB->setText(sFichier);

    bool result=true;

    QString sVerBDD = "";
    if (sNew==""){//Vérifier une BDD existante.
        if (pQuery.ExecShowErr("SELECT Valeur FROM Info_Potaléger WHERE N=1")) {//Si la vue Info n'existe pas ou pas correcte, on tente pas de mettre cette BDD à jour.
            pQuery.next();
            sVerBDD = pQuery.value(0).toString();
        } else {
            MessageDlg("Potaléger "+ui->lVer->text(),tr("Cette BDD n'est pas une BDD %1.").arg("Potaléger"),
                          sFichier,QStyle::SP_MessageBoxCritical,600);
            dbClose();
            result=false;
        }
        // } else  if(OkCancelDialog("Potaléger "+ui->lVer->text(),
        //                           tr("Cette BDD ne semble pas être une BDD %1.").arg("Potaléger")+"\n"+
        //                           sFichier+"\n\n"+
        //                           tr("Tenter quand même de la mettre à jour ?"),QStyle::SP_MessageBoxCritical,600)) {
        //     sVerBDD=DbVersion;
        //     bUpdate=true;
        // } else {
        //     dbClose();
        //     result=false;
        // }

        if (result and (sVerBDD < "2024-12-30")) {
            MessageDlg("Potaléger "+ui->lVer->text(),tr("La version de cette BDD %1 est trop ancienne: ").arg("Potaléger")+sVerBDD,
                          sFichier,QStyle::SP_MessageBoxCritical,600);
            dbClose();
            result=false;
        }

        if (result and (sVerBDD > DbVersion)) {
            MessageDlg("Potaléger "+ui->lVer->text(),tr("La version de cette BDD est trop %1, vous ne pouvez pas la modifier et\n"
                             "certains onglets peuvent ne pas fonctionner.").arg("récente")+"\n\n"+
                          sFichier+"\n"+
                          tr("Version de la BDD: %1").arg(sVerBDD)+"\n"+
                          tr("Version attendue: %1").arg(DbVersion)+"\n\n"+
                          tr("Vous devriez désinstaller %1 et intaller une version plus récente.").arg("Potaléger"),"",
                          QStyle::SP_MessageBoxWarning,600);
            // dbClose();
            // return false;
        } else if (result and (bUpdate or (sVerBDD != DbVersion))) {
            if (bUpdate or
                YesNoDialog("Potaléger "+ui->lVer->text(),tr("Base de données trop ancienne.")+"\n\n"+
                               sFichier+"\n"+
                               tr("Version de la BDD: %1").arg(sVerBDD)+"\n"+
                               tr("Version attendue: %1").arg(DbVersion)+"\n\n"+
                               tr("Mettre à jour cette BDD vers la version %1 ?").arg(DbVersion),
                               QStyle::SP_MessageBoxQuestion,600)) {   //Mettre à jour la BDD.
                //Delete previous backup file.
                QFile FileInfo;
                QString FileName=ui->lDB->text(); //Pourquoi pas sFichier ?
                FileInfo.setFileName(FileName+"-backup");
                if (FileInfo.exists()) {
                    if (!FileInfo.remove()) {
                        MessageDlg("Potaléger "+ui->lVer->text(),tr("Impossible de supprimer le fichier")+"\n"+
                                          FileName+"-backup","",QStyle::SP_MessageBoxCritical,600);
                        dbClose();
                        result=false;
                    }
                }
                if (result) {
                    //Backup.
                    FileInfo.setFileName(FileName);
                    if (!FileInfo.copy(FileName+"-backup"))  {
                        MessageDlg("Potaléger "+ui->lVer->text(),tr("Impossible de copier le fichier")+"\n"+
                                          FileName+"\n"+
                                          tr("vers le fichier")+"\n"+
                                          FileName+"-backup","",QStyle::SP_MessageBoxCritical,600);
                        dbClose();
                        return false;
                    }

                    //Update schema.
                    if (UpdateDBShema(sVerBDD)) {
                        sVerBDD = DbVersion;
                        ReadOnlyDb=false;

                        //Delete backup file.
                        FileInfo.setFileName(FileName+"-backup");
                        if (!FileInfo.remove()) {
                            MessageDlg("Potaléger "+ui->lVer->text(),tr("Impossible de supprimer le fichier")+"\n"+
                                              FileName+"-backup","",QStyle::SP_MessageBoxWarning,600);
                        }
                    } else {
                        dbClose();

                        //Restore old db file.
                        FileInfo.setFileName(FileName+"-crashed");
                        if (FileInfo.exists())
                            FileInfo.remove();

                        FileInfo.setFileName(FileName);
                        FileInfo.copy(FileName+"-crashed");
                        FileInfo.remove();
                        FileInfo.setFileName(FileName+"-backup");
                        if (FileInfo.copy(FileName))
                            MessageDlg("Potaléger "+ui->lVer->text(),tr("Le fichier")+"\n"+
                                              FileName+"\n"+
                                              tr("n'a pas été modifié."),"",QStyle::SP_MessageBoxInformation,600);
                        else
                            MessageDlg("Potaléger "+ui->lVer->text(),tr("Impossible de copier le fichier")+"\n"+
                                              FileName+"-backup\n"+
                                              tr("vers le fichier")+"\n"+
                                              FileName,"",QStyle::SP_MessageBoxCritical,600);

                        result=false;
                    }
                }
            } else {
                MessageDlg("Potaléger "+ui->lVer->text(),
                              tr("La version de cette BDD est trop %1, vous ne pouvez pas\n"
                                 "la modifier et certains onglets peuvent ne pas fonctionner.").arg("ancienne")+"\n\n"+
                              sFichier+"\n"+
                              tr("Version de la BDD: %1").arg(sVerBDD)+"\n"+
                              tr("Version attendue: %1").arg(DbVersion),"",QStyle::SP_MessageBoxWarning,600);

            }
        } else if (result) {
            ReadOnlyDb=false;
        }

        // if (sVerBDD != ui->lVerBDDAttendue->text()) {
        //     MessageDlg(tr("La version de cette BDD est incorrecte: ")+sVerBDD,
        //                   sFichier,QStyle::SP_MessageBoxCritical,600);
        //     dbClose();
        //     return false;
        // }
    } else if (result and UpdateDBShema(sNew)) {
        sVerBDD=DbVersion;
        ReadOnlyDb=false;
    } else {
        dbClose();
        result=false;
    }

    // if (result and !pQuery.ExecMultiShowErr("CREATE TEMP TABLE LocParams (Paramètre TEXT PRIMARY KEY, Valeur TEXT);"
    //                                         "INSERT INTO LocParams VALUES ('Saison',(SELECT Valeur FROM Params WHERE Paramètre='Année_culture'))",";",nullptr)) {
    //     dbClose();
    //     result=false;
    // }

    if (result) {
        setWindowTitle("Potaléger"+pQuery.Selec0ShowErr("SELECT ' - '||Valeur FROM Params WHERE Paramètre='Utilisateur'").toString());

        //Activer les menus
        SetEnabledDataMenuEntries(true);
        ui->lDBErr->clear();
    }


    //dbSuspend(&db,true,userDataEditing,ui->lDBErr);

    QFileInfo file(sFichier);
    QFile imgFile(file.absolutePath()+QDir::toNativeSeparators("/imgtab1.png"));
    if (result and imgFile.exists()) {
        QPixmap pixmap(imgFile.fileName());
        ui->graphicsView->setImage(pixmap);
    } else {
        imgFile.setFileName(QApplication::applicationDirPath()+QDir::toNativeSeparators("/infotab1.png"));
        if (imgFile.exists()) {
            //Image on first tab.
            QPixmap pixmap(imgFile.fileName());
            ui->graphicsView->setImage(pixmap);
        } else {
            // ui->graphicsView->setVisible(false);
            QGraphicsScene *scene = new QGraphicsScene(this);
            QPixmap pixmap(700,40);
            pixmap.fill(Qt::transparent);
            QColor cPen=QColor();
            if (!isDarkTheme())
                cPen=QColor("#000000");
            else
                cPen=QColor("#ffffff");
            cPen=QApplication::palette().color(QPalette::WindowText);
            QPainter painter(&pixmap);
            painter.setPen(cPen);
            QFont font( "Arial", 10); //, QFont::Bold
            painter.setFont(font);

            QRect rect(5, 2, 690, 15);
            painter.drawText(rect, Qt::AlignLeft,tr("Fichier non trouvé :"));
            rect.setRect(5, 17, 690, 15);
            painter.drawText(rect, Qt::AlignLeft,QApplication::applicationDirPath()+QDir::toNativeSeparators("/infotab1.png"));
            scene->addPixmap(pixmap);
            ui->graphicsView->setScene(scene);
        }
    }

    QFile mdFile(file.absolutePath()+QDir::toNativeSeparators("/texttab1.md"));
    if (result and mdFile.exists()) {
        mdFile.open(QFile::ReadOnly);
        ui->pteNotes->setMarkdown(mdFile.readAll());
    } else {
        mdFile.setFileName(QApplication::applicationDirPath()+QDir::toNativeSeparators("/readme.md"));
        if (mdFile.exists()) {
            mdFile.open(QFile::ReadOnly);
            ui->pteNotes->setMarkdown(mdFile.readAll());
        } else
            ui->pteNotes->setPlainText(tr("Fichiers non trouvés :")+"\n"+
                                       file.absolutePath()+QDir::toNativeSeparators("/texttab1.md")+"\n"+
                                       QApplication::applicationDirPath()+QDir::toNativeSeparators("/readme.md"));
    }

    if (result) {
        if (ReadOnlyDb)
            SetColoredText(ui->lDBErr, tr("Base de données en lecture seule (%1).").arg(sVerBDD), "Info");
        else
            SetColoredText(ui->lDBErr, tr("Base de données ouverte."), "Ok");
    }

    return result;
}

void MainWindow::PotaDbClose()
{
    ClosePotaTabs();
    SetEnabledDataMenuEntries(false);

    dbClose();
    //userDataEditing=false;
    ui->lDB->clear();
    ui->lDBErr->clear();
}

void MainWindow::RestaureParams()
{
    QSettings settings("greli.net", "Potaléger");
    settings.beginGroup("MainWindow");
    const auto geometry = settings.value("geometry").toByteArray();
    if (geometry.isEmpty())
        setGeometry(50, 50, 800, 600);
    else
        restoreGeometry(geometry);
    settings.endGroup();

    if (settings.value("theme").toString()=="dark")
        ui->cbTheme->setCurrentIndex(1);
    else
        ui->cbTheme->setCurrentIndex(0);

    if (settings.value("font").toInt()>=8 and settings.value("font").toInt()<=16)
        ui->cbFont->setCurrentText(str(settings.value("font").toInt()));
    else
        ui->cbFont->setCurrentText("10");

    if (settings.value("database_path").toString().isEmpty() or
        !PotaDbOpen(settings.value("database_path").toString(),"",false)) {
        int choice = RadiobuttonDialog("Potaléger "+ui->lVer->text(),
                                        tr("%1 stoque ses données dans un fichier unique à l'emplacement de votre choix.").arg("Potaléger"),
                                       {tr("Sélectionner une base de données existante"),
                                        tr("Créer une BDD avec les données de base"),
                                        tr("Créer une BDD vide")},1,QStyle::NStandardPixmap);
        if (choice==0)
            on_mSelecDB_triggered();
        else if (choice==1)
            on_mCreateDB_triggered();
        else if (choice==2)
            on_mCreateEmptyDB_triggered();
    }
     // else
     //    PotaDbOpen(settings.value("database_path").toString(),"",false);

    PathExport=settings.value("PathExport").toString();
    PathImport=settings.value("PathImport").toString();
    TypeImport=settings.value("TypeImport").toInt();

}

void MainWindow::SauvParams()
{
    QSettings settings("greli.net", "Potaléger");
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();

    if (ui->cbTheme->currentIndex()==1)
        settings.setValue("theme","dark");
    else
        settings.setValue("theme","");

    settings.setValue("font",ui->cbFont->currentText());

    if (!ui->lDB->text().isEmpty()) {
        QFile file(ui->lDB->text());
        if (file.exists())
            settings.setValue("database_path", ui->lDB->text());
    }

    settings.setValue("PathExport",PathExport);
    settings.setValue("PathImport",PathImport);
    settings.setValue("TypeImport",TypeImport);
}

void MainWindow::SetUi(){

    //QLocale customLocale(QLocale::French, QLocale::France);
    //customLocale.setNumberOptions(QLocale::RejectGroupSeparator);
    //customLocale.setNumberOptions(QLocale::OmitGroupSeparator);
    //QLocale::setDefault(customLocale);
    //std::setlocale(LC_NUMERIC, "C");

    ui->progressBar->setVisible(false);
    ui->progressBar->setMinimumSize(300,ui->progressBar->height());
    ui->tabWidget->widget(1)->deleteLater();//Used at UI design time.
    ui->Info->setLayout(ui->verticalLayout);

    ui->cbTheme->addItem(tr("clair"));
    ui->cbTheme->addItem(tr("sombre"));

    ui->cbFont->addItem(tr("8"));
    ui->cbFont->addItem(tr("9"));
    ui->cbFont->addItem(tr("10"));
    ui->cbFont->addItem(tr("12"));
    ui->cbFont->addItem(tr("14"));
    ui->cbFont->addItem(tr("16"));

    RestaureParams();

    MarkdownFont=false;

    SetMenuIcons();


    if (false) {
        QPalette palette = QApplication::palette();

        QList<QPalette::ColorRole> roles = {
            QPalette::Window, QPalette::WindowText, QPalette::Base, QPalette::AlternateBase,
            QPalette::ToolTipBase, QPalette::ToolTipText, QPalette::Text, QPalette::Button,
            QPalette::ButtonText, QPalette::BrightText, QPalette::Light, QPalette::Midlight,
            QPalette::Dark, QPalette::Mid, QPalette::Shadow, QPalette::Highlight, QPalette::HighlightedText
        };

        QList<QPalette::ColorGroup> groups = {QPalette::Active,QPalette::Inactive,QPalette::Disabled};

        for (auto group : groups) {
            qDebug() << "        // " << group;
            for (auto role : roles) {
                QColor color = palette.color(group, role);
                qDebug() << "        palette.setColor(" << group << "," << role << ",QColor(" << color.name() << "));";
            }
        }
    }
}

void MainWindow::SetMenuIcons() {
    ui->mNotes->setIcon(QIcon(TablePixmap("Notes","T")));

    ui->mFamilles->setIcon(QIcon(TablePixmap("Familles","T")));
    ui->mEspecesA->setIcon(QIcon(TablePixmap("Espèces","T")));
    ui->mEspecesV->setIcon(QIcon(TablePixmap("Espèces","T")));
    ui->mVarietes->setIcon(QIcon(TablePixmap("Variétés","T")));
    //ui->mApports->setIcon(QIcon(TablePixmap("Apports","T")));
    ui->mFournisseurs->setIcon(QIcon(TablePixmap("Fournisseurs","T")));
    // ui->mTypes_de_planche->setIcon(QIcon(TablePixmap("Types_planche","T")));
    ui->mITPTempo->setIcon(QIcon(TablePixmap("ITP__Tempo","T")));

    ui->mRotations->setIcon(QIcon(TablePixmap("Rotations","T")));
    ui->mDetailsRotations->setIcon(QIcon(TablePixmap("Rotations_détails__Tempo","T")));
    ui->mRotationManquants->setIcon(QIcon(TablePixmap("Espèces__manquantes","")));
    ui->mPlanches->setIcon(QIcon(TablePixmap("Planches","T")));
    //ui->mSuccessionParPlanche->setIcon(QIcon(TablePixmap("Successions_par_planche","")));
    ui->mSuccessionParPlanche->setIcon(QIcon(TablePixmap("Cultures__Tempo","")));
    ui->mIlots->setIcon(QIcon(TablePixmap("Planches_Ilots","")));

    ui->mCulturesParIlots->setIcon(QIcon(TablePixmap("Cult_planif_ilots","")));
    ui->mCulturesParplante->setIcon(QIcon(TablePixmap("Cult_planif_espèces","")));
    ui->mCulturesParPlanche->setIcon(QIcon(TablePixmap("Cult_planif","")));
    ui->mSemences->setIcon(QIcon(TablePixmap("Variétés__inv_et_cde","")));

    ui->mCuNonTer->setIcon(QIcon(TablePixmap("Cultures__non_terminées","")));
    ui->mCouverture->setIcon(QIcon(TablePixmap("Espèces__couverture","")));
    ui->mCuASemer->setIcon(QIcon(TablePixmap("Cultures__à_semer","")));
    ui->mCuASemerPep->setIcon(QIcon(TablePixmap("Cultures__à_semer_pep","")));
    ui->mCuASemerEP->setIcon(QIcon(TablePixmap("Cultures__à_semer_EP","")));
    ui->mCuAPlanter->setIcon(QIcon(TablePixmap("Cultures__à_planter","")));
    ui->mCuAIrriguer->setIcon(QIcon(TablePixmap("Cultures__à_irriguer","")));
    ui->mCuARecolter->setIcon(QIcon(TablePixmap("Cultures__à_récolter","")));
    ui->mCuSaisieRecoltes->setIcon(QIcon(TablePixmap("Récoltes__Saisies","T")));
    ui->mCuATerminer->setIcon(QIcon(TablePixmap("Cultures__à_terminer","")));
    ui->mCuAFaire->setIcon(QIcon(TablePixmap("Cultures__A_faire","")));
    ui->mCuVivaces->setIcon(QIcon(TablePixmap("Cultures__vivaces","")));
    ui->mCuToutes->setIcon(QIcon(TablePixmap("Cultures","T")));

    ui->mAnalysesSol->setIcon(QIcon(TablePixmap("Analyses_de_sol","T")));
    ui->mFertilisants->setIcon(QIcon(TablePixmap("Fertilisants","T")));
    ui->mInventaireFert->setIcon(QIcon(TablePixmap("Fertilisants__inventaire","")));
    ui->mCuAFertiliser->setIcon(QIcon(TablePixmap("Cultures__à_fertiliser","")));
    ui->mFertilisations->setIcon(QIcon(TablePixmap("Fertilisations__Saisies","T")));
    ui->mBilanPlanches->setIcon(QIcon(TablePixmap("Planches__bilan_fert","")));
    ui->mPlanchesDeficit->setIcon(QIcon(TablePixmap("Planches__deficit_fert","")));

    ui->mDestinations->setIcon(QIcon(TablePixmap("Destinations","T")));
    ui->mEsSaisieSorties->setIcon(QIcon(TablePixmap("Consommations__Saisies","T")));
    ui->mInventaire->setIcon(QIcon(TablePixmap("Espèces__inventaire","")));

    ui->mAnaITPA->setIcon(QIcon(TablePixmap("ITP__analyse_a","")));
    ui->mAnaITPV->setIcon(QIcon(TablePixmap("ITP__analyse_v","")));
    ui->mAnaCultures->setIcon(QIcon(TablePixmap("Cultures__analyse","")));
    ui->mIncDatesCultures->setIcon(QIcon(TablePixmap("Cultures__inc_dates","")));
    ui->mRequeteSQL->setIcon(QIcon(TablePixmap("","")));
}

void MainWindow::showIfDdOpen() {
}

void MainWindow::showEvent(QShowEvent *){
    // QTimer *dbTimer = new QTimer(this);
    // connect(dbTimer, &QTimer::timeout, this, &MainWindow::showIfDdOpen);
    // dbTimer->start(1000);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    SauvParams();
    PotaDbClose();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale(), "qt", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        a.installTranslator(&qtTranslator);
    }

    MainWindow w;
    //w.db = NEW;
    //w.db.addDatabase("QSQLITE");
    w.SetUi();

    w.show();
    return a.exec();

}

