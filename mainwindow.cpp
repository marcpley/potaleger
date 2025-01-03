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
#include <QToolButton>
#include "data/Data.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->widget(1)->deleteLater();//Used at UI design time.
    ui->lVer->setText("1.0b6");//Application version.
    ui->lVerBDDAttendue->setText("2024-12-30");//Expected database version.
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::PotaBDDInfo()
{
    PotaQuery pQuery;
    pQuery.lErr = ui->lDBErr;
    if (pQuery.ExecShowErr("SELECT * FROM Info_Potaléger"))
    {
        ui->tbInfoDB->clear();
        while (pQuery.next())
            ui->tbInfoDB->append(pQuery.value(1).toString()+": "+
                                 pQuery.value(2).toString());
        return true;
    }
    else
    {
        ui->tbInfoDB->append(tr("Impossible de lire la vue 'Info_Potaléger'."));
        return false;
    }
}

bool MainWindow::OuvrirOnglet(QString const sObjName,QString const sTableName, QString const sTitre,bool bView)
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
        PotaWidget *w = new PotaWidget(ui->tabWidget);
        w->setObjectName("PW"+sObjName);
        w->lErr = ui->lDBErr;
        w->Init(sTableName);

        if (w->model->SelectShowErr())
        {
            ui->tabWidget->addTab(w,sTitre);
            ui->tabWidget->setCurrentWidget(w);

            //View special settings
            w->isView=bView;
            if (bView)
            {
                w->sbInsertRows->setEnabled(false);
                w->pbInsertRow->setEnabled(false);
                w->pbDeleteRow->setEnabled(false);
                w->lFilter->setText(str(w->model->rowCount())+" "+tr("lignes"));
            }
            else
                w->lFilter->setText(str(w->model->rowCount())+" "+w->model->tableName().toLower());

            w->delegate->cTableColor=TableColor(sTableName,"");
            for (int i=0; i<w->model->columnCount();i++)
            {
                //Color.
                w->delegate->cColColors[i]=TableColor(sTableName,w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString());

                //Tooltip
                QString sTT=ToolTip(sTableName,w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString());
                if (sTT!="")
                    w->model->setHeaderData(i, Qt::Horizontal, sTT, Qt::ToolTipRole);

                //Read only columns
                if (bView or ReadOnly(sTableName,w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString())) {
                    w->model->setColumnEditable(i, false);
                }
            }


            //Tab user settings
            QSettings settings("greli.net", "Potaléger");

            //filter
            settings.beginGroup("Filter");
            w->sbFilter->setValue(settings.value(sTableName+"-nCar").toInt());
            settings.endGroup();

            //col width
            settings.beginGroup("ColWidth");
            for (int i=0; i<w->model->columnCount();i++)
            {
                int iWidth=settings.value(sTableName+"-"+w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString()).toInt(nullptr);
                if (iWidth>0)
                    w->tv->setColumnWidth(i,iWidth);
                else
                    w->tv->resizeColumnToContents(i);
            }
            settings.endGroup();

            ui->mFermerOnglets->setEnabled(true);
            ui->mFermerOnglet->setEnabled(true);

            SetColoredText(ui->lDBErr,sTableName+" - "+str(w->model->rowCount()),"Ok");

            w->tv->setFocus();

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
            if (YesNoDialog(ui->tabWidget->tabText(ui->tabWidget->currentIndex())+"\n\n"+
                               tr("Valider les modifications avant de fermer ?")))
            {
                if (!w->model->SubmitAllShowErr())
                    return;
            }
            else
            {
                if (!w->model->RevertAllShowErr())
                    return;
            }
        }

        //Save user settings
        QSettings settings("greli.net", "Potaléger");

        //Filter
        settings.beginGroup("Filter");
        settings.setValue(w->model->tableName()+"-nCar",w->sbFilter->value());
        settings.endGroup();

        //ColWidth
        settings.beginGroup("ColWidth");
            for (int i=0; i<w->model->columnCount();i++)
            {
                settings.setValue(w->model->tableName()+"-"+w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString(),w->tv->columnWidth(i));
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
    const QString sFileName = QFileDialog::getOpenFileName( this, tr("Base de donnée Potaléger"), ui->lDB->text(), "*.sqlite3");
    if (sFileName != "")
    {
        FermerBDD();
        OuvrirBDD(sFileName);
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
    QFileInfo FileInfo,FileInfoVerif;
    FileInfo.setFile(ui->lDB->text());
    if (!FileInfo.exists())
    {
        MessageDialog(tr("Le fichier de BDD n'existe pas.")+"\n"+ui->lDB->text(),QMessageBox::Critical);
        return;
    }

    const QString sFileName = QFileDialog::getSaveFileName(this, tr("Copie de la base de donnée Potaléger"), ui->lDB->text(), "*.sqlite3",nullptr,QFileDialog::DontConfirmOverwrite);
    FileInfoVerif.setFile(sFileName);
    if (!FileInfoVerif.exists() or
        OkCancelDialog(tr("Le fichier existe déjà")+"\n"+
                       sFileName+"\n"+
                       FileInfoVerif.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfoVerif.size()/1000)+" ko\n\n"+
                       tr("Remplacer par")+"\n"+
                       ui->lDB->text()+"\n"+
                       FileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfo.size()/1000)+" ko ?"))
    {
        QFile FileInfo1,FileInfo2,FileInfo3;
        if (FileInfoVerif.exists())
        {
            FileInfo2.setFileName(sFileName);
            if (!FileInfo2.moveToTrash())
            {
                MessageDialog(tr("Impossible de supprimer le fichier")+"\n"+
                              sFileName,QMessageBox::Critical);
                return;
            };
        }
        QString sFileNameSave=ui->lDB->text();
        FileInfo1.setFileName(ui->lDB->text());
        FermerBDD();
        if (FileInfo1.copy(sFileName))
        {   //fail to keep original date file. todo
            //FileInfo3.setFileName(sFileName);
            //qDebug() << FileInfo.lastModified();
            //qDebug() << sFileName;
            //FileInfo3.setFileTime(FileInfo.lastModified(),QFileDevice::FileAccessTime);
        }
        else
            MessageDialog(tr("Impossible de copier le fichier")+"\n"+
                          sFileNameSave+"\n"+
                          tr("vers le fichier")+"\n"+
                          sFileName,QMessageBox::Critical);
        OuvrirBDD(sFileNameSave);
    }
}

