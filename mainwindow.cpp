#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QStackedLayout>
#include <QBoxLayout>
#include <QSizePolicy>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include "potawidget.h"
#include <QSqlQuery>
#include <QSettings>
#include "PotaUtils.h"
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->widget(1)->deleteLater();//Utilisé comme modèle pour concevoir les onglets data.
    ui->lVer->setText("1.0b4");//Version de l'application.
    ui->lVerBDDAttendue->setText("2024-12-17");//Version de la BDD attendue par l'application.

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::OuvrirOnglet(QString const sObjName,QString const sTableName, QString const sTitre)
{
    //Recherche parmis les onglets existants.
    for (int i = 1; i < ui->tabWidget->count(); i++)
    {
        if (ui->tabWidget->widget(i)->objectName()=="PW"+sObjName )//Widget Onglet Data
        {
            ui->tabWidget->setCurrentIndex(i);
            break;
        }
    }
    if (ui->tabWidget->currentWidget()->objectName()!="PW"+sObjName)
    {
        //Create tab
        PotaWidget *w = new PotaWidget(this);
        w->setObjectName("PW"+sObjName);
        w->lErr = ui->lDBErr;
        w->Init(sTableName);

        if (w->model->SelectShowErr())
        {
            ui->tabWidget->addTab(w,sTitre);
            ui->tabWidget->setCurrentWidget(w);

            //col width
            QSettings settings("greli.net", "Potaléger");
            settings.beginGroup("LargeursColonnes");
            for (int i=0; i<w->model->columnCount();i++)
            {
                int iWidth=settings.value(sTableName+"-"+w->model->FieldName(i)).toInt(nullptr);
                if (iWidth>0)
                    w->tv->setColumnWidth(i,iWidth);
                else
                    w->tv->resizeColumnToContents(i);
            }
            settings.endGroup();

            ui->mFermerOnglets->setEnabled(true);
            ui->mFermerOnglet->setEnabled(true);

            QString s;
            s.setNum(w->model->rowCount());
            SetColoredText(ui->lDBErr,sTableName+" - "+s,"Ok");

            return true;
        }
        else
            w->deleteLater();//Echec de la création de l'onglet.
    }
    return false;
};

void MainWindow::FermerOnglet(QWidget *Tab)
{
    if (ui->tabWidget->currentWidget()->objectName().startsWith("PW"))
    {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());

        if (w->pbCommit->isEnabled())
        {
            if (!OkCancelDialog(w->windowTitle()+"\n\n"+//A faire titre marche pas
                               tr("Valider les données en cours de modification ?")))
                return;
            if (!w->model->SubmitAllShowErr())
            {
                w->isCommittingError=true;
                return;
            }
        }

        //Largeurs de colonne
        QSettings settings("greli.net", "Potaléger");
        settings.beginGroup("LargeursColonnes");
            for (int i=0; i<w->model->columnCount();i++)
            {
                settings.setValue(w->model->tableName()+"-"+w->model->FieldName(i),w->tv->columnWidth(i));
            }
        settings.endGroup();

        if (ui->tabWidget->count()<3)//Fermeture du dernier onglet data ouvert.
        {
            ui->mFermerOnglets->setEnabled(false);
            ui->mFermerOnglet->setEnabled(false);
        }
        Tab->deleteLater();
    }
}

void MainWindow::FermerOnglets()
{
    for (int i = ui->tabWidget->count()-1; i >0 ; i--)
    {
        FermerOnglet(ui->tabWidget->widget(i));
    }
}

void MainWindow::on_mSelecDB_triggered()
{
    const QString sFichier = QFileDialog::getOpenFileName( this, tr("Base de donnée Potaléger"), ui->lDB->text(), "*.sqlite3");
    if (sFichier != "")
    {
        FermerBDD();
        OuvrirBDD(sFichier);
    }
}

void MainWindow::on_mFermerOnglet_triggered()
{
    FermerOnglet(ui->tabWidget->currentWidget());
}

void MainWindow::on_mFermerOnglets_triggered()
{
    FermerOnglets();
}

