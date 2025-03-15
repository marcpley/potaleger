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
#include <QLocale>
//#include "qtimer.h"
#include "SQL/FunctionsSQLite.h"

void MainWindow::SetEnabledDataMenuEntries(bool b)
{
    ui->mCopyBDD->setEnabled(b);
    ui->mUpdateSchema->setEnabled(b);
    ui->mParam->setEnabled(b);
    for (int i = 0; i < ui->mBaseData->actions().count(); i++)
        ui->mBaseData->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mAssolement->actions().count(); i++)
        ui->mAssolement->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mPlanif->actions().count(); i++)
        ui->mPlanif->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mCultures->actions().count(); i++)
        ui->mCultures->actions().at(i)->setEnabled(b);

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
    if (!query.exec("PRAGMA journal_mode = WAL;") or //query.exec("PRAGMA journal_mode=DELETE;");
        !query.exec("PRAGMA locking_mode = NORMAL;") or
        !query.exec("PRAGMA quick_check;") or
        !query.next()) {
        dbClose();
        SetColoredText(ui->lDBErr, tr("Impossible d'initialiser %1.").arg("SQLite"), "Err");
        return false;
    }

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

    if (!dbOpen(sFichier,(sNew!=""),false,true))
        return false;

    PotaQuery pQuery(db);
    pQuery.lErr = ui->lDBErr;
    ui->lDBErr->clear();
    ui->lDB->setText(sFichier);

    if (sNew==""){//Vérifier une BDD existante.
        QString sVerBDD = "";
        if (pQuery.ExecShowErr("SELECT Valeur FROM Info_Potaléger WHERE N=1")) {//Si la vue Info n'existe pas ou pas correcte, on tente pas de mettre cette BDD à jour.
            pQuery.next();
            sVerBDD = pQuery.value(0).toString();
        }
        else if (bUpdate)
            sVerBDD = ui->lVerBDDAttendue->text();
        else {
            MessageDialog(tr("Cette BDD n'est pas une BDD Potaléger."),
                          sFichier,QStyle::SP_MessageBoxCritical);
            //ui->tbInfoDB->append(tr("Cette BDD n'est pas une BDD Potaléger."));
            dbClose();
            return false;
        }

        if (sVerBDD < "2024-12-30") {
            MessageDialog(tr("La version de cette BDD Potaléger est trop ancienne: ")+sVerBDD,
                          sFichier,QStyle::SP_MessageBoxCritical);
            //ui->tbInfoDB->append(tr("La version de cette BDD Potaléger est trop ancienne: ")+sVerBDD);
            dbClose();
            return false;
        }

        if (sVerBDD > ui->lVerBDDAttendue->text()) {
            MessageDialog(tr("La version de cette BDD est trop récente: ")+sVerBDD+"\n\n"+
                          tr("Utilisez une version plus récente de Potaléger."),
                          sFichier,QStyle::SP_MessageBoxCritical);
            // ui->tbInfoDB->append(tr("La version de cette BDD est trop récente: ")+sVerBDD);
            // ui->tbInfoDB->append("-> "+tr("Utilisez une version plus récente de Potaléger."));
            dbClose();
            return false;
        }

        if (bUpdate or
            ((sVerBDD != ui->lVerBDDAttendue->text())and
             (OkCancelDialog(tr("Base de données trop ancienne:")+"\n"+
                             ui->lDB->text()+"\n" +
                             tr("Version ")+sVerBDD + "\n\n" +
                             tr("Mettre à jour cette BDD vers la version %1 ?").arg(ui->lVerBDDAttendue->text())+" ?",QStyle::SP_MessageBoxQuestion)))) {   //Mettre à jour la BDD.
            //Delete previous backup file.
            QFile FileInfo;
            QString FileName=ui->lDB->text();
            FileInfo.setFileName(FileName+"-backup");
            if (FileInfo.exists()) {
                if (!FileInfo.moveToTrash()) {
                    MessageDialog(tr("Impossible de supprimer le fichier")+"\n"+
                                      FileName+"-backup","",QStyle::SP_MessageBoxCritical);
                    dbClose();
                    return false;
                }
            }
            //Backup.
            FileInfo.setFileName(FileName);
            if (!FileInfo.copy(FileName+"-backup"))  {
                MessageDialog(tr("Impossible de copier le fichier")+"\n"+
                                  FileName+"\n"+
                                  tr("vers le fichier")+"\n"+
                                  FileName+"-backup","",QStyle::SP_MessageBoxCritical);
                dbClose();
                return false;
            }

            //Update schema.
            if (UpdateDBShema(sVerBDD)) {
                sVerBDD = ui->lVerBDDAttendue->text();
            }
            else {
                dbClose();

                //Restore old db file.
                FileInfo.setFileName(FileName+"-crashed");
                if (FileInfo.exists())
                    FileInfo.moveToTrash();

                FileInfo.setFileName(FileName);
                FileInfo.copy(FileName+"-crashed");
                FileInfo.moveToTrash();
                FileInfo.setFileName(FileName+"-backup");
                if (FileInfo.copy(FileName))
                    MessageDialog(tr("Le fichier")+"\n"+
                                      FileName+"\n"+
                                      tr("n'a pas été modifié."),"",QStyle::SP_MessageBoxInformation);
                else
                    MessageDialog(tr("Impossible de copier le fichier")+"\n"+
                                      FileName+"-backup\n"+
                                      tr("vers le fichier")+"\n"+
                                      FileName,"",QStyle::SP_MessageBoxCritical);

                return false;
            }
        }

        if (sVerBDD != ui->lVerBDDAttendue->text()) {
            MessageDialog(tr("La version de cette BDD est incorrecte: ")+sVerBDD,
                          sFichier,QStyle::SP_MessageBoxCritical);
            // ui->tbInfoDB->append(tr("La version de cette BDD est incorrecte: ")+sVerBDD);
            dbClose();
            return false;
        }
    }
    else if (!UpdateDBShema(sNew)) {
        dbClose();
        return false;
    }

    setWindowTitle("Potaléger"+pQuery.Selec0ShowErr("SELECT ' - '||Valeur FROM Params WHERE Paramètre='Utilisateur'").toString());

    //Activer les menus
    SetEnabledDataMenuEntries(true);
    ui->lDBErr->clear();

   //dbSuspend(&db,true,userDataEditing,ui->lDBErr);

    SetColoredText(ui->lDBErr, tr("Base de données ouverte."), "Ok");

    return true;
}

