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
#include <QSettings>
#include "PotaUtils.h"
#include <QToolButton>
#include "data/Data.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qInfo() << "Potaléger" << Version;
    qInfo() << "Expected database version:" << DbVersion;

    ui->setupUi(this);

    ui->lVer->setText(Version);
    ui->lVerBDDAttendue->setText(DbVersion);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// bool MainWindow::PotaBDDInfo()
// {
//     PotaQuery pQuery(db);
//     pQuery.lErr = ui->lDBErr;
//     setWindowTitle("Potaléger");
//     if (pQuery.ExecShowErr("SELECT * FROM Info_Potaléger"))
//     {
//         ui->tbInfoDB->clear();
//         while (pQuery.next()) {
//             ui->tbInfoDB->append(pQuery.value(1).toString()+": "+
//                                  pQuery.value(2).toString());
//             if (pQuery.value(1).toString()=="Utilisateur")
//                 setWindowTitle("Potaléger"+iif(pQuery.value(2).isNull(),""," - "+pQuery.value(2).toString()).toString());
//         }
//         return true;
//     }
//     else
//     {
//         ui->tbInfoDB->append(tr("Impossible de lire la vue 'Info_Potaléger'."));
//         return false;
//     }
// }

bool MainWindow::OpenPotaTab(QString const sObjName, QString const sTableName, QString const sTitre)
{
    //Recherche parmis les onglets existants.
    for (int i = 0; i < ui->tabWidget->count(); i++) {
        if (ui->tabWidget->widget(i)->objectName()=="PW"+sObjName ) {//Widget Onglet Data
            ui->tabWidget->setCurrentIndex(i);
            PotaWidget *wc=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
            if (!wc->pbCommit->isEnabled())
                wc->pbRefreshClick();
            break;
        }
    }
    if (ui->tabWidget->currentWidget()->objectName()!="PW"+sObjName) {
        //Create tab
        PotaWidget *w = new PotaWidget(ui->tabWidget);
        w->setObjectName("PW"+sObjName);
        w->lErr=ui->lDBErr;
        w->cbFontSize=ui->cbFont;
        //w->mFilterFind = ui->mFilterFind;
        w->model->db=&db;
        w->model->progressBar=ui->progressBar;
        //w->query->lErr=ui->lDBErr;
        //w->query->db
        //w->userDataEditing=&userDataEditing;

       //dbSuspend(&db,false,userDataEditing,ui->lDBErr);

        w->delegate->cTableColor=TableColor(sTableName,"");//before init

        w->Init(sTableName);

        if (w->model->SelectShowErr()) {
            bool bEdit=false;
            PotaQuery query(db);
            if (!ReadOnlyDb and
                (query.Selec0ShowErr("SELECT count() FROM sqlite_schema "      //Table
                                     "WHERE (tbl_name='"+sTableName+"')AND"
                                           "(sql LIKE 'CREATE TABLE "+sTableName+" (%')").toInt()+
                query.Selec0ShowErr("SELECT count() FROM sqlite_schema "
                                    "WHERE (tbl_name='"+sTableName+"')AND"    //View with trigger instead of insert
                                          "(sql LIKE 'CREATE TRIGGER "+sTableName+"_INSERT INSTEAD OF INSERT ON "+sTableName+" %')").toInt()==1)){
                QPalette palette = w->sbInsertRows->palette();
                palette.setColor(QPalette::Text, Qt::white);
                palette.setColor(QPalette::Base, QColor(234,117,0,110));
                palette.setColor(QPalette::Button, QColor(234,117,0,110));
                w->sbInsertRows->setPalette(palette);
                w->sbInsertRows->setEnabled(true);
                w->pbInsertRow->setEnabled(true);
                w->bAllowInsert=true;
                w->pbDeleteRow->setEnabled(true);
                w->bAllowDelete=true;
                bEdit=true;
            } else if (ReadOnlyDb or
                       (query.Selec0ShowErr("SELECT count() FROM sqlite_schema "
                                            "WHERE (tbl_name='"+sTableName+"')AND"    //View without trigger instead of.
                                                  "(sql LIKE 'CREATE TRIGGER "+sTableName+"_UPDATE INSTEAD OF UPDATE ON "+sTableName+" %')").toInt()==0)) {
                w->pbEdit->setVisible(false);
            }

            if(w->model->rowCount()==0) {
                if (bEdit and(FkFilter(&db,w->model->RealTableName(),"",w->model->index(0,0),true)!="NoFk")){
                    w->lRowSummary->setText(tr("<- cliquez ici pour saisir des %1").arg(w->model->RealTableName()));
                } else {
                    SetColoredText(ui->lDBErr,"","");
                    MessageDialog(sTitre,NoData(w->model->tableName()),QStyle::SP_MessageBoxInformation);
                    w->deleteLater();
                    return false;
                }
            }

            ui->tabWidget->addTab(w,sTitre);
            ui->tabWidget->setCurrentWidget(w);

            //w->lFilterResult->setText(str(w->model->rowCount())+" "+tr("lignes"));

            for (int i=0; i<w->model->columnCount();i++){
                //Store field name in header EditRole.
                w->model->setHeaderData(i,Qt::Horizontal,w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole),Qt::EditRole);
                //Store corrected field name in header DisplayRole.
                w->model->setHeaderData(i,Qt::Horizontal,w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString().replace("_pc","%").replace("_"," "),Qt::DisplayRole);

                //Table color.
                w->delegate->cColColors[i]=TableColor(sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString());

                if (sTableName.startsWith("Cultures") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Etat")
                    w->delegate->RowColorCol=i;
                else if (sTableName.startsWith("Cultures") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="num_planche")
                    w->delegate->RowColorCol=i;
                else if (sTableName.startsWith("Params") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Paramètre")
                    w->delegate->RowColorCol=i;

                //Tooltip
                QString sTT=ToolTipField(sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),w->model->dataTypes[i]);
                if (sTT!="")
                    w->model->setHeaderData(i, Qt::Horizontal, sTT, Qt::ToolTipRole);

                //All columns read only for start
                w->model->nonEditableColumns.insert(i);

                if (DataType(&db, sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString())=="DATE")
                    w->model->dateColumns.insert(i);
                else if (sTableName!="Params" and
                         FieldIsMoney(w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()))
                    w->model->moneyColumns.insert(i);
            }

            w->RefreshHorizontalHeader();

            //Colored tab title
            ui->tabWidget->tabBar()->setTabText(ui->tabWidget->currentIndex(), ""); // Remove normal text
            QFont font = this->font();
            w->lTabTitle->setFont(font);
            w->lTabTitle->setText(" "+sTitre+" ");
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

            ui->tabWidget->setTabToolTip(ui->tabWidget->currentIndex(),ToolTipTable(w->model->tableName()));

            //Tab user settings
            QSettings settings("greli.net", "Potaléger");

            //filter
            settings.beginGroup("Filter");
            w->iTypeText=settings.value(sTableName+"-FilterTypeText").toInt();
            w->iTypeDate=settings.value(sTableName+"-FilterTypeDate").toInt();
            w->iTypeReal=settings.value(sTableName+"-FilterTypeReal").toInt();
            settings.endGroup();

            //col width
            settings.beginGroup("ColWidth");
            for (int i=0; i<w->model->columnCount();i++) {
                int iWidth;
                if (!w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString().startsWith("TEMPO_")){
                    iWidth=settings.value(sTableName+"-"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()).toInt(nullptr);
                    if (iWidth<=0 or iWidth>700)
                        iWidth=DefColWidth(&db, sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString());
                } else {
                    iWidth=DefColWidth(&db, sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString());
                }
                if (iWidth<=0 or iWidth>700)
                    w->tv->resizeColumnToContents(i);
                else
                    w->tv->setColumnWidth(i,iWidth);
                if (w->tv->columnWidth(i)>700)
                    w->tv->setColumnWidth(i,700);

            }
            settings.endGroup();

            ui->mFermerOnglets->setEnabled(true);
            ui->mFermerOnglet->setEnabled(true);
            ui->mImporter->setEnabled(w->pbInsertRow->isEnabled());
            ui->mExporter->setEnabled(true);
            ui->mImporter->setIcon(QIcon(TablePixmap(w->model->tableName(),">>  ")));
            ui->mExporter->setIcon(QIcon(TablePixmap(w->model->tableName(),"  >>")));

            w->tv->setFocus();
           //dbSuspend(&db,true,userDataEditing,ui->lDBErr);
            SetColoredText(ui->lDBErr,sTableName+
                                      iif(ReadOnlyDb," ("+tr("lecture seule")+")","").toString(),
                                      iif(ReadOnlyDb,"Info","Ok").toString());

            return true;
        }
        else {
            w->deleteLater();//Echec de la création de l'onglet.
           //dbSuspend(&db,true,userDataEditing,ui->lDBErr);
        }
    }
    return false;
};

