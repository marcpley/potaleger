#include "mainwindow.h"
#include "Dialogs.h"
#include "qsqlerror.h"
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
#include <QLocale>

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

bool MainWindow::OpenPotaTab(QString const sObjName, QString const sTableName, QString const sTitre, QString const sDesc) {
    AppBusy(true,ui->progressBar,0,0,sTitre);
    //Recherche parmis les onglets existants.
    for (int i=0; i < ui->tabWidget->count(); i++) {
        if (ui->tabWidget->widget(i)->objectName()=="PW"+sObjName ) {
            ui->tabWidget->setCurrentIndex(i);
            if (sTableName=="Temp_UserSQL") {
                ClosePotaTab(ui->tabWidget->currentWidget()); //Close and reopen tab to reset columns
                ui->tabWidget->setCurrentIndex(fmax(i-1,0));
            } else {
                PotaWidget *wc=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
                if (!wc->pbCommit->isEnabled())
                    wc->pbRefreshClick();
                break;
            }
        }
    }
    AppBusy(false,ui->progressBar);

    if (ui->tabWidget->currentWidget()->objectName()!="PW"+sObjName) { //Existing tab not found.
        AppBusy(true,ui->progressBar,100,0,sTableName+" - init");
        PotaQuery query(db);
        int fieldCount=query.Select0ShowErr("SELECT count(*) FROM pragma_table_xinfo('"+sTableName+"')").toInt();
        ui->progressBar->setMaximum(fieldCount*2+12);
        //Create tab
        PotaWidget *w=new PotaWidget(ui->tabWidget);
        w->setObjectName("PW"+sObjName);
        w->lErr=ui->lDBErr;
        w->cbFontSize=ui->cbFont;
        w->model->db=&db;
        w->model->progressBar=ui->progressBar;
        w->delegate->cTableColor=TableColor(sTableName,"");//before init
        w->model->progressBar->setValue(w->model->progressBar->value()+1);

        w->Init(sTableName);

        int pos=ui->progressBar->value();
        AppBusy(false,ui->progressBar);

        if (w->model->SelectShowErr()) {
            AppBusy(true,ui->progressBar,fieldCount*2+12,pos,sTableName+" - UI");
            bool bEdit=false;
            PotaQuery query(db);
            if (!ReadOnlyDb and
                (query.Select0ShowErr("SELECT count() FROM sqlite_schema "      //Table
                                     "WHERE (tbl_name='"+sTableName+"')AND"
                                           "(sql LIKE 'CREATE TABLE "+sTableName+" (%')").toInt()+
                query.Select0ShowErr("SELECT count() FROM sqlite_schema "
                                    "WHERE (tbl_name='"+sTableName+"')AND"    //View with trigger instead of insert
                                          "(sql LIKE 'CREATE TRIGGER "+sTableName+"_INSERT INSTEAD OF INSERT ON "+sTableName+" %')").toInt()==1)){
                QPalette palette=w->sbInsertRows->palette();
                palette.setColor(QPalette::Text, Qt::white);
                palette.setColor(QPalette::Base, QColor(234,117,0,110));
                palette.setColor(QPalette::Button, QColor(234,117,0,110));
                w->sbInsertRows->setPalette(palette);
                w->sbInsertRows->setEnabled(true);
                w->pbInsertRow->setEnabled(true);
                w->pbDuplicRow->setEnabled(true);
                //w->bAllowInsert=true;
                w->pbDeleteRow->setEnabled(true);
                //w->bAllowDelete=true;
                bEdit=true;
            } else if (ReadOnlyDb or
                       (query.Select0ShowErr("SELECT count() FROM sqlite_schema "
                                            "WHERE (tbl_name='"+sTableName+"')AND"    //View without trigger instead of.
                                                  "(sql LIKE 'CREATE TRIGGER "+sTableName+"_UPDATE INSTEAD OF UPDATE ON "+sTableName+" %')").toInt()==0)) {
                w->pbEdit->setVisible(false);
            }
            w->model->progressBar->setValue(w->model->progressBar->value()+1);
            if(w->model->rowCount()==0) {
                if (bEdit and(FkFilter(&db,w->model->RealTableName(),"","",w->model->index(0,0),true)!="NoFk")){
                    w->lRowSummary->setText(tr("<- cliquez ici pour saisir des %1").arg(w->model->RealTableName().replace("_"," ").toLower()));
                } else {
                    SetColoredText(ui->lDBErr,"","");
                    AppBusy(false,ui->progressBar);
                    MessageDlg(windowTitle(),sTitre,NoData(&db,w->model->tableName()),QStyle::SP_MessageBoxInformation,450);
                    w->deleteLater();
                    return false;
                }
            }

            w->model->progressBar->setValue(w->model->progressBar->value()+1);

            for (int i=0; i<w->model->columnCount();i++){
                //Store field name in header EditRole.
                w->model->setHeaderData(i,Qt::Horizontal,w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole),Qt::EditRole);
                //Store corrected field name in header DisplayRole.
                QString headerDataDisplayRole=w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString();
                headerDataDisplayRole=headerDataDisplayRole.replace("_pc",""); //'%' rigth added to the data.
                headerDataDisplayRole=headerDataDisplayRole.replace("_"," ");
                if (w->model->baseDataFields[i]=='x') headerDataDisplayRole=headerDataDisplayRole+" 🔺️";
                w->model->setHeaderData(i,Qt::Horizontal,headerDataDisplayRole,Qt::DisplayRole);

                //Table color.
                w->delegate->cColColors[i]=TableColor(sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString());

                if (sTableName.startsWith("Cultures") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Etat")
                    w->delegate->RowColorCol=i;
                else if (sTableName.startsWith("Cultures") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="num_planche")
                    w->delegate->RowColorCol=i;
                else if (sTableName.startsWith("Params") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Section")
                    w->delegate->RowColorCol=i;

                //Tooltip
                QString sTT=ToolTipField(&db,sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),
                                         w->model->dataTypes[i],w->model->baseDataFields[i]);
                if (sTT!="")
                    w->model->setHeaderData(i, Qt::Horizontal, sTT, Qt::ToolTipRole);

                //All columns read only for start
                w->model->nonEditableColumns.insert(i);

                if (w->model->dataTypes[i]=="DATE")
                    w->model->dateColumns.insert(i);
                else if (sTableName!="Params" and
                         FieldIsMoney(w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()))
                    w->model->moneyColumns.insert(i);

                //Hide _columns
                if (w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString().startsWith("_"))
                    w->tv->hideColumn(i);

                w->model->progressBar->setValue(w->model->progressBar->value()+1);

            }

            w->RefreshHorizontalHeader();

            w->model->progressBar->setValue(w->model->progressBar->value()+1);

            //Tab user settings
            QSettings settings;//("greli.net", "Potaléger");

            //filter
            settings.beginGroup("Filter");
            w->iTypeText=settings.value(sTableName+"-FilterTypeText").toInt();
            w->iTypeDate=settings.value(sTableName+"-FilterTypeDate").toInt();
            w->iTypeReal=settings.value(sTableName+"-FilterTypeReal").toInt();
            settings.endGroup();

            w->model->progressBar->setValue(w->model->progressBar->value()+1);

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

            w->model->progressBar->setValue(w->model->progressBar->value()+1);

            settings.beginGroup("ColVisible");
            for (int i=0; i<w->model->columnCount();i++) {
                if (settings.value(sTableName+"-"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()).isValid())
                    w->tv->setColumnHidden(i,settings.value(sTableName+"-"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()).toBool());
                settings.setValue(w->model->tableName()+"-"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),w->tv->isColumnHidden(i));
            }
            settings.endGroup();

            w->model->progressBar->setValue(w->model->progressBar->value()+1);

            ui->mCloseTabs->setEnabled(true);
            ui->mCloseTab->setEnabled(true);

            w->tv->setFocus();
            SetColoredText(ui->lDBErr,sTableName+
                                      iif(ReadOnlyDb," ("+tr("lecture seule")+")","").toString(),
                                      iif(ReadOnlyDb,"Info","Ok").toString());

            AppBusy(false,ui->progressBar);
            AppBusy(true);

            ui->tabWidget->addTab(w,sTitre);
            ui->tabWidget->setCurrentWidget(w);

            //Colored tab title
            ui->tabWidget->tabBar()->setTabText(ui->tabWidget->currentIndex(), ""); // Remove normal text
            QFont font=this->font();
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

            if (!sDesc.isEmpty())
                ui->tabWidget->setTabToolTip(ui->tabWidget->currentIndex(),sDesc);
            else
                ui->tabWidget->setTabToolTip(ui->tabWidget->currentIndex(),ToolTipTable(&db,w->model->tableName()));

            if (lastRow(sTableName)) // go to last row
                w->tv->setCurrentIndex(w->model->index(w->model->rowCount()-1,1));

            w->tv->setFocus();
            AppBusy(false);
            return true;
        }
        else {
            w->deleteLater();//Echec de la création de l'onglet.
            return false;
        }
    }
    return false;
};

