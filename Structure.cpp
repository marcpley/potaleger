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
    PotaQuery *query = new PotaQuery(db);
    PotaQuery *qNewTableFields = new PotaQuery(db);
    PotaQuery *qOldTableFields = new PotaQuery(db);
    query->lErr = ui->lDBErr;
    bool bNew=false;
    bool bResult=true;
    QString sResult="";

    AppBusy(true,ui->progressBar,0,"%p%");

    if (bResult){
        bResult=query->ExecShowErr("PRAGMA locking_mode = EXCLUSIVE;")and
                query->ExecShowErr("PRAGMA locking_mode = NORMAL;");
        if (!bResult)
            sResult.append(tr("Impossible d'obtenir un accès exclusif à la BDD.")+"\n");
    }
    if (bResult){
        bResult=query->ExecShowErr("PRAGMA foreign_keys = OFF");
        if (!bResult)
            sResult.append("Foreign keys : Err\n");
    }

    if (sDBVersion == "New" or sDBVersion == "NewWithBaseData") {
        //bNew=true; mettre à false quand debug fait.
        if (bResult){
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("CREATE TABLES %p%");
            bResult=query->ExecMultiShowErr(DynDDL(sDDLTables),";",ui->progressBar);
            sResult.append("Create tables (").append(DbVersion).append(") : "+iif(bResult,"ok","Err").toString()+"\n");
        }
        if (bResult){
            //Create scalar functions
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Scalar functions %p%");
            bResult = registerScalarFunctions(&db);
            sResult.append("Scalar functions : "+iif(bResult,"ok","Err").toString()+"\n");
        }
        if (bResult and(sDBVersion == "NewWithBaseData")) {
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("BASE DATA %p%");
            bResult=query->ExecMultiShowErr(sSQLBaseData,";",ui->progressBar,true);
            sResult.append("Base data : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult)
            sDBVersion = DbVersion;

    } else {    //Updating an existing db.
        if (bResult){ //DROP all views
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("DROP VIEWS %p%");
            QString sDropViews="";
            query->clear();
            query->ExecShowErr("PRAGMA table_list;");
            while (query->next()) {
                if (query->value("type").toString()=="view"){
                    sDropViews = sDropViews + "DROP VIEW IF EXISTS \""+query->value("name").toString()+"\";";
                }
            }
            bResult=query->ExecMultiShowErr(sDropViews,";",ui->progressBar);
            if (!bResult)
                sResult.append("Reset views : Err");
        }

        if (bResult){ //DROP SQLean functions
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("DROP SQLean functions %p%");
            //QSqlDatabase db = QSqlDatabase::database();
            QString sDbFile=db.databaseName();
            dbClose();
            bResult=dbOpen(sDbFile,false,true,false);
            if (bResult){
                dbClose();
                bResult=dbOpen(sDbFile,false,false,false);
            }
            if (!bResult)
                sResult.append("Reset SQLean : Err");
        }

        if (bResult){ //Create scalar functions
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Scalar functions %p%");
            bResult = registerScalarFunctions(&db);
            sResult.append("Scalar functions : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        // Update shema (specific).
        if (bResult and(sDBVersion == "2024-12-30")) { //Useless specific update shema.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult = query->ExecMultiShowErr(sDDL20250120,";",ui->progressBar);
            sResult.append(sDBVersion+" -> 2025-01-20 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-01-20";
        }
        if (bResult and(sDBVersion == "2025-01-20")) { //Adding tables: Destinations, Consommations.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult = query->ExecMultiShowErr(sDDL20250227,";",ui->progressBar);
            sResult.append(sDBVersion+" -> 2025-02-27 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-02-27";
        }
        if (bResult and(sDBVersion == "2025-02-27")) { //Adding field: Culture.Récolte_comm.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult = query->ExecMultiShowErr(sDDL20250305,";",ui->progressBar);
            sResult.append(sDBVersion+" -> 2025-03-05 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-03-05";
        }
        if (bResult and(sDBVersion == "2025-03-05")) { //Drop field: Culture.Récolte_comm.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult = query->ExecMultiShowErr(sDDL20250325,";",ui->progressBar);
            sResult.append(sDBVersion+" -> 2025-03-25 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-03-25";
        }
        if (bResult and(sDBVersion == "2025-03-25")) { //Views and triggers update.
            sResult.append(sDBVersion+" -> 2025-04-04 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-04-04";
        }
        if (bResult and(sDBVersion == "2025-04-04")) { //Views and triggers update.
            sResult.append(sDBVersion+" -> 2025-04-12 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-04-12";
        }
        if (bResult) { //Update schema.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Update shema %p%");
            QString sUpdateSchema="BEGIN TRANSACTION;";
            //Rename old tables.
            query->clear();
            query->ExecShowErr("PRAGMA table_list;");
            while (query->next()) {
                if (query->value("type").toString()=="table" and !query->value("name").toString().startsWith("sql"))//No sqlite or sqlean tables.
                    sUpdateSchema +="DROP TABLE IF EXISTS Temp_"+query->value("name").toString()+";"
                                    "CREATE TABLE Temp_"+query->value("name").toString()+" AS "
                                           "SELECT * FROM "+query->value("name").toString()+";"
                                    "DROP TABLE "+query->value("name").toString()+";";
            }

            //Create new tables.
            sUpdateSchema += DynDDL(sDDLTables);

            bResult = query->ExecMultiShowErr(sUpdateSchema,";",ui->progressBar);
            if (bResult) {
                sUpdateSchema="";
                ui->progressBar->setValue(0);
                ui->progressBar->setMaximum(0);
                ui->progressBar->setFormat("Data transfert %p%");
                //Import data from old to new tables.
                QString sFieldsList;
                query->clear();
                query->ExecShowErr("PRAGMA table_list;");
                while (query->next()) {
                    if (query->value("type").toString()=="table" and
                        !query->value("name").toString().startsWith("Temp_") and
                        !query->value("name").toString().startsWith("sql")) {
                        sFieldsList="";
                        qNewTableFields->ExecShowErr("PRAGMA table_xinfo("+query->value("name").toString()+");");//New table
                        while (qNewTableFields->next()) {
                            if (qNewTableFields->value("hidden").toInt()==0) {
                                qOldTableFields->ExecShowErr("PRAGMA table_xinfo(Temp_"+query->value("name").toString()+");");//Old table
                                qDebug() << query->value("name").toString() << " - " << qNewTableFields->value("name").toString();
                                while (qOldTableFields->next()) {
                                    qDebug() << qOldTableFields->value("name").toString();
                                    if (qOldTableFields->value("name").toString()==qNewTableFields->value("name").toString())//Fields exists in old and new tables
                                        sFieldsList +=qNewTableFields->value("name").toString()+",";
                                }
                            }
                        }
                        sFieldsList=sFieldsList.removeLast();
                        if (!sFieldsList.isEmpty())
                            sUpdateSchema +="INSERT INTO "+query->value("name").toString()+" ("+sFieldsList+") " //todo: spécifier les champs si pas le même nom.
                                            "SELECT "+sFieldsList+" FROM Temp_"+query->value("name").toString()+";";
                    }
                }

                //DROP Temp tables.
                query->clear();
                query->ExecShowErr("PRAGMA table_list;");
                while (query->next()) {
                    if (query->value("type").toString()=="table" and !query->value("name").toString().startsWith("sql"))
                        sUpdateSchema +="DROP TABLE IF EXISTS Temp_"+query->value("name").toString()+";";
                }

                //Update schema.
                sUpdateSchema += "COMMIT TRANSACTION;";
                bResult = query->ExecMultiShowErr(sUpdateSchema,";",ui->progressBar);
            }
            sResult.append("Data transfert : "+iif(bResult,"ok","Err").toString()+"\n");
        }
    }

    if (bResult and(sDBVersion == DbVersion)) { //Tables shema ok.
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
            bResult = query->ExecMultiShowErr(DynDDL(sDDLViews),";",ui->progressBar);
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
            bResult = query->ExecMultiShowErr(sDDLTriggers,";;",ui->progressBar);//";" exists in CREATE TRIGGER statments
            sResult.append("Triggers : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){
            //Update params table
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Params %p%");
            bResult = query->ExecMultiShowErr(sDDLTableParams,";",ui->progressBar);
            sResult.append("Params : "+iif(bResult,"ok","Err").toString()+"\n");
        }
    }

    if (bResult){
        bResult = query->ExecShowErr("PRAGMA foreign_keys = ON");
        if (!bResult)
            sResult.append("Foreign keys : Err\n");
    }
    if (bResult){
        bResult = query->ExecShowErr("PRAGMA locking_mode = NORMAL;");
        if (!bResult)
            sResult.append(tr("Impossible d'annuler l'accès exclusif.")+"\n");
    }

    AppBusy(false,ui->progressBar);

    if (bResult) {
        if (sDBVersion == DbVersion) {
            if (!bNew)
                MessageDialog(tr("Mise à jour réussie."),
                                  sResult,QStyle::SP_MessageBoxInformation);
            return true;
        }
        else {
            MessageDialog(tr("Version de BDD inconnue:")+sDBVersion,
                          sResult,QStyle::SP_MessageBoxCritical);
            return false;
        }
    } else {
        MessageDialog(tr("Echec de la mise à jour."),
                          sResult,QStyle::SP_MessageBoxCritical);
        return false;
    }
}