void MainWindow::on_mCopyBDD_triggered()
{
    QFileInfo FileInfo,FileInfoSauv;
    FileInfo.setFile(ui->lDB->text());
    if (!FileInfo.exists())
    {
        MessageDialog(tr("Le fichier de BDD n'existe pas.")+"\n"+ui->lDB->text(),QMessageBox::Critical);
        return;
    }

    const QString sFichierSauv = QFileDialog::getSaveFileName(this, tr("Copie de la base de donnée Potaléger"), ui->lDB->text(), "*.sqlite3",nullptr,QFileDialog::DontConfirmOverwrite);
    FileInfoSauv.setFile(sFichierSauv);
    if (!FileInfoSauv.exists() or
        OkCancelDialog(tr("Le fichier existe déjà")+"\n"+
                       sFichierSauv+"\n"+
                       FileInfoSauv.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfoSauv.size()/1000)+" ko\n\n"+
                       tr("Remplacer par")+"\n"+
                       ui->lDB->text()+"\n"+
                       FileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfo.size()/1000)+" ko ?"))
    {
        QFile file,fileSauv,fileSauv2;
        if (FileInfoSauv.exists())
        {
            fileSauv.setFileName(sFichierSauv);
            if (!fileSauv.moveToTrash())
            {
                MessageDialog(tr("Impossible de supprimer le fichier")+"\n"+
                              sFichierSauv,QMessageBox::Critical);
                return;
            };
        }
        QString sFichier=ui->lDB->text();
        file.setFileName(ui->lDB->text());
        FermerBDD();
        if (file.copy(sFichierSauv))
        {   //impossible de remettre la date de modif du fichier original. A faire.
            //fileSauv2.setFileName(sFichierSauv);
            //qDebug() << FileInfo.lastModified();
            //qDebug() << sFichierSauv;
            //fileSauv2.setFileTime(FileInfo.lastModified(),QFileDevice::FileAccessTime);
        }
        else
            MessageDialog(tr("Impossible de copier le fichier")+"\n"+
                          sFichier+"\n"+
                          tr("vers le fichier")+"\n"+
                          sFichierSauv,QMessageBox::Critical);
        OuvrirBDD(sFichier);
    }
}

void MainWindow::on_mCreerBDDVide_triggered()
{
    QFileInfo FileInfoSauv;

    const QString sFichierSauv = QFileDialog::getSaveFileName(this, tr("Base de donnée vide Potaléger à créer"), ui->lDB->text(), "*.sqlite3",nullptr,QFileDialog::DontConfirmOverwrite);
    FileInfoSauv.setFile(sFichierSauv);
    if (!FileInfoSauv.exists() or
        OkCancelDialog(tr("Le fichier existe déjà")+"\n"+
                       sFichierSauv+"\n"+
                       FileInfoSauv.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfoSauv.size()/1000)+" ko\n\n"+
                                                 tr("Remplacer par une base de donnée vide ?")))
    {
        QFile file,fileSauv,fileSauv2;
        if (FileInfoSauv.exists())
        {
            fileSauv.setFileName(sFichierSauv);
            if (!fileSauv.moveToTrash())
            {
                MessageDialog(tr("Impossible de supprimer le fichier")+"\n"+
                                  sFichierSauv,QMessageBox::Critical);
                return;
            };
        }
        QString sFichier=ui->lDB->text();
        file.setFileName(ui->lDB->text());
        FermerBDD();
        QSqlDatabase db = QSqlDatabase::database();
        db.setDatabaseName(sFichierSauv);
        db.open();
        if (db.tables(QSql::Tables).count()>0)
        {
            MessageDialog(tr("La BDD vide ne l'est pas!")+"\n"+
                              sFichierSauv,QMessageBox::Critical);
            db.close();
            OuvrirBDD(sFichier);
        }
        else if (MaJStruBDD("Nouvelle vide"))
        {
            db.close();
            OuvrirBDD(sFichierSauv);
        }
        else
        {
            MessageDialog(tr("Impossible de créer la BDD vide")+"\n"+
                              sFichierSauv,QMessageBox::Critical);
            db.close();
            OuvrirBDD(sFichier);
        }
    }

}

void MainWindow::on_mParam_triggered()
{
    OuvrirOnglet("Param","Params",tr("Paramètres"));
}

void MainWindow::on_mFamilles_triggered()
{
    OuvrirOnglet("Familles","Familles",tr("Familles"));
}

void MainWindow::on_mEspeces_triggered()
{
    OuvrirOnglet("Especes","Espèces",tr("Espèces"));
}

void MainWindow::on_mVarietes_triggered()
{
    OuvrirOnglet("Varietes","Variétés",tr("Variétés"));
}

void MainWindow::on_mApports_triggered()
{
    OuvrirOnglet("Apports","Apports",tr("Apports"));
}

void MainWindow::on_mFournisseurs_triggered()
{
    OuvrirOnglet("Fournisseurs","Fournisseurs",tr("Fournisseurs"));
}

void MainWindow::on_mITP_triggered()
{
    OuvrirOnglet("Itp","ITP",tr("ITP"));
}

void MainWindow::on_mRotations_triggered()
{
    OuvrirOnglet("Rotations","Rotations",tr("Rotations"));
}

void MainWindow::on_mDetailsRotations_triggered()
{
    OuvrirOnglet("Rotations_details","Rotations_détails",tr("Rotations_détails"));
}

void MainWindow::on_mPlanches_triggered()
{
    OuvrirOnglet("Planches","Planches",tr("Planches"));
}

void MainWindow::on_mIlots_triggered()
{

}