void MainWindow::ClosePotaTab(QWidget *Tab)
{
    if (Tab->objectName().startsWith("PW")) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(Tab);

        if (w->pbCommit->isEnabled()) {
            if (YesNoDialog(windowTitle(),
                            w->lTabTitle->text().trimmed()+"\n\n"+
                               tr("Valider les modifications avant de fermer ?"))) {
                if (!w->model->SubmitAllShowErr())
                    return;
            } else {
                if (!w->model->RevertAllShowErr())
                    return;
            }
        }

        //Save user settings
        QSettings settings;//("greli.net", "Potaléger");

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

        //Visible col
        settings.beginGroup("ColVisible");
        for (int i=0; i<w->model->columnCount();i++)
            settings.setValue(w->model->tableName()+"-"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),w->tv->isColumnHidden(i));
        settings.endGroup();

        settings.setValue(w->model->tableName()+"-pageFilter",w->cbPageFilter->currentIndex());

        if (w->model->tableName()=="Params") {
            PotaQuery pQuery(db);
            setWindowTitle("Potaléger"+pQuery.Select0ShowErr("SELECT ' - '||Valeur FROM Params WHERE Paramètre='Utilisateur'").toString());
        } else if (w->model->tableName()=="Cultures__A_faire") {
            PotaQuery pQuery(db);
            pQuery.ExecShowErr("UPDATE Cultures SET A_faire=NULL WHERE A_faire='. '");
        }

        if (ui->tabWidget->count()<3) {//Fermeture du dernier onglet data ouvert.
            ui->mCloseTabs->setEnabled(false);
            ui->mCloseTab->setEnabled(false);
        }
        Tab->deleteLater();
    }
}

void MainWindow::ClosePotaTabs()
{
    for (int i=ui->tabWidget->count()-1; i >=0 ; i--) {
        ClosePotaTab(ui->tabWidget->widget(i));
    }
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    //Set to bolt title of new tab, unbolt others.
    for (int i=0; i < ui->tabWidget->count(); ++i) {
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
    } else {
        PotaWidget *wc=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        wc->SetSizes();
    }
    SetColoredText(ui->lDBErr,"","");
}

//File menu

void MainWindow::on_mSelecDB_triggered()
{
    ClosePotaTabs();
    const QString sFileName=QFileDialog::getOpenFileName( this, tr("Base de données %1").arg("Potaléger"), ui->lDB->text(), "*.sqlite3");
    if (sFileName != "") {
        PotaDbClose();
        PotaDbOpen(sFileName,"",false);
    }
}

void MainWindow::on_mUpdateSchema_triggered()
{
    ClosePotaTabs();
    if (OkCancelDialog("Potaléger "+ui->lVer->text(),tr("Mettre à jour le schéma de la BDD ?")+"\n\n"+
                       ui->lDB->text()+"\n\n"+
                       tr("La structures des tables, les vues et les déclencheurs vont être recréés.")+"\n"+
                       tr("Vos données vont être conservées.")+"\n"+
                       tr("En cas d'échec, votre BDD sera remise dans son état initial."),QStyle::SP_MessageBoxQuestion,600)){
        QString sFileName=ui->lDB->text();
        PotaDbClose();
        if(!PotaDbOpen(sFileName,"",true))
            PotaDbOpen(sFileName, "",false);
    }
}

void MainWindow::on_mCopyDB_triggered()
{
    ClosePotaTabs();
    QFileInfo FileInfo,FileInfoVerif;
    FileInfo.setFile(ui->lDB->text());
    if (!FileInfo.exists()) {
        MessageDlg("Potaléger "+ui->lVer->text(),tr("Le fichier de BDD n'existe pas.")+"\n"+ui->lDB->text(),"",QStyle::SP_MessageBoxCritical);
        return;
    }

    const QString sFileName=QFileDialog::getSaveFileName(this, tr("Copie de la base de données %1").arg("Potaléger"), ui->lDB->text(), "*.sqlite3",nullptr,QFileDialog::DontConfirmOverwrite);
    if (sFileName.isEmpty()) return;
    FileInfoVerif.setFile(sFileName);
    if (!FileInfoVerif.exists() or
        OkCancelDialog("Potaléger "+ui->lVer->text(),tr("Le fichier existe déjà")+"\n"+
                       sFileName+"\n"+
                       FileInfoVerif.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfoVerif.size()/1000)+" ko\n\n"+
                       tr("Remplacer par")+"\n"+
                       ui->lDB->text()+"\n"+
                       FileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfo.size()/1000)+" ko ?",QStyle::SP_MessageBoxWarning,600)) {
        QFile FileInfo1,FileInfo2,FileInfo3;
        if (FileInfoVerif.exists()) {
            FileInfo2.setFileName(sFileName);
            if (!FileInfo2.remove())
            {
                MessageDlg("Potaléger "+ui->lVer->text(),tr("Impossible de supprimer le fichier")+"\n"+
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
            MessageDlg("Potaléger "+ui->lVer->text(),tr("Impossible de copier le fichier")+"\n"+
                          sFileNameSave+"\n"+
                          tr("vers le fichier")+"\n"+
                          sFileName,"",QStyle::SP_MessageBoxCritical);
        PotaDbOpen(sFileNameSave,"",false);
    }
}

void MainWindow::on_mCreateDB_triggered()
{
    CreateNewDB(false);
}

void MainWindow::on_mCreateEmptyDB_triggered()
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
//         sFileName="C:\\Users\\NomUtilisateur\\Documents";
// #else
//         sFileName="/home/NomUtilisateur/Documents";
// #endif
        sFileName=QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+
                  QDir::toNativeSeparators("/Documents/Potaleger.sqlite3");
    }

    sFileName=QFileDialog::getSaveFileName(this, tr("Nom pour la BDD %1").arg("Potaléger "+sEmpty), sFileName, "*.sqlite3",nullptr,QFileDialog::DontConfirmOverwrite);
    if (sFileName.isEmpty()) return;

    FileInfoVerif.setFile(sFileName);
    if (!FileInfoVerif.exists() or
        OkCancelDialog("Potaléger "+ui->lVer->text(),tr("Le fichier existe déjà")+"\n"+
                       sFileName+"\n"+
                       FileInfoVerif.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfoVerif.size()/1000)+" ko\n\n"+
                       tr("Remplacer par une base de données %1 ?").arg(sEmpty),QStyle::SP_MessageBoxWarning,600))
    {
        QFile FileInfo1,FileInfo2,FileInfo3;
        if (FileInfoVerif.exists())
        {
            FileInfo2.setFileName(sFileName);
            if (!FileInfo2.remove())
            {
                MessageDlg("Potaléger "+ui->lVer->text(),tr("Impossible de supprimer le fichier")+"\n"+
                                  sFileName,"",QStyle::SP_MessageBoxCritical);
                return;
            };
        }
        QString sFileNameSave=ui->lDB->text();
        FileInfo1.setFileName(ui->lDB->text());
        PotaDbClose();

        if (!PotaDbOpen(sFileName,iif(bEmpty,"New","NewWithBaseData").toString(),false)) {
            MessageDlg("Potaléger "+ui->lVer->text(),tr("Impossible de créer la BDD %1").arg(sEmpty)+"\n"+
                              sFileName,"",QStyle::SP_MessageBoxCritical);
            dbClose();
            PotaDbOpen(sFileNameSave,"",false);
        }
    }
}


