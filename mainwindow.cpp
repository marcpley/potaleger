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

    //QObject::connect(ui->tabWidget, &QTabWidget::currentChanged, [=](int) {
    //                 updateTabStyles(ui->tabWidget);});

    ui->lVer->setText("1.0b11");//Application version.
    ui->lVerBDDAttendue->setText("2024-12-30");//Expected database version.
}

MainWindow::~MainWindow()
{
    delete ui;
}

void updateTabStyles(QTabWidget *tabWidget) {

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

bool MainWindow::OuvrirOnglet(QString const sObjName, QString const sTableName, QString const sTitre)
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

            if (w->query->Selec0ShowErr("SELECT count() FROM sqlite_schema "      //Table
                                        "WHERE (tbl_name='"+sTableName+"')AND"
                                              "(sql LIKE 'CREATE TABLE "+sTableName+" (%')").toInt()+
                w->query->Selec0ShowErr("SELECT count() FROM sqlite_schema "
                                        "WHERE (tbl_name='"+sTableName+"')AND"    //View with trigger instead of insert
                                              "(sql LIKE 'CREATE TRIGGER "+sTableName+"_INSERT INSTEAD OF INSERT ON "+sTableName+" %')").toInt()==1){
                w->sbInsertRows->setEnabled(true);
                w->pbInsertRow->setEnabled(true);
                w->pbDeleteRow->setEnabled(true);
            }
            w->lFilter->setText(str(w->model->rowCount())+" "+tr("lignes"));

            w->delegate->cTableColor=TableColor(sTableName,"");
            for (int i=0; i<w->model->columnCount();i++)
            {
                //Table color.
                w->delegate->cColColors[i]=TableColor(sTableName,w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString());
                if (sTableName.startsWith("Cultures") and w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString()=="Etat"){
                    w->delegate->RowColorCol=i;
                }
                if (w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString()=="TEMPO"){
                    w->delegate->TempoCol=i;
                    dynamic_cast<PotaHeaderView*>(w->tv->horizontalHeader())->TempoCol=i;
                }

                //Tooltip
                QString sTT=ToolTipField(sTableName,w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString());
                if (sTT!="")
                    w->model->setHeaderData(i, Qt::Horizontal, sTT, Qt::ToolTipRole);

                //Read only columns
                if (ReadOnly(sTableName,w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString())) {
                    w->model->nonEditableColumns.insert(i);
                }
            }

            //Colored tab title
            ui->tabWidget->tabBar()->setTabText(ui->tabWidget->currentIndex(), ""); // Remove normal text
            QColor c=w->delegate->cTableColor;
            if (sTableName.startsWith("Cultures"))
                c=cCulture;
            w->lTabTitle->setStyleSheet(QString(
                                     "background-color: rgba(%1, %2, %3, %4);"
                                     "font-weight: bold;"
                                     )
                                     .arg(c.red())
                                     .arg(c.green())
                                     .arg(c.blue())
                                     .arg(60));
            w->lTabTitle->setText(sTitre);
            w->lTabTitle->setContentsMargins(10, 4, 10, 4);
            w->lTabTitle->setAlignment(Qt::AlignCenter);

            ui->tabWidget->tabBar()->setTabButton(ui->tabWidget->currentIndex(), QTabBar::LeftSide, w->lTabTitle);

            // ui->tabWidget->tabBar()->setStyleSheet("QTabBar::tab {" //Impossible d'obtenir le visuel natif.
            //                                        "padding-top: 5px;"
            //                                        "padding-right: 3px;"
            //                                        "background: palette(light);"
            //                                        "border: 1px solid palette(hilight);"
            //                                        "}"
            //                                        "QTabBar::tab:selected {"
            //                                        "border-bottom: 0px;"
            //                                        "border-top-left-radius: 8px 8px"
            //                                        "border-top-right-radius: 8px 8px"
            //                                        "}"
            //                                        "QTabBar::tab:!selected {"
            //                                        "margin-top: 2px;"
            //                                        "}");

            ui->tabWidget->setTabToolTip(ui->tabWidget->currentIndex(),ToolTipTable(sTableName));

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
                if (iWidth>0 and iWidth<1000)
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
            if (YesNoDialog(w->lTabTitle->text()+"\n\n"+
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
    // //OuvrirOnglet("sqlean_define","sqlean_define","test");
    // OuvrirOnglet("test","Rotations_détails","test");
    // return;

    if (OuvrirOnglet("Param","Params",tr("Paramètres"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->model->sort(0,Qt::SortOrder::AscendingOrder);
        w->sbInsertRows->setVisible(false);
        w->pbInsertRow->setEnabled(false);
        w->pbInsertRow->setVisible(false);
        w->pbDeleteRow->setEnabled(false);
        w->pbDeleteRow->setVisible(false);
    }
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

void MainWindow::on_mTypes_de_planche_triggered()
{
    OuvrirOnglet("TypesPlanche","Types_planche",tr("Types planche"));
}

void MainWindow::on_mITP_triggered()
{
    OuvrirOnglet("Itp","ITP",tr("ITP"));
}

void MainWindow::on_mITPTempo_triggered()
{
    OuvrirOnglet("ITP_tempo","ITP__Tempo",tr("ITP"));
}

void MainWindow::on_mRotations_triggered()
{
    OuvrirOnglet("Rotations","Rotations",tr("Rotations"));
}

void MainWindow::on_mDetailsRotations_triggered()
{
    if (OuvrirOnglet("Rotations_Tempo","Rotations_détails__Tempo",tr("Rot. (détails)"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//ID, necessary in the view for the triggers to update the real table.
    }
}

void MainWindow::on_mRotationManquants_triggered()
{
    OuvrirOnglet("IT_rotations_manquants","IT_rotations_manquants",tr("Espèces manquantes"));
}

void MainWindow::on_mPlanches_triggered()
{
    OuvrirOnglet("Planches","Planches",tr("Planches"));
}

void MainWindow::on_mIlots_triggered()
{
    OuvrirOnglet("Planches_Ilots","Planches_Ilots",tr("Ilots"));
}

void MainWindow::on_mSuccessionParPlanche_triggered()
{
    OuvrirOnglet("SuccPlanches","Successions_par_planche",tr("Succ. planches"));
}

void MainWindow::on_mCulturesParIlots_triggered()
{
    OuvrirOnglet("IT_rotations_ilots","IT_rotations_ilots",tr("Cult.prévues ilots"));
}

void MainWindow::on_mCulturesParEspeces_triggered()
{

}

void MainWindow::on_mCulturesParplante_triggered()
{
    OuvrirOnglet("IT_rotations","IT_rotations",tr("Plantes prévues"));
}

void MainWindow::on_mCulturesParPlanche_triggered()
{
    OuvrirOnglet("Cult_planif","Cult_planif",tr("Cult.prévues"));
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

void MainWindow::on_mSemences_triggered()
{
    OuvrirOnglet("Varietes_inv_et_cde","Variétés__inv_et_cde",tr("Inv. et cde semence"));
}

void MainWindow::on_mCuNonTer_triggered()
{
    OuvrirOnglet("Cultures_non_terminees","Cultures__non_terminées",tr("Non terminées"));
}

void MainWindow::on_mCuSemisAFaire_triggered()
{
    OuvrirOnglet("Cultures_Semis_a_faire","Cultures__Semis_à_faire",tr("A semer"));
}

void MainWindow::on_mCuPlantationsAFaire_triggered()
{
    OuvrirOnglet("Cultures_Plantations_a_faire","Cultures__Plantations_à_faire",tr("A planter"));
}

void MainWindow::on_mCuRecoltesAFaire_triggered()
{
    OuvrirOnglet("Cultures_Recoltes_a_faire","Cultures__Récoltes_à_faire",tr("A récolter"));
}

void MainWindow::on_mCuSaisieRecoltes_triggered()
{
    OuvrirOnglet("Saisie_recoltes","Saisie_récoltes",tr("Récoltes"));
}

void MainWindow::on_mCuATerminer_triggered()
{
    OuvrirOnglet("Cultures_a_terminer","Cultures__à_terminer",tr("A terminer"));
}

void MainWindow::on_mCuToutes_triggered()
{
    OuvrirOnglet("Cultures","Cultures",tr("Cultures"));
}

void MainWindow::on_mAnaITP_triggered()
{
    OuvrirOnglet("ITP_analyse","ITP__analyse",tr("Analyse IT"));
}

void MainWindow::on_mAnaEspeces_triggered()
{
    OuvrirOnglet("Cultures_Tempo_Espece","Cultures__Tempo_Espèce",tr("Analyse espèces"));
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    for (int i = 1; i < ui->tabWidget->count(); ++i) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->widget(i));
        if (i==ui->tabWidget->currentIndex())
            w->lTabTitle->setStyleSheet(w->lTabTitle->styleSheet().replace("font-weight: normal;", "font-weight: bold;"));
        else
            w->lTabTitle->setStyleSheet(w->lTabTitle->styleSheet().replace("font-weight: bold;", "font-weight: normal;"));
    }
}

