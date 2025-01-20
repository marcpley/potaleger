#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include "SQL/UpdateStru2024-12-30_2025-01-xx.sql"
#include "SQL/CreateTables.sql"
#include "SQL/CreateViews.sql"
#include "SQL/CreateBaseData.sql"
#include "SQL/CreateTriggers.sql"
#include "PotaUtils.h"
#include "data/Data.h"

bool MainWindow::UpdateDBShema(QString sDBVersion)
{
    //QSqlDatabase db = QSqlDatabase::database();
    PotaQuery *model = new PotaQuery;
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
            sResult.append("Create tables : ok\n");
            if (sDBVersion == "NewWithBaseData")
            {
                if (!model->ExecMultiShowErr(DynDDL(sSQLBaseData),";"))
                {
                    ui->tbInfoDB->append(tr("Echec de la création des données de base")+" ("+ui->lVerBDDAttendue->text()+")");
                    return false;
                }
                sResult.append("Create base data : ok\n");
            }
            sDBVersion = "2024-12-30";
        }
        else
        {
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
        dbOpen(sDbFile,false,true);
        dbClose();
        dbOpen(sDbFile,false,false);
    }


    if (bResult and(sDBVersion == "2024-12-xx")) { //Update shema.
        bResult = model->ExecMultiShowErr(sDDL202501xx,";");
        sResult.append(sDBVersion+" -> 2025-01-xx : "+iif(bResult,"ok","Err").toString()+"\n");
        if (bResult) sDBVersion = "2025-01-xx";
    }
    // if (bResult and(sDBVersion == "2024-12-xx")){ //Update shema.
    //     bResult = model->ExecMultiShowErr(sDDL20241230,";");
    //     sResult.append(sDBVersion+" -> 2024-12-30 : "+iif(bResult,"ok","Err").toString()+"\n");
    //     if (bResult) sDBVersion = "2024-12-30";
    // }

    if (bResult and(sDBVersion == ui->lVerBDDAttendue->text())) { //Tables shema ok.

        if (bResult){
            //Create scalar functions
            bResult = registerScalarFunctions();
            sResult.append(sDBVersion+" -> Scalar functions : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){
            //Create views
            bResult = model->ExecMultiShowErr(sDDLViews,";");
            sResult.append(sDBVersion+" -> views : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){
            //Create table valued functions
            bResult = registerTableValuedFunctions();
            sResult.append(sDBVersion+" -> Table valued functions : "+iif(bResult,"ok","Err").toString()+"\n");
        }

        if (bResult){
            //Test functions
            QString sErrFunc = testCustomFunctions();
            bResult = (sErrFunc=="");
            sResult.append(sDBVersion+" -> Function test : "+iif(bResult,"ok","Err ("+sErrFunc+")").toString()+"\n");
        }

        if (bResult){
            //Create triggers
            bResult = model->ExecMultiShowErr(sDDLTriggers,";;");//";" exists in CREATE TRIGGER statments
            sResult.append(sDBVersion+" -> triggers : "+iif(bResult,"ok","Err").toString()+"\n");
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
                MessageDialog(tr("Mise à jour réussie.")+"\n\n"+
                                  sResult,QMessageBox::Information);
            return true;
        }
        else {
            ui->tbInfoDB->append(tr("Version de BDD inconnue:")+sDBVersion);
            return false;
        }
    }
    else
    {
        MessageDialog(tr("Echec de la mise à jour.")+"\n\n"+
                          sResult,QMessageBox::Critical);
        ui->tbInfoDB->append(tr("Echec de la mise à jour de la version de la BDD ")+" ("+sDBVersion+" > "+ui->lVerBDDAttendue->text()+")");
        return false;
    }
}