void MainWindow::on_mTableList_triggered()
{
    QString sQuery="";
    QString sTableName="";
    QString sPrimaryKey="";
    int fieldCount;
    PotaQuery query1(db);
    PotaQuery query2(db);

    query1.ExecShowErr("PRAGMA table_list;");
    while (query1.next()) {
        if ((query1.value("type").toString()=="table") and
            (query1.value("name").toString()!="Temp_Table_list") and
            !query1.value("name").toString().startsWith("sqlite")) {
            sTableName=query1.value("name").toString();
            sPrimaryKey="";
            fieldCount=0;

            query2.ExecShowErr("PRAGMA table_xinfo("+sTableName+")");
            while (query2.next()){
                fieldCount+=1;
                if (sPrimaryKey.isEmpty() and query2.value(5).toInt()==1) {
                    sPrimaryKey=query2.value(1).toString();
                }
            }

            int triggerCount=query2.Select0ShowErr("SELECT count() FROM sqlite_schema "
                                                  "WHERE (tbl_name='"+sTableName+"')AND"
                                                        "(type='trigger')").toInt();
            int useCount=query2.Select0ShowErr("SELECT count() FROM sqlite_schema "
                                              "WHERE (tbl_name!='"+sTableName+"')AND "
                                                    "NOT(tbl_name LIKE 'Temp_%')AND"
                                                    "((sql LIKE '% "+sTableName+" %')OR"
                                                     "(sql LIKE '% "+sTableName+")%')OR"
                                                     "(sql LIKE '% "+sTableName+";'))").toInt();


            sQuery +="SELECT " //+query1.value("type").toString()+"' Type, "+
                     "'"+sTableName+"' Name, "
                     "'"+iif(sPrimaryKey.isEmpty()," ",sPrimaryKey).toString()+"' PK_field_name, "+
                     str(fieldCount)+" Field_count, "+
                     str(triggerCount)+" Trigger_count, "+
                     str(useCount)+" Use_count, "+
                     "(SELECT count() FROM "+sTableName+") Rec_count "
                     "UNION \n";
        }
    }

    sQuery=StrRemoveLasts(sQuery,8);

    if (!sQuery.isEmpty()) {
        query1.ExecShowErr("DROP VIEW IF EXISTS Table_list;");
        query1.ExecShowErr("CREATE VIEW Table_list AS "+sQuery);
        OpenPotaTab("Table_list","Table_list",tr("Liste des tables"),"");
    }
}

void MainWindow::on_mViewList_triggered()
{
    QString sQuery="";
    PotaQuery query1(db);

    sQuery="SELECT "
           "SS.name Name, "
           "(SELECT count() FROM pragma_table_xinfo(SS.tbl_name)) Field_count, "
           "(SELECT count() FROM sqlite_schema SS2 WHERE (SS2.tbl_name=SS.tbl_name)AND(SS2.type='trigger')) Trigger_count,"
           "(SELECT count() FROM sqlite_schema SS3 WHERE (SS3.tbl_name!=SS.tbl_name)AND "
                                                        "NOT(SS3.tbl_name LIKE 'Temp_%')AND"
                                                        "((SS3.sql LIKE '% '||SS.tbl_name||' %')OR"
                                                         "(SS3.sql LIKE '% '||SS.tbl_name||')%')OR"
                                                         "(SS3.sql LIKE '% '||SS.tbl_name||';'))) Use_count "
           "FROM sqlite_schema SS "
           "WHERE SS.type='view' AND NOT(SS.name='View_list')";// AND NOT(SS.name LIKE 'Temp_%'

    if (!sQuery.isEmpty()) {
        query1.ExecShowErr("DROP VIEW IF EXISTS View_list;");
        query1.ExecShowErr("CREATE VIEW View_list AS "+sQuery);
        OpenPotaTab("View_list","View_list",tr("Liste des vues"),"");
    }
}

void MainWindow::on_mFKErrors_triggered()
{
    QString sQuery="";
    QString sTableName="";
    QString sPrimaryKey="";
    PotaQuery query1(db);
    PotaQuery query2(db);

    query1.ExecShowErr("PRAGMA table_list;");
    while (query1.next()) {
        if (query1.value("type").toString()=="table" and
            query1.value("name").toString()!="Params" and
            !query1.value("name").toString().startsWith("sqlite")) {//No sqlite tables.
            sTableName=query1.value("name").toString();
            sPrimaryKey="";

            query2.ExecShowErr("PRAGMA table_xinfo("+sTableName+")");
            while (query2.next()){
                if (query2.value(5).toInt()==1) {
                    sPrimaryKey=query2.value(1).toString();
                    break;
                } else if (sPrimaryKey.isEmpty()) {
                    sPrimaryKey=query2.value(1).toString();
                }
            }

            query2.ExecShowErr("PRAGMA foreign_key_list("+sTableName+");");
            while (query2.next()) {
                QString referencedTable=query2.value("table").toString();
                QString localColumn=query2.value("from").toString();
                QString referencedClumn=query2.value("to").toString();
                sQuery +="SELECT '"+sTableName+"' Table_name, "
                         "'"+sPrimaryKey+"' PK_field_name, "+
                         sPrimaryKey+" PK_value, "
                         "'"+localColumn+"' FK_field_name, "+
                         localColumn+" FK_value "
                         "FROM "+sTableName+" WHERE "+localColumn+" NOTNULL AND NOT("+localColumn+" IN(SELECT "+referencedClumn+" FROM "+referencedTable+")) \n"
                         "UNION \n";
            }
        }
    }

    sQuery=StrRemoveLasts(sQuery,8);
    if (!sQuery.isEmpty()) {
        query1.ExecShowErr("DROP VIEW IF EXISTS FK_errors;");
        query1.ExecShowErr("CREATE VIEW FK_errors AS "+sQuery);
        OpenPotaTab("FK_errors","FK_errors",tr("Erreurs d'intégrité"),"");
    }
}

