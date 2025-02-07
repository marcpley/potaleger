#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include "SQL/UpdateStru2024-12-30_2025-01-20.sql"
#include "SQL/CreateTables.sql"
#include "SQL/CreateViews.sql"
#include "SQL/CreateBaseData.sql"
#include "SQL/CreateTriggers.sql"
#include "SQL/UpdateTableParams.sql"
#include "PotaUtils.h"
#include "data/Data.h"

bool MainWindow::UpdateDBShema(QString sDBVersion)
{
    //QSqlDatabase db = QSqlDatabase::database();
    PotaQuery *model = new PotaQuery;
    PotaQuery *model2 = new PotaQuery;
    PotaQuery *model3 = new PotaQuery;
    model->lErr = ui->lDBErr;
    bool bNew=false;
    bool bResult=true;
    QString sResult="";

    if (!model->ExecShowErr("PRAGMA foreign_keys = OFF")) {
        ui->tbInfoDB->append(tr("Impossible de désactiver les clés étrangères."));
        return false;
    }

    if (sDBVersion == "New" or sDBVersion == "NewWithBaseData")
    {
        //bNew=true; mettre à false quand debug fait.
        if (model->ExecMultiShowErr(DynDDL(sDDLTables),";"))
        {
            sResult.append("Create tables : ok (").append(DbVersion).append(")\n");
            if (sDBVersion == "NewWithBaseData") {
                if (!model->ExecMultiShowErr(sSQLBaseData,";"))
                {
                    ui->tbInfoDB->append(tr("Echec de la création des données de base")+" ("+ui->lVerBDDAttendue->text()+")");
                    return false;
                }
                sResult.append("Create base data : ok\n");
            }
            sDBVersion = DbVersion;//"2024-12-30";
        } else {
            ui->tbInfoDB->append(tr("Echec de la création de la BDD vide")+" ("+ui->lVerBDDAttendue->text()+")");
            return false;
        }
    } else {    //Updating an existing db.
        //DROP all views
        QString sDropViews="";
        model->clear();
        model->ExecShowErr("PRAGMA table_list;");
        while (model->next()) {
            if (model->value("type").toString()=="view")
                sDropViews = sDropViews + "DROP VIEW IF EXISTS \""+model->value("name").toString()+"\";";
        }
        model->ExecMultiShowErr(sDropViews,";");

        //DROP SQLean functions
        QSqlDatabase db = QSqlDatabase::database();
        QString sDbFile=db.databaseName();
        dbClose();
        dbOpen(sDbFile,false,true,false);
        dbClose();
        dbOpen(sDbFile,false,false,false);

        if (bResult and(sDBVersion == "2024-12-30")) { //Spécific update shema.
            bResult = model->ExecMultiShowErr(sDDL20250120,";");
            sResult.append(sDBVersion+" -> 2025-01-20 : "+iif(bResult,"ok","Err").toString()+"\n");
            if (bResult) sDBVersion = "2025-01-20";
        }

        if (bResult) { //Update schema.
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

            bResult = model->ExecMultiShowErr(sUpdateSchema,";");
            if (bResult) {
                sUpdateSchema="";
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
                bResult = model->ExecMultiShowErr(sUpdateSchema,";");
            }
            sResult.append("Data transfert : "+iif(bResult,"ok","Err").toString()+"\n");
        }
    }

    if (bResult and(sDBVersion == ui->lVerBDDAttendue->text())) { //Tables shema ok.
        //if (sResult=="")
        //    sResult.append("Version : "+sDBVersion+"\n");

        if (bResult){
            //Create scalar functions
            bResult = registerScalarFunctions();
            sResult.append("Scalar functions : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){
            //Create views
            bResult = model->ExecMultiShowErr(DynDDL(sDDLViews),";");
            sResult.append("Views : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){
            //Create table valued functions
            bResult = registerTableValuedFunctions();
            sResult.append("Table valued functions : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){
            //Test functions
            QString sErrFunc = testCustomFunctions();
            bResult = (sErrFunc=="");
            sResult.append("Function test : "+iif(bResult,"ok","Err ("+sErrFunc+")").toString()+"\n");
        }

        if (bResult){
            //Create triggers
            bResult = model->ExecMultiShowErr(sDDLTriggers,";;");//";" exists in CREATE TRIGGER statments
            sResult.append("Triggers : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){
            //Update params table
            bResult = model->ExecMultiShowErr(sDDLTableParams,";");
            sResult.append("Params : "+iif(bResult,"ok","Err").toString()+"\n");
        }
    }


    if (bResult and !model->ExecShowErr("PRAGMA foreign_keys = ON")) {
        ui->tbInfoDB->append(tr("Impossible d'activer les clés étrangères."));
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