void MainWindow::ClosePotaTab(QWidget *Tab)
{
    if (Tab->objectName().startsWith("PW")) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(Tab);

        if (w->pbCommit->isEnabled()) {
            if (YesNoDialog(w->lTabTitle->text().trimmed()+"\n\n"+
                               tr("Valider les modifications avant de fermer ?"))) {
                if (!w->model->SubmitAllShowErr())
                    return;
            } else {
                if (!w->model->RevertAllShowErr())
                    return;
            }
        }

        //Save user settings
        QSettings settings("greli.net", "Potaléger");

        //Filter
        settings.beginGroup("Filter");
        settings.setValue(w->model->tableName()+"-FilterTypeText",w->iTypeText);
        settings.setValue(w->model->tableName()+"-FilterTypeDate",w->iTypeDate);
        settings.setValue(w->model->tableName()+"-FilterTypeReal",w->iTypeReal);
        settings.endGroup();

        //ColWidth
        settings.beginGroup("ColWidth");
            for (int i=0; i<w->model->columnCount();i++) {
                if (!w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString().startsWith("TEMPO_"))
                    settings.setValue(w->model->tableName()+"-"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),w->tv->columnWidth(i));
            }
        settings.endGroup();

        if (w->model->tableName()=="Params") {
            PotaQuery pQuery(db);
            setWindowTitle("Potaléger"+pQuery.Selec0ShowErr("SELECT ' - '||Valeur FROM Params WHERE Paramètre='Utilisateur'").toString());
        }

        if (ui->tabWidget->count()<3) {//Fermeture du dernier onglet data ouvert.
            ui->mFermerOnglets->setEnabled(false);
            ui->mFermerOnglet->setEnabled(false);
            ui->mImporter->setEnabled(false);
            ui->mExporter->setEnabled(false);
        }
        Tab->deleteLater();
    }
}

void MainWindow::ClosePotaTabs()
{
    for (int i = ui->tabWidget->count()-1; i >=0 ; i--) {
        ClosePotaTab(ui->tabWidget->widget(i));
    }
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    //Set to bolt title of new tab, unbolt others.
    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        if (ui->tabWidget->widget(i)->objectName().startsWith("PW")){
            PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->widget(i));
            if (i==index)//ui->tabWidget->currentIndex()
                w->lTabTitle->setStyleSheet(w->lTabTitle->styleSheet().replace("font-weight: normal;", "font-weight: bold;"));
            else
                w->lTabTitle->setStyleSheet(w->lTabTitle->styleSheet().replace("font-weight: bold;", "font-weight: normal;"));
            //w->lTabTitle->updateGeometry();
            //ui->tabWidget->tabBar()->setTabButton(i, QTabBar::LeftSide, w->lTabTitle);
            //ui->tabWidget->tabBar()->tabButton(i,QTabBar::LeftSide)->updateGeometry();
        }
    }

    if (!ui->tabWidget->widget(index)->objectName().startsWith("PW")) {
        ui->mImporter->setEnabled(false);
        ui->mExporter->setEnabled(false);
    } else {
        PotaWidget *wc=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        ui->mImporter->setEnabled(wc->pbInsertRow->isEnabled());
        ui->mExporter->setEnabled(true);
        ui->mImporter->setIcon(QIcon(TablePixmap(wc->model->tableName(),">>  ")));
        ui->mExporter->setIcon(QIcon(TablePixmap(wc->model->tableName(),"  >>")));
        wc->SetSizes();
    }
    SetColoredText(ui->lDBErr,"","");
}

//File menu

void MainWindow::on_mSelecDB_triggered()
{
    ClosePotaTabs();
    const QString sFileName = QFileDialog::getOpenFileName( this, tr("Base de donnée Potaléger"), ui->lDB->text(), "*.sqlite3");
    if (sFileName != "") {
        PotaDbClose();
        PotaDbOpen(sFileName,"",false);
    }
}

void MainWindow::on_mUpdateSchema_triggered()
{
    ClosePotaTabs();
    if (OkCancelDialog(tr("Mettre à jour le schéma de la BDD ?")+"\n\n"+
                       ui->lDB->text()+"\n\n"+
                       tr("La structures des tables, les vues et les déclencheurs vont être recréés.")+"\n"+
                       tr("Vos données vont être conservées.")+"\n"+
                       tr("En cas d'échec, votre BDD sera remise dans sont état initial."))){
        QString sFileName=ui->lDB->text();
        PotaDbClose();
        PotaDbOpen(sFileName,"",true);
    }
}

