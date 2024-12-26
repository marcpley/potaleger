#include "mainwindow.h"
#include "qdir.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlError>
#include "QDebug"
#include "PotaUtils.h"
// #include "sqlite3.h"
// #include "define.h"


void MainWindow::ActiverMenusData(bool b)
{
    if (!b)
    {   //Menus qui ne sont pas systématiquement activés quand une BDD valide est ouverte.
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
        ui->mPlanif->actions().at(i)->setEnabled(false);
    for (int i = 0; i < ui->mCultures->actions().count(); i++)
        ui->mCultures->actions().at(i)->setEnabled(false);
    for (int i = 0; i < ui->mAnalyses->actions().count(); i++)
        ui->mAnalyses->actions().at(i)->setEnabled(false);
}

void MainWindow::FermerBDD()
{
    QSqlDatabase db = QSqlDatabase::database();
    //fermer tous les onglets de données et désactiver les menus.
    FermerOnglets();
    ActiverMenusData(false);

    //fermer la BDD (si ouverte)
    db.close();
    ui->tbInfoDB->clear();
    ui->lDB->clear();
    ui->lDBErr->clear();

}

void MainWindow::OuvrirBDD(QString sFichier)
{
    QSqlDatabase db = QSqlDatabase::database();
    PotaQuery pQuery;
    pQuery.lErr = ui->lDBErr;
    ui->lDBErr->clear();
    ui->lDB->setText(sFichier);
    db.setDatabaseName(sFichier);
    QFile fBDD(sFichier);
    if (!fBDD.exists())
    {
        SetColoredText(ui->lDBErr,tr("Le fichier de BDD n'existe pas."),"Err");
        return;
    }

    db.open();
    if (db.tables(QSql::Tables).count()==0)
    {
        SetColoredText(ui->lDBErr,tr("Impossible d'ouvrir la BDD."),"Err");

        db.close();
        return;
    }

    QString sVerBDD = "";
    if (!pQuery.ExecShowErr("SELECT Valeur FROM Info_Potaléger WHERE N=1")) //La vue Info n'existe pas ou pas correcte. On tente pas de mettre cette BDD à jour.
    {
        ui->tbInfoDB->append(tr("Cette BDD n'est pas une BDD Potaléger."));
        db.close();
        return;
    }
    else
    {
        pQuery.next();
        sVerBDD = pQuery.value(0).toString();
    }

    if (sVerBDD < "2024-12-16")
    {
        ui->tbInfoDB->append(tr("La version de cette BDD Potaléger est trop ancienne: ")+sVerBDD);
        db.close();
        return;
    }

    if (sVerBDD > ui->lVerBDDAttendue->text())
    {
        ui->tbInfoDB->append(tr("La version de cette BDD est trop récente: ")+sVerBDD);
        ui->tbInfoDB->append(tr("-> Utilisez une version plus récente de Potaléger."));
        db.close();
        return;
    }

    if ((sVerBDD != ui->lVerBDDAttendue->text())and
        (OkCancelDialog("Base de données trop ancienne:\n"+
                        ui->lDB->text()+"\n" +
                        "version "+sVerBDD + "\n\n" +
                        "Mettre à jour cette BDD vers la version "+ ui->lVerBDDAttendue->text()+" ?\n\n" +
                        "Cette opération est irréversible")))
    {   //Mettre à jour la BDD.
        if (MaJStruBDD(sVerBDD))
        {
            sVerBDD = ui->lVerBDDAttendue->text();
        }
        else
        {
            db.close();
            return;
        }
    }

    if (sVerBDD != ui->lVerBDDAttendue->text())
    {
        ui->tbInfoDB->append(tr("La version de cette BDD est incorrecte: ")+sVerBDD);
        db.close();
        return;
    }

    //Les Foreing Key ne semblent pas activées lors de l'ouverture. Pourquoi ?
    //qDebug() << query.value(0);
    if (!pQuery.ExecShowErr("PRAGMA foreign_keys = ON"))
    {
        ui->tbInfoDB->append(tr("Impossible d'activer les clés étrangères."));
        db.close();
        return;
    }

    //Afficher infos
    if (InfosBDD())
    {
        //Activer les menus
        ActiverMenusData(true);
    }
    else
    {   //Ce cas ne devrait pas arriver, le SELECT précédent à validé l'existence de la vue Info_Potaléger.
        db.close();
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
    settings.setValue("database_path", ui->lDB->text());
    settings.setValue("notes", ui->pteNotes->toPlainText());
}

void MainWindow::ShowEvent(QShowEvent *)
{
}

void MainWindow::closeEvent(QCloseEvent *)
{
    SauvParams();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QSettings settings("greli.net", "Potaléger");
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    w.RestaureParams();

    w.show();
    return a.exec();

}