void MainWindow::on_mSQLiteSchema_triggered()
{
    OpenPotaTab("sqlite_schema","sqlite_schema",tr("Schéma %1").arg("SQLite"),"");
}

void MainWindow::on_mAddSchema_triggered()
{
    OpenPotaTab("fda_schema","fda_schema",tr("Schéma %1").arg("FDA"),"");

}

void MainWindow::on_mAbout_triggered()
{
    MessageDlg("Potaléger "+ui->lVer->text(),
                  "Auteur: Marc Pleysier<br>"
                  "<a href=\"https://www.greli.net\">www.greli.net</a><br>"
                  "Sources: <a href=\"https://github.com/marcpley/potaleger\">github.com/marcpley/potaleger</a>",
                  "<b>Crédits</b>:<br>"
                  "Qt Creator community 6.9.2 <a href=\"https://www.qt.io/\">www.qt.io/</a><br>"
                  "SQLite 3 <a href=\"https://www.sqlite.org/\">www.sqlite.org/</a><br>"
                  "SQLiteStudio <a href=\"https://sqlitestudio.pl/\">sqlitestudio.pl/</a>, thanks Pawel !<br>"
                  "muParser <a href=\"https://github.com/beltoforion/muparser/\">github.com/beltoforion/muparser</a><br>"
                  "Ferme Légère <a href=\"https://fermelegere.greli.net\">fermelegere.greli.net</a>, merci Silvère !<br>"
                  "IA: ChatGPT, Mistral et Copilot<br>"
                  "Les Editions Terre Vivante <a href=\"https://www.terrevivante.org\">www.terrevivante.org</a>",
                  QStyle::NStandardPixmap,500);
}