void MainWindow::on_mCopyBDD_triggered()
{
    ClosePotaTabs();
    QFileInfo FileInfo,FileInfoVerif;
    FileInfo.setFile(ui->lDB->text());
    if (!FileInfo.exists()) {
        MessageDialog(tr("Le fichier de BDD n'existe pas.")+"\n"+ui->lDB->text(),"",QStyle::SP_MessageBoxCritical);
        return;
    }

    const QString sFileName = QFileDialog::getSaveFileName(this, tr("Copie de la base de donnée Potaléger"), ui->lDB->text(), "*.sqlite3",nullptr,QFileDialog::DontConfirmOverwrite);
    if (sFileName.isEmpty()) return;
    FileInfoVerif.setFile(sFileName);
    if (!FileInfoVerif.exists() or
        OkCancelDialog(tr("Le fichier existe déjà")+"\n"+
                       sFileName+"\n"+
                       FileInfoVerif.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfoVerif.size()/1000)+" ko\n\n"+
                       tr("Remplacer par")+"\n"+
                       ui->lDB->text()+"\n"+
                       FileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfo.size()/1000)+" ko ?",QStyle::SP_MessageBoxWarning)) {
        QFile FileInfo1,FileInfo2,FileInfo3;
        if (FileInfoVerif.exists()) {
            FileInfo2.setFileName(sFileName);
            if (!FileInfo2.remove())
            {
                MessageDialog(tr("Impossible de supprimer le fichier")+"\n"+
                              sFileName,"",QStyle::SP_MessageBoxCritical);
                return;
            };
        }
        QString sFileNameSave=ui->lDB->text();
        FileInfo1.setFileName(ui->lDB->text());
        PotaDbClose();
        if (FileInfo1.copy(sFileName)) {   //fail to keep original date file. todo
            //FileInfo3.setFileName(sFileName);
            //qDebug() << FileInfo.lastModified();
            //qDebug() << sFileName;
            //FileInfo3.setFileTime(FileInfo.lastModified(),QFileDevice::FileAccessTime);
        }
        else
            MessageDialog(tr("Impossible de copier le fichier")+"\n"+
                          sFileNameSave+"\n"+
                          tr("vers le fichier")+"\n"+
                          sFileName,"",QStyle::SP_MessageBoxCritical);
        PotaDbOpen(sFileNameSave,"",false);
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
    ClosePotaTabs();
    QString sEmpty= iif(bEmpty,tr("vide"),tr("avec données de base")).toString();
    QFileInfo FileInfoVerif;

    QString sFileName=ui->lDB->text();
    if (sFileName=="..."){ //First run, no db file.
        QDir dir;
        // if (!dir.exists("data"))
        //     dir.mkdir("data");
// #ifdef Q_OS_WIN
//         sFileName = "C:\\Users\\NomUtilisateur\\Documents";
// #else
//         sFileName = "/home/NomUtilisateur/Documents";
// #endif
        sFileName=QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+
                  QDir::toNativeSeparators("/Documents/Potaleger.sqlite3");
    }

    sFileName = QFileDialog::getSaveFileName(this, tr("Nom pour la BDD Potaléger %1").arg(sEmpty), sFileName, "*.sqlite3",nullptr,QFileDialog::DontConfirmOverwrite);
    if (sFileName.isEmpty()) return;

    FileInfoVerif.setFile(sFileName);
    if (!FileInfoVerif.exists() or
        OkCancelDialog(tr("Le fichier existe déjà")+"\n"+
                       sFileName+"\n"+
                       FileInfoVerif.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfoVerif.size()/1000)+" ko\n\n"+
                       tr("Remplacer par une base de données %1 ?").arg(sEmpty),QStyle::SP_MessageBoxWarning))
    {
        QFile FileInfo1,FileInfo2,FileInfo3;
        if (FileInfoVerif.exists())
        {
            FileInfo2.setFileName(sFileName);
            if (!FileInfo2.remove())
            {
                MessageDialog(tr("Impossible de supprimer le fichier")+"\n"+
                                  sFileName,"",QStyle::SP_MessageBoxCritical);
                return;
            };
        }
        QString sFileNameSave=ui->lDB->text();
        FileInfo1.setFileName(ui->lDB->text());
        PotaDbClose();

        if (!PotaDbOpen(sFileName,iif(bEmpty,"New","NewWithBaseData").toString(),false)) {
            MessageDialog(tr("Impossible de créer la BDD %1").arg(sEmpty)+"\n"+
                              sFileName,"",QStyle::SP_MessageBoxCritical);
            dbClose();
            PotaDbOpen(sFileNameSave,"",false);
        }
    }
}

void MainWindow::on_mAPropos_triggered()
{
    MessageDialog("Auteur: Marc Pleysier ...................................................................<br>"
                  "<a href=\"https://www.greli.net\">www.greli.net</a><br>"
                  "Sources: <a href=\"https://github.com/marcpley/potaleger\">github.com/marcpley/potaleger</a>",
                  "<b>Crédits</b>:<br>"
                  "Qt Creator community 6.8.1 <a href=\"https://www.qt.io/\">www.qt.io/</a><br>"
                  "SQLite 3 <a href=\"https://www.sqlite.org/\">www.sqlite.org/</a><br>"
                  "SQLean <a href=\"https://github.com/nalgeon/sqlean\">github.com/nalgeon/sqlean</a><br>"
                  "SQLiteStudio <a href=\"https://sqlitestudio.pl/\">sqlitestudio.pl/</a>, thanks Pawel !<br>"
                  "Ferme Légère <a href=\"https://fermelegere.greli.net\">fermelegere.greli.net</a><br>"
                  "ChatGPT, hé oui...<br>"
                  "Le Guide Terre Vivante du potager bio <a href=\"https://www.terrevivante.org\">www.terrevivante.org</a>",
                  QStyle::NStandardPixmap);
}

//edit menu

void MainWindow::on_mFermerOnglet_triggered()
{
    ClosePotaTab(ui->tabWidget->currentWidget());
}

void MainWindow::on_mFermerOnglets_triggered()
{
    ClosePotaTabs();
}