void MainWindow::on_mCreerBDD_triggered()
{
    CreateNewDB(false);
}

void MainWindow::on_mCreerBDDVide_triggered()
{
    CreateNewDB(true);
}

void MainWindow::CreateNewDB(bool bEmpty)
{
    QString sEmpty= iif(bEmpty,tr("vide"),tr("avec données de base")).toString();
    QFileInfo FileInfoVerif;

    const QString sFileName = QFileDialog::getSaveFileName(this, tr("Nom pour la BDD Potaléger %1").arg(sEmpty), ui->lDB->text(), "*.sqlite3",nullptr,QFileDialog::DontConfirmOverwrite);
    if (sFileName.isEmpty()) return;

    FileInfoVerif.setFile(sFileName);
    if (!FileInfoVerif.exists() or
        OkCancelDialog(tr("Le fichier existe déjà")+"\n"+
                       sFileName+"\n"+
                       FileInfoVerif.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfoVerif.size()/1000)+" ko\n\n"+
                       tr("Remplacer par une base de données %1 ?").arg(sEmpty)))
    {
        QFile FileInfo1,FileInfo2,FileInfo3;
        if (FileInfoVerif.exists())
        {
            FileInfo2.setFileName(sFileName);
            if (!FileInfo2.moveToTrash())
            {
                MessageDialog(tr("Impossible de supprimer le fichier")+"\n"+
                                  sFileName,QMessageBox::Critical);
                return;
            };
        }
        QString sFileNameSave=ui->lDB->text();
        FileInfo1.setFileName(ui->lDB->text());
        FermerBDD();
        QSqlDatabase db = QSqlDatabase::database();
        db.setDatabaseName(sFileName);
        db.open();
        if (db.tables(QSql::Tables).count()>0)
        {
            MessageDialog("Empty file has tables!\n"+sFileName,QMessageBox::Critical);
            db.close();
            OuvrirBDD(sFileNameSave);
        }
        else if (UpdateDBShema(iif(bEmpty,"New","NewWithBaseData").toString()))
        {
            db.close();
            OuvrirBDD(sFileName);
        }
        else
        {
            MessageDialog(tr("Impossible de créer la BDD %1").arg(sEmpty)+"\n"+
                              sFileName,QMessageBox::Critical);
            db.close();
            OuvrirBDD(sFileNameSave);
        }
    }
}

