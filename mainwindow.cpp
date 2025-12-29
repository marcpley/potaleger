#include "mainwindow.h"
#include "Dialogs.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QStackedLayout>
#include <QBoxLayout>
#include <QSizePolicy>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include "FdaWidget.h"
#include <QSettings>
#include "FdaUtils.h"
#include <QToolButton>
#include "data/FdaCalls.h"
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

bool MainWindow::OpenPotaTab(QString const sTableName, QString const sTitre, QString const sDesc) {
    AppBusy(true,ui->progressBar,0,0,sTitre);
    //Recherche parmis les onglets existants.
    for (int i=0; i < ui->tabWidget->count(); i++) {
        if (ui->tabWidget->widget(i)->objectName()=="PW"+sTableName ) {
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

    if (ui->tabWidget->currentWidget()->objectName()!="PW"+sTableName) { //Existing tab not found.
        AppBusy(true,ui->progressBar,100,0,sTableName+" - init");
        PotaQuery query(db);
        int fieldCount=query.Select0ShowErr("SELECT count(*) FROM pragma_table_xinfo('"+sTableName+"')").toInt();
        ui->progressBar->setMaximum(fieldCount*2+12);
        //Create tab
        PotaWidget *w=new PotaWidget(ui->tabWidget);
        w->setObjectName("PW"+sTableName);
        w->lErr=ui->lDBErr;
        w->cbFontSize=ui->cbFont;
        w->model->db=&db;
        w->model->progressBar=ui->progressBar;
        w->delegate->cTableColor=FdaColor(w->model->db,sTableName,"");//before init
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
                w->pbDeleteRow->setEnabled(true);
                bEdit=true;
            } else if (ReadOnlyDb or
                       (query.Select0ShowErr("SELECT count() FROM sqlite_schema "
                                             "WHERE (tbl_name='"+sTableName+"')AND"    //View without trigger INSTEAD OF UPDATE.
                                                   "(sql LIKE 'CREATE TRIGGER "+sTableName+"_UPDATE INSTEAD OF UPDATE ON "+sTableName+" %')").toInt()==0)) {
                w->pbEdit->setVisible(false);
            }
            w->model->progressBar->setValue(w->model->progressBar->value()+1);
            bool bCanOpen=FdaCanOpenTab(&db,w->model->RealTableName());
            if(w->model->rowCount()==0 or !bCanOpen) {
                if (bEdit and bCanOpen) {
                    w->lRowSummary->setText(tr("<- cliquez ici pour saisir des %1").arg(w->model->RealTableName().replace("_"," ").toLower()));
                } else {
                    SetColoredText(ui->lDBErr,"","");
                    AppBusy(false,ui->progressBar);
                    MessageDlg(windowTitle(),sTitre,FdaNoDataText(&db,w->model->tableName()),QStyle::SP_MessageBoxInformation,450);
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
                w->delegate->cColColors[i]=FdaColor(w->model->db,sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString());

                if (w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="color")
                    w->delegate->RowColorCol=i;
                // else if (sTableName.startsWith("Cultures") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Etat")
                //     w->delegate->RowColorCol=i;
                // else if (sTableName.startsWith("Cultures") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="num_planche")
                //     w->delegate->RowColorCol=i;
                // else if (sTableName.startsWith("Params") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="color")
                //     w->delegate->RowColorCol=i;

                if (w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="break")
                    w->delegate->breakCol=i;

                //Tooltip
                QString sTT=FdaToolTip(&db,sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),
                                         w->model->dataTypes[i],w->model->baseDataFields[i]);
                if (sTT!="")
                    w->model->setHeaderData(i, Qt::Horizontal, sTT, Qt::ToolTipRole);

                //All columns read only for start
                w->model->nonEditableColumns.insert(i);

                if (w->model->dataTypes[i]=="DATE")
                    w->model->dateColumns.insert(i);
                else if (sTableName!="Params" and
                         FdaMoney(w->model->db,w->model->tableName(),w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()))
                    w->model->moneyColumns.insert(i);

                w->model->progressBar->setValue(w->model->progressBar->value()+1);

            }

            w->RefreshHorizontalHeader();

            w->model->progressBar->setValue(w->model->progressBar->value()+1);

            QString natSortCols=FdaNaturalSortFields(&db,sTableName);
            //PotaHeaderView *phv=dynamic_cast<PotaHeaderView*>(w->tv->horizontalHeader());
            w->iSortCol=w->model->FieldIndex(natSortCols.split(',')[0]);
            for (int i=0;i<natSortCols.split(',').count();i++)
                w->model->setHeaderData(w->model->FieldIndex(natSortCols.split(',')[i]), Qt::Horizontal, QVariant::fromValue(QIcon(":/images/Arrow_BlueDown"+str(i)+".svg")), Qt::DecorationRole);
            w->showBreaks=(w->iSortCol!=w->model->FieldIndex(w->model->sPrimaryKey));

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
                //if (!w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString().startsWith("TEMPO_")){
                    iWidth=settings.value(sTableName+"-"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()).toInt(nullptr);
                    if (iWidth<=0 or iWidth>700)
                        iWidth=FdaColWidth(&db, sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString());
                // } else {
                //     iWidth=FdaColWidth(&db, sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString());
                // }
                if (iWidth<=0)
                    w->tv->resizeColumnToContents(i);
                else
                    w->tv->setColumnWidth(i,iWidth);
                if (w->tv->columnWidth(i)>700)
                    w->tv->setColumnWidth(i,700);

            }
            settings.endGroup();

            w->model->progressBar->setValue(w->model->progressBar->value()+1);

            settings.beginGroup("ColHidden");
            for (int i=0; i<w->model->columnCount();i++) {
                if (settings.value(sTableName+"-"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()).toBool() or
                    FdaHidden(w->model->db,sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()))
                    w->tv->hideColumn(i);
                //settings.setValue(w->model->tableName()+"-"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),w->tv->isColumnHidden(i));
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
                ui->tabWidget->setTabToolTip(ui->tabWidget->currentIndex(),FdaToolTip(&db,w->model->tableName()));

            if (FdaGotoLast(w->model->db,sTableName)) // go to last row
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
            //if (!w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString().startsWith("TEMPO_"))
                settings.setValue(w->model->tableName()+"-"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),w->tv->columnWidth(i));
        }
        settings.endGroup();

        //Visible col
        settings.beginGroup("ColHidden");
        for (int i=0; i<w->model->columnCount();i++)
            settings.setValue(w->model->tableName()+"-"+w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),w->tv->isColumnHidden(i));
        settings.endGroup();

        settings.setValue(w->model->tableName()+"-pageFilter",fmax(w->cbPageFilter->currentIndex(),0));

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