void MainWindow::on_mFilterFind_triggered()
{
    return;
    // if (ui->tabWidget->currentWidget()->objectName().startsWith("PW")) {
    //     PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
    //     if (w->filterFrame->isVisible()) {
    //         ui->mFilterFind->setChecked(false);
    //         w->pbFilter->setChecked(false);
    //         w->pbFilterClick(false);
    //         w->ffFrame->setVisible(false);
    //     } else {
    //         ui->mFilterFind->setChecked(true);
    //         w->ffFrame->setVisible(true);
    //         w->curChanged(w->tv->selectionModel()->currentIndex());
    //     }
    // }
}

void MainWindow::on_mParam_triggered()
{
    // //OuvrirOnglet("sqlean_define","sqlean_define","test");
    // OuvrirOnglet("test","Rotations_détails","test");
    // return;

    if (OpenPotaTab("Param","Params",tr("Paramètres"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->sbInsertRows->setVisible(false);
        w->pbInsertRow->setEnabled(false);
        w->pbInsertRow->setVisible(false);
        w->bAllowInsert=false;
        w->pbDeleteRow->setEnabled(false);
        w->pbDeleteRow->setVisible(false);
        w->bAllowDelete=false;
    }
}

void MainWindow::on_mImporter_triggered()
{
    if (ui->tabWidget->currentWidget()->objectName().startsWith("PW")){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());

        if (w->pbCommit->isEnabled()) {
            if (YesNoDialog(w->lTabTitle->text().trimmed()+"\n\n"+
                            tr("Valider les modifications en cours ?"))) {
                if (!w->model->SubmitAllShowErr())
                    return;
            } else {
                if (!w->model->RevertAllShowErr())
                    return;
            }
        } else if (!w->pbEdit->isChecked()){
            w->pbEditClick();
        }

        if (PathImport.isEmpty())
            PathImport=PathExport;

        QString sFileName = QFileDialog::getOpenFileName(this, tr("Importer des données"),
                                                         PathImport+w->lTabTitle->text().trimmed(), "*.csv");

        //Check filename.
        if (sFileName.isEmpty()) return;

        QFileInfo FileInfoVerif;
        FileInfoVerif.setFile(sFileName);
        PathImport=FileInfoVerif.absolutePath()+QDir::separator();

        QFile FileImport(sFileName);
        if (!FileImport.open(QIODevice::ReadOnly)) {
            MessageDialog(tr("Impossible d'ouvrir le fichier")+"\n"+
                              sFileName,"",QStyle::SP_MessageBoxCritical);
            return;
        }

        QString data,info,info2;
        QStringList lines,linesToImport,fieldNames,dataTypes,valuesToImport;
        QList<int> fieldindexes;
        data.append(FileImport.readAll().toStdString());
        lines=data.split("\n");

        //Header check
        fieldNames=lines[0].split(";");
        int nbColImport=fieldNames.count();
        int primaryFieldImport=-1;
        info="";
        for (int col = 0; col < nbColImport; ++col) {
            fieldindexes.append(w->model->FieldIndex(fieldNames[col]));
            if (fieldindexes[col]==-1 or//Column don't exists in the table.
                w->model->nonEditableColumns.contains(fieldindexes[col])) {//Column is readonly.
                dataTypes.append("");
                fieldindexes[col]=-1;
            } else {
                dataTypes.append(DataType(&db, w->model->tableName(),fieldNames[col]));
                info+=iif(info.isEmpty(),"",", ").toString()+fieldNames[col]+" ("+dataTypes[col]+")";
            }
            if(fieldNames[col]==w->model->sPrimaryKey)
                primaryFieldImport=col;
        }

        if (primaryFieldImport==-1)
            TypeImport=4; //Append only

        int choice=-1;
        if (info.isEmpty()) {
            MessageDialog(QObject::tr("Aucun champ dans le fichier %1 n'est modifiable dans l'onglet %2.")
                              .arg(FileInfoVerif.fileName())
                              .arg(w->lTabTitle->text().trimmed()),"",QStyle::SP_MessageBoxWarning);
            return;
        // } else if (primaryFieldImport==-1) {
        //     TypeImport=4; //Append only
        //     MessageDialog(QObject::tr("Champ %1 non trouvée dans le fichier %2.").arg(w->model->sPrimaryKey).arg(FileInfoVerif.fileName()),"",QStyle::SP_MessageBoxWarning);
        //     return;
        } else if (lines.count()<2) {
            MessageDialog(QObject::tr("Aucune ligne à importer dans le fichier %1.").arg(FileInfoVerif.fileName()),"",QStyle::SP_MessageBoxWarning);
            return;
        } else {
            //Concat lines for each records.
            parseCSV(data,"\n",linesToImport);
            linesToImport.removeFirst();//Remove header
            info2="";
            for (int i=0;i<4;i++){
                if (i<linesToImport.count())
                    info2+="<br>"+
                             StrFirst(linesToImport[i],60)+iif(linesToImport[i].length()>60,"...","").toString();
            }
            if(linesToImport.count()>4)
                info2+="<br>...";

            choice = RadiobuttonDialog(w->lTabTitle->text().trimmed()+"<br><br>"+
                                           "<b>"+tr("Importer des données depuis un fichier %1.").arg("CSV")+"</b><br>"+
                                           sFileName+"<br><br>"+
                                           "<b>"+tr("Les champs suivants vont être importés:")+"</b><br>"+info+"<br><br>"+
                                           "<b>"+tr("%1 lignes à importer:").arg(linesToImport.count()-1)+"</b>"+info2+"<br>"+
                                           tr("<u>Fusionner</u>: les lignes déjà présentes seront mises à jour, les autres seront créées.")+"<br>"+
                                           tr("<u>Mettre à jour</u>: seules les lignes visibles seront mises à jour, aucune nouvelle ligne ne sera créée.")+"<br>"+
                                           tr("<u>Supprimer</u>: les lignes visibles seront supprimées si elles ne sont pas utilisées ailleurs)."),
                                       {tr("Fusionner, priorité aux données importées"),                               //0
                                        tr("Fusionner, priorité aux données déjà présentes"),                          //1
                                        tr("Mettre à jour, priorité aux données importées"),                           //2
                                        tr("Mettre à jour, priorité aux données déjà présentes"),                      //3
                                        tr("Ajouter les lignes absentes, ne pas modifier les lignes déjà présentes"),  //4
                                        tr("Supprimer puis importer, priorité aux données importées"),                 //5
                                        tr("Supprimer puis importer, priorité aux données déjà présentes")},TypeImport);//6
            if (choice==-1) return;

            TypeImport=choice;
        }

        if (primaryFieldImport==-1 and choice!=4) {
            MessageDialog(QObject::tr("Champ %1 non trouvée dans le fichier %2.\nSeul l'ajout de ligne est éventuellement possible.").arg(w->model->sPrimaryKey).arg(FileInfoVerif.fileName()),"",QStyle::SP_MessageBoxWarning);
            return;
        }

       //dbSuspend(&db,false,userDataEditing,ui->lDBErr);

        //Backup the table.
        //dbOpen(ui->lDB->text(),false,false,true);//Enable the DROP TABLE.
        PotaQuery pQuery(db);
        pQuery.lErr=ui->lDBErr;
        //pQuery.ExecMultiShowErr("DROP TABLE IF EXISTS temp.Temp_"+w->model->tableName()+";"+
        //                         "CREATE TEMP TABLE Temp_"+w->model->tableName()+" AS SELECT * FROM "+w->model->tableName(),";",nullptr);
        //QString tempTableName="Temp"+QDateTime::currentDateTime().toString("hhmmss")+w->model->tableName();
        //pQuery.ExecShowErr("CREATE TEMP TABLE "+tempTableName+" AS SELECT * FROM "+w->model->tableName());

        int nbDeletedRows=0;
        int nbCreatedRows=0;
        int nbModifiedRows=0;
        int nbErrors=0;
        if (choice==5 or choice==6) {//Delete selected lines
            if(w->model->rowCount()>1 and
                !OkCancelDialog(tr("Attention, %1 lignes sont susceptibles d'être supprimées!").arg(w->model->rowCount()),QStyle::SP_MessageBoxWarning)) {
               //dbSuspend(&db,true,userDataEditing,ui->lDBErr);
                return;
            }
            AppBusy(true,ui->progressBar,w->model->rowCount(),"Delete %p%");
            for (int i=w->model->rowCount()-1;i>=0;i--) {
                ui->progressBar->setValue(ui->progressBar->value()+1);
                if(w->model->removeRow(i) and w->model->submitAll())
                    nbDeletedRows++;
                else {
                    qInfo() << "Import (delete) "+w->model->data(w->model->index(i,primaryFieldImport)).toString()+" : "+w->model->lastError().text();
                    w->model->revertAll();
                    nbErrors++;
                }
            }
            AppBusy(false,ui->progressBar);
        }

        //Remove filter
        if (choice!=2 and choice!=3 and w->pbFilter->isChecked()) {
            w->pbFilterClick(false);
            w->pbFilter->setChecked(false);
        }
        //Edit mode
        if (!w->pbEdit->isChecked()) {
            w->pbEdit->setChecked(true);
            w->pbEditClick();
        }


        //Import

        AppBusy(true,ui->progressBar,linesToImport.count(),FileInfoVerif.fileName()+" %p%");

        w->model->bBatch=true;

        bool bModified;

        for(int i=0;i<linesToImport.count();i++) {
            ui->progressBar->setValue(i);
            if (!linesToImport[i].isEmpty()) {
                parseCSV(linesToImport[i],";",valuesToImport);

                int recordToUpdate=-1;
                if(primaryFieldImport>-1 and valuesToImport.count()>primaryFieldImport and !valuesToImport[primaryFieldImport].isEmpty()){
                    //Search existing record
                    for (int i=0;i<w->model->rowCount();i++) {
                        if (w->model->data(w->model->index(i,0)).toString()==valuesToImport[primaryFieldImport]){
                            recordToUpdate=i;
                            break;
                        }
                    }
                }
                if(recordToUpdate>-1){
                    if(choice!=4){
                        //Update existing record.
                        bModified=false;
                        for (int col = 0; col < valuesToImport.count(); col++) {
                            if (fieldindexes[col]>-1){//Col exists in table.
                                if (choice==0 or choice==2 or choice==5 or//Priority to imported data
                                    w->model->data(w->model->index(recordToUpdate,fieldindexes[col])).toString()=="") {
                                    if (w->model->data(w->model->index(recordToUpdate,fieldindexes[col])).toString()!=valuesToImport[col]){
                                        w->model->setData(w->model->index(recordToUpdate,fieldindexes[col]),valuesToImport[col]);
                                        bModified=true;
                                    }
                                }
                            }
                        }

                        if (bModified) {
                            if(w->model->submitAll()) {
                                nbModifiedRows++;
                            } else {
                                qInfo() << "Import (update) "+linesToImport[i]+" : "+w->model->lastError().text();
                                w->model->revertAll();
                                nbErrors++;
                            }
                        }
                    }
                } else {
                    if(choice!=2 and choice!=3){
                        //Create new record.
                        int row=w->model->rowCount();
                        if (w->model->insertRow(row)){
                            for (int col = 0; col < valuesToImport.count(); col++) {
                                if (fieldindexes[col]>-1)//Col exists in table.
                                    w->model->setData(w->model->index(row,fieldindexes[col]),valuesToImport[col]);
                            }
                            if (w->model->submitAll()) {
                                nbCreatedRows++;
                            } else {
                                qInfo() << "Import (create) "+linesToImport[i]+" : "+w->model->lastError().text();
                                w->model->revertAll();
                                nbErrors++;
                            }
                        } else {
                            qInfo() << "Import (insert) "+linesToImport[i]+" : "+w->model->lastError().text();
                            nbErrors++;
                        }
                    }
                }
            }
        }
        w->model->bBatch=false;
        AppBusy(false,ui->progressBar);

        w->model->SubmitAllShowErr();//To deactivate commit and rollback buttons, and show modified cells.

        //dbSuspend(&db,true,userDataEditing,ui->lDBErr);

        MessageDialog(QObject::tr("%1 lignes supprimées").arg(nbDeletedRows)+"\n"+
                      QObject::tr("%1 lignes créées").arg(nbCreatedRows)+"\n"+
                      QObject::tr("%1 lignes modifiées").arg(nbModifiedRows)+"\n"+
                      QObject::tr("%1 erreurs").arg(nbErrors));
    }
}

void MainWindow::on_mExporter_triggered()
{
    if (ui->tabWidget->currentWidget()->objectName().startsWith("PW")){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());

        QString sFileName = QFileDialog::getSaveFileName(this, tr("Exporter les données dans un fichier %1").arg("CSV"),
                                                         PathExport+w->lTabTitle->text().trimmed(), "*.csv",
                                                         nullptr,QFileDialog::DontConfirmOverwrite);
        //Check filename.
        if (sFileName.isEmpty()) return;
        if (StrLast(sFileName,4)!=".csv")
            sFileName+=".csv";

        QFileInfo FileInfoVerif;
        FileInfoVerif.setFile(sFileName);
        if (!FileInfoVerif.exists() or
            OkCancelDialog(tr("Le fichier existe déjà")+"\n"+
                               sFileName+"\n"+
                               FileInfoVerif.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfoVerif.size()/1000)+" ko\n\n"+
                               tr("Remplacer ?"),QStyle::SP_MessageBoxWarning)) {
            QFile FileInfo2;
            if (FileInfoVerif.exists()) {
                FileInfo2.setFileName(sFileName);
                if (!FileInfo2.remove()) {
                    MessageDialog(tr("Impossible de supprimer le fichier")+"\n"+
                                      sFileName,"",QStyle::SP_MessageBoxCritical);
                    return;
                }
            }

            PathExport=FileInfoVerif.absolutePath()+QDir::separator();

            //Export
            QFile FileExport(sFileName);
            if (!FileExport.open(QIODevice::WriteOnly)) {// | QIODevice::Text
                MessageDialog(tr("Impossible de créer le fichier")+"\n"+
                                  sFileName,"",QStyle::SP_MessageBoxCritical);
                return;
            }

            QByteArray data;
            QStringList dataTypes;

            //Header export
            for (int col = 0; col < w->model->columnCount(); ++col) {
                if (col > 0)
                    data.append(";");
                data.append(w->model->headerData(col,Qt::Horizontal,Qt::EditRole).toString().toUtf8());
                dataTypes.append(DataType(&db, w->model->tableName(),w->model->headerData(col,Qt::Horizontal,Qt::EditRole).toString()));
            }
            data.append("\n");
            int row=0;
            int exportedRow=0;
            int totalRow=w->model->rowCount();
            if (FileExport.write(data)!=-1) {
                data.clear();

                AppBusy(true,ui->progressBar,totalRow,w->lTabTitle->text().trimmed()+" %p%");

                //Data export
                for (row = 0; row < w->model->rowCount(); ++row) {
                    for (int col = 0; col < w->model->columnCount(); ++col) {
                        if (col > 0) data.append(";");
                        if (dataTypes[col]=="REAL")
                            data.append(EscapeCSV(StrReplace(w->model->data(w->model->index(row, col)).toString(),".",",")).toUtf8());//todo param decimal separator
                        else
                            data.append(EscapeCSV(w->model->data(w->model->index(row, col)).toString()).toUtf8());
                    }
                    data.append("\n");
                    if (FileExport.write(data)!=-1)
                        exportedRow+=1;
                    data.clear();
                    ui->progressBar->setValue(row);
                }

                FileExport.close();
                AppBusy(false,ui->progressBar);
            }

            if (exportedRow==totalRow) {
                MessageDialog(tr("%1 lignes exportées vers le fichier").arg(str(totalRow))+"\n"+
                                  sFileName,"",QStyle::SP_MessageBoxInformation);
            } else
                MessageDialog(tr("%1 sur %2 lignes exportées vers le fichier").arg(str(exportedRow)).arg(str(totalRow))+"\n"+
                                  sFileName,"",QStyle::SP_MessageBoxWarning);
        }
    }
}

