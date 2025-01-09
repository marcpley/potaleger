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

//#include <sqlite3.h>

void MainWindow::ActiverMenusData(bool b)
{
    if (!b)
    {   //These menus are enabled elsewhere.
        ui->mFermerOnglet->setEnabled(false);
        ui->mFermerOnglets->setEnabled(false);
    }

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

void MainWindow::FermerBDD()
{
    //fermer tous les onglets de données et désactiver les menus.
    FermerOnglets();
    ActiverMenusData(false);

    //fermer la BDD (si ouverte)
    dbClose();
    ui->tbInfoDB->clear();
    ui->lDB->clear();
    ui->lDBErr->clear();
}

bool MainWindow::dbOpen(QString sFichier)
{
    dbClose();

    QSqlDatabase db = QSqlDatabase::database();
    db.setDatabaseName(sFichier);

    if (!db.open()) {
        dbClose();
        SetColoredText(ui->lDBErr, tr("Impossible d'ouvrir la base de données."), "Err");
        return false;
    }

    if (db.tables(QSql::Tables).count() == 0) {
        dbClose();
        SetColoredText(ui->lDBErr, tr("Aucune table dans la BDD."), "Err");
        return false;
    }

    QSqlQuery query;
    query.exec("PRAGMA journal_mode = DELETE;");
    query.exec("PRAGMA locking_mode = NORMAL;");
    query.exec("PRAGMA quick_check;");
    query.next();
    qDebug() << "quick_check: " << query.value(0).toString();

    if (true) {
        if (!initCustomFunctions()) {
            dbClose();
            SetColoredText(ui->lDBErr, tr("Impossible d'implémenter %1.").arg("sqlean"), "Err");
            return false;
        }

        if (!registerCustomFunctions()) {
            dbClose();
            SetColoredText(ui->lDBErr, tr("Impossible d'implémenter les fonctions %1.").arg("sqlean"), "Err");
            return false;
        }

        QString s=testCustomFunctions();
        if (!s.isEmpty()){
            dbClose();
            SetColoredText(ui->lDBErr, tr("La fonction %1 ne fonctionnent pas.").arg(s), "Err");
            return false;
        }
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

void MainWindow::OuvrirBDD(QString sFichier)
{
    QFile fBDD(sFichier);
    if (!fBDD.exists())
    {
        SetColoredText(ui->lDBErr,tr("Le fichier de BDD n'existe pas.")+"\n"+
                                      sFichier,"Err");
        return;
    }

    bool const bForceUpdateViewsAndTriggers=false;

    PotaQuery pQuery;
    pQuery.lErr = ui->lDBErr;
    ui->lDBErr->clear();
    ui->lDB->setText(sFichier);

    if (!dbOpen(sFichier))
        return;

    QString sVerBDD = "";
    if (pQuery.ExecShowErr("SELECT Valeur FROM Info_Potaléger WHERE N=1")) //La vue Info n'existe pas ou pas correcte. On tente pas de mettre cette BDD à jour.
    {
        pQuery.next();
        sVerBDD = pQuery.value(0).toString();
    }
    else if (bForceUpdateViewsAndTriggers)
        sVerBDD = ui->lVerBDDAttendue->text();
    else
    {
        ui->tbInfoDB->append(tr("Cette BDD n'est pas une BDD Potaléger."));
        dbClose();
        return;
    }


    if (sVerBDD < "2024-12-30")
    {
        ui->tbInfoDB->append(tr("La version de cette BDD Potaléger est trop ancienne: ")+sVerBDD);
        dbClose();
        return;
    }

    if (sVerBDD > ui->lVerBDDAttendue->text())
    {
        ui->tbInfoDB->append(tr("La version de cette BDD est trop récente: ")+sVerBDD);
        ui->tbInfoDB->append(tr("-> Utilisez une version plus récente de Potaléger."));
        dbClose();
        return;
    }

    if (bForceUpdateViewsAndTriggers or
        ((sVerBDD != ui->lVerBDDAttendue->text())and
         (OkCancelDialog("Base de données trop ancienne:\n"+
                         ui->lDB->text()+"\n" +
                         "Version "+sVerBDD + "\n\n" +
                         "Mettre à jour cette BDD vers la version "+ ui->lVerBDDAttendue->text()+" ?\n\n" +
                                                                               "Cette opération est irréversible"))))
    {   //Mettre à jour la BDD.
        if (!pQuery.ExecShowErr("PRAGMA foreign_keys = OFF"))
        {
            ui->tbInfoDB->append(tr("Impossible de désactiver les clés étrangères."));
            dbClose();
            return;
        }
        if (UpdateDBShema(sVerBDD))
        {
            sVerBDD = ui->lVerBDDAttendue->text();
        }
        else
        {
            dbClose();
            return;
        }
    }

    if (sVerBDD != ui->lVerBDDAttendue->text())
    {
        ui->tbInfoDB->append(tr("La version de cette BDD est incorrecte: ")+sVerBDD);
        dbClose();
        return;
    }

    //Les Foreing Key ne semblent pas activées lors de l'ouverture. Pourquoi ?
    //qDebug() << query.value(0);
    if (!pQuery.ExecShowErr("PRAGMA foreign_keys = ON"))
    {
        ui->tbInfoDB->append(tr("Impossible d'activer les clés étrangères."));
        dbClose();
        return;
    }

    //Afficher infos
    if (PotaBDDInfo())
    {
        //Activer les menus
        ActiverMenusData(true);
        ui->lDBErr->clear();
    }
    else
    {   //Ce cas ne devrait pas arriver, le SELECT précédent à validé l'existence de la vue Info_Potaléger.
        dbClose();
        return;
    }
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
    OuvrirBDD(settings.value("database_path").toString());
    ui->pteNotes->setPlainText(settings.value("notes").toString());
}

void MainWindow::SauvParams()
{
    QSettings settings("greli.net", "Potaléger");
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
    if (!ui->lDB->text().isEmpty())
        settings.setValue("database_path", ui->lDB->text());
    settings.setValue("notes", ui->pteNotes->toPlainText());
}

void MainWindow::ShowEvent(QShowEvent *)
{
}

void MainWindow::closeEvent(QCloseEvent *)
{
    SauvParams();
    FermerBDD();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    w.RestaureParams();

    w.show();
    return a.exec();

}

