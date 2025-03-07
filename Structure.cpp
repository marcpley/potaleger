#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include "SQL/UpdateStru.sql"
#include "SQL/CreateTables.sql"
#include "SQL/CreateViews.sql"
#include "SQL/CreateBaseData.sql"
#include "SQL/CreateTriggers.sql"
#include "SQL/UpdateTableParams.sql"
#include "PotaUtils.h"
#include "data/Data.h"
#include "SQL/FunctionsSQLite.h"

bool MainWindow::UpdateDBShema(QString sDBVersion)
{
    PotaQuery *model = new PotaQuery(db);
    PotaQuery *model2 = new PotaQuery(db);
    PotaQuery *model3 = new PotaQuery(db);
    model->lErr = ui->lDBErr;
    bool bNew=false;
    bool bResult=true;
    QString sResult="";

    if (!model->ExecShowErr("PRAGMA locking_mode = EXCLUSIVE;")) {
        ui->tbInfoDB->append(tr("Impossible d'obtenir un accès exclusif à la BDD."));
        model->ExecShowErr("PRAGMA locking_mode = NORMAL;");
        return false;
    }
    if (!model->ExecShowErr("PRAGMA foreign_keys = OFF")) {
        ui->tbInfoDB->append(tr("Impossible de désactiver les clés étrangères."));
        return false;
    }
    AppBusy(true,ui->progressBar,0,"%p%");

    if (sDBVersion == "New" or sDBVersion == "NewWithBaseData")
    {
        //bNew=true; mettre à false quand debug fait.
        ui->progressBar->setFormat("CREATE TABLES %p%");
        if (model->ExecMultiShowErr(DynDDL(sDDLTables),";",ui->progressBar)) {
            sResult.append("Create tables : ok (").append(DbVersion).append(")\n");
            if (sDBVersion == "NewWithBaseData") {
                ui->progressBar->setFormat("BASE DATA %p%");
                if (!model->ExecMultiShowErr(sSQLBaseData,";",ui->progressBar,true)) {
                    ui->tbInfoDB->append(tr("Echec de la création des données de base")+" ("+ui->lVerBDDAttendue->text()+")");
                    AppBusy(false,ui->progressBar);
                    return false;
                }
                sResult.append("Create base data : ok\n");
            }
            sDBVersion = DbVersion;//"2024-12-30";
        } else {
            ui->tbInfoDB->append(tr("Echec de la création de la BDD vide")+" ("+ui->lVerBDDAttendue->text()+")");
            AppBusy(false,ui->progressBar);
            return false;
        }
    } else {    //Updating an existing db.
        //DROP all views
        ui->progressBar->setValue(0);
        ui->progressBar->setMaximum(0);
        ui->progressBar->setFormat("DROP VIEWS %p%");
        QString sDropViews="";
        model->clear();
        model->ExecShowErr("PRAGMA table_list;");
        while (model->next()) {
            if (model->value("type").toString()=="view"){
                sDropViews = sDropViews + "DROP VIEW IF EXISTS \""+model->value("name").toString()+"\";";
            }
        }
        model->ExecMultiShowErr(sDropViews,";",ui->progressBar);

        //DROP SQLean functions
        ui->progressBar->setValue(0);
        ui->progressBar->setMaximum(0);
        ui->progressBar->setFormat("DROP SQLean functions %p%");
        //QSqlDatabase db = QSqlDatabase::database();
        QString sDbFile=db.databaseName();
        dbClose();
        dbOpen(sDbFile,false,true,false);
        dbClose();
        dbOpen(sDbFile,false,false,false);

        if (bResult){
            //Create scalar functions
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Scalar functions %p%");
            bResult = registerScalarFunctions(&db);
            sResult.append("Scalar functions : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult and(sDBVersion == "2024-12-30")) { //Useless specific update shema.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult = model->ExecMultiShowErr(sDDL20250120,";",ui->progressBar);
            sResult.append(sDBVersion+" -> 2025-01-20 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-01-20";
        }
        if (bResult and(sDBVersion == "2025-01-20")) { //Adding tables: Destinations, Consommations.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult = model->ExecMultiShowErr(sDDL20250227,";",ui->progressBar);
            sResult.append(sDBVersion+" -> 2025-02-27 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-02-27";
        }
        if (bResult and(sDBVersion == "2025-02-27")) { //Adding field: Culture.Récolte_comm.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult = model->ExecMultiShowErr(sDDL20250305,";",ui->progressBar);
            sResult.append(sDBVersion+" -> 2025-03-05 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-03-05";
        }
        if (bResult) { //Update schema.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Update shema %p%");
            QString sUpdateSchema="BEGIN TRANSACTION;";
            //Rename old tables.
            model->clear();
            model->ExecShowErr("PRAGMA table_list;");
            while (model->next()) {
                if (model->value("type").toString()=="table" and !model->value("name").toString().startsWith("sql"))//No sqlite or sqlean tables.
                    sUpdateSchema +="DROP TABLE IF EXISTS Temp_"+model->value("name").toString()+";"
                                    "CREATE TABLE Temp_"+model->value("name").toString()+" AS "
                                           "SELECT * FROM "+model->value("name").toString()+";"
                                    "DROP TABLE "+model->value("name").toString()+";";
            }

            //Create new tables.
            sUpdateSchema += DynDDL(sDDLTables);

            bResult = model->ExecMultiShowErr(sUpdateSchema,";",ui->progressBar);
            if (bResult) {
                sUpdateSchema="";
                ui->progressBar->setValue(0);
                ui->progressBar->setMaximum(0);
                ui->progressBar->setFormat("Data transfert %p%");
                //Import data from old to new tables.
                QString sFieldsList;
                model->clear();
                model->ExecShowErr("PRAGMA table_list;");
                while (model->next()) {
                    if (model->value("type").toString()=="table" and
                        !model->value("name").toString().startsWith("Temp_") and
                        !model->value("name").toString().startsWith("sql")) {
                        sFieldsList="";
                        model2->ExecShowErr("PRAGMA table_xinfo("+model->value("name").toString()+");");//New table
                        while (model2->next()) {
                            if (model2->value("hidden").toInt()==0) {
                                model3->ExecShowErr("PRAGMA table_xinfo(Temp_"+model->value("name").toString()+");");//Old table
                                qDebug() << model->value("name").toString() << " - " << model2->value("name").toString();
                                while (model3->next()) {
                                    qDebug() << model3->value("name").toString();
                                    if (model3->value("name").toString()==model2->value("name").toString())//Fields exists in old and new tables
                                        sFieldsList +=model2->value("name").toString()+",";
                                }
                            }
                        }
                        sFieldsList=sFieldsList.removeLast();
                        sUpdateSchema +="INSERT INTO "+model->value("name").toString()+" ("+sFieldsList+") " //todo: spécifier les champs si pas le même nom.
                                        "SELECT "+sFieldsList+" FROM Temp_"+model->value("name").toString()+";";
                    }
                }

                //DROP Temp tables.
                model->clear();
                model->ExecShowErr("PRAGMA table_list;");
                while (model->next()) {
                    if (model->value("type").toString()=="table" and !model->value("name").toString().startsWith("sql"))
                        sUpdateSchema +="DROP TABLE IF EXISTS Temp_"+model->value("name").toString()+";";
                }

                //Update schema.
                sUpdateSchema += "COMMIT TRANSACTION;";
                bResult = model->ExecMultiShowErr(sUpdateSchema,";",ui->progressBar);
            }
            sResult.append("Data transfert : "+iif(bResult,"ok","Err").toString()+"\n");
        }
    }

    if (bResult and(sDBVersion == ui->lVerBDDAttendue->text())) { //Tables shema ok.
        //if (sResult=="")
        //    sResult.append("Version : "+sDBVersion+"\n");

        // if (bResult){
        //     //Create scalar functions
        //     ui->progressBar->setValue(0);
        //     ui->progressBar->setMaximum(0);
        //     ui->progressBar->setFormat("Scalar functions %p%");
        //     bResult = registerScalarFunctions(&db);
        //     sResult.append("Scalar functions : "+iif(bResult,"ok","Err").toString()+"\n");
        // }

        if (bResult){
            //Create views
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Views %p%");
            bResult = model->ExecMultiShowErr(DynDDL(sDDLViews),";",ui->progressBar);
            sResult.append("Views : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){
            //Create table valued functions
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Table valued functions %p%");
            bResult = registerTableValuedFunctions(&db);
            sResult.append("Table valued functions : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){
            //Test functions
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Function test %p%");
            QString sErrFunc = testCustomFunctions(&db);
            bResult = (sErrFunc=="");
            sResult.append("Function test : "+iif(bResult,"ok","Err ("+sErrFunc+")").toString()+"\n");
        }

        if (bResult){
            //Create triggers
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Triggers %p%");
            bResult = model->ExecMultiShowErr(sDDLTriggers,";;",ui->progressBar);//";" exists in CREATE TRIGGER statments
            sResult.append("Triggers : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){
            //Update params table
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Params %p%");
            bResult = model->ExecMultiShowErr(sDDLTableParams,";",ui->progressBar);
            sResult.append("Params : "+iif(bResult,"ok","Err").toString()+"\n");
        }
    }

    AppBusy(false,ui->progressBar);

    if (bResult and !model->ExecShowErr("PRAGMA foreign_keys = ON")) {
        ui->tbInfoDB->append(tr("Impossible d'activer les clés étrangères."));
        return false;
    }
    if (bResult and !model->ExecShowErr("PRAGMA locking_mode = NORMAL;")) {//No test yet
        ui->tbInfoDB->append(tr("Impossible d'annuler l'accès exclusif."));
        return false;
    }
    if (bResult)
    {
        if (sDBVersion == ui->lVerBDDAttendue->text())
        {
            if (!bNew)
                MessageDialog(tr("Mise à jour réussie."),
                                  sResult,QStyle::SP_MessageBoxInformation);
            return true;
        }
        else {
            ui->tbInfoDB->append(tr("Version de BDD inconnue:")+sDBVersion);
            return false;
        }
    }
    else
    {
        MessageDialog(tr("Echec de la mise à jour."),
                          sResult,QStyle::SP_MessageBoxCritical);
        ui->tbInfoDB->append(tr("Echec de la mise à jour de la version de la BDD ")+" ("+sDBVersion+" > "+ui->lVerBDDAttendue->text()+")");
        return false;
    }
}
