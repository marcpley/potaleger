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

bool MainWindow::OpenPotaTab(QString const sObjName, QString const sTableName, QString const sTitre, QString const sDesc)
{
    //Recherche parmis les onglets existants.
    for (int i = 0; i < ui->tabWidget->count(); i++) {
        if (ui->tabWidget->widget(i)->objectName()=="PW"+sObjName ) {
            ui->tabWidget->setCurrentIndex(i);
            if (sTableName=="UserSQL") {
                ClosePotaTab(ui->tabWidget->currentWidget());
                ui->tabWidget->setCurrentIndex(fmax(i-1,0));
            } else {
                PotaWidget *wc=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
                if (!wc->pbCommit->isEnabled())
                    wc->pbRefreshClick();
                break;
            }
        }
    }
    if (ui->tabWidget->currentWidget()->objectName()!="PW"+sObjName) {
        //Create tab
        PotaWidget *w = new PotaWidget(ui->tabWidget);
        w->setObjectName("PW"+sObjName);
        w->lErr=ui->lDBErr;
        w->cbFontSize=ui->cbFont;
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
                w->pbDuplicRow->setEnabled(true);
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
                if (bEdit and(FkFilter(&db,w->model->RealTableName(),"","",w->model->index(0,0),true)!="NoFk")){
                    w->lRowSummary->setText(tr("<- cliquez ici pour saisir des %1").arg(w->model->RealTableName().replace("_"," ").toLower()));
                } else {
                    SetColoredText(ui->lDBErr,"","");
                    MessageDlg(windowTitle(),sTitre,NoData(w->model->tableName()),QStyle::SP_MessageBoxInformation);
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
                w->model->setHeaderData(i,Qt::Horizontal,w->model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString().replace("_pc","").replace("_"," "),Qt::DisplayRole);

                //Table color.
                w->delegate->cColColors[i]=TableColor(sTableName,w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString());

                if (sTableName.startsWith("Cultures") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Etat")
                    w->delegate->RowColorCol=i;
                else if (sTableName.startsWith("Cultures") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="num_planche")
                    w->delegate->RowColorCol=i;
                else if (sTableName.startsWith("Params") and w->model->headerData(i,Qt::Horizontal,Qt::EditRole).toString()=="Section")
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

            if (!sDesc.isEmpty())
                ui->tabWidget->setTabToolTip(ui->tabWidget->currentIndex(),sDesc);
            else
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

            ui->mCloseTabs->setEnabled(true);
            ui->mCloseTab->setEnabled(true);
            ui->mImport->setEnabled(w->pbInsertRow->isEnabled());
            ui->mExport->setEnabled(true);
            ui->mImport->setIcon(QIcon(TablePixmap(w->model->tableName(),">>  ")));
            ui->mExport->setIcon(QIcon(TablePixmap(w->model->tableName(),"  >>")));

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

        settings.setValue(w->model->tableName()+"-pageFilter",w->cbPageFilter->currentIndex());

        if (w->model->tableName()=="Params") {
            PotaQuery pQuery(db);
            setWindowTitle("Potaléger"+pQuery.Selec0ShowErr("SELECT ' - '||Valeur FROM Params WHERE Paramètre='Utilisateur'").toString());
        } else if (w->model->tableName()=="Cultures__A_faire") {
            PotaQuery pQuery(db);
            pQuery.ExecShowErr("UPDATE Cultures SET A_faire=NULL WHERE A_faire='.'");
        }

        if (ui->tabWidget->count()<3) {//Fermeture du dernier onglet data ouvert.
            ui->mCloseTabs->setEnabled(false);
            ui->mCloseTab->setEnabled(false);
            ui->mImport->setEnabled(false);
            ui->mExport->setEnabled(false);
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
        ui->mImport->setEnabled(false);
        ui->mExport->setEnabled(false);
    } else {
        PotaWidget *wc=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        ui->mImport->setEnabled(wc->pbInsertRow->isEnabled());
        ui->mExport->setEnabled(true);
        ui->mImport->setIcon(QIcon(TablePixmap(wc->model->tableName(),">>  ")));
        ui->mExport->setIcon(QIcon(TablePixmap(wc->model->tableName(),"  >>")));
        wc->SetSizes();
    }
    SetColoredText(ui->lDBErr,"","");
}

//File menu

void MainWindow::on_mSelecDB_triggered()
{
    ClosePotaTabs();
    const QString sFileName = QFileDialog::getOpenFileName( this, tr("Base de données %1").arg("Potaléger"), ui->lDB->text(), "*.sqlite3");
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

void MainWindow::on_mCopyBDD_triggered()
{
    ClosePotaTabs();
    QFileInfo FileInfo,FileInfoVerif;
    FileInfo.setFile(ui->lDB->text());
    if (!FileInfo.exists()) {
        MessageDlg("Potaléger "+ui->lVer->text(),tr("Le fichier de BDD n'existe pas.")+"\n"+ui->lDB->text(),"",QStyle::SP_MessageBoxCritical);
        return;
    }

    const QString sFileName = QFileDialog::getSaveFileName(this, tr("Copie de la base de données %1").arg("Potaléger"), ui->lDB->text(), "*.sqlite3",nullptr,QFileDialog::DontConfirmOverwrite);
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
//         sFileName = "C:\\Users\\NomUtilisateur\\Documents";
// #else
//         sFileName = "/home/NomUtilisateur/Documents";
// #endif
        sFileName=QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+
                  QDir::toNativeSeparators("/Documents/Potaleger.sqlite3");
    }

    sFileName = QFileDialog::getSaveFileName(this, tr("Nom pour la BDD %1").arg("Potaléger "+sEmpty), sFileName, "*.sqlite3",nullptr,QFileDialog::DontConfirmOverwrite);
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

void MainWindow::on_mAbout_triggered()
{
    MessageDlg("Potaléger "+ui->lVer->text(),
                  "Auteur: Marc Pleysier<br>"
                  "<a href=\"https://www.greli.net\">www.greli.net</a><br>"
                  "Sources: <a href=\"https://github.com/marcpley/potaleger\">github.com/marcpley/potaleger</a>",
                  "<b>Crédits</b>:<br>"
                  "Qt Creator community 6.8 <a href=\"https://www.qt.io/\">www.qt.io/</a><br>"
                  "SQLite 3 <a href=\"https://www.sqlite.org/\">www.sqlite.org/</a><br>"
                  "SQLean <a href=\"https://github.com/nalgeon/sqlean\">github.com/nalgeon/sqlean</a><br>"
                  "SQLiteStudio <a href=\"https://sqlitestudio.pl/\">sqlitestudio.pl/</a>, thanks Pawel !<br>"
                  "ExprTK <a href=\"https://github.com/ArashPartow/exprtk/\">github.com/ArashPartow/exprtk</a><br>"
                  "Ferme Légère <a href=\"https://fermelegere.greli.net\">fermelegere.greli.net</a>, merci Silvère !<br>"
                  "IA: ChatGPT, Mistral et Copilot<br>"
                  "Le Guide Terre Vivante du potager bio <a href=\"https://www.terrevivante.org\">www.terrevivante.org</a>",
                  QStyle::NStandardPixmap,500);
}

void MainWindow::on_mWhatSNew_triggered()
{
    MessageDlg("Potaléger "+ui->lVer->text(),
                  tr("Evolutions et corrections de bugs"),
                  "<b>Potaléger 1.20</b> - 01/07/2025<br>"
                  "<u>"+tr("Evolutions")+" :</u><br>"+
                  "- "+tr("<b>Culture de vivaces</b>.")+"<br>"+
                  "- "+tr("Fonction calculatrice dans les cellules numériques.")+"<br>"+
                  "- "+tr("Alerte visuelle (cellule en rouge) si le contenu se termine par un '!'.")+"<br>"+
                  "- "+tr("Champ 'A faire' sur les cultures et nouvel onglet des cultures ayant quelque chose à faire.")+"<br>"+
                  "- "+tr("Amélioration visu graphique des cultures (dépérissement).")+"<br>"+
                  "- "+tr("La planification prend en compte le décalage de l'opération précédente par rapport au début de période de l'ITP.")+"<br>"+
                  "- "+tr("Planches en déficit de fertilisation.")+"<br>"+
                  "- "+tr("Bilan fertilisation planche pour les saisons passées.")+"<br>"+
                  "- "+tr("Requête SQL utilisateur (SELECT uniquement), permet par exemple de faire un export vers une plateforme de distribution.")+"<br>"+
                  "<u>"+tr("Corrections")+" :</u><br>"+
                  "- "+tr("Cultures de la dernière année d'une rotation non planifiées l'année N+1.")+"<br>"+
                  "- "+tr("Choix de l'année de replanification d'une culture (en forçant une année dans 'D_planif').")+"<br>"+
                  "- "+tr("Affichage du nombre de lignes fonctionne (à coté du bouton 'Filtrer').")+"<br>"+
                  "<br>"+
                  "<b>Potaléger 1.10</b> - 06/06/2025<br>"
                  "<u>"+tr("Evolutions")+" :</u><br>"+
                  "- "+tr("<b>Gestion de l'irrigation</b>.")+"<br>"+
                  "- "+tr("Appellation 'Semis sous abris' remplacée par 'Semis pépinière'.")+"<br>"+
                  "- "+tr("Appellation 'Semis direct' remplacée par 'Semis en place'.")+"<br>"+
                  "- "+tr("<b>Fertilisations</b>: besoins NPK, fertilisants, bilan par culture et par planche pour la saison courante.")+"<br>"+
                  "<u>"+tr("Corrections")+" :</u><br>"+
                  "- "+tr("Copier/Coller de données REAL avec séparateur décimal local différent du point ne produit plus une donnée TEXT.")+"<br>"+
                  "- "+tr("Import de données REAL avec séparateur décimal local différent du point ne produit plus une donnée TEXT.")+"<br>"+
                  "- "+tr("Plus de possibilité de saisir des récoltes avant la date de mise en place de la culture.")+"<br>"+
                  "- "+tr("Infos (min, max, etc) sur une sélection de pourcentages.")+"<br>"+
                  "- "+tr("Bugs minimes sur les fenêtres de dialogue..")+"<br>"+
                  "<br>"+
                  "<b>Potaléger 1.02</b> - 13/05/2025<br>"+
                  "<u>"+tr("Corrections")+" :</u><br>"+
                  "- "+tr("Cultures à planter: contient les 'Semis fait' non nuls (et pas seulement commençant par 'x').")+"<br>"+
                  "- "+tr("Cultures à récolter: contient les 'Semis fait'/'Plantation faite' non nuls (et pas seulement commençant par 'x').")+"<br>"+
                  "- "+tr("Cultures à terminer: contient les 'Semis fait'/'Plantation faite'/'Récolte faite' non nuls (et pas seulement commençant par 'x').")+"<br>"+
                  "- "+tr("Planification: les rotations sont maintenant correctement appliquées.")+"<br>"+
                  "- "+tr("Correction itinéraires techniques (création nouvelle BDD): navet.")+"<br>"+
                  "- "+tr("Amélioration de l'aide en ligne: saison et production.")+"<br>"+
                  "<br>"+
                  "<b>Potaléger 1.0</b> - 16/04/2025<br>"+
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
    // //OuvrirOnglet("sqlean_define","sqlean_define","test");
    // OuvrirOnglet("test","Rotations_détails","test");
    // return;

    if (OpenPotaTab("Param","Params",tr("Paramètres"))) {
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->sbInsertRows->setVisible(false);
        w->pbInsertRow->setEnabled(false);
        w->pbInsertRow->setVisible(false);
        w->pbDuplicRow->setEnabled(false);
        w->pbDuplicRow->setVisible(false);
        w->bAllowInsert=false;
        w->pbDeleteRow->setEnabled(false);
        w->pbDeleteRow->setVisible(false);
        w->bAllowDelete=false;
    }
}

void MainWindow::on_mImport_triggered()
{
    if (ui->tabWidget->currentWidget()->objectName().startsWith("PW")){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());

        if (w->pbCommit->isEnabled()) {
            if (YesNoDialog(windowTitle(),w->lTabTitle->text().trimmed()+"\n\n"+
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
            MessageDlg("Potaléger "+ui->lVer->text(),tr("Impossible d'ouvrir le fichier")+"\n"+
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
            MessageDlg("Potaléger "+ui->lVer->text(),QObject::tr("Aucun champ dans le fichier %1 n'est modifiable dans l'onglet %2.")
                              .arg(FileInfoVerif.fileName())
                              .arg(w->lTabTitle->text().trimmed()),"",QStyle::SP_MessageBoxWarning);
            return;
        // } else if (primaryFieldImport==-1) {
        //     TypeImport=4; //Append only
        //     MessageDlg(QObject::tr("Champ %1 non trouvée dans le fichier %2.").arg(w->model->sPrimaryKey).arg(FileInfoVerif.fileName()),"",QStyle::SP_MessageBoxWarning);
        //     return;
        } else if (lines.count()<2) {
            MessageDlg("Potaléger "+ui->lVer->text(),QObject::tr("Aucune ligne à importer dans le fichier %1.").arg(FileInfoVerif.fileName()),"",QStyle::SP_MessageBoxWarning);
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

            choice = RadiobuttonDialog("Potaléger "+ui->lVer->text(),w->lTabTitle->text().trimmed()+"<br><br>"+
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
            MessageDlg("Potaléger "+ui->lVer->text(),QObject::tr("Champ %1 non trouvée dans le fichier %2.\nSeul l'ajout de ligne est éventuellement possible.").arg(w->model->sPrimaryKey).arg(FileInfoVerif.fileName()),"",QStyle::SP_MessageBoxWarning);
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
        QLocale locale;
        QString decimalSep = QString(locale.decimalPoint());
        if (choice==5 or choice==6) {//Delete selected lines
            if(w->model->rowCount()>1 and
                !OkCancelDialog("Potaléger "+ui->lVer->text(),tr("Attention, %1 lignes sont susceptibles d'être supprimées!").arg(w->model->rowCount()),QStyle::SP_MessageBoxWarning,600)) {
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
                        if (w->model->data(w->model->index(i,0),Qt::EditRole).toString()==valuesToImport[primaryFieldImport]){
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
                                if(dataTypes[col]=="REAL")
                                    valuesToImport[col]=StrReplace(valuesToImport[col],decimalSep,".");
                                if (choice==0 or choice==2 or choice==5 or//Priority to imported data
                                    w->model->data(w->model->index(recordToUpdate,fieldindexes[col]),Qt::EditRole).toString()=="") {
                                    if (w->model->data(w->model->index(recordToUpdate,fieldindexes[col]),Qt::EditRole).toString()!=valuesToImport[col]){
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
                                if (fieldindexes[col]>-1){//Col exists in table.
                                    if(dataTypes[col]=="REAL")
                                        w->model->setData(w->model->index(row,fieldindexes[col]),StrReplace(valuesToImport[col],decimalSep,"."));
                                    else
                                        w->model->setData(w->model->index(row,fieldindexes[col]),valuesToImport[col]);

                                }
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

        MessageDlg("Potaléger "+ui->lVer->text(),QObject::tr("%1 lignes supprimées").arg(nbDeletedRows)+"\n"+
                      QObject::tr("%1 lignes créées").arg(nbCreatedRows)+"\n"+
                      QObject::tr("%1 lignes modifiées").arg(nbModifiedRows)+"\n"+
                      QObject::tr("%1 erreurs").arg(nbErrors));
    }
}

void MainWindow::on_mExport_triggered()
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
            OkCancelDialog("Potaléger "+ui->lVer->text(),tr("Le fichier existe déjà")+"\n"+
                               sFileName+"\n"+
                               FileInfoVerif.lastModified().toString("yyyy-MM-dd HH:mm:ss")+" - " + QString::number(FileInfoVerif.size()/1000)+" ko\n\n"+
                               tr("Remplacer ?"),QStyle::SP_MessageBoxWarning,600)) {
            QFile FileInfo2;
            if (FileInfoVerif.exists()) {
                FileInfo2.setFileName(sFileName);
                if (!FileInfo2.remove()) {
                    MessageDlg("Potaléger "+ui->lVer->text(),tr("Impossible de supprimer le fichier")+"\n"+
                                      sFileName,"",QStyle::SP_MessageBoxCritical);
                    return;
                }
            }

            PathExport=FileInfoVerif.absolutePath()+QDir::separator();

            //Export
            QFile FileExport(sFileName);
            if (!FileExport.open(QIODevice::WriteOnly)) {// | QIODevice::Text
                MessageDlg("Potaléger "+ui->lVer->text(),tr("Impossible de créer le fichier")+"\n"+
                                  sFileName,"",QStyle::SP_MessageBoxCritical);
                return;
            }

            QByteArray data;
            QStringList dataTypes;
            QLocale locale;
            QString decimalSep = QString(locale.decimalPoint());

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
                            data.append(EscapeCSV(StrReplace(w->model->data(w->model->index(row, col),Qt::EditRole).toString(),".",decimalSep)).toUtf8());
                        else
                            data.append(EscapeCSV(w->model->data(w->model->index(row, col),Qt::EditRole).toString()).toUtf8());
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
                MessageDlg("Potaléger "+ui->lVer->text(),tr("%1 lignes exportées vers le fichier").arg(str(totalRow))+"\n"+
                                  sFileName,"",QStyle::SP_MessageBoxInformation);
            } else
                MessageDlg("Potaléger "+ui->lVer->text(),tr("%1 sur %2 lignes exportées vers le fichier").arg(str(exportedRow)).arg(str(totalRow))+"\n"+
                                  sFileName,"",QStyle::SP_MessageBoxWarning);
        }
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
        QSettings settings("greli.net", "Potaléger");
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
        QSettings settings("greli.net", "Potaléger");
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
        MessageDlg(windowTitle(),tr("Aucune culture à planifier:")+"\n\n"+
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
        CultAVenir="<br><br>"+tr("IL Y A DÉJÀ %1 CULTURES NI SEMÉES NI PLANTÉES.").arg(NbCultAVenir)+"<br>"+
                   iif(NbCultAVenir>NbCultPlanif*0.9,tr("Peut-être avez-vous déjà généré les prochaines cultures."),"").toString();
    } else {
        icon=QStyle::SP_MessageBoxQuestion;
        CultAVenir="";
    }
    if (OkCancelDialog(windowTitle(),
                       tr("Créer les cultures de la saison %1 ?").arg("<b>"+pQuery.Selec0ShowErr("SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'").toString()+"</b>")+"<br><br>"+
                       tr("La saison courante peut être modifiée dans les paramètres (menu 'Edition').")+"<br><br>"+
                       tr("%1 cultures vont être créées en fonction des rotations.").arg("<b>"+str(NbCultPlanif)+"</b>")+"<br>"+
                       tr("Id de la dernière culture:")+" "+str(NbCultAVenir)+
                       CultAVenir,
                       icon)) {
        int choice=2;
        if (NbCultPlanifRetard>0){
            choice = RadiobuttonDialog(windowTitle(),tr("Parmis les %1 cultures à créer, il y en a %2 dont la date de la 1ère opération (semis ou plantation) est déjà passée.").arg(NbCultPlanif).arg(NbCultPlanifRetard),
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
            result=pQuery.ExecShowErr("INSERT INTO Cultures (Espèce,IT_plante,Variété,Fournisseur,Planche,D_planif,Longueur,Nb_rangs,Espacement) "
                                       "SELECT Espèce,IT_plante,Variété,Fournisseur,Planche,(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'),Longueur,Nb_rangs,Espacement "
                                       "FROM Cult_planif WHERE coalesce(Date_semis,Date_plantation)>=DATE('now')");
        else
            result=pQuery.ExecShowErr("INSERT INTO Cultures (Espèce,IT_plante,Variété,Fournisseur,Planche,D_planif,Longueur,Nb_rangs,Espacement) "
                                       "SELECT Espèce,IT_plante,Variété,Fournisseur,Planche,(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'),Longueur,Nb_rangs,Espacement "
                                       "FROM Cult_planif");
        if (result) {
            int IdCult2=pQuery.Selec0ShowErr("SELECT min(Culture) FROM Cultures WHERE Culture>"+str(IdCult1)).toInt();
            int IdCult3=pQuery.Selec0ShowErr("SELECT max(Culture) FROM Cultures").toInt();
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
    OpenPotaTab("Varietes_inv_et_cde","Variétés__inv_et_cde",tr("Inv. et cde semence"));
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
        QSettings settings("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Cultures__non_terminées-pageFilter").toInt());
        w->pageFilterFrame->setVisible(true);
        if (w->cbPageFilter->currentIndex()>0)
            w->pbFilterClick(false);
    }
}

void MainWindow::on_mCouverture_triggered()
{
    if (OpenPotaTab("Especes__couverture","Espèces__couverture",tr("Couverture obj."))){
        PotaWidget *w=dynamic_cast<PotaWidget*>(ui->tabWidget->currentWidget());
        w->tv->hideColumn(0);//Saison.
        w->lPageFilter->setText(tr("Saison"));
        PotaQuery query(db);
        int saison,smin,smax;
        saison=query.Selec0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='Année_culture'").toInt();
        smin=query.Selec0ShowErr("SELECT min(Saison) FROM Espèces__couverture").toInt();
        smax=query.Selec0ShowErr("SELECT max(Saison) FROM Espèces__couverture").toInt();
        for (int i = smin; i <= smax; ++i) {
            w->cbPageFilter->addItem(str(i));
            w->pageFilterFilters.append("Saison='"+str(i)+"'");
        }
        w->cbPageFilter->setCurrentText(str(saison));
        w->pageFilterFrame->setVisible(true);
        w->pbFilterClick(false);
    };
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
        QSettings settings("greli.net", "Potaléger");
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
        QSettings settings("greli.net", "Potaléger");
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
        QSettings settings("greli.net", "Potaléger");
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
        //Go to last row.
        w->tv->setCurrentIndex(w->model->index(w->model->rowCount()-1,1));
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
        QSettings settings("greli.net", "Potaléger");
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
        w->lPageFilter->setText(tr("Voir"));
        w->cbPageFilter->addItem(tr("Toutes"));
        w->pageFilterFilters.append("TRUE");
        w->cbPageFilter->addItem(tr("Non ter."));
        w->pageFilterFilters.append("(Terminée='v')OR(Terminée='V')");
        QSettings settings("greli.net", "Potaléger");
        w->cbPageFilter->setCurrentIndex(settings.value("Cultures__vivaces-pageFilter").toInt());
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
        w->cbPageFilter->addItem(tr("Annuelles"));
        w->pageFilterFilters.append("(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        w->cbPageFilter->addItem(tr("Vivaces"));
        w->pageFilterFilters.append("(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)");
        QSettings settings("greli.net", "Potaléger");
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
        //Go to last row.
        w->tv->setCurrentIndex(w->model->index(w->model->rowCount()-1,1));
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
        saison=query.Selec0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='Année_culture'").toInt();
        smin=query.Selec0ShowErr("SELECT min(Saison) FROM Planches__bilan_fert").toInt();
        smax=query.Selec0ShowErr("SELECT max(Saison) FROM Planches__bilan_fert").toInt();
        for (int i = smin; i <= smax; ++i) {
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
    OpenPotaTab( "Especes__inventaire","Espèces__inventaire",tr("Inventaire E."));
}

//Menu Analyses

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
        pQuery.ExecShowErr("DROP VIEW UserSQL;");
        pQuery.ExecShowErr("CREATE VIEW UserSQL AS "+values[0]);
        QString sTitre=tr("Requête SQL");
        QString sDesc="";
        if(values.count()>1) sTitre=iif(values[1][0]=="\n",values[1].mid(1),values[1]).toString();
        if(values.count()>2) sDesc=iif(values[2][0]=="\n",values[2].mid(1),values[2]).toString();
        OpenPotaTab("SQLQuery","UserSQL",sTitre,sDesc);
    }
}

// Local params

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


