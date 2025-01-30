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

void MainWindow::SetEnabledDataMenuEntries(bool b)
{
    ui->mCopyBDD->setEnabled(b);
    ui->mParam->setEnabled(b);
    for (int i = 0; i < ui->mBaseData->actions().count(); i++)
        ui->mBaseData->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mAssolement->actions().count(); i++)
        ui->mAssolement->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mPlanif->actions().count(); i++)
        ui->mPlanif->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mCultures->actions().count(); i++)
        ui->mCultures->actions().at(i)->setEnabled(b);

    for (int i = 0; i < ui->mAnalyses->actions().count(); i++)
        ui->mAnalyses->actions().at(i)->setEnabled(b);
}

void MainWindow::PotaDbClose()
{
    ClosePotaTabs();
    SetEnabledDataMenuEntries(false);

    dbClose();
    ui->tbInfoDB->clear();
    ui->lDB->clear();
    ui->lDBErr->clear();
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

    QSqlDatabase db = QSqlDatabase::database();
    db.setDatabaseName(sFichier);

    if (!db.open()) {
        dbClose();
        SetColoredText(ui->lDBErr, tr("Impossible d'ouvrir la base de données.")+"\n"+
                                       sFichier, "Err");
        return false;
    }

    QSqlQuery query;
    if (!query.exec("PRAGMA journal_mode = DELETE;") or
        !query.exec("PRAGMA locking_mode = NORMAL;") or
        !query.exec("PRAGMA quick_check;") or
        !query.next()) {
        dbClose();
        SetColoredText(ui->lDBErr, tr("Impossible d'initialiser %1.").arg("SQLite"), "Err");
        return false;
    } else
        qInfo() << "SQLite quick_check:" << query.value(0).toString();

    if (SetFkOn and !query.exec("PRAGMA foreign_keys = ON")) {
        dbClose();
        SetColoredText(ui->lDBErr, tr("Impossible d'initialiser les clés étrangères."), "Err");
        return false;
    }

    if (true) {
        if (bResetSQLean and !query.exec("DELETE FROM sqlean_define")) {//"SELECT define_free('<function_name>')" don't work.
            dbClose();
            SetColoredText(ui->lDBErr, tr("Impossible d'effacer les fonctions %1.").arg("SQLean"), "Err");
            return false;
        }
        if (!initSQLean()) {
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
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()){
        QSqlQuery q1;
        q1.exec("SELECT define_free();");

        db.close();
    }
}

bool MainWindow::PotaDbOpen(QString sFichier, QString sNew)
{
    bool const bForceUpdateViewsAndTriggers=false;

    if (!dbOpen(sFichier,(sNew!=""),false,true))
        return false;

    PotaQuery pQuery;
    pQuery.lErr = ui->lDBErr;
    ui->lDBErr->clear();
    ui->lDB->setText(sFichier);

    if (sNew==""){//Vérifier une BDD existante.
        QString sVerBDD = "";
        if (pQuery.ExecShowErr("SELECT Valeur FROM Info_Potaléger WHERE N=1")) {//Si la vue Info n'existe pas ou pas correcte, on tente pas de mettre cette BDD à jour.
            pQuery.next();
            sVerBDD = pQuery.value(0).toString();
        }
        else if (bForceUpdateViewsAndTriggers)
            sVerBDD = ui->lVerBDDAttendue->text();
        else {
            ui->tbInfoDB->append(tr("Cette BDD n'est pas une BDD Potaléger."));
            dbClose();
            return false;
        }


        if (sVerBDD < "2024-12-30") {
            ui->tbInfoDB->append(tr("La version de cette BDD Potaléger est trop ancienne: ")+sVerBDD);
            dbClose();
            return false;
        }

        if (sVerBDD > ui->lVerBDDAttendue->text()) {
            ui->tbInfoDB->append(tr("La version de cette BDD est trop récente: ")+sVerBDD);
            ui->tbInfoDB->append("-> "+tr("Utilisez une version plus récente de Potaléger."));
            dbClose();
            return false;
        }

        if (bForceUpdateViewsAndTriggers or
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
            ui->tbInfoDB->append(tr("La version de cette BDD est incorrecte: ")+sVerBDD);
            dbClose();
            return false;
        }
    }
    else if (!UpdateDBShema(sNew)) {
        dbClose();
        return false;
    }

    //Afficher infos
    if (PotaBDDInfo()) {
        //Activer les menus
        SetEnabledDataMenuEntries(true);
        ui->lDBErr->clear();
    }
    else {   //Ce cas ne devrait pas arriver, le SELECT précédent à validé l'existence de la vue Info_Potaléger.
        dbClose();
        return false;
    }

    SetColoredText(ui->lDBErr, tr("Base de données ouverte."), "Ok");
    return true;
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
        PotaDbOpen(settings.value("database_path").toString(),"");

    if (!settings.value("notes").toString().isEmpty())
        ui->pteNotes->setMarkdown(settings.value("notes").toString());
    else {
        QFile mdFile;
        mdFile.setFileName(QApplication::applicationDirPath()+"/readme.md");
        qDebug() << QApplication::applicationDirPath()+"/readme.md";
        if (mdFile.exists()) {
            mdFile.open(QFile::ReadOnly);
            ui->pteNotes->setMarkdown(mdFile.readAll());
        } else
            ui->pteNotes->setPlainText("Notes, format Markdown, ctrl+N pour éditer.");
    }
}

void MainWindow::SauvParams()
{
    QSettings settings("greli.net", "Potaléger");
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
    if (!ui->lDB->text().isEmpty()) {
        QFile file(ui->lDB->text());
        if (file.exists())
            settings.setValue("database_path", ui->lDB->text());
    }

    if (ui->pteNotes->isReadOnly())
        settings.setValue("notes", ui->pteNotes->toMarkdown().trimmed());
    else
        settings.setValue("notes", ui->pteNotes->toPlainText().trimmed());
}

void MainWindow::ShowEvent(QShowEvent *)
{
}

void MainWindow::closeEvent(QCloseEvent *)
{
    SauvParams();
    PotaDbClose();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //a.addLibraryPath("/libs_potaleger");
    MainWindow w;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    w.RestaureParams();

    w.show();
    return a.exec();

}