void MainWindow::on_mParam_triggered()
{
    OuvrirOnglet("test","test","test",false);
    return;

    if (OuvrirOnglet("Param","Params",tr("Paramètres"),false))
    {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->model->sort(0,Qt::SortOrder::AscendingOrder);
        w->tv->verticalHeader()->hide();
        w->sbInsertRows->setVisible(false);
        w->pbInsertRow->setEnabled(false);
        w->pbInsertRow->setVisible(false);
        w->pbDeleteRow->setEnabled(false);
        w->pbDeleteRow->setVisible(false);
    }
}

void MainWindow::on_mFamilles_triggered()
{
    OuvrirOnglet("Familles","Familles",tr("Familles"),false);
}

void MainWindow::on_mEspeces_triggered()
{
    OuvrirOnglet("Especes","Espèces",tr("Espèces"),false);
}

void MainWindow::on_mVarietes_triggered()
{
    OuvrirOnglet("Varietes","Variétés",tr("Variétés"),false);
}

void MainWindow::on_mApports_triggered()
{
    OuvrirOnglet("Apports","Apports",tr("Apports"),false);
}

void MainWindow::on_mFournisseurs_triggered()
{
    OuvrirOnglet("Fournisseurs","Fournisseurs",tr("Fournisseurs"),false);
}

void MainWindow::on_mTypes_de_planche_triggered()
{
    OuvrirOnglet("TypesPlanche","Types_planche",tr("Types planche"),false);
}

void MainWindow::on_mITP_triggered()
{
    OuvrirOnglet("Itp","ITP",tr("ITP"),false);
}

void MainWindow::on_mITPTempo_triggered()
{
    OuvrirOnglet("ITP_tempo","ITP_tempo",tr("ITP (tempo)"),true);
}

void MainWindow::on_mRotations_triggered()
{
    OuvrirOnglet("Rotations","Rotations",tr("Rotations"),false);
}

void MainWindow::on_mDetailsRotations_triggered()
{
    OuvrirOnglet("Rotations_Tempo","Rotations_Tempo",tr("Rot. (détails)"),true);
}

void MainWindow::on_mRotationManquants_triggered()
{
    OuvrirOnglet("IT_rotations_manquants","IT_rotations_manquants",tr("Cult.manquantes"),true);
}

void MainWindow::on_mPlanches_triggered()
{
    OuvrirOnglet("Planches","Planches",tr("Planches"),false);
}

void MainWindow::on_mIlots_triggered()
{
    OuvrirOnglet("Planches_Ilots","Planches_Ilots",tr("Ilots"),true);
}

void MainWindow::on_mSuccessionParPlanche_triggered()
{
    OuvrirOnglet("SuccPlanches","Successions_par_planche",tr("Succ. planches"),true);
}

void MainWindow::on_mCulturesParIlots_triggered()
{
    OuvrirOnglet("IT_rotations_ilots","IT_rotations_ilots",tr("Cult.prévues ilots"),true);
}

void MainWindow::on_mCulturesParEspeces_triggered()
{
    OuvrirOnglet("IT_rotations","IT_rotations",tr("Cult.prévues esp."),true);
}

void MainWindow::on_mCulturesParPlanche_triggered()
{
    OuvrirOnglet("Cult_planif","Cult_planif",tr("Cult.prévues"),true);
}