void MainWindow::PotaDbClose()
{
    ClosePotaTabs();
    SetEnabledDataMenuEntries(false);

    dbClose();
    //userDataEditing=false;
    //ui->tbInfoDB->clear();
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


    if (settings.value("database_path").toString().isEmpty()) {
        int choice = RadiobuttonDialog(tr("Potaléger stoque ses données dans un fichier unique à l'emplacement de votre choix."),
                                       {tr("Sélectionner une base de données existante"),
                                        tr("Créer une BDD avec les données de base"),
                                        tr("Créer une BDD vide")},1,QStyle::NStandardPixmap);
        if (choice==0)
            on_mSelecDB_triggered();
        else if (choice==1)
            on_mCreerBDD_triggered();
        else if (choice==2)
            on_mCreerBDDVide_triggered();
    } else
        PotaDbOpen(settings.value("database_path").toString(),"",false);

    if (!settings.value("notes").toString().isEmpty())
        ui->pteNotes->setMarkdown(settings.value("notes").toString());
    else {
        QFile mdFile;
        mdFile.setFileName(QApplication::applicationDirPath()+"/readme.md");
        if (mdFile.exists()) {
            qDebug() << mdFile.fileName();
            mdFile.open(QFile::ReadOnly);
            ui->pteNotes->setMarkdown(mdFile.readAll());
        } else
            ui->pteNotes->setPlainText(tr("Notes au format Markdown (CTRL+N pour éditer).")+"\n"+
                                       tr("Ce texte est enregistré sur votre ordinateur (pas dans la BDD)."));
    }


    QFile imgFile(QApplication::applicationDirPath()+"/infotab1.png");
    if (imgFile.exists()) {
        qDebug() << imgFile.fileName();
        //Image on first tab.
        QGraphicsScene *scene = new QGraphicsScene(this);
        QPixmap pixmap(imgFile.fileName());
        scene->addPixmap(pixmap);
        ui->graphicsView->setScene(scene);
        //ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    } else {
        ui->graphicsView->setVisible(false);
    }

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

    if (!ui->lDB->text().isEmpty()) {
        QFile file(ui->lDB->text());
        if (file.exists())
            settings.setValue("database_path", ui->lDB->text());
    }

    if (ui->pteNotes->isReadOnly())
        settings.setValue("notes", ui->pteNotes->toMarkdown().trimmed());
    else
        settings.setValue("notes", ui->pteNotes->toPlainText().trimmed());

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
    ui->progressBar->setMinimumSize(200,ui->progressBar->height());
    ui->tabWidget->widget(1)->deleteLater();//Used at UI design time.
    ui->Info->setLayout(ui->verticalLayout);

    ui->cbTheme->addItem(tr("clair"));
    ui->cbTheme->addItem(tr("sombre"));

    ui->mFamilles->setIcon(QIcon(TablePixmap("Familles","T")));
    ui->mEspeces->setIcon(QIcon(TablePixmap("Espèces","T")));
    ui->mVarietes->setIcon(QIcon(TablePixmap("Variétés","T")));
    ui->mApports->setIcon(QIcon(TablePixmap("Apports","T")));
    ui->mFournisseurs->setIcon(QIcon(TablePixmap("Fournisseurs","T")));
    ui->mTypes_de_planche->setIcon(QIcon(TablePixmap("Types_planche","T")));
    ui->mITPTempo->setIcon(QIcon(TablePixmap("ITP__Tempo","T")));

    ui->mRotations->setIcon(QIcon(TablePixmap("Rotations","T")));
    ui->mDetailsRotations->setIcon(QIcon(TablePixmap("Rotations_détails__Tempo","T")));
    ui->mRotationManquants->setIcon(QIcon(TablePixmap("IT_rotations_manquants","")));
    ui->mPlanches->setIcon(QIcon(TablePixmap("Planches","T")));
    ui->mSuccessionParPlanche->setIcon(QIcon(TablePixmap("Successions_par_planche","")));
    ui->mIlots->setIcon(QIcon(TablePixmap("Planches_Ilots","")));

    ui->mCulturesParIlots->setIcon(QIcon(TablePixmap("IT_rotations_ilots","")));
    ui->mCulturesParplante->setIcon(QIcon(TablePixmap("IT_rotations","")));
    ui->mCulturesParPlanche->setIcon(QIcon(TablePixmap("Cult_planif","")));
    ui->mSemences->setIcon(QIcon(TablePixmap("Variétés__inv_et_cde","")));

    ui->mCuNonTer->setIcon(QIcon(TablePixmap("Cultures__non_terminées","")));
    ui->mCuSemisAFaire->setIcon(QIcon(TablePixmap("Cultures__Semis_à_faire","")));
    ui->mCuPlantationsAFaire->setIcon(QIcon(TablePixmap("Cultures__Plantations_à_faire","")));
    ui->mCuRecoltesAFaire->setIcon(QIcon(TablePixmap("Cultures__Récoltes_à_faire","")));
    ui->mCuSaisieRecoltes->setIcon(QIcon(TablePixmap("Récoltes__Saisies","T")));
    ui->mCuATerminer->setIcon(QIcon(TablePixmap("Cultures__à_terminer","")));
    ui->mCuToutes->setIcon(QIcon(TablePixmap("Cultures","T")));

    ui->mDestinations->setIcon(QIcon(TablePixmap("Destinations","T")));
    ui->mEsSaisieSorties->setIcon(QIcon(TablePixmap("Consommations__Saisies","T")));
    ui->mInventaire->setIcon(QIcon(TablePixmap("Espèces__inventaire","")));

    ui->mAnaITP->setIcon(QIcon(TablePixmap("ITP__analyse","")));
    ui->mAnaCultures->setIcon(QIcon(TablePixmap("Cultures__Tempo","")));
    ui->mIncDatesCultures->setIcon(QIcon(TablePixmap("Cultures__inc_dates","")));
    ui->mAnaDestinations->setIcon(QIcon(TablePixmap("Destinations__conso","")));

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

void MainWindow::showIfDdOpen() {
    // if(db.isOpen()){
    //     if(ui->lVerBDDAttendue->text().last(1)=="!")
    //         SetColoredText(ui->lVerBDDAttendue,"Locked","Err");
    //     else
    //         SetColoredText(ui->lVerBDDAttendue,"Locked!","Err");
    // } else {
    //     SetColoredText(ui->lVerBDDAttendue,DbVersion,"");
    // }
}

void MainWindow::showEvent(QShowEvent *){
    SetUi();
    RestaureParams();
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
    MainWindow w;
    //w.db = NEW;
    //w.db.addDatabase("QSQLITE");

    w.show();
    return a.exec();

}

