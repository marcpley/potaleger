#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include "SQL/MajStru2024-12-16_2024-12-17.sql"
#include "SQL/Stru2024-12-17.sql"
#include "potawidget.h"

bool MainWindow::MaJStruBDD(QString sVersionBDD)
{
    QSqlDatabase db = QSqlDatabase::database();
    PotaQueryModel model;
    model.lErr = ui->lDBErr;
    if (sVersionBDD == "2024-12-16")
    {
        if (!model.setQueryShowErr(sDDL20241217))
        {
            ui->tbInfoDB->append(tr("Echec de la mise à jour de la version de la BDD ")+" ("+sVersionBDD+" > "+ui->lVerBDDAttendue->text()+")");
            db.rollback();
            return false;
        }
    }
    else if (sVersionBDD == "Nouvelle vide")
    {
        if (!model.setMultiQueryShowErr(sDDLNouvelle))
        {
            ui->tbInfoDB->append(tr("Echec de la création de la BDD vide")+" ("+ui->lVerBDDAttendue->text()+")");
            db.rollback();
            return false;
        }
    }
    else {
        ui->tbInfoDB->append(tr("Version de BDD inconnue:")+sVersionBDD);
        return false;
    }
    return true;
}
