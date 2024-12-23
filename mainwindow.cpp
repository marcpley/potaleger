#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QStackedLayout>
#include <QSizePolicy>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include "potawidget.h"
#include <QSqlQuery>
#include <QSettings>

// class QPotaWidget: public QWidget {
// public:
//     QSqlRelationalTableModel *model = new QSqlRelationalTableModel(this);
//     QTableView *tv = new QTableView(this);
//     };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->widget(1)->deleteLater();//Utilisé comme modèle pour concevoir les onglets data.
    ui->lVer->setText("1.0b3");//Version de l'application.
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
        //Créer l'onglet
        PotaWidget *w = new PotaWidget();
        w->setObjectName("PW"+sObjName);
        w->lErr = ui->lDBErr;
        w->model->setTable(sTableName);
        w->model->setEditStrategy(QSqlTableModel::OnManualSubmit);//OnFieldChange
        if (w->model->SelectShowErr())
        {
            //Pawel code :-)
            QSqlQuery query("PRAGMA foreign_key_list("+sTableName+");");
            while (query.next()) {
                QString referencedTable = query.value("table").toString();
                QString localColumn = query.value("from").toString();
                QString referencedClumn = query.value("to").toString();
                int localColumnIndex = w->model->fieldIndex(localColumn);
                w->model->setRelation(localColumnIndex, QSqlRelation(referencedTable, referencedClumn, referencedClumn));
                // qDebug() << query.value(0);
                // qDebug() << query.value("table");
                // qDebug() << query.value("from");
                // qDebug() << query.value("to");
            }
            // QSqlQuery query2("PRAGMA foreign_keys");
            // query2.next();
            // qDebug() << query2.value(0);

            w->tv->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
            w->tv->setModel(w->model);
            w->tv->verticalHeader()->setDefaultSectionSize(0);//Mini.
            w->tv->setItemDelegate(new QSqlRelationalDelegate(w));
            //w->tv->itemDelegate()->

            //Largeurs de colonne
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

            QStackedLayout *l1 = new QStackedLayout(w);
            l1->setContentsMargins(2,2,2,2);
            l1->setSpacing(2);
            l1->addWidget(w->tv);
            w->setLayout(l1);
            ui->tabWidget->addTab(w,sTitre);
            ui->tabWidget->setCurrentWidget(w);

            ui->mFermerOnglets->setEnabled(true);
            ui->mFermerOnglet->setEnabled(true);
            ui->mValiderModifs->setEnabled(true);
            ui->mAbandonnerModifs->setEnabled(true);

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
        //A faire : commit ?

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
            ui->mValiderModifs->setEnabled(false);
            ui->mAbandonnerModifs->setEnabled(false);
        }
        Tab->deleteLater();
    }
}

void MainWindow::FermerOnglets()
{
    for (int i = ui->tabWidget->count()-1; i >0 ; i--)
    {
        //if (ui->tabWidget->widget(i)->objectName().startsWith("PW"))
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


void MainWindow::on_mRafraichir_triggered()
{
    if (ui->tabWidget->currentIndex()==0)
        InfosBDD();
    else if (ui->tabWidget->currentWidget()->objectName().startsWith("PW"))
    {
        dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget())->model->submitAll();
        dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget())->model->SelectShowErr();
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

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index==0)
    {
        ui->mValiderModifs->setEnabled(false);
        ui->mAbandonnerModifs->setEnabled(false);
    }
    else if (ui->tabWidget->widget(index)->objectName().startsWith("PW"))
    {
        ui->mValiderModifs->setEnabled(dynamic_cast<PotaWidget*>(ui->tabWidget->widget(index))->ModifsEnCours);
        ui->mAbandonnerModifs->setEnabled(dynamic_cast<PotaWidget*>(ui->tabWidget->widget(index))->ModifsEnCours);
        ui->mValiderModifs->setEnabled(true);//A faire
        ui->mAbandonnerModifs->setEnabled(true);
    }
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
        //ui->lDB->setText(sFichier);
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


void MainWindow::on_mValiderModifs_triggered()
{
    if (ui->tabWidget->currentWidget()->objectName().startsWith("PW"))
    {
        dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget())->model->SubmitAllShowErr();
    }
}

void MainWindow::on_mAbandonnerModifs_triggered()
{
    if (ui->tabWidget->currentWidget()->objectName().startsWith("PW"))
    {
        dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget())->model->RevertAllShowErr();
    }
}
