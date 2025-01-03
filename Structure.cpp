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

    if (sDBVersion == "New" or sDBVersion == "NewWithBaseData")
    {
        //bNew=true; todo mettre à false quand debug fait.
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
    }

    //Updating an existing db.
    if (bResult and(sDBVersion == "2024-12-xx"))
    {
        bResult = model->ExecMultiShowErr(sDDL202501xx,";");
        sResult.append(sDBVersion+" -> 2025-01-xx : "+iif(bResult,"ok","Err").toString()+"\n");
        if (bResult) sDBVersion = "2025-01-xx";
    }
    // if (bResult and(sDBVersion == "2024-12-xx"))
    // {
    //     bResult = model->ExecMultiShowErr(sDDL20241230,";");
    //     sResult.append(sDBVersion+" -> 2024-12-30 : "+iif(bResult,"ok","Err").toString()+"\n");
    //     if (bResult) sDBVersion = "2024-12-30";
    // }

    if (bResult and(sDBVersion == ui->lVerBDDAttendue->text()))
    {
        //DROP all views
        QString sDropViews="";
        model->ExecShowErr("PRAGMA table_list;");
        while (model->next()) {
            if (model->value("type").toString()=="view")
                sDropViews = sDropViews + "DROP VIEW IF EXISTS \""+model->value("name").toString()+"\";";
        }
        model->ExecMultiShowErr(sDropViews,";");

        //Create up to date views
        bResult = model->ExecMultiShowErr(sDDLViews,";");
        sResult.append(sDBVersion+" -> views : "+iif(bResult,"ok","Err").toString()+"\n");
    }

    if (bResult and(sDBVersion == ui->lVerBDDAttendue->text()))
    {
        //Create up to date triggers
        bResult = model->ExecMultiShowErr(sDDLTriggers,";;");//";" exists in CREATE TRIGGER statments
        sResult.append(sDBVersion+" -> triggers : "+iif(bResult,"ok","Err").toString()+"\n");
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
