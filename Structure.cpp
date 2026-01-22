#include "Dialogs.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include "FdaUtils.h"
#include "data/FdaCalls.h"

bool MainWindow::UpdateDBShema(QString sDBVersion)
{
    //QString modelPath="/home/marc/Bureau/greli.net/potaleger/model"; //todo
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

    QProgressBar *pb;
    pb=ui->progressBar;

    AppBusy(true,pb,0,0,"%p%");

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

        if (bResult){ //Create fada tables
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Fada schema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("CreateFadaTables"),";",pb, true, true);
            sResult.append("Init Fada schema : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){ //Create empty tables.
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("CREATE TABLES %p%");
            bResult=query->ExecMultiShowErr(DynDDL(loadSQLFromResource("CreateTables")),";",pb,true,true);
            sResult.append("Create tables (").append(DbVersion).append(") : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult and(sDBVersion=="NewWithBaseData")) { //Insert general base data.
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("BASE DATA %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("CreateBaseData"),";",pb); //keepReturns true
            sResult.append("Base data : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult) {
            //bInsertBaseData=(sDBVersion=="NewWithBaseData");
            sDBVersion=DbVersion;
        }

    } else { //Update existing db.
        if (bResult){ //DROP all trigers
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("DROP TRIGGER %p%");
            QString sDropTrigger=""; //"BEGIN TRANSACTION;";
            query->clear();
            query->ExecShowErr("SELECT * FROM sqlite_schema ORDER BY name;");
            while (query->next()) {
                if (query->value("type").toString()=="trigger"){
                    sDropTrigger=sDropTrigger + "DROP TRIGGER IF EXISTS \""+query->value("name").toString()+"\";";
                }
            }
            //sDropTrigger+="COMMIT TRANSACTION;";
            bResult=query->ExecMultiShowErr(sDropTrigger,";",pb);
            if (!bResult)
                sResult.append("Reset triggers : Err");
        }
        if (bResult){ //DROP all views
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("DROP VIEWS %p%");
            QString sDropViews="";//"BEGIN TRANSACTION;";
            query->clear();
            query->ExecShowErr("SELECT * FROM sqlite_schema ORDER BY name;");
            while (query->next()) {
                if (query->value("type").toString()=="view"){
                    sDropViews=sDropViews + "DROP VIEW IF EXISTS \""+query->value("name").toString()+"\";";
                }
            }
            //sDropViews+="COMMIT TRANSACTION;";
            bResult=query->ExecMultiShowErr(sDropViews,";",pb);
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
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250120"),";",pb,false);
            sResult.append(sDBVersion+" -> 2025-01-20 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-01-20";
        }
        if (bResult and(sDBVersion=="2025-01-20")) { //Adding tables: Destinations, Consommations.
            // pb->setValue(0);
            // pb->setMaximum(0);
            // pb->setFormat("Specific update shema %p%");
            // bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250227"),";",pb);
            sResult.append(sDBVersion+" -> 2025-02-27 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-02-27";
        }
        if (bResult and(sDBVersion=="2025-02-27")) { //Adding field: Culture.Récolte_comm.
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250305"),";",pb,false);
            sResult.append(sDBVersion+" -> 2025-03-05 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-03-05";
        }
        if (bResult and(sDBVersion=="2025-03-05")) { //Drop field: Culture.Récolte_comm.
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250325"),";",pb,false);
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
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250514"),";",pb,false);
            sResult.append(sDBVersion+" -> 2025-05-14 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-05-14";
        }
        if (bResult and(sDBVersion=="2025-05-14")) { //Views update.
            sResult.append(sDBVersion+" -> 2025-06-08 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-06-08";
        }
        if (bResult and(sDBVersion=="2025-06-08")) { //Adding field: Espèces.Vivace.
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250622"),";",pb,false);
            sResult.append(sDBVersion+" -> 2025-06-22 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-06-22";
        }
        if (bResult and(sDBVersion=="2025-06-22")) { //Adding field: S_xxx,D_récolte,Décal_max
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250728"),";",pb,false);
            sResult.append(sDBVersion+" -> 2025-07-28 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-07-28";
        }
        if (bResult and(sDBVersion=="2025-07-28")) { //Adding field: Rotations_détails.Décalage. Adding associations.
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20250925"),";",pb,false);
            //bUpdateBaseData=true;
            sResult.append(sDBVersion+" -> 2025-09-25 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-09-25";
        }
        if (bResult and(sDBVersion=="2025-09-25")) { //Adding field: Rotations.Active.
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Specific update shema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateStru20251128"),";",pb,false);
            sResult.append(sDBVersion+" -> 2025-11-28 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion="2025-11-28";
        }

        //Update schema (general).

        if (bResult){ //Create fada tables
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Fada schema %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("CreateFadaTables"),";",pb, true,true);
            //bResult=false;
            sResult.append("Init Fada schema : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult) { //Other tables.
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Update shema %p%");
            QString sUpdateSchema="BEGIN TRANSACTION;";
            //SQL statements for renaming old tables.
            query->clear();
            query->ExecShowErr("PRAGMA table_list;");
            while (query->next()) {
                if (query->value("type").toString()=="table" and
                    query->value("name").toString()!="Params" and
                    !query->value("name").toString().startsWith("fada_") and
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
            bResult=query->ExecMultiShowErr(sUpdateSchema,";",pb,true,true);

            if (bResult) { //Import data from old to new tables.
                sUpdateSchema="";
                pb->setValue(0);
                pb->setMaximum(0);
                pb->setFormat("Data transfert %p%");

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
                bResult=query->ExecMultiShowErr(sUpdateSchema,";",pb);

                if (bResult and sDBVersion=="?")
                    sDBVersion=DbVersion;
            }
            sResult.append("Data transfert : "+iif(bResult,"ok","Err").toString()+"\n");
        }
    }

    if (bResult and(sDBVersion==DbVersion)) { //Tables shema ok, create views, triggers and update params table..
        if (bResult){ //Create views
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Views %p%");
            bResult=query->ExecMultiShowErr(DynDDL(loadSQLFromResource("CreateViews")),";",pb,true,true);
            sResult.append("Views : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){ //Update params table
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Params %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateTableParams"),";",pb,false,true);
            sResult.append("Params : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){ //Create triggers
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Triggers %p%");
            bResult=query->ExecMultiShowErr(loadSQLFromResource("CreateTriggers"),";;",pb);//";" exists in CREATE TRIGGER statments
            sResult.append("Triggers : "+iif(bResult,"ok","Err").toString()+"\n");
        }
        if (bResult){ //Update fada schema model properties
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Fada model properties %p%");

            bResult=query->ExecMultiShowErr(loadSQLFromResource("UpdateFadaSchema")+
                                            loadSQLFromResource("UpdateFadaSchemaModel"),";",pb, true,false);
            sResult.append("Fada model properties : "+iif(bResult,"ok","Err").toString()+"\n");
        }
        if (bResult){ //Update fada table with SQLite info
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("SQLite info %p%");
            QString sAddSQLiteInfoInFadaSchema="";
            PotaQuery query2(db);

            query->ExecShowErr("SELECT tv_name,tv_type FROM fada_t_schema;");
            while (query->next()) {
                QString sTableName=query->value("tv_name").toString();
                QString sPK="";
                int fieldCount=0;

                query2.ExecShowErr("PRAGMA table_xinfo("+sTableName+")");
                while (query2.next()){
                    fieldCount+=1;
                    if (sPK.isEmpty() and query2.value(5).toInt()==1) {
                        sPK=query2.value(1).toString();
                    }
                }

                int FadaFieldCount=query2.Select0ShowErr("SELECT count() FROM fada_f_schema "
                                                        "WHERE (tv_name='"+sTableName+"')").toInt();

                int triggerCount=query2.Select0ShowErr("SELECT count() FROM sqlite_schema "
                                                      "WHERE (tbl_name='"+sTableName+"')AND"
                                                            "(type='trigger')").toInt();
                int useCount=query2.Select0ShowErr("SELECT count() FROM sqlite_schema "
                                                  "WHERE (tbl_name!='"+sTableName+"')AND "
                                                        "NOT(tbl_name LIKE 'Temp_%')AND"
                                                        "((sql LIKE '% "+sTableName+" %')OR"
                                                         "(sql LIKE '% "+sTableName+")%')OR"
                                                         "(sql LIKE '% "+sTableName+"'))").toInt();
                sAddSQLiteInfoInFadaSchema+="UPDATE fada_t_schema SET "
                         "PK_field_name="+iif(sPK.isEmpty(),"NULL","'"+sPK+"'").toString()+","+
                         "SQLite_field_count="+str(fieldCount)+","+
                         "FDA_field_count="+str(FadaFieldCount)+","+
                         "Trigger_count="+str(triggerCount)+","+
                         "Internal_use_count="+str(useCount)+","+
                         "Menu_use_count="+str(0)+","+
                         "Total_use_count="+str(useCount)+","+
                         "Rec_count="+str(0)+" "+
                         "WHERE tv_name='"+sTableName+"';";
            }




            bResult=query->ExecMultiShowErr(sAddSQLiteInfoInFadaSchema,";",pb, true,false);
            sResult.append("SQLite info : "+iif(bResult,"ok","Err").toString()+"\n");
        }
        if (bResult){ //Create launchers
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Fada launchers %p%");

            bResult=query->ExecMultiShowErr(loadSQLFromResource("CreateLaunchers")+
                                            loadSQLFromResource("UpdateLaunchers"),";",pb,true,false);
            sResult.append("Fada launchers : "+iif(bResult,"ok","Err").toString()+"\n");
        }
        if (bResult){ //Create scripts
            pb->setValue(0);
            pb->setMaximum(0);
            pb->setFormat("Fada scripts %p%");

            QDir dir(":/model");
            dir.setFilter(QDir::Files | QDir::NoSymLinks);
            QStringList fileList = dir.entryList();
            QString sInsertScripts="";
            for (int i=0;i<fileList.count();i++) {
                if (fileList[i].toUpper().endsWith(".SQL")) {
                    QFile scriptFile(dir.absolutePath()+"/"+fileList[i]);
                    if (scriptFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        QString script_name=fileList[i].removeLast().removeLast().removeLast().removeLast();
                        QTextStream in(&scriptFile);
                        QString script=in.readAll();
                        scriptFile.close();
                        sInsertScripts+="INSERT INTO Fada_scripts (script_name,script,created) "
                                        "VALUES ('"+script_name+"',"+
                                        EscapeSQL(script)+",CURRENT_TIMESTAMP);;\n";
                    }

                }
            }

            qDebug() << sInsertScripts;
            bResult=query->ExecMultiShowErr(sInsertScripts,";;",pb,true,false);
            sResult.append("Fada scripts : "+iif(bResult,"ok","Err").toString()+"\n");
        }
    }

    // if (bResult and bInsertBaseData) { //Insert base data.
    //     pb->setValue(0);
    //     pb->setMaximum(0);
    //     pb->setFormat("Insert base data %p%");
    //     bResult=query->ExecMultiShowErr(sInsertBaseData,";",pb);
    // }

    // if (bResult and bUpdateBaseData) { //Update base data.
    //     pb->setValue(0);
    //     pb->setMaximum(0);
    //     pb->setFormat("Insert base data %p%");
    //     bResult=query->ExecMultiShowErr(sUpdateBaseData,";",pb);
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

    AppBusy(false,pb);

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