void MainWindow::on_mWhatSNew_triggered()
{
    MessageDlg("Potaléger "+ui->lVer->text(),
                  tr("Evolutions et corrections de bugs"),
                  "<h2>Potaléger 1.4.0</h2>17/11/2025<br>"
                  "<br><u>"+tr("Evolutions métiers")+" :</u><br>"+
                  "- "+tr("<b>Bilans annuels</b> avec pourcentages de réalisation des objectifs, surface occupée, etc.")+"<br>"+
                  "- "+tr("<b>Associations d'espèces ou familles de plante</b>, aide à la création des plans de rotation.")+"<br>"+
                  "- "+tr("<b>Plans de rotation avec échelonnage (en semaine) des cultures d'une même espèce</b>.")+"<br>"+
                  "- "+tr("Liste des cultures suivantes pour chaque culture à terminer.")+"<br>"+
                  "- "+tr("Correction des infos récolte sur les cultures lors de leur passage à terminée.")+"<br>"+
                  "- "+tr("Champ 'Catégories' pour les espèces (légume fruit, feuille, etc).")+"<br>"+
                  "- "+tr("Cultures à fertiliser, les cultures dont la récolte est commencée ne sont plus incluses).")+"<br>"+
                  "- "+tr("Onglet 'Saisie des récoltes', possibilité d'indiquer que la récolte est terminée, plus affichage du total déjà récolté.")+"<br>"+
                  "- "+tr("Quantité restant en stock et total des sorties pour la destination dans l'onglet 'Saisie des consommations'.")+"<br>"+
                  "- "+tr("Recalcul des dates de plantation et récolte (Cultures à semer - Toutes) lors de la saisie de la date de semis.")+"<br>"+
                  "- "+tr("Dates Début_récolte et Fin_récolte (cultures) automatique en fonction des récoltes.")+"<br>"+
                  "<br><u>"+tr("Evolutions noyau et interface")+" :</u><br>"+
                  "- "+tr("<b>Possibilité de masquer des colonnes.</b>")+"<br>"+
                  "- "+tr("<b>Possibilité de réinitialiser les données de base</b> (fusion avec ou remplacement de vos données).")+"<br>"+
                  "- "+tr("<b>Menu 'Maintenance'</b> avec listes des tables, des vues et des requêtes SQL constituant le schéma de la BDD.")+"<br>"+
                  "- "+tr("Lignes horizontales plus visibles dans les tableaux de données.")+"<br>"+
                  "- "+tr("Ouverture des onglets plus rapide.")+"<br>"+
                  "- "+tr("Affichage des champs Vrai/Faux avec ✔️ à la place de 'x'.")+"<br>"+
                  "- "+tr("Amélioration de l'affichage et l'édition des champs textes multi-lignes.")+"<br>"+
                  "- "+tr("Export des données au format SQL.")+"<br>"+
                  "- "+tr("Possibilité de supprimer ET d'ajouter des lignes dans une même transaction.")+"<br>"+
                  "- "+tr("Surlignage des cellules modifiées depuis le dernier rechargement des données.")+"<br>"+
                  "<br><u>"+tr("Corrections")+" :</u><br>"+
                  "- "+tr("Calcul du nombre de plants dans la simulation de planification.")+"<br>"+
                  "- "+tr("ITP, Nb_rangs alors que la largeur de planche n'est pas connue, remplacé par Esp_rang (espacement des rangs).")+"<br>"+
                  "- "+tr("Table temporaire pas supprimée dans certains cas lors de la mise à jour du schéma de BDD.")+"<br>"+
                  "- "+tr("Possibilité  d'ouvrir l'onglet 'Rot. (détails)' alors qu'il n'existe pas d'ITP d'annuelle.")+"<br>"+
                  "- "+tr("Surlignage des cellules modifiées dans l'onglet 'Saisie des récoltes'.")+"<br>"+

                  "<h3>Potaléger 1.3.1</h3>4/10/2025<br>"
                  "<br><u>"+tr("Corrections")+" :</u><br>"+
                  "- "+tr("Intégrités référentielles: modification d'une info pas transmise dans les tables enfants.")+"<br>"+

                  "<h3>Potaléger 1.3.0</h3>17/09/2025<br>"
                  "<br><u>"+tr("Evolutions")+" :</u><br>"+
                  "- "+tr("<b>Graphique 'Récoltes prévues par semaine'</b> à partir des plans de rotation.")+"<br>"+
                  "- "+tr("<b>Graphiques paramétrables</b> à partir des données de tous les onglets.")+"<br>"+
                  "- "+tr("<b>Précision des ITP à la semaine</b> au lieu de 15 jours. Saisie de n° de semaine.")+"<br>"+
                  "- "+tr("<b>Meilleure portabilité de la base de données</b> avec l'abandon de SQLean.")+"<br>"+
                  "- "+tr("Rotations, amélioration de la détection de retour trop rapide d'une même famille sur une planche (calcul plus rapide).")+"<br>"+
                  "- "+tr("Rotations, amélioration de la détection de conflit de cultures sur une planche.")+"<br>"+
                  "- "+tr("Saisies des récoltes et fertilisations améliorées.")+"<br>"+
                  "- "+tr("Planification, nouvel onglet 'Plants nécessaires'.")+"<br>"+
                  "- "+tr("Périodes semis dans affichage graphique des plans de rotation.")+"<br>"+
                  "- "+tr("Amélioration affichage graphique des ITP (chevauchement des périodes semis/plantation/récolte).")+"<br>"+
                  "- "+tr("Chaque installation de Potaléger a ses propres paramètres (chemin BDD, dispositions fenêtres, etc).")+"<br>"+
                  "- "+tr("Séparateur de colonne et séparateur décimal paramétrables pour les exports de données.")+"<br>"+
                  "<br><u>"+tr("Régressions (temporaires)")+" :</u><br>"+
                  "- "+tr("Conflits familles plus détectés dans les rotations.")+"<br>"+
                  "<br><u>"+tr("Corrections")+" :</u><br>"+
                  "- "+tr("Erreur d'arrondi sur 'Qté prév' et 'Qté réc' dans l'onglet 'Couverture des objectifs'.")+"<br>"+
                  "- "+tr("Plantage sur requête SQL utilisateur sans titre ni description.")+"<br>"+
                  "- "+tr("Modification dans l'onglet 'Semis pépinière', des cultures dont le n° est la fin du n° de cultures modifiées sont modifiées aussi par erreur (ex 5 est la fin de 15).")+"<br>"+
                  "- "+tr("Onglet 'Semences nécessaires', les cultures 'Plants' ne sont plus incluses.")+"<br>"+
                  "- "+tr("Sélection Annuelles/Vivaces dans onglet 'Cultures à récolter'.")+"<br>"+
                  "- "+tr("Vivaces dans l'onglet 'Cultures à terminer'.")+"<br>"+
                  "- "+tr("Affichage dans onglets 'Successions de cultures par planche' et 'A irriguer' "
                          "des cultures prévues 'semis en pépinière' et finalement plantées mais non semées (plant acheté) ; "
                          "des cultures sans date de récoltes ni date de destruction ; "
                          "des cultures semées l'année précédant l'année de leur mise en place.")+"<br>"+

                  "<h3>Potaléger 1.20</h3>23/07/2025<br>"
                  "<br><u>"+tr("Evolutions")+" :</u><br>"+
                  "- "+tr("<b>Culture de vivaces</b>.")+"<br>"+
                  "- "+tr("Fonction calculatrice dans les cellules numériques.")+"<br>"+
                  "- "+tr("Alerte visuelle (cellule en rouge) si le contenu se termine par un '!'.")+"<br>"+
                  "- "+tr("Champ 'A faire' sur les cultures et nouvel onglet des cultures ayant quelque chose à faire.")+"<br>"+
                  "- "+tr("Amélioration affichage graphique des cultures (dépérissement).")+"<br>"+
                  "- "+tr("La planification prend en compte le décalage de l'opération précédente par rapport au début de période de l'ITP.")+"<br>"+
                  "- "+tr("Planches en déficit de fertilisation.")+"<br>"+
                  "- "+tr("Bilan fertilisation planche pour les saisons passées.")+"<br>"+
                  "- "+tr("Requête SQL utilisateur (SELECT uniquement), permet par exemple de faire un export vers une plateforme de distribution.")+"<br>"+
                  "<br><u>"+tr("Corrections")+" :</u><br>"+
                  "- "+tr("Cultures de la dernière année d'une rotation non planifiées l'année N+1.")+"<br>"+
                  "- "+tr("Choix de l'année de replanification d'une culture (en forçant une année dans 'D_planif').")+"<br>"+
                  "- "+tr("Affichage du nombre de lignes fonctionne (à coté du bouton 'Filtrer').")+"<br>"+

                  "<h3>Potaléger 1.10</h3>06/06/2025<br>"
                  "<br><u>"+tr("Evolutions")+" :</u><br>"+
                  "- "+tr("<b>Gestion de l'irrigation</b>.")+"<br>"+
                  "- "+tr("Appellation 'Semis sous abris' remplacée par 'Semis pépinière'.")+"<br>"+
                  "- "+tr("Appellation 'Semis direct' remplacée par 'Semis en place'.")+"<br>"+
                  "- "+tr("<b>Fertilisations</b>: besoins NPK, fertilisants, bilan par culture et par planche pour la saison courante.")+"<br>"+
                  "<br><u>"+tr("Corrections")+" :</u><br>"+
                  "- "+tr("Copier/Coller de données REAL avec séparateur décimal local différent du point ne produit plus une donnée TEXT.")+"<br>"+
                  "- "+tr("Import de données REAL avec séparateur décimal local différent du point ne produit plus une donnée TEXT.")+"<br>"+
                  "- "+tr("Plus de possibilité de saisir des récoltes avant la date de mise en place de la culture.")+"<br>"+
                  "- "+tr("Infos (min, max, etc) sur une sélection de pourcentages.")+"<br>"+
                  "- "+tr("Bugs minimes sur les fenêtres de dialogue..")+"<br>"+

                  "<h3>Potaléger 1.02</h3>13/05/2025<br>"+
                  "<br><u>"+tr("Corrections")+" :</u><br>"+
                  "- "+tr("Cultures à planter: contient les 'Semis fait' non nuls (et pas seulement commençant par 'x').")+"<br>"+
                  "- "+tr("Cultures à récolter: contient les 'Semis fait'/'Plantation faite' non nuls (et pas seulement commençant par 'x').")+"<br>"+
                  "- "+tr("Cultures à terminer: contient les 'Semis fait'/'Plantation faite'/'Récolte faite' non nuls (et pas seulement commençant par 'x').")+"<br>"+
                  "- "+tr("Planification: les rotations sont maintenant correctement appliquées.")+"<br>"+
                  "- "+tr("Correction itinéraires techniques (création nouvelle BDD): navet.")+"<br>"+
                  "- "+tr("Amélioration de l'aide en ligne: saison et production.")+"<br>"+

                  "<h3>Potaléger 1.0</h3>16/04/2025<br><br>"+
                  "- "+tr("Données de base: Espèces, itinéraires techniques, variétés...")+"<br>"+
                  "- "+tr("Plans de rotation et planification des cultures.")+"<br>"+
                  "- "+tr("Gestion des cultures.")+"<br>"+
                  "- "+tr("Récoltes et objectifs de production.")+"<br>",
                  QStyle::NStandardPixmap,800);
}

//edit menu

void MainWindow::on_mCloseTab_triggered()
{
    ClosePotaTab(ui->tabWidget->currentWidget());
}

void MainWindow::on_mCloseTabs_triggered()
{
    ClosePotaTabs();
}