//Base data menu

void MainWindow::on_mFamilles_triggered()
{
    OpenPotaTab("Familles","Familles",tr("Familles"));
}

void MainWindow::on_mEspeces_triggered()
{
    OpenPotaTab("Especes","Espèces",tr("Espèces"));
}

void MainWindow::on_mVarietes_triggered()
{
    OpenPotaTab("Varietes","Variétés",tr("Variétés"));
}

void MainWindow::on_mApports_triggered()
{
    OpenPotaTab("Apports","Apports",tr("Apports"));
}

void MainWindow::on_mFournisseurs_triggered()
{
    OpenPotaTab("Fournisseurs","Fournisseurs",tr("Fournisseurs"));
}

void MainWindow::on_mTypes_de_planche_triggered()
{
    OpenPotaTab("TypesPlanche","Types_planche",tr("Types planche"));
}

void MainWindow::on_mITPTempo_triggered()
{
    OpenPotaTab("ITP_tempo","ITP__Tempo",tr("ITP"));
}

void MainWindow::on_mNotes_triggered()
{
    OpenPotaTab("Notes","Notes",tr("Notes"));
}

//Menu Assolement

void MainWindow::on_mRotations_triggered()
{
    OpenPotaTab("Rotations","Rotations",tr("Rotations"));
}