void MainWindow::on_mCreerCultures_triggered()
{
    PotaQuery pQuery;
    pQuery.lErr = ui->lDBErr;
    int NbCultPlanif=pQuery.Selec0ShowErr("SELECT count() FROM Cult_planif").toInt();
    if (NbCultPlanif==0)
    {
        MessageDialog(tr("Aucune culture à planifier:")+"\n\n"+
                          tr("- Créez des rotations")+"\n"+
                          tr("- Vérifiez que le paramètre 'Planifier_planches' n'exclut pas toutes les planches."),QMessageBox::Information);
        return;
    }

    int NbCultAVenir=pQuery.Selec0ShowErr("SELECT count() FROM C_non_commencées").toInt();
    if (OkCancelDialog(tr("Créer les prochaines cultures en fonction des rotations?")+"\n\n"+
                       tr("%1 cultures vont être créées").arg(NbCultPlanif)+"\n"+
                       tr("Il y a déjà %1 cultures ni semées ni plantées.").arg(NbCultAVenir)+"\n"+
                       tr("Id de la dernière culture:")+" "+str(NbCultAVenir)))
    {
        int IdCult1=pQuery.Selec0ShowErr("SELECT max(Culture) FROM Cultures").toInt();
        if (pQuery.ExecShowErr("INSERT INTO Cultures (IT_Plante,Variété,Fournisseur,Planche,Longueur,Nb_rangs,Espacement) "
                               "SELECT IT_plante,Variété,Fournisseur,Planche,Longueur,Nb_rangs,Espacement "
                               "FROM Cult_planif"))
        {
            int IdCult2=pQuery.Selec0ShowErr("SELECT min(Culture) FROM Cultures WHERE Culture>"+str(IdCult1)).toInt();
            int IdCult3=pQuery.Selec0ShowErr("SELECT max(Culture) FROM Cultures").toInt();
            if (IdCult3>IdCult2 and IdCult2>IdCult1)
                MessageDialog(tr("%1 cultures créées sur %2 cultures prévues.").arg(IdCult3-IdCult2+1).arg(NbCultPlanif)+"\n\n"+
                                  tr("Id culture:")+" "+str(IdCult2)+" > "+str(IdCult3),QMessageBox::Information);
            else
                MessageDialog(tr("%1 culture créée sur %2 cultures prévues.").arg("0").arg(NbCultPlanif),QMessageBox::Warning);
        }
        else
            MessageDialog(tr("Impossible de créer les cultures."),QMessageBox::Critical);

    }
}

void MainWindow::on_mSemencesNecessaires_triggered()
{
    OuvrirOnglet("Cultures_semences_necessaires","Cultures_semences_nécessaires",tr("Semence nécessaire"),true);
}

void MainWindow::on_mSemences_triggered()
{
    OuvrirOnglet("Varietes_inv_et_cde","Variétés_inv_et_cde",tr("Inv. et cde semence"),true);
}

void MainWindow::on_mCuNonTer_triggered()
{
    OuvrirOnglet("Cultures_non_terminees","Cultures_non_terminées",tr("Non terminées"),true);
}

void MainWindow::on_mCuSemisAFaire_triggered()
{
    OuvrirOnglet("Cultures_Semis_a_faire","Cultures_Semis_à_faire",tr("A semer"),true);
}

void MainWindow::on_mCuPlantationsAFaire_triggered()
{
    OuvrirOnglet("Cultures_Plantations_a_faire","Cultures_Plantations_à_faire",tr("A planter"),true);
}

void MainWindow::on_mCuRecoltesAFaire_triggered()
{
    OuvrirOnglet("Cultures_Recoltes_a_faire","Cultures_Récoltes_à_faire",tr("A récolter"),true);
}

void MainWindow::on_mCuSaisieRecoltes_triggered()
{
    OuvrirOnglet("Saisie_recoltes","Saisie_récoltes",tr("Récoltes"),true);
}

void MainWindow::on_mCuATerminer_triggered()
{
    OuvrirOnglet("Cultures_a_terminer","Cultures_à_terminer",tr("A terminer"),true);
}

void MainWindow::on_mCuToutes_triggered()
{
    OuvrirOnglet("Cultures","Cultures",tr("Cultures"),false);
}

void MainWindow::on_mAnaITP_triggered()
{
    OuvrirOnglet("ITP_analyse","ITP_analyse",tr("Analyse IT"),true);
}

void MainWindow::on_mAnaEspeces_triggered()
{
    OuvrirOnglet("Cultures_Tempo_Espece","Cultures_Tempo_Espèce",tr("Analyse espèces"),true);
}