void MainWindow::on_mParam_triggered()
{
    if (OpenPotaTab("Param","Params",tr("Paramètres"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->sbInsertRows->setVisible(false);
        w->pbInsertRow->setEnabled(false);
        w->pbInsertRow->setVisible(false);
        w->pbDuplicRow->setEnabled(false);
        w->pbDuplicRow->setVisible(false);
        //w->bAllowInsert=false;
        w->pbDeleteRow->setEnabled(false);
        w->pbDeleteRow->setVisible(false);
        //w->bAllowDelete=false;
    }
}

//Base data menu

void MainWindow::on_mFamilles_triggered()
{
    OpenPotaTab("Familles","Familles",tr("Familles"));
    //OpenPotaTab("Test","Cultures__à_irriguer2",tr("Cultures__à_irriguer2"));
}

void MainWindow::on_mEspecesA_triggered()
{
    OpenPotaTab("EspecesA","Espèces__a",tr("Espèces an."));
}

void MainWindow::on_mEspecesV_triggered()
{
    OpenPotaTab("EspecesV","Espèces__v",tr("Espèces vi."));
}

void MainWindow::on_mEspecesToutes_triggered()
{
    OpenPotaTab("Especes","Espèces",tr("Espèces"));
}

void MainWindow::on_mAssociations_triggered()
{
    if (OpenPotaTab("Associations", "Associations_détails__Saisies",tr("Associations"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//ID, necessary in the view for the triggers to update the real table.
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Favorables"));
        w->pageFilterFilters.append("Association LIKE '% +'");
        w->cbPageFilter->addItem(tr("Défavorables"));
        w->pageFilterFilters.append("Association LIKE '% -'");
        w->cbPageFilter->addItem(tr("Autres"));
        w->pageFilterFilters.append("NOT(Association LIKE '% +') AND NOT(Association LIKE '% -')");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Associations_détails__Saisies-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    }
}

void MainWindow::on_mVarietes_triggered()
{
    if (OpenPotaTab("Varietes","Variétés",tr("Variétés"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Annuelles"));
        w->pageFilterFilters.append("(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Vivaces"));
        w->pageFilterFilters.append("(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Variétés-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    };
}

// void MainWindow::on_mApports_triggered()
// {
//     OpenPotaTab("Apports","Apports",tr("Apports"));
// }

void MainWindow::on_mFournisseurs_triggered()
{
    OpenPotaTab("Fournisseurs","Fournisseurs",tr("Fournisseurs"));
}

void MainWindow::on_mITPTempo_triggered()
{
    if (OpenPotaTab("ITP_tempo","ITP__Tempo",tr("ITP"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->lPageFilter->setText(tr("Pour"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Annuelles"));
        w->pageFilterFilters.append("(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Vivaces"));
        w->pageFilterFilters.append("(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Génériques"));
        w->pageFilterFilters.append("(Espèce ISNULL)");
        w->cbPageFilter->addItem(tr("A planifier"));
        w->pageFilterFilters.append("(SELECT (E.A_planifier NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("ITP__Tempo-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    };
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
        w->tv->hideColumn(1);//IdxAsReEsGrFa, débug use.
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
    if (OpenPotaTab("SuccPlanches","Cultures__Tempo",tr("Succ. planches"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//num_planche, necessary in the view for row painting.
    }
}

void MainWindow::on_mIlots_triggered()
{
    OpenPotaTab("Planches_Ilots","Planches_Ilots",tr("Ilots"));
}

void MainWindow::on_mUnitesProd_triggered()
{
    OpenPotaTab("Planches_Unites_prod","Planches_Unités_prod",tr("Unités prod."));
}

//Menu Planification

void MainWindow::on_mPlanifEspeces_triggered()
{
    OpenPotaTab("Planif_espèces","Planif_espèces",tr("Cult.prévues espèces"));
}

void MainWindow::on_mPlanifIlots_triggered()
{
    OpenPotaTab("Planif_ilots","Planif_ilots",tr("Cult.prévues ilots"));
}

void MainWindow::on_mPlanifPlanches_triggered()
{
    OpenPotaTab("Planif_planches","Planif_planches",tr("Cult.prévues"));
}

void MainWindow::on_mPlanifAsso_triggered()
{
    OpenPotaTab("Planif_associations","Planif_associations",tr("Asso.prévues"));
}

void MainWindow::on_mRecoltesParSemaine_triggered()
{
    if (OpenPotaTab("Planif_recoltes","Planif_récoltes",tr("Récoltes prévues"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());


        w->sGraph.clear();
        w->sGraph.resize(23);
        w->sGraph[0]=str(w->model->FieldIndex("Date"));
        w->sGraph[1]=str(xAxisGroupWeek);
        w->sGraph[3]=str(w->model->FieldIndex("Qté_réc"));
        w->sGraph[4]=str(calcSeriesSum);
        w->sGraph[5]=str(typeSeriesLine);
        w->sGraph[8]=str(w->model->FieldIndex("Valeur"));
        w->sGraph[9]=str(calcSeriesSum);
        w->sGraph[10]=str(typeSeriesLine);
        w->sGraph[13]=str(-1);
        w->sGraph[18]=str(-1);
        w->showGraphDialog();
    }
}

void MainWindow::on_mCreerCultures_triggered()
{
    PotaQuery pQuery(db);
    pQuery.lErr=ui->lDBErr;
    int NbCultPlanif=pQuery.Select0ShowErr("SELECT count() FROM Planif_planches").toInt();
    if (NbCultPlanif==0) {
        MessageDlg(windowTitle(),tr("Aucune culture à planifier:")+"\n\n"+
                          tr("- Créez des rotations")+"\n"+
                          tr("- Vérifiez que le paramètre 'Planifier_planches' n'exclut pas toutes les planches."),"",QStyle::SP_MessageBoxInformation);
        return;
    }

    int NbCultPlanifRetard=pQuery.Select0ShowErr("SELECT count() FROM Planif_planches WHERE coalesce(Date_semis,Date_plantation)<DATE('now')").toInt();
    int NbCultAVenir=pQuery.Select0ShowErr("SELECT count() FROM Cu_non_commencées").toInt();
    QStyle::StandardPixmap icon;
    QString CultAVenir;
    if(NbCultAVenir>0) {
        icon=QStyle::SP_MessageBoxWarning;
        CultAVenir="<br><br>"+tr("IL Y A DÉJÀ %1 CULTURES NI SEMÉES NI PLANTÉES.").arg(NbCultAVenir)+"<br>"+
                   iif(NbCultAVenir>NbCultPlanif*0.9,tr("Peut-être avez-vous déjà généré les prochaines cultures.\n"
                                                        "Si c'est le cas, vous devriez les supprimer avant d'aller plus loin."),"").toString();
    } else {
        icon=QStyle::SP_MessageBoxQuestion;
        CultAVenir="";
    }
    if (OkCancelDialog(windowTitle(),
                       tr("La saison courante est : %1").arg(pQuery.Select0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='Année_culture'").toString())+"<br><br>"+
                       "<b>"+tr("Créer les cultures de la saison suivante (%1) ?").arg(pQuery.Select0ShowErr("SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'").toString())+"</b><br><br>"+
                       tr("La saison courante peut être modifiée dans les paramètres (menu 'Edition', paramétre 'Année_culture').")+"<br><br>"+
                       tr("%1 cultures vont être créées en fonction des rotations.").arg("<b>"+str(NbCultPlanif)+"</b>")+"<br>"+
                       tr("Id de la dernière culture:")+" "+str(NbCultAVenir)+
                       CultAVenir,
                       icon)) {
        int choice=2;
        if (NbCultPlanifRetard>0){
            choice=RadiobuttonDialog(windowTitle(),tr("Parmis les %1 cultures à créer, il y en a %2 dont la date de la 1ère opération (semis ou plantation) est déjà passée.").arg(NbCultPlanif).arg(NbCultPlanifRetard),
                                           {tr("Ne pas créer ces cultures en retard"),
                                            tr("Créer aussi ces cultures en retard")},
                                            iif(NbCultPlanifRetard<NbCultPlanif/10,1,0).toInt(),{},
                                            QStyle::SP_MessageBoxWarning);
            if (choice<0)
                return;
        }
        int IdCult1=pQuery.Select0ShowErr("SELECT max(Culture) FROM Cultures").toInt();
        bool result;

        if (choice==0)
            result=pQuery.ExecShowErr("INSERT INTO Cultures (Espèce,IT_plante,Variété,Fournisseur,Planche,D_planif,Longueur,Nb_rangs,Espacement) "
                                       "SELECT Espèce,IT_plante,Variété,Fournisseur,Planche,(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'),Longueur,Nb_rangs,Espacement "
                                       "FROM Planif_planches WHERE coalesce(Date_semis,Date_plantation)>=DATE('now')");
        else
            result=pQuery.ExecShowErr("INSERT INTO Cultures (Espèce,IT_plante,Variété,Fournisseur,Planche,D_planif,Longueur,Nb_rangs,Espacement) "
                                       "SELECT Espèce,IT_plante,Variété,Fournisseur,Planche,(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'),Longueur,Nb_rangs,Espacement "
                                       "FROM Planif_planches");
        if (result) {
            int IdCult2=pQuery.Select0ShowErr("SELECT min(Culture) FROM Cultures WHERE Culture>"+str(IdCult1)).toInt();
            int IdCult3=pQuery.Select0ShowErr("SELECT max(Culture) FROM Cultures").toInt();
            if (IdCult3>IdCult2 and IdCult2>IdCult1)
                MessageDlg(windowTitle(),tr("%1 cultures créées sur %2 cultures prévues.").arg(IdCult3-IdCult2+1).arg(NbCultPlanif)+"\n\n"+
                                  tr("Id culture:")+" "+str(IdCult2)+" > "+str(IdCult3),"",QStyle::SP_MessageBoxInformation);
            else
                MessageDlg(windowTitle(),tr("%1 culture créée sur %2 cultures prévues.").arg("0").arg(NbCultPlanif),"",QStyle::SP_MessageBoxWarning);
        }
        else
            MessageDlg(windowTitle(),tr("Impossible de créer les cultures."),"",QStyle::SP_MessageBoxCritical);

    }
}

void MainWindow::on_mSemences_triggered()
{
    if (OpenPotaTab("Varietes_inv_et_cde","Variétés__inv_et_cde",tr("Inv. et cde semences"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Annuelles"));
        w->pageFilterFilters.append("(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Vivaces"));
        w->pageFilterFilters.append("(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Espèces concernées"));
        w->pageFilterFilters.append("TN.Espèce IN(SELECT V.Espèce FROM Variétés__inv_et_cde V WHERE (V.Qté_nécess>0)OR(V.Qté_cde>0)OR(Notes LIKE '%!'))");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Variétés__inv_et_cde-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    }
}

void MainWindow::on_mPlants_triggered()
{
    if (OpenPotaTab("Varietes_cde_plants","Variétés__cde_plants",tr("Commande plants"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Annuelles"));
        w->pageFilterFilters.append("(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Vivaces"));
        w->pageFilterFilters.append("(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Espèces concernées"));
        w->pageFilterFilters.append("TN.Espèce IN(SELECT V.Espèce FROM Variétés__cde_plants V WHERE (V.Qté_nécess>0)OR(V.Qté_cde>0)OR(Notes LIKE '%!'))");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Variétés__cde_plants-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    }
}

//Menu Cultures

void MainWindow::on_mCuNonTer_triggered()
{
    if (OpenPotaTab("Cultures_non_terminees","Cultures__non_terminées",tr("Non terminées"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Annuelles"));
        w->pageFilterFilters.append("(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Vivaces"));
        w->pageFilterFilters.append("(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Cultures__non_terminées-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    }
}

void MainWindow::on_mCuASemer_triggered()
{
    if (OpenPotaTab("Cultures_a_semer","Cultures__à_semer",tr("A semer"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Annuelles"));
        w->pageFilterFilters.append("(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Vivaces"));
        w->pageFilterFilters.append("(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Cultures__à_semer-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    };
}

void MainWindow::on_mCuASemerPep_triggered()
{
    if (OpenPotaTab("Cultures_a_semer_pep","Cultures__à_semer_pep",tr("A semer (pépinière)"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//Culture, necessary in the view for sow commited celles.
    }
}

void MainWindow::on_mCuASemerEP_triggered()
{
    OpenPotaTab("Cultures_a_semer_EP","Cultures__à_semer_EP",tr("A semer (en place)"));
}

void MainWindow::on_mCuAPlanter_triggered()
{
    if (OpenPotaTab("Cultures_a_planter","Cultures__à_planter",tr("A planter"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Annuelles"));
        w->pageFilterFilters.append("(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Vivaces"));
        w->pageFilterFilters.append("(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Cultures__à_planter-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    };
}

void MainWindow::on_mCuAIrriguer_triggered()
{
    if (OpenPotaTab("Cultures__a_irriguer","Cultures__à_irriguer",tr("A irriguer"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//num_planche, necessary in the view for row painting.
    }
}

void MainWindow::on_mCuARecolter_triggered()
{
    if (OpenPotaTab("Cultures_a_recolter","Cultures__à_récolter",tr("A récolter"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Annuelles"));
        w->pageFilterFilters.append("(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Vivaces"));
        w->pageFilterFilters.append("(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Cultures__à_récolter-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    };
}

void MainWindow::on_mCuSaisieRecoltes_triggered()
{
    if (OpenPotaTab("Recoltes__Saisies","Récoltes__Saisies",tr("Récoltes"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//ID, necessary in the view for the triggers to update the real table.
    }
}

void MainWindow::on_mCuATerminer_triggered()
{
    OpenPotaTab("Cultures_a_terminer","Cultures__à_terminer",tr("A terminer"));
}

void MainWindow::on_mCuAFaire_triggered()
{
    if (OpenPotaTab("Cultures_A_faire","Cultures__A_faire",tr("A faire"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Annuelles"));
        w->pageFilterFilters.append("(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Vivaces"));
        w->pageFilterFilters.append("(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Cultures__A_faire-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    };
}

void MainWindow::on_mCuVivaces_triggered()
{
    if (OpenPotaTab("Cultures_vivaces","Cultures__vivaces",tr("Vivaces"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(w->model->FieldIndex("Terminée"));
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Non ter."));
        w->pageFilterFilters.append("(Terminée='v')OR(Terminée='V')");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Cultures__vivaces-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    };
}

void MainWindow::on_mCuAssociations_triggered()
{
    if (OpenPotaTab("Associations__présentes","Associations__présentes",tr("Ass. en cours"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Bénéfiques"));
        w->pageFilterFilters.append("Association LIKE '%'||(SELECT Valeur FROM Params WHERE Paramètre='Asso_bénéfique')");
        w->cbPageFilter->addItem(tr("Non bénéfiques"));
        w->pageFilterFilters.append("NOT(Association LIKE '%'||(SELECT Valeur FROM Params WHERE Paramètre='Asso_bénéfique'))");
        w->cbPageFilter->addItem(tr("Avec annuelles"));
        w->pageFilterFilters.append("Nb_cultures>Nb_vivaces");
        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Associations__présentes-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    };
}

void MainWindow::on_mCuToutes_triggered()
{
    if (OpenPotaTab("Cultures","Cultures",tr("Cultures"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        PotaQuery query(db);
        int smin,smax;
        smin=query.Select0ShowErr("SELECT min(Saison) FROM Cultures WHERE coalesce(Terminée,'') NOT LIKE ('v%')").toInt();
        smax=query.Select0ShowErr("SELECT max(Saison) FROM Cultures WHERE coalesce(Terminée,'') NOT LIKE ('v%')").toInt();
        for (int i=smin; i <= smax; ++i) {
            w->cbPageFilter->addItem(tr("Annuelles")+" "+str(i));
            w->pageFilterFilters.append("(Saison='"+str(i)+"')AND(coalesce(Terminée,'') NOT LIKE ('v%'))");
        }
        w->cbPageFilter->addItem(tr("Annuelles"));
        w->pageFilterFilters.append("(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Vivaces"));
        w->pageFilterFilters.append("(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");

        QSettings settings;//("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Cultures-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    };
}

// Menu Fertilisation
void MainWindow::on_mAnalysesSol_triggered()
{
    OpenPotaTab("Analyses_de_sol", "Analyses_de_sol",tr("Analyses sol"));
}

void MainWindow::on_mFertilisants_triggered()
{
    OpenPotaTab("Fertilisants","Fertilisants",tr("Fertilisants"));
}

void MainWindow::on_mInventaireFert_triggered()
{
    OpenPotaTab("Fertilisants__inventaire","Fertilisants__inventaire",tr("Inventaire F."));
}

void MainWindow::on_mCuAFertiliser_triggered()
{
    OpenPotaTab( "Cultures__a_fertiliser","Cultures__à_fertiliser",tr("A fertiliser"));
}

void MainWindow::on_mFertilisations_triggered()
{
    if (OpenPotaTab("Fertilisations__Saisies","Fertilisations__Saisies",tr("Fertilisations"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//ID, necessary in the view for the triggers to update the real table.
    }
}

void MainWindow::on_mBilanPlanches_triggered()
{
    if (OpenPotaTab( "Planches__bilan_fert","Planches__bilan_fert",tr("Bilan fert."))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//Saison.
        w->lPageFilter->setText(tr("Saison"));
        PotaQuery query(db);
        int saison,smin,smax;
        saison=query.Select0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='Année_culture'").toInt();
        smin=query.Select0ShowErr("SELECT min(Saison) FROM Cultures").toInt();
        smax=query.Select0ShowErr("SELECT max(Saison) FROM Cultures").toInt();
        for (int i=smin; i <= smax; ++i) {
            w->cbPageFilter->addItem(str(i));
            w->pageFilterFilters.append("Saison='"+str(i)+"'");
        }
        w->cbPageFilter->setCurrentText(str(saison));
        w->pageFilterFrame->setVisible(true);
        w->pbFilterClick(false);
    };
}

void MainWindow::on_mPlanchesDeficit_triggered()
{
    OpenPotaTab( "Planches__deficit_fert","Planches__deficit_fert",tr("Déficit"));
}

//Menu Stock
void MainWindow::on_mDestinations_triggered()
{
    OpenPotaTab("Destinations__conso","Destinations__conso",tr("Destinations"));
}

void MainWindow::on_mEsSaisieConso_triggered()
{
    if (OpenPotaTab( "Consommations__Saisies","Consommations__Saisies",tr("Consommations"))) {
            PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
            w->tv->hideColumn(0);//ID, necessary in the view for the triggers to update the real table.
        }
}

void MainWindow::on_mInventaire_triggered()
{
    OpenPotaTab( "Especes__inventaire","Espèces__inventaire",tr("Inventaire E."));
}

//Menu Analyses

void MainWindow::on_mBilans_triggered()
{
    OpenPotaTab("Bilans_annuels","Bilans_annuels",tr("Bilans"));
}

void MainWindow::on_mCouverture_triggered()
{
    if (OpenPotaTab("Especes__couverture","Espèces__Bilans_annuels",tr("Bilan espèces"))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        //w->tv->hideColumn(0);//Saison.
        w->lPageFilter->setText(tr("Saison"));
        PotaQuery query(db);
        int saison,smin,smax;
        saison=query.Select0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='Année_culture'").toInt();
        smin=query.Select0ShowErr("SELECT min(Saison) FROM Cultures WHERE coalesce(Terminée,'') NOT LIKE ('v%')").toInt();
        smax=query.Select0ShowErr("SELECT max(Saison) FROM Cultures WHERE coalesce(Terminée,'') NOT LIKE ('v%')").toInt();
        for (int i=smin; i <= smax; ++i) {
            w->cbPageFilter->addItem(str(i));
            w->pageFilterFilters.append("Saison='"+str(i)+"'");
        }
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->setCurrentText(str(saison));
        w->pageFilterFrame->setVisible(true);
        w->pbFilterClick(false);
    };
}

void MainWindow::on_mAnaITPA_triggered()
{
    OpenPotaTab("ITP_analyse_a","ITP__analyse_a",tr("Analyse IT"));
}

void MainWindow::on_mAnaITPV_triggered()
{
    OpenPotaTab("ITP_analyse_v","ITP__analyse_v",tr("Analyse IT"));
}

void MainWindow::on_mAnaCultures_triggered()
{
    OpenPotaTab("Cultures_analyse","Cultures__analyse",tr("Analyse cultures"));
}

void MainWindow::on_mIncDatesCultures_triggered()
{
    OpenPotaTab("Cultures_inc_dates","Cultures__inc_dates",tr("Inc. dates cultures"));
}

void MainWindow::on_mRequeteSQL_triggered()
{
    QString sQuery=QueryDialog(tr("Requête SQL"),tr("Saisissez une requête SQL du type:")+"\n"
                               "SELECT C.Culture,C.Planche,C.Saison,C.Longueur*P.Largeur Surface\n"
                               "FROM Cultures C\n"
                               "LEFT JOIN Planches P USING(Planche)"
                               "WHERE C.Saison=2024;\n"+
                               tr("Titre")+";\n"+tr("Description")+";\n\n"+
                               tr("Cette requête sera enregistrée et utilisable sur cet ordinateur uniquement."),db);
    QStringList values=sQuery.split(";\n");
    if (values.count()>0 and values[0]!="") {
        PotaQuery pQuery(db);
        pQuery.ExecShowErr("DROP VIEW IF EXISTS Temp_UserSQL;");
        pQuery.ExecShowErr("CREATE VIEW Temp_UserSQL AS "+values[0]);
        QString sTitre=tr("Requête SQL");
        QString sDesc="";
        if(values.count()>1 and !values[1].isEmpty()) sTitre=iif(values[1][0]=="\n",values[1].mid(1),values[1]).toString();
        if(values.count()>2 and !values[2].isEmpty()) sDesc=iif(values[2][0]=="\n",values[2].mid(1),values[2]).toString();
        OpenPotaTab("Temp_UserSQL","Temp_UserSQL",sTitre,sDesc);
    }
}

// Local params

void MainWindow::on_cbTheme_currentIndexChanged(int index)
{
    QPalette palette=QApplication::palette();
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

    QFont font=this->font();
    font.setPointSize(arg1.toInt());
    setFont(font);

    QList<QWidget*> widgets=findChildren<QWidget*>();
    foreach (QWidget* widget, widgets) {
        widget->setFont(font);
    }

    //Tab titles
    for (int i=0; i < ui->tabWidget->count(); ++i) {
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