void MainWindow::on_mDetailsRotations_triggered()
{
    if (OpenPotaTab("Rotations_Tempo","Rotations_détails__Tempo",tr("Rot. (détails)"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//ID, necessary in the view for the triggers to update the real table.
    }
}

void MainWindow::on_mRotationManquants_triggered()
{
    OpenPotaTab("Especes__manquantes","Espèces__manquantes",tr("Espèces manquantes"));
}

void MainWindow::on_mPlanches_triggered()
{
    OpenPotaTab("Planches","Planches",tr("Planches"));
}

void MainWindow::on_mSuccessionParPlanche_triggered()
{
    //OpenPotaTab("SuccPlanches","Successions_par_planche",tr("Succ. planches"));
    if (OpenPotaTab("SuccPlanches","Cultures__Tempo",tr("Succ. planches"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//num_planche, necessary in the view for row painting.
    }
}

void MainWindow::on_mIlots_triggered()
{
    OpenPotaTab("Planches_Ilots","Planches_Ilots",tr("Ilots"));
}

//Menu Planification

void MainWindow::on_mCulturesParplante_triggered()
{
    OpenPotaTab("Cult_planif_espèces","Cult_planif_espèces",tr("Cult.prévues espèces"));
}

void MainWindow::on_mCulturesParIlots_triggered()
{
    OpenPotaTab("Cult_planif_ilots","Cult_planif_ilots",tr("Cult.prévues ilots"));
}

void MainWindow::on_mCulturesParPlanche_triggered()
{
    OpenPotaTab("Cult_planif","Cult_planif",tr("Cult.prévues"));
}

void MainWindow::on_mCreerCultures_triggered()
{
    PotaQuery pQuery(db);
    pQuery.lErr=ui->lDBErr;
    int NbCultPlanif=pQuery.Selec0ShowErr("SELECT count() FROM Cult_planif").toInt();
    if (NbCultPlanif==0) {
        MessageDialog(tr("Aucune culture à planifier:")+"\n\n"+
                          tr("- Créez des rotations")+"\n"+
                          tr("- Vérifiez que le paramètre 'Planifier_planches' n'exclut pas toutes les planches."),"",QStyle::SP_MessageBoxInformation);
        return;
    }

    int NbCultPlanifRetard=pQuery.Selec0ShowErr("SELECT count() FROM Cult_planif WHERE coalesce(Date_semis,Date_plantation)<DATE('now')").toInt();
    int NbCultAVenir=pQuery.Selec0ShowErr("SELECT count() FROM C_non_commencées").toInt();
    QStyle::StandardPixmap icon;
    QString CultAVenir;
    if(NbCultAVenir>0) {
        icon=QStyle::SP_MessageBoxWarning;
        CultAVenir="\n\n"+tr("IL Y A DÉJÀ %1 CULTURES NI SEMÉES NI PLANTÉES.").arg(NbCultAVenir)+"\n"+
                   iif(NbCultAVenir>NbCultPlanif*0.9,tr("Peut-être avez-vous déjà généré les prochaines cultures."),"").toString();
    } else {
        icon=QStyle::SP_MessageBoxQuestion;
        CultAVenir="";
    }
    if (OkCancelDialog(tr("Créer les cultures de la saison %1 ?").arg("<b>"+pQuery.Selec0ShowErr("SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'").toString()+"</b>")+"<br><br>"+
                       tr("La saison courante peut être modifiée dans les paramètres (menu 'Edition').")+"<br><br>"+
                       tr("%1 cultures vont être créées en fonction des rotations.").arg("<b>"+str(NbCultPlanif)+"</b>")+"<br>"+
                       tr("Id de la dernière culture:")+" "+str(NbCultAVenir)+
                       CultAVenir,
                       icon)) {
        int choice=2;
        if (NbCultPlanifRetard>0){
            choice = RadiobuttonDialog(tr("Parmis les %1 cultures à créer, il y en a %2 dont la date de la 1ère opération (semis ou plantation) est déjà passée.").arg(NbCultPlanif).arg(NbCultPlanifRetard),
                                           {tr("Ne pas créer ces cultures en retard"),
                                            tr("Créer aussi ces cultures en retard")},
                                            iif(NbCultPlanifRetard<NbCultPlanif/10,1,0).toInt(),
                                            QStyle::SP_MessageBoxWarning);
            if (choice<0)
                return;
        }
        int IdCult1=pQuery.Selec0ShowErr("SELECT max(Culture) FROM Cultures").toInt();
        bool result;

        if (choice==0)
            result=pQuery.ExecShowErr("INSERT INTO Cultures (IT_Plante,Variété,Fournisseur,Planche,D_planif,Longueur,Nb_rangs,Espacement) "
                                       "SELECT IT_plante,Variété,Fournisseur,Planche,(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'),Longueur,Nb_rangs,Espacement "
                                       "FROM Cult_planif WHERE coalesce(Date_semis,Date_plantation)>=DATE('now')");
        else
            result=pQuery.ExecShowErr("INSERT INTO Cultures (IT_Plante,Variété,Fournisseur,Planche,D_planif,Longueur,Nb_rangs,Espacement) "
                                       "SELECT IT_plante,Variété,Fournisseur,Planche,(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'),Longueur,Nb_rangs,Espacement "
                                       "FROM Cult_planif");
        if (result) {
            int IdCult2=pQuery.Selec0ShowErr("SELECT min(Culture) FROM Cultures WHERE Culture>"+str(IdCult1)).toInt();
            int IdCult3=pQuery.Selec0ShowErr("SELECT max(Culture) FROM Cultures").toInt();
            if (IdCult3>IdCult2 and IdCult2>IdCult1)
                MessageDialog(tr("%1 cultures créées sur %2 cultures prévues.").arg(IdCult3-IdCult2+1).arg(NbCultPlanif)+"\n\n"+
                                  tr("Id culture:")+" "+str(IdCult2)+" > "+str(IdCult3),"",QStyle::SP_MessageBoxInformation);
            else
                MessageDialog(tr("%1 culture créée sur %2 cultures prévues.").arg("0").arg(NbCultPlanif),"",QStyle::SP_MessageBoxWarning);
        }
        else
            MessageDialog(tr("Impossible de créer les cultures."),"",QStyle::SP_MessageBoxCritical);

    }
}

void MainWindow::on_mSemences_triggered()
{
    OpenPotaTab("Varietes_inv_et_cde","Variétés__inv_et_cde",tr("Inv. et cde semence"));
}

//Menu Cultures

void MainWindow::on_mCuNonTer_triggered()
{
    OpenPotaTab("Cultures_non_terminees","Cultures__non_terminées",tr("Non terminées"));
}

void MainWindow::on_mCouverture_triggered()
{
     OpenPotaTab("Especes__couverture","Espèces__couverture",tr("Couverture obj."));
}

void MainWindow::on_mCuASemer_triggered()
{
    OpenPotaTab("Cultures_a_semer","Cultures__à_semer",tr("A semer"));
}

void MainWindow::on_mCuASemerSA_triggered()
{
    if (OpenPotaTab("Cultures_a_semer_SA","Cultures__à_semer_SA",tr("A semer (SA)"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//Culture, necessary in the view for sow commited celles.
    }
}

void MainWindow::on_mCuASemerD_triggered()
{
    OpenPotaTab("Cultures_a_semer_D","Cultures__à_semer_D",tr("A semer (D)"));
}

void MainWindow::on_mCuAPlanter_triggered()
{
    OpenPotaTab("Cultures_a_planter","Cultures__à_planter",tr("A planter"));
}

void MainWindow::on_mCuARecolter_triggered()
{
    OpenPotaTab("Cultures_a_recolter","Cultures__à_récolter",tr("A récolter"));
}

void MainWindow::on_mCuSaisieRecoltes_triggered()
{
    if (OpenPotaTab("Recoltes__Saisies","Récoltes__Saisies",tr("Récoltes"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//ID, necessary in the view for the triggers to update the real table.
        //Go to last row.
        w->tv->setCurrentIndex(w->model->index(w->model->rowCount()-1,1));
    }
}

void MainWindow::on_mCuATerminer_triggered()
{
    OpenPotaTab("Cultures_a_terminer","Cultures__à_terminer",tr("A terminer"));
}

void MainWindow::on_mCuToutes_triggered()
{
    OpenPotaTab("Cultures","Cultures",tr("Cultures"));
}

//Menu Stock
void MainWindow::on_mDestinations_triggered()
{
    OpenPotaTab("Destinations__conso","Destinations__conso",tr("Destinations"));
}

void MainWindow::on_mEsSaisieSorties_triggered()
{
    if (OpenPotaTab( "Consommations__Saisies","Consommations__Saisies",tr("Consommations"))) {
            PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
            w->tv->hideColumn(0);//ID, necessary in the view for the triggers to update the real table.
            //Go to last row.
            w->tv->setCurrentIndex(w->model->index(w->model->rowCount()-1,1));
        }
}

void MainWindow::on_mInventaire_triggered()
{
    OpenPotaTab( "Especes__inventaire","Espèces__inventaire",tr("Inventaire"));
}

//Menu Analyses

void MainWindow::on_mAnaITP_triggered()
{
    OpenPotaTab("ITP_analyse","ITP__analyse",tr("Analyse IT"));
}

void MainWindow::on_mAnaCultures_triggered()
{
    OpenPotaTab("Cultures_analyse","Cultures__analyse",tr("Analyse cultures"));
}

void MainWindow::on_mIncDatesCultures_triggered()
{
    OpenPotaTab("Cultures_inc_dates","Cultures__inc_dates",tr("Inc. dates cultures"));
}

void MainWindow::on_cbTheme_currentIndexChanged(int index)
{
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::ToolTipBase ,QColor( "#d7c367" )); //ffffdc
    palette.setColor(QPalette::ToolTipText ,QColor( "#000000" ));
    palette.setColor(QPalette::BrightText ,QColor( "#ffffff" ));
    palette.setColor(QPalette::Highlight ,QColor( "#0c75de" ));
    palette.setColor(QPalette::Active , QPalette::HighlightedText ,QColor( "#ffffff" ));
    palette.setColor(QPalette::Inactive , QPalette::HighlightedText ,QColor( "#ffffff" ));
    if (index==1) { //Dark
        palette.setColor(QPalette::Window ,QColor( "#3d3d3d" ));
        palette.setColor(QPalette::Base ,QColor( "#3d3d3d" ));
        palette.setColor(QPalette::Button ,QColor( "#3d3d3d" ));
        palette.setColor(QPalette::AlternateBase ,QColor( "#393939" )); //Unused ?
        palette.setColor(QPalette::Light ,QColor( "#4c4c4c" ));
        palette.setColor(QPalette::Midlight ,QColor( "#454545" ));
        palette.setColor(QPalette::Dark ,QColor( "#363636" ));
        palette.setColor(QPalette::Mid ,QColor( "#454545" )); //Unused ?
        palette.setColor(QPalette::Shadow ,QColor( "#030303" )); //Unused ?
        //  QPalette::Active
        palette.setColor( QPalette::Active , QPalette::WindowText ,QColor( "#dadada" ));
        palette.setColor( QPalette::Active , QPalette::Text ,QColor( "#dadada" ));
        palette.setColor( QPalette::Active , QPalette::ButtonText ,QColor( "#dadada" ));
        //  QPalette::Inactive
        palette.setColor( QPalette::Inactive , QPalette::WindowText ,QColor( "#ffffff" ));
        palette.setColor( QPalette::Inactive , QPalette::Text ,QColor( "#dadada" ));
        palette.setColor( QPalette::Inactive , QPalette::ButtonText ,QColor( "#dadada" ));
        //  QPalette::Disabled
        palette.setColor( QPalette::Disabled , QPalette::WindowText ,QColor( "#8b8b8b" ));
        palette.setColor( QPalette::Disabled , QPalette::Text ,QColor( "#8b8b8b" ));
        palette.setColor( QPalette::Disabled , QPalette::ButtonText ,QColor( "#8b8b8b" ));
        palette.setColor( QPalette::Disabled , QPalette::HighlightedText ,QColor( "#dadada" ));
    } else {
        palette.setColor(QPalette::Window ,QColor( "#fbfbfb" ));
        palette.setColor(QPalette::Base ,QColor( "#fbfbfb" ));
        palette.setColor(QPalette::Button ,QColor( "#dbdbdb" ));
        palette.setColor(QPalette::AlternateBase ,QColor( "#ebebeb" )); //Unused ?
        palette.setColor(QPalette::Light ,QColor( "#ffffff" ));
        palette.setColor(QPalette::Midlight ,QColor( "#dcdcdc" ));
        palette.setColor(QPalette::Dark ,QColor( "#bcbcbc" ));
        palette.setColor(QPalette::Mid ,QColor( "#ffffff" )); //Unused ?
        palette.setColor(QPalette::Shadow ,QColor( "#0d0d0d" )); //Unused ?
        //  QPalette::Active
        palette.setColor( QPalette::Active , QPalette::WindowText ,QColor( "#303030" ));
        palette.setColor( QPalette::Active , QPalette::Text ,QColor( "#303030" ));
        palette.setColor( QPalette::Active , QPalette::ButtonText ,QColor( "#303030" ));
        //  QPalette::Inactive
        palette.setColor( QPalette::Inactive , QPalette::WindowText ,QColor( "#000000" ));
        palette.setColor( QPalette::Inactive , QPalette::Text ,QColor( "#303030" ));
        palette.setColor( QPalette::Inactive , QPalette::ButtonText ,QColor( "#303030" ));
        //  QPalette::Disabled
        palette.setColor( QPalette::Disabled , QPalette::WindowText ,QColor( "#959595" ));
        palette.setColor( QPalette::Disabled , QPalette::Text ,QColor( "#959595" ));
        palette.setColor( QPalette::Disabled , QPalette::ButtonText ,QColor( "#959595" ));
        palette.setColor( QPalette::Disabled , QPalette::HighlightedText ,QColor( "#303030" ));
    }
    QApplication::setPalette(palette);

    SetMenuIcons();
}

void MainWindow::on_cbFont_currentTextChanged(const QString &arg1)
{
    if (ui->cbFont->count()<2) return;//Initialisation of cbfont entries.

    QFont font = this->font();
    font.setPointSize(arg1.toInt());
    setFont(font);

    QList<QWidget*> widgets = findChildren<QWidget*>();
    foreach (QWidget* widget, widgets) {
        widget->setFont(font);
    }

    //Tab titles
    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        if (ui->tabWidget->widget(i)->objectName().startsWith("PW")){
            PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->widget(i));
            ui->tabWidget->tabBar()->setTabButton(i, QTabBar::LeftSide, nullptr);
            ui->tabWidget->tabBar()->setTabButton(i, QTabBar::LeftSide, w->lTabTitle);
            // w->cbFilterType->setFont(font);
            // w->cbFilterType->setMinimumHeight(w->cbFilterType->sizeHint().height());
            // w->cbFilterType->updateGeometry();
        }
    }

    ui->lDBlabel->setFixedWidth(110*arg1.toInt()/10);
}