void MainWindow::on_mFKErrors_triggered()
{
    QString sQuery="";
    QString sTableName="";
    QString sPK="";
    PotaQuery query1(db);
    PotaQuery query2(db);

    query1.ExecShowErr("PRAGMA table_list;");
    while (query1.next()) {
        if (query1.value("type").toString()=="table" and
            query1.value("name").toString()!="Params" and
            !query1.value("name").toString().startsWith("sqlite")) {//No sqlite tables.
            sTableName=query1.value("name").toString();
            sPK="";

            query2.ExecShowErr("PRAGMA table_xinfo("+sTableName+")");
            while (query2.next()){
                if (query2.value(5).toInt()==1) {
                    sPK=query2.value(1).toString();
                    break;
                } else if (sPK.isEmpty()) {
                    sPK=query2.value(1).toString();
                }
            }

            query2.ExecShowErr("PRAGMA foreign_key_list("+sTableName+");");
            while (query2.next()) {
                QString referencedTable=query2.value("table").toString();
                QString localColumn=query2.value("from").toString();
                QString referencedClumn=query2.value("to").toString();
                sQuery +="SELECT '"+sTableName+"' Table_name, "
                         "'"+sPK+"' PK_field_name, "+
                         sPK+" PK_value, "
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
        OpenPotaTab("FK_errors",tr("Erreurs d'intégrité"),"");
    }
}

void MainWindow::on_mSQLiteSchema_triggered()
{
    OpenPotaTab("sqlite_schema",tr("Schéma %1").arg("SQLite"),"");
}

void MainWindow::on_mFdaTSchema_triggered()
{
    OpenPotaTab("fda_t_schema__view",tr("Schéma %1 tables").arg("FDA"),"");
}

void MainWindow::on_mFdaFSchema_triggered()
{
    OpenPotaTab("fda_f_schema__view",tr("Schéma %1 champs").arg("FDA"),"");
}

void MainWindow::on_mLaunchers_triggered()
{
    OpenPotaTab("fda_l_schema",tr("Lanceurs %1").arg("FDA"),"");
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
                  "muParserX <a href=\"https://github.com/beltoforion/muparserx/\">github.com/beltoforion/muparserx</a><br>"
                  "Ferme Légère <a href=\"https://fermelegere.greli.net\">fermelegere.greli.net</a>, merci Silvère !<br>"
                  "IA: ChatGPT, Mistral et Copilot<br>"
                  "Le Guide du Potager Bio (éditions Terre Vivante) <a href=\"https://www.terrevivante.org\">www.terrevivante.org</a>",
                  QStyle::NStandardPixmap,500);
}

void MainWindow::on_mWhatSNew_triggered()
{
    MessageDlg("Potaléger "+ui->lVer->text(),
                  tr("Evolutions et corrections de bugs"),
                  "<h2>Potaléger 1.5.0</h2>Janvier 2026<br>"
                  "<br><u>"+tr("Evolutions métiers")+" :</u><br>"+
                  "- "+tr("<b>Possibilité de générer les cultures de la saison suivante en plusieurs fois.</b>")+"<br>"+
                  "- "+tr("<b>Détection et gestion des conflits de planche</b> entre cultures en cours et planification de la saison suivante.")+"<br>"+
                  "- "+tr("Listes des semences/plants nécessaires avec date semis/plantation de la culture la plus proche.")+"<br>"+
                  "- "+tr("Les besoins en semence et en plants prennent aussi en compte les cultures sans variété renseignée.")+"<br>"+
                  "- "+tr("Analyses, calcul des rendements moyens par espèces, itinéraires techniques, variétés, espèces, espèces/saison, espèces/type planche.")+"<br>"+
                  "- "+tr("Rotations, nouvel onglet donnant l'occupation des planches années par année.")+"<br>"+
                  "<br><u>"+tr("Evolutions noyau et interface")+" :</u><br>"+
                  "- "+tr("<b>Logique métier et définition des menus entièrement SQL</b> (sauf Planification/Créer les cultures).")+"<br>"+
                  "- "+tr("<b>Formatteur automatique</b> dans l'éditeur de requête SQL.")+"<br>"+
                  "- "+tr("Affichage de la liste des enregistrements enfants qui empêchent une suppression.")+"<br>"+
                  "- "+tr("Barre de progression pendant la mise à jour de la BDD à l'ouverture.")+"<br>"+
                  "<br><u>"+tr("Corrections")+" :</u><br>"+
                  "- "+tr("Planification: le décalage des dates de semis, plantation, récolte n'était pas pris en compte lors de la création des cultures.")+"<br>"+
                  "- "+tr("Liste de cultures possibles dans les fertilisations était incorrecte.")+"<br>"+
                  "- "+tr("Certaines associations n'étaient pas détectées dans les plans de rotation, cultures planifiées et cultures réelles.")+"<br>"+
                  "- "+tr("Bilans annuels, la quantité planifiée était fausse (pas d'incidence sur les autres colonnes).")+"<br>"+
                  "- "+tr("Sur les cultures, 'Début_récolte' prenait la date du jour si 'Début_récolte' vide, 'Récolte_faire' vide, 'Terminé' non vide et aucune récolte.")+"<br>"+
                  "- "+tr("Onglet 'Incohérences dates cultures' donnait des faux positifs pour les vivaces.")+"<br>"+
                  "- "+tr("L'import de données dans une table dont la clé primaire est AUTOINCREMENT ne modifie plus les enregistrements existants.")+"<br>"+
                  "- "+tr("Création d'une nouvelle BBD, le schéma additionnel n'était pas créé.")+"<br>"+

                  "<h3>Potaléger 1.4.0</h3>17/11/2025<br>"
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
    if (OpenPotaTab("Params",tr("Paramètres"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->sbInsertRows->setVisible(false);
        w->pbInsertRow->setEnabled(false);
        w->pbInsertRow->setVisible(false);
        w->pbDuplicRow->setEnabled(false);
        w->pbDuplicRow->setVisible(false);
        w->pbDeleteRow->setEnabled(false);
        w->pbDeleteRow->setVisible(false);
    }
}

void MainWindow::on_mNotes_triggered()
{
    OpenPotaTab("Notes",tr("Notes"));
}

void MainWindow::on_mRequeteSQL_triggered()
{
    QString sQuery=QueryDialog(tr("Requête SQL"),
                               // tr("Saisissez une requête SQL du type:")+"\n"
                               // "SELECT C.Culture,C.Planche,C.Saison,C.Longueur*P.Largeur Surface\n"
                               // "FROM Cultures C\n"
                               // "LEFT JOIN Planches P USING(Planche)"
                               // "WHERE C.Saison=2024;\n"+
                               // tr("Titre")+";\n"+tr("Description")+";\n\n"+
                               tr("Cette requête sera enregistrée et utilisable sur cet ordinateur uniquement."),db);
    QStringList values=sQuery.split(";\n");
    if (values.count()>0 and values[0]!="") {
        PotaQuery pQuery(db);
        pQuery.ExecShowErr("DROP VIEW IF EXISTS Temp_UserSQL;");
        pQuery.ExecShowErr("DELETE FROM fda_f_schema WHERE name='Temp_UserSQL';");
        pQuery.ExecShowErr("DELETE FROM fda_t_schema WHERE name='Temp_UserSQL';");
        //pQuery.ExecShowErr("CREATE VIEW Temp_UserSQL AS "+values[0]);

        QString fda_cmd="";
        QString sQuery;
        sQuery=RemoveSQLcomment("CREATE VIEW Temp_UserSQL AS "+values[0],false,&fda_cmd); //keepReturns
        pQuery.ExecShowErr(sQuery);
        if (!fda_cmd.isEmpty())
            pQuery.ExecMultiShowErr(fda_cmd,";",nullptr);

        QString sTitre=tr("Requête SQL");
        QString sDesc="";
        if(values.count()>1 and !values[1].isEmpty()) {
            sTitre=values[1].trimmed();
            while (sTitre.startsWith("\n")) sTitre=sTitre.mid(1);
            if (sTitre.contains("\n")) sTitre=sTitre.split("\n")[0];
            if (sTitre.length()>30) sTitre=sTitre.first(30);
        }
        if(values.count()>2 and !values[2].isEmpty()) sDesc=iif(values[2][0]=="\n",values[2].mid(1),values[2]).toString();

        OpenPotaTab("Temp_UserSQL",sTitre,sDesc);
    }
}

void MainWindow::on_mCreerCultures_triggered()
{
    PotaQuery pQuery(db);
    pQuery.lErr=ui->lDBErr;
    int NbCultPlanif=pQuery.Select0ShowErr("SELECT count() FROM Planif_planches").toInt();
    int NbCultPlanifValid=pQuery.Select0ShowErr("SELECT count() FROM Planif_planches WHERE (Validée NOTNULL)").toInt();
    if (NbCultPlanifValid==0) {
        if (NbCultPlanif==0)
            MessageDlg(windowTitle(),tr("Aucune culture à planifier:")+"\n\n"+
                              tr("- Créez des rotations")+"\n"+
                              tr("- Vérifiez que le paramètre 'Planifier_planches' n'exclut pas toutes les planches."),"",QStyle::SP_MessageBoxInformation);
        else
            MessageDlg(windowTitle(),tr("Aucune culture à planifier validée.")+"\n\n"+
                                     tr("Menu 'Planification', onglet 'Cultures prévues par planche', colonne 'Validée'."),"",QStyle::SP_MessageBoxInformation);
        return;
    }

    int NbCultPlanifRetard=pQuery.Select0ShowErr("SELECT count() FROM Planif_planches WHERE (coalesce(Date_semis,Date_plantation)<DATE('now'))AND(Validée NOTNULL)").toInt();
    int NbCultPlanifConflitCreer=pQuery.Select0ShowErr("SELECT count() FROM Planif_planches WHERE (Déjà_en_place NOTNULL)AND(Validée NOTNULL)").toInt();
    int NbCultPlanifNonValidees=pQuery.Select0ShowErr("SELECT count() FROM Planif_planches WHERE (Validée ISNULL)").toInt();
    int NbCultAVenir=pQuery.Select0ShowErr("SELECT count() FROM Cu_non_commencées").toInt();
    int NDerCult=pQuery.Select0ShowErr("SELECT max(Culture) FROM Cultures").toInt();
    QStyle::StandardPixmap icon;
    QString CultAVenir;
    if(NbCultAVenir>0) {
        icon=QStyle::SP_MessageBoxWarning;
        CultAVenir="<br><br>"+tr("Il y a déjà %1 cultures ni semées ni plantés.").arg(NbCultAVenir)+"<br>"+
                   iif(NbCultAVenir>NbCultPlanifValid*0.9,tr("Peut-être avez-vous déjà généré les prochaines cultures.\n"
                                                        "Si c'est le cas, vous devriez les supprimer avant d'aller plus loin."),"").toString();
    } else {
        icon=QStyle::SP_MessageBoxQuestion;
        CultAVenir="";
    }
    if (OkCancelDialog(windowTitle(),
                       tr("La saison courante est : %1").arg(pQuery.Select0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='Année_culture'").toString())+"<br><br>"+
                       "<b>"+tr("Créer les cultures de la saison suivante (%1) à partir des plans de rotation ?").arg(pQuery.Select0ShowErr("SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'").toString())+"</b><br><br>"+
                       tr("La saison courante peut être modifiée dans les paramètres (menu 'Edition', paramétre 'Année_culture').")+"<br><br>"+
                       tr("%1 cultures vont être créées.").arg("<b>"+str(NbCultPlanifValid)+"</b>")+"<br>"+
                       tr("%1 cultures vont être en conflit avec des cultures déjà existantes.").arg(NbCultPlanifConflitCreer)+"<br>"+
                       tr("%1 cultures ne vont pas être créées car non validée.").arg(NbCultPlanifNonValidees)+"<br>"+
                       tr("Id de la dernière culture:")+" "+str(NDerCult)+
                       CultAVenir,
                       icon,400)) {
        int choice=2;
        if (NbCultPlanifRetard>0){
            choice=RadiobuttonDialog(windowTitle(),tr("Parmis les %1 cultures à créer, il y en a %2 dont la date de la 1ère opération (semis ou plantation) est déjà passée.").arg(NbCultPlanifValid).arg(NbCultPlanifRetard),
                                           {tr("Ne pas créer ces cultures en retard"),
                                            tr("Créer aussi ces cultures en retard")},
                                            iif(NbCultPlanifRetard<NbCultPlanifValid/10,1,0).toInt(),{},
                                            false,QStyle::SP_MessageBoxWarning);
            if (choice<0)
                return;
        }
        int IdCult1=pQuery.Select0ShowErr("SELECT max(Culture) FROM Cultures").toInt();
        bool result;

        if (choice==0)
            result=pQuery.ExecShowErr("INSERT INTO Cultures (Espèce,IT_plante,Variété,Fournisseur,Planche,D_planif,Longueur,Nb_rangs,Espacement,Notes) "
                                       "SELECT Espèce,IT_plante,NULL,NULL,Planche,(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'),"
                                       "Longueur,Nb_rangs,Espacement,Notes||iif(Validée!='x',x'0a0a'||Validée,'') "
                                       "FROM Planif_planches WHERE coalesce(Date_semis,Date_plantation)>=DATE('now')AND(Validée NOTNULL)");
        else
            result=pQuery.ExecShowErr("INSERT INTO Cultures (Espèce,IT_plante,Variété,Fournisseur,Planche,D_planif,"
                                                            "Date_semis,Date_plantation,Début_récolte,Fin_Récolte,"
                                                            "Longueur,Nb_rangs,Espacement,Notes) "
                                       "SELECT Espèce,IT_plante,NULL,NULL,Planche,(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'),"
                                       "Date_semis,Date_plantation,Début_récolte,Fin_Récolte,"
                                       "Longueur,Nb_rangs,Espacement,iif(Validée!='x',coalesce(Notes||x'0a0a','')||Validée,Notes) "
                                       "FROM Planif_planches WHERE (Validée NOTNULL)");
            // result=pQuery.ExecShowErr("INSERT INTO Cultures (Espèce,IT_plante,Variété,Fournisseur,Planche,D_planif,Longueur,Nb_rangs,Espacement,Notes) "
            //                            "SELECT Espèce,IT_plante,Variété,Fournisseur,Planche,(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'),"
            //                            "Longueur,Nb_rangs,Espacement,iif(Validée!='x',coalesce(Notes||x'0a0a','')||Validée,Notes) "
            //                            "FROM Planif_planches WHERE (Validée NOTNULL)");
        if (result) {
            pQuery.ExecShowErr("DELETE FROM Planif_validations "
                               "WHERE (SELECT (PP.Déjà_créée NOTNULL) FROM Planif_pl_date2 PP WHERE PP.IdxIdPl=Planif_validations.IdxIdPl)");
            int NbValidNonCrees=pQuery.Select0ShowErr("SELECT count() FROM Planif_validations WHERE Validée NOTNULL").toInt();
            int IdCult2=pQuery.Select0ShowErr("SELECT min(Culture) FROM Cultures WHERE Culture>"+str(IdCult1)).toInt();
            int IdCult3=pQuery.Select0ShowErr("SELECT max(Culture) FROM Cultures").toInt();
            QString mess=tr("%1 cultures créées sur %2 cultures prévues.").arg(IdCult3-IdCult2+1).arg(NbCultPlanifValid)+"\n\n"+
                         tr("Id culture:")+" "+str(IdCult2)+" > "+str(IdCult3)+
                         iif(NbValidNonCrees>0,"\n"+tr("%1 culture validées mais non crées.").arg(NbValidNonCrees),"").toString();
            if (IdCult3>IdCult2 and IdCult2>IdCult1 and NbValidNonCrees==0)
                MessageDlg(windowTitle(),mess,"",QStyle::SP_MessageBoxInformation);
            else
                MessageDlg(windowTitle(),mess,"",QStyle::SP_MessageBoxWarning);
        }
        else
            MessageDlg(windowTitle(),tr("Impossible de créer les cultures."),"",QStyle::SP_MessageBoxCritical);

    }
}

//Model menu
void MainWindow::on_FdaMenu(const QString &sTableName, const QString &sTitre, const QString &sDesc, const QString &sFilters, const QString &sGraph) {
    if (sTableName=="CreerCultures") { //todo
        on_mCreerCultures_triggered();
        return;
    }

    if (OpenPotaTab(sTableName,sTitre,sDesc)) {
        if (!sFilters.isEmpty()) {
            PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
            QStringList filters=sFilters.split("\n");
            w->lPageFilter->setText(filters[0]); //Combox label.
            for (int i=1;i<filters.count();i++) {
                QString filter=filters[i];
                int sep=filter.indexOf("|");
                if (sep>0 and filter.length()>sep) {
                    w->cbPageFilter->addItem(filter.first(sep));
                    w->pageFilterFilters.append(filter.mid(sep+1));
                }
            }
            QSettings settings;//("greli.net", "Potaléger");
            w->cbPageFilter->setCurrentIndex(settings.value(sTableName+"-pageFilter").toInt());
            w->pageFilterFrame->setVisible(true);
            if (w->cbPageFilter->currentIndex()>0)
                w->pbFilterClick(false);
        } else if (!sGraph.isEmpty()) {
            PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
            w->sGraph.clear();
            w->sGraph.resize(23);
            QStringList graphParams=sGraph.split("|");
            for (int i=0;i<graphParams.count();i++) {
                QString graphParam=graphParams[i];
                QList<int> fieldNameParams={0,3,8,13,18};
                if (i==0 or i==3 or i==8 or i==13 or i==18) {
                    w->sGraph[i]=str(w->model->FieldIndex(graphParam));
                } else if (i==1) {
                    if (graphParam=="GroupSame") w->sGraph[i]=str(xAxisGroupSame);
                    else if (graphParam=="GroupFirstWord") w->sGraph[i]=str(xAxisGroupFirstWord);
                    else if (graphParam=="GroupFirstChar") w->sGraph[i]=str(xAxisGroupFirstChar);
                    else if (graphParam=="GroupFirstChar2") w->sGraph[i]=str(xAxisGroupFirstChar2);
                    else if (graphParam=="GroupFirstChar3") w->sGraph[i]=str(xAxisGroupFirstChar3);
                    else if (graphParam=="GroupFirstChar4") w->sGraph[i]=str(xAxisGroupFirstChar4);
                    else if (graphParam=="GroupFirstChar5") w->sGraph[i]=str(xAxisGroupFirstChar5);
                    else if (graphParam=="GroupFirstChar6") w->sGraph[i]=str(xAxisGroupFirstChar6);
                    else if (graphParam=="GroupFirstChar7") w->sGraph[i]=str(xAxisGroupFirstChar7);
                    else if (graphParam=="GroupFirstChar8") w->sGraph[i]=str(xAxisGroupFirstChar8);
                    else if (graphParam=="GroupFirstChar9") w->sGraph[i]=str(xAxisGroupFirstChar9);
                    else if (graphParam=="GroupYear") w->sGraph[i]=str(xAxisGroupYear);
                    else if (graphParam=="GroupMonth") w->sGraph[i]=str(xAxisGroupMonth);
                    else if (graphParam=="GroupWeek") w->sGraph[i]=str(xAxisGroupWeek);
                    else if (graphParam=="GroupDay") w->sGraph[i]=str(xAxisGroupDay);
                    else if (graphParam=="Group1000") w->sGraph[i]=str(xAxisGroup1000);
                    else if (graphParam=="Group100") w->sGraph[i]=str(xAxisGroup100);
                    else if (graphParam=="Group10") w->sGraph[i]=str(xAxisGroup10);
                    else if (graphParam=="Group1") w->sGraph[i]=str(xAxisGroup1);
                    else if (graphParam=="Group1Dec") w->sGraph[i]=str(xAxisGroup1Decimal);
                    else if (graphParam=="Group2Dec") w->sGraph[i]=str(xAxisGroup2Decimals);
                    else if (graphParam=="Group3Dec") w->sGraph[i]=str(xAxisGroup3Decimals);
                    else if (graphParam=="Group4Dec") w->sGraph[i]=str(xAxisGroup4Decimals);
                    else if (graphParam=="Group5Dec") w->sGraph[i]=str(xAxisGroup5Decimals);
                    else if (graphParam=="Group6Dec") w->sGraph[i]=str(xAxisGroup6Decimals);
                    else w->sGraph[i]=str(xAxisGroupNo);
                } else if (i==2) {
                    if (!graphParam.isEmpty()) w->sGraph[i]="1";
                } else if (i==4 or i==9 or i==14 or i==19) {
                    if (graphParam=="NotNull") w->sGraph[i]=str(calcSeriesNotNull);
                    else if (graphParam=="Distinct") w->sGraph[i]=str(calcSeriesDistinct);
                    else if (graphParam=="First") w->sGraph[i]=str(calcSeriesFirst);
                    else if (graphParam=="Last") w->sGraph[i]=str(calcSeriesLast);
                    else if (graphParam=="Average") w->sGraph[i]=str(calcSeriesAverage);
                    else if (graphParam=="Min") w->sGraph[i]=str(calcSeriesMin);
                    else if (graphParam=="Max") w->sGraph[i]=str(calcSeriesMax);
                    else if (graphParam=="Sum") w->sGraph[i]=str(calcSeriesSum);
                } else if (i==5 or i==10 or i==15 or i==20) {
                    if (graphParam=="Line") w->sGraph[i]=str(typeSeriesLine);
                    else if (graphParam=="Points") w->sGraph[i]=str(typeSeriesScatter);
                    else if (graphParam=="NotNullPoints") w->sGraph[i]=str(typeSeriesScatNotNull);
                    else if (graphParam=="Bars") w->sGraph[i]=str(typeSeriesBar);
                } else if (i==6 or i==11 or i==16 or i==21) { //Serie color
                    if (QColor(graphParam).isValid())
                        w->sGraph[i]=graphParam;
                } else if (i==7 or i==12 or i==17 or i==22) { //Right y-axis
                    if (!graphParam.isEmpty()) w->sGraph[i]="1";
                }
            }
            if (graphParams.count()<9) w->sGraph[8]="-1";
            if (graphParams.count()<14) w->sGraph[13]="-1";
            if (graphParams.count()<19) w->sGraph[18]="-1";
            w->showGraphDialog();
        }
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




