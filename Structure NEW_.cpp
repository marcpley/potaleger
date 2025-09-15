#include "Dialogs.h"
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
#include "SQL/UpdateBaseData.sql"
#include "PotaUtils.h"
#include "data/Data.h"

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
        // if (bResult){
        //     //Create scalar functions
        //     ui->progressBar->setValue(0);
        //     ui->progressBar->setMaximum(0);
        //     ui->progressBar->setFormat("Scalar functions %p%");
        //     bResult = registerScalarFunctions(&db);
        //     sResult.append("Scalar functions : "+iif(bResult,"ok","Err").toString()+"\n");
        // }
        if (bResult and(sDBVersion == "NewWithBaseData")) {
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("BASE DATA %p%");
            bResult=query->ExecMultiShowErr(sSQLBaseData,";",ui->progressBar,true);
            sResult.append("Base data : "+iif(bResult,"ok","Err").toString()+"\n");
        }
        if (bResult and(sDBVersion == "NewWithBaseData")) {
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("UPDATE NPK DATA (Espèces) %p%");
            bResult=query->ExecMultiShowErr(sUpdateBaseDataNPKE,";",ui->progressBar,true);
            sResult.append("Update base data (Espèces) : "+iif(bResult,"ok","Err").toString()+"\n");
        }
        if (bResult and(sDBVersion == "NewWithBaseData")) {
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("UPDATE NPK DATA (Fertilisants) %p%");
            bResult=query->ExecMultiShowErr(sUpdateBaseDataNPKF,";",ui->progressBar,true);
            sResult.append("Update base data (Fertilisants) : "+iif(bResult,"ok","Err").toString()+"\n");
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
            query->ExecShowErr("SELECT * FROM sqlite_master ORDER BY name;");
            while (query->next()) {
                if (query->value("type").toString()=="view"){
                    sDropViews = sDropViews + "DROP VIEW IF EXISTS \""+query->value("name").toString()+"\";";
                }
            }
            bResult=query->ExecMultiShowErr(sDropViews,";",ui->progressBar);
            if (!bResult)
                sResult.append("Reset views : Err");
        }
        if (bResult){ //DROP all trigers
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("DROP TRIGGER %p%");
            QString sDropTrigger="";
            query->clear();
            query->ExecShowErr("SELECT * FROM sqlite_master ORDER BY name;");
            while (query->next()) {
                if (query->value("type").toString()=="trigger"){
                    sDropTrigger = sDropTrigger + "DROP TRIGGER IF EXISTS \""+query->value("name").toString()+"\";";
                }
            }
            bResult=query->ExecMultiShowErr(sDropTrigger,";",ui->progressBar);
            if (!bResult)
                sResult.append("Reset triggers : Err");
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
        if (bResult and(sDBVersion == "2025-04-12")) { //Views update.
            sResult.append(sDBVersion+" -> 2025-05-13 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-05-13";
        }
        if (bResult and(sDBVersion == "2025-05-13")) { //Adding field: Espèces.N P K.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult = query->ExecMultiShowErr(sDDL20250514,";",ui->progressBar) and
                      query->ExecMultiShowErr(sUpdateBaseDataNPKE,";",ui->progressBar) and
                      query->ExecMultiShowErr(sUpdateBaseDataNPKF,";",ui->progressBar);
            sResult.append(sDBVersion+" -> 2025-05-14 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-05-14";
        }
        if (bResult and(sDBVersion == "2025-05-14")) { //Views update.
            sResult.append(sDBVersion+" -> 2025-06-08 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-06-08";
        }
        if (bResult and(sDBVersion == "2025-06-08")) { //Adding field: Espèces.Vivace.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult = query->ExecMultiShowErr(sDDL20250622,";",ui->progressBar);
            sResult.append(sDBVersion+" -> 2025-06-22 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-06-22";
        }
        if (bResult and(sDBVersion == "2025-06-22")) { //Adding field: S_xxx,D_récolte,Décal_max
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult = query->ExecMultiShowErr(sDDL20250728,";",ui->progressBar);
            sResult.append(sDBVersion+" -> 2025-07-28 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-07-28";
        }

        if (bResult) { //Update schema.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Update shema %p%");
            QString sUpdateSchema="BEGIN TRANSACTION;";
            //drop old temp tables.
            query->clear();
            query->ExecShowErr("SELECT * FROM sqlite_master ORDER BY name;");
            while (query->next()) {
                if (query->value("type").toString()=="table" and
                    (query->value("name").toString().startsWith("Temp_") or query->value("name").toString().startsWith("NEW_")))
                    sUpdateSchema +="DROP TABLE IF EXISTS "+query->value("name").toString()+";";
                                    // "CREATE TABLE Temp_"+query->value("name").toString()+" AS "
                                    //        "SELECT * FROM "+query->value("name").toString()+";"
                                    // "DROP TABLE "+query->value("name").toString()+";";
                    // sUpdateSchema +="DROP TABLE IF EXISTS Temp_"+query->value("name").toString()+";"
                    //                 "ALTER TABLE "+query->value("name").toString()+" RENAME TO "
                    //                        "Temp_"+query->value("name").toString()+";";
            }

            //Create new tables.
            sUpdateSchema += DynDDL(StrReplace(sDDLTables,"CREATE TABLE ","CREATE TABLE NEW_"));
            sUpdateSchema += "COMMIT TRANSACTION;";

            bResult = query->ExecMultiShowErr(sUpdateSchema,";",ui->progressBar);
            if (bResult) {
                sUpdateSchema="BEGIN TRANSACTION;";
                ui->progressBar->setValue(0);
                ui->progressBar->setMaximum(0);
                ui->progressBar->setFormat("Data transfert %p%");
                //Import data from old to new tables.
                QString sFieldsListInsert,sFieldsListSelect;
                query->clear();
                query->ExecShowErr("SELECT * FROM sqlite_master ORDER BY name;");
                while (query->next()) {
                    if (query->value("type").toString()=="table" and query->value("name").toString().startsWith("NEW_")) {
                        sFieldsListInsert="";
                        sFieldsListSelect="";
                        qNewTableFields->clear();
                        qNewTableFields->ExecShowErr("PRAGMA table_xinfo("+query->value("name").toString()+");");//New table
                        while (qNewTableFields->next()) {
                            if (qNewTableFields->value("hidden").toInt()==0) {
                                qOldTableFields->ExecShowErr("PRAGMA table_xinfo("+query->value("name").toString().remove("NEW_")+");");//Old table
                                // qDebug() << query->value("name").toString() << " - " << qNewTableFields->value("name").toString();
                                while (qOldTableFields->next()) {
                                    // qDebug() << qOldTableFields->value("name").toString();
                                    if (qOldTableFields->value("name").toString()==qNewTableFields->value("name").toString()) {//Fields exists in old and new tables
                                        sFieldsListInsert +=qNewTableFields->value("name").toString()+",";
                                        // if (qOldTableFields->value("type").toString()=="TEXT")
                                        //     sFieldsListSelect +="CAST("+qNewTableFields->value("name").toString()+" AS BLOB),";
                                        // else
                                            sFieldsListSelect +=qNewTableFields->value("name").toString()+",";
                                    }
                                }
                            }
                        }
                        sFieldsListInsert=sFieldsListInsert.removeLast();
                        sFieldsListSelect=sFieldsListSelect.removeLast();
                        if (!sFieldsListInsert.isEmpty()) {
                            // sUpdateSchema +="INSERT INTO "+query->value("name").toString()+" ("+sFieldsList+") " //todo: spécifier les champs si pas le même nom.
                            //                 "SELECT "+sFieldsList+" FROM Temp_"+query->value("name").toString()+";";
                            sUpdateSchema +="INSERT INTO "+query->value("name").toString()+" ("+sFieldsListInsert+") " //todo: spécifier les champs si pas le même nom.
                                            "SELECT "+sFieldsListSelect+" FROM "+query->value("name").toString().remove("NEW_")+";";
                        }
                    }
                }
                sUpdateSchema += "COMMIT TRANSACTION;";
                sUpdateSchema += "BEGIN TRANSACTION;";

                //DROP old tables and rename new tables.
                query->clear();
                query->ExecShowErr("SELECT * FROM sqlite_master ORDER BY name;");
                while (query->next()) {
                    if (query->value("type").toString()=="table" and query->value("name").toString().startsWith("NEW_"))
                        sUpdateSchema +="DROP TABLE IF EXISTS "+query->value("name").toString().remove("NEW_")+";"
                                        "ALTER TABLE "+query->value("name").toString()+" RENAME TO "+query->value("name").toString().remove("NEW_")+";";
                }

                //Update schema.
                sUpdateSchema += "COMMIT TRANSACTION;";
                bResult = query->ExecMultiShowErr(sUpdateSchema,";",ui->progressBar);
                if (bResult and sDBVersion=="?")
                    sDBVersion = DbVersion;
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
                MessageDlg("Potaléger "+ui->lVer->text(),tr("Mise à jour réussie."),
                           sResult,QStyle::SP_MessageBoxInformation);
            return true;
        }
        else {
            MessageDlg("Potaléger "+ui->lVer->text(),tr("Version de BDD inconnue:")+sDBVersion,
                       sResult,QStyle::SP_MessageBoxCritical);
            return false;
        }
    } else {
        MessageDlg("Potaléger "+ui->lVer->text(),tr("Echec de la mise à jour."),
                   sResult,QStyle::SP_MessageBoxCritical);
        return false;
    }
}
