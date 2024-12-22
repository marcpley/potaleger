#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QStackedLayout>
#include <QSizePolicy>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <filesystem>

class QPotaWidget: public QWidget {
public:
    QSqlTableModel *model = new QSqlTableModel(this);
    QTableView *tv = new QTableView(this);
    };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->widget(1)->deleteLater();//Utilisé pour concevoir les onglets data.
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OuvrirOnglet(QString const sNames,QString const sTableName, QString const sTitre)
{
    //Recherche parmis les onglets existants.
    for (int i = 1; i < ui->tabWidget->count(); i++)
    {
        if (ui->tabWidget->widget(i)->objectName()=="wOD"+sNames )//Widget Onglet Data
        {
            ui->tabWidget->setCurrentIndex(i);
            break;
        }
    }
    if (ui->tabWidget->currentWidget()->objectName()!="wDO"+sNames)
    {
        //Créer l'onglet
        QPotaWidget *w = new QPotaWidget;
        w->setObjectName("wDO"+sNames);
        //w->model->setObjectName("mod"+sNames);
        w->model->setTable(sTableName);
        w->model->setEditStrategy(QSqlTableModel::OnFieldChange);//OnManualSubmit
        //model->select();
        if (ExecSql(*w->model))
        {
            //model->removeColumn(0);
            w->tv->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
            w->tv->setModel(w->model);
            w->tv->verticalHeader()->setDefaultSectionSize(0);//Mini.
            w->tv->setItemDelegate(new QSqlRelationalDelegate(w));
            //w->tv->itemDelegate()->

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
        }
        else
            w->deleteLater();//Echec de la création de l'onglet.
    }

};

void MainWindow::FermerOnglet(QWidget *Tab)
{
    //A faire : commit ?
    if (ui->tabWidget->count()<3)//Fermeture du dernier onglet data ouvert.
    {
        ui->mFermerOnglets->setEnabled(false);
        ui->mFermerOnglet->setEnabled(false);
        ui->mValiderModifs->setEnabled(false);
        ui->mAbandonnerModifs->setEnabled(false);
    }
    Tab->deleteLater();
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
{//A faire
    if (ui->tabWidget->currentIndex()==0)
      InfosBDD();
    else
        ui->tabWidget->currentWidget()->focusWidget();
}


void MainWindow::on_mParam_triggered()
{
    OuvrirOnglet("Param","Params",tr("Paramètres"));
}


void MainWindow::on_mFermerOnglet_triggered()
{
    FermerOnglet(ui->tabWidget->currentWidget());
}


void MainWindow::on_mFermerOnglets_triggered()
{
    int const TabCount = ui->tabWidget->count();
    for (int i = TabCount-1; i >0 ; i--)
    {
        if (ui->tabWidget->widget(i)->objectName().startsWith("wDO"))
            FermerOnglet(ui->tabWidget->widget(i));
    }

}

void MainWindow::on_mAbandonnerModifs_triggered()
{
    //A faire : valider et abandonner les modifs.
    MessageDialog(ui->tabWidget->currentWidget()->objectName());
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    //A faire : Activer les menus valider et abandonner modifs en fonction de l'état de l'onget courant.
    //MessageDialog(ui->tabWidget->widget(index)::QPotaWidget->tv->objectName());
    //QPotaWidget *ptw = ui->tabWidget->widget(index);
    //ui->mValiderModifs->setEnabled(dynamic_cast<QPotaWidget*>(ui->tabWidget->currentWidget())->model)
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

