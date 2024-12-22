#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include "SQL/MajStru2024-12-16_2024-12-17.sql"

bool MainWindow::MaJStruBDD(QString sVersionBDD)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQueryModel model;
    db.transaction();
    if (sVersionBDD == "2024-12-16")
    {
        ExecSql(model,"DROP VIEW IF EXISTS Info_Potaléger");
        if (!ExecSql(model,sDDL20241217))
        {
            ui->tbInfoDB->append(tr("Echec de la mise à jour de la structure de la base de données ")+sVersionBDD+" > "+ui->lVerBDDAttendue->text());
            db.rollback();
            return false;
        }
    }
    else {
        ui->tbInfoDB->append(tr("Version de base de données inconnue:")+sVersionBDD);
        return false;
    }
    db.commit();
    return true;
}
