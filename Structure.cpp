#include "Dialogs.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include "PotaUtils.h"
#include "data/Data.h"

bool MainWindow::UpdateDBShema(QString sDBVersion)
{
    PotaQuery *query=new PotaQuery(db);
    PotaQuery *qNewTableFields=new PotaQuery(db);
    PotaQuery *qOldTableFields=new PotaQuery(db);
    query->lErr=ui->lDBErr;
    #ifdef QT_NO_DEBUG
    bool bNew=false;
    #endif
    bool bResult=true;
    // bool bInsertBaseData=false;
    // bool bUpdateBaseData=false;
    QString sResult="";

    AppBusy(true,ui->progressBar,0,"%p%");

    if (bResult){ //Set exclusive access.
        bResult=query->ExecShowErr("PRAGMA locking_mode=EXCLUSIVE;")and
                query->ExecShowErr("PRAGMA locking_mode=NORMAL;");
        if (!bResult)
            sResult.append(tr("Impossible d'obtenir un accès exclusif à la BDD.")+"\n");
    }
    if (bResult){ //Set FK OFF
        bResult=query->ExecShowErr("PRAGMA foreign_keys=OFF");
        if (!bResult)
            sResult.append("Foreign keys : Err\n");
    }

    if (sDBVersion=="New" or sDBVersion=="NewWithBaseData") { //Create new DB.
        if (bResult){ //Create empty tables.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("CREATE TABLES %p%");
            bResult=query->ExecMultiShowErr(DynDDL(loadSQLFromResource("CreateTables")),";",ui->progressBar);
            sResult.append("Create tables (").append(DbVersion).append(") : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult and(sDBVersion=="NewWithBaseData")) { //Insert general base data.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("BASE DATA %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("CreateBaseData"),";",ui->progressBar); //keepReturns true
            sResult.append("Base data : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult) {
            //bInsertBaseData=(sDBVersion=="NewWithBaseData");
            sDBVersion=DbVersion;
        }

    } else { //Update existing db.
        if (bResult){ //DROP all trigers
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("DROP TRIGGER %p%");
            QString sDropTrigger="BEGIN TRANSACTION;";
            query->clear();
            query->ExecShowErr("SELECT * FROM sqlite_master ORDER BY name;");
            while (query->next()) {
                if (query->value("type").toString()=="trigger"){
                    sDropTrigger=sDropTrigger + "DROP TRIGGER IF EXISTS \""+query->value("name").toString()+"\";";
                }
            }
            sDropTrigger+="COMMIT TRANSACTION;";
            bResult=query->ExecMultiShowErr(sDropTrigger,";",ui->progressBar);
            if (!bResult)
                sResult.append("Reset triggers : Err");
        }
        if (bResult){ //DROP all views
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("DROP VIEWS %p%");
            QString sDropViews="BEGIN TRANSACTION;";
            query->clear();
            query->ExecShowErr("SELECT * FROM sqlite_master ORDER BY name;");
            while (query->next()) {
                if (query->value("type").toString()=="view"){
                    sDropViews=sDropViews + "DROP VIEW IF EXISTS \""+query->value("name").toString()+"\";";
                }
            }
            sDropViews+="COMMIT TRANSACTION;";
            bResult=query->ExecMultiShowErr(sDropViews,";",ui->progressBar);
            if (!bResult)
                sResult.append("Reset views : Err");
        }

        if (bResult){ //Close and reopen to avoid database lock.
            QString sDbFile=db.databaseName();
            dbClose();
            bResult=dbOpen(sDbFile,false,false);
            if (!bResult)
                sResult.append("Reopen : Err");
        }

        // Update shema (specific).
        if (bResult and(sDBVersion=="2024-12-30")) { //Useless specific update shema.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250120"),";",ui->progressBar,false);
            sResult.append(sDBVersion+" -> 2025-01-20 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-01-20";
        }
        if (bResult and(sDBVersion=="2025-01-20")) { //Adding tables: Destinations, Consommations.
            // ui->progressBar->setValue(0);
            // ui->progressBar->setMaximum(0);
            // ui->progressBar->setFormat("Specific update shema %p%");
            // bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250227"),";",ui->progressBar);
            sResult.append(sDBVersion+" -> 2025-02-27 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-02-27";
        }
        if (bResult and(sDBVersion=="2025-02-27")) { //Adding field: Culture.Récolte_comm.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250305"),";",ui->progressBar,false);
            sResult.append(sDBVersion+" -> 2025-03-05 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-03-05";
        }
        if (bResult and(sDBVersion=="2025-03-05")) { //Drop field: Culture.Récolte_comm.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250325"),";",ui->progressBar,false);
            sResult.append(sDBVersion+" -> 2025-03-25 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-03-25";
        }
        if (bResult and(sDBVersion=="2025-03-25")) { //Views and triggers update.
            sResult.append(sDBVersion+" -> 2025-04-04 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-04-04";
        }
        if (bResult and(sDBVersion=="2025-04-04")) { //Views and triggers update.
            sResult.append(sDBVersion+" -> 2025-04-12 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-04-12";
        }
        if (bResult and(sDBVersion=="2025-04-12")) { //Views update.
            sResult.append(sDBVersion+" -> 2025-05-13 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-05-13";
        }
        if (bResult and(sDBVersion=="2025-05-13")) { //Adding field: Espèces.N P K.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250514"),";",ui->progressBar,false);
            sResult.append(sDBVersion+" -> 2025-05-14 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-05-14";
        }
        if (bResult and(sDBVersion=="2025-05-14")) { //Views update.
            sResult.append(sDBVersion+" -> 2025-06-08 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-06-08";
        }
        if (bResult and(sDBVersion=="2025-06-08")) { //Adding field: Espèces.Vivace.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250622"),";",ui->progressBar,false);
            sResult.append(sDBVersion+" -> 2025-06-22 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-06-22";
        }
        if (bResult and(sDBVersion=="2025-06-22")) { //Adding field: S_xxx,D_récolte,Décal_max
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250728"),";",ui->progressBar,false);
            sResult.append(sDBVersion+" -> 2025-07-28 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-07-28";
        }
        if (bResult and(sDBVersion=="2025-07-28")) { //Adding field: Rotations_détails.Décalage. Adding associations.
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250925"),";",ui->progressBar,false);
            //bUpdateBaseData=true;
            sResult.append(sDBVersion+" -> 2025-09-25 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-09-25";
        }

        if (bResult) { //Update schema (general).
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Update shema %p%");
            QString sUpdateSchema="BEGIN TRANSACTION;";
            //SQL statements for renaming old tables.
            query->clear();
            query->ExecShowErr("PRAGMA table_list;");
            while (query->next()) {
                if (query->value("type").toString()=="table" and
                    query->value("name").toString()!="Params" and
                    query->value("name").toString()!="fda_schema" and
                    !query->value("name").toString().startsWith("sqlite")) {//No sqlite tables.
                    if (query->value("name").toString().startsWith("Temp_"))
                        sUpdateSchema +="DROP TABLE IF EXISTS "+query->value("name").toString()+";";
                    else
                        sUpdateSchema +="DROP TABLE IF EXISTS Temp_"+query->value("name").toString()+";"
                                        "CREATE TABLE Temp_"+query->value("name").toString()+" AS "
                                               "SELECT * FROM "+query->value("name").toString()+";"
                                        "DROP TABLE "+query->value("name").toString()+";";
                }
            }

            //SQL statements for creating new tables.
            sUpdateSchema += DynDDL(loadSQLFromResource("CreateTables"));

            //Execute SQL.
            bResult=query->ExecMultiShowErr(sUpdateSchema,";",ui->progressBar,true,true);

            if (bResult) { //Import data from old to new tables.
                sUpdateSchema="";
                ui->progressBar->setValue(0);
                ui->progressBar->setMaximum(0);
                ui->progressBar->setFormat("Data transfert %p%");

                QString sFieldsList;
                query->clear();
                query->ExecShowErr("PRAGMA table_list;");
                while (query->next()) { //SQL statements for importing data from old to new tables.
                    if (query->value("type").toString()=="table" and
                        !query->value("name").toString().startsWith("Temp_") and
                        query->value("name").toString()!="Params" and
                        !query->value("name").toString().startsWith("sqlite")) {
                        sFieldsList="";
                        qNewTableFields->clear();
                        qNewTableFields->ExecShowErr("PRAGMA table_xinfo("+query->value("name").toString()+");");//New table
                        while (qNewTableFields->next()) {
                            if (qNewTableFields->value("hidden").toInt()==0) {
                                qOldTableFields->ExecShowErr("PRAGMA table_xinfo(Temp_"+query->value("name").toString()+");");//Old table
                                // qDebug() << query->value("name").toString() << " - " << qNewTableFields->value("name").toString();
                                while (qOldTableFields->next()) {
                                    // qDebug() << qOldTableFields->value("name").toString();
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

                query->clear();
                query->ExecShowErr("PRAGMA table_list;");
                while (query->next()) { //SQL statements for DROPing Temp tables.
                    if (query->value("type").toString()=="table" and
                        query->value("name").toString()!="Params" and
                        !query->value("name").toString().startsWith("sqlite"))
                        sUpdateSchema +="DROP TABLE IF EXISTS Temp_"+query->value("name").toString()+";";
                }

                //Execute SQL.
                sUpdateSchema += "COMMIT TRANSACTION;";
                bResult=query->ExecMultiShowErr(sUpdateSchema,";",ui->progressBar);

                if (bResult and sDBVersion=="?")
                    sDBVersion=DbVersion;
            }
            sResult.append("Data transfert : "+iif(bResult,"ok","Err").toString()+"\n");
        }
    }

    if (bResult and(sDBVersion==DbVersion)) { //Tables shema ok, create views, triggers and update params table..
        if (bResult){ //Create views
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Views %p%");
            //Update View fields info with same field info in the table.
            QString addSchema="UPDATE fda_schema SET "
                                "type=(SELECT A1.type FROM fda_schema A1 "
                                      "WHERE (A1.tbl_type='Table')AND"
                                            "(A1.field_name=fda_schema.field_name)AND"
                                            "(fda_schema.name LIKE A1.name||'__%')) "
                              "WHERE (type ISNULL)AND(tbl_type='View as table')AND(field_name NOTNULL);"
                              "UPDATE fda_schema SET "
                                "description=(SELECT A1.description FROM fda_schema A1 "
                                             "WHERE (A1.tbl_type='Table')AND"
                                                   "(A1.field_name=fda_schema.field_name)AND"
                                                   "(fda_schema.name LIKE A1.name||'__%')) "
                              "WHERE (description ISNULL)AND(tbl_type LIKE 'View%')AND(field_name NOTNULL);" //todo
                              ;
            bResult=query->ExecMultiShowErr(DynDDL(loadSQLFromResource("CreateViews"))+addSchema,";",ui->progressBar,true,true);
            sResult.append("Views : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){ //Create triggers
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Triggers %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("CreateTriggers"),";;",ui->progressBar);//";" exists in CREATE TRIGGER statments
            sResult.append("Triggers : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){ //Update params table
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(0);
            ui->progressBar->setFormat("Params %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateTableParams"),";",ui->progressBar,false);
            sResult.append("Params : "+iif(bResult,"ok","Err").toString()+"\n");
        }
    }

    // if (bResult and bInsertBaseData) { //Insert base data.
    //     ui->progressBar->setValue(0);
    //     ui->progressBar->setMaximum(0);
    //     ui->progressBar->setFormat("Insert base data %p%");
    //     bResult=query->ExecMultiShowErr(sInsertBaseData,";",ui->progressBar);
    // }

    // if (bResult and bUpdateBaseData) { //Update base data.
    //     ui->progressBar->setValue(0);
    //     ui->progressBar->setMaximum(0);
    //     ui->progressBar->setFormat("Insert base data %p%");
    //     bResult=query->ExecMultiShowErr(sUpdateBaseData,";",ui->progressBar);
    // }

    if (bResult){ //Set FK ON.
        bResult=query->ExecShowErr("PRAGMA foreign_keys=ON");
        if (!bResult)
            sResult.append("Foreign keys : Err\n");
    }
    if (bResult){ //Set normal access.
        bResult=query->ExecShowErr("PRAGMA locking_mode=NORMAL;");
        if (!bResult)
            sResult.append(tr("Impossible d'annuler l'accès exclusif.")+"\n");
    }

    AppBusy(false,ui->progressBar);

    if (bResult) { //Final user dialog.
        if (sDBVersion==DbVersion) {
            #ifdef QT_NO_DEBUG
            if (!bNew)
                MessageDlg("Potaléger "+ui->lVer->text(),tr("Mise à jour réussie."),
                           sResult,QStyle::SP_MessageBoxInformation);
            #endif
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
