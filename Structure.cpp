#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include "SQL/MajStru2024-12-16_2024-12-17.sql"
#include "potawidget.h"

bool MainWindow::MaJStruBDD(QString sVersionBDD)
{
    QSqlDatabase db = QSqlDatabase::database();
    PotaQueryModel model;
    db.transaction();
    if (sVersionBDD == "2024-12-16")
    {
        if (!model.setQueryShowErr("DROP VIEW IF EXISTS Info_Potaléger") or
            !model.setQueryShowErr(sDDL20241217))
        {
            ui->tbInfoDB->append(tr("Echec de la mise à jour de la version de la BDD ")+" ("+sVersionBDD+" > "+ui->lVerBDDAttendue->text()+")");
            db.rollback();
            return false;
        }
    }
    else {
        ui->tbInfoDB->append(tr("Version de BDD inconnue:")+sVersionBDD);
        return false;
    }
    db.commit();
    return true;
}
