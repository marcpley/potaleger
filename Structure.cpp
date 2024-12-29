#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include "SQL/UpdateStru2024-12-16_2024-12-17.sql"
#include "SQL/UpdateStru2024-12-17_2024-12-27.sql"
#include "SQL/Stru2024-12-17.sql"
#include "potawidget.h"

bool MainWindow::MaJStruBDD(QString sVersionBDD)
{
    QSqlDatabase db = QSqlDatabase::database();
    PotaQueryModel *model = new PotaQueryModel;
    model->lErr = ui->lDBErr;
    if (sVersionBDD == "2024-12-16")
    {
        if ((model->setMultiQueryShowErr(sDDL20241217))and
            (model->setMultiQueryShowErr(sDDL20241227)))
        {
            MessageDialog(tr("Mise à jour réussie.")+"\n\n"+
                              sVersionBDD+" -> "+ui->lVerBDDAttendue->text(),QMessageBox::Information);
        }
        else
        {
            ui->tbInfoDB->append(tr("Echec de la mise à jour de la version de la BDD ")+" ("+sVersionBDD+" > "+ui->lVerBDDAttendue->text()+")");
            db.rollback();
            return false;
        }
    }
    else if (sVersionBDD == "2024-12-17")
    {
        if (model->setMultiQueryShowErr(sDDL20241227))
        {
            MessageDialog(tr("Mise à jour réussie.")+"\n\n"+
                              sVersionBDD+" -> "+ui->lVerBDDAttendue->text(),QMessageBox::Information);
        }
        else
        {
            ui->tbInfoDB->append(tr("Echec de la mise à jour de la version de la BDD ")+" ("+sVersionBDD+" > "+ui->lVerBDDAttendue->text()+")");
            db.rollback();
            return false;
        }
    }
    else if (sVersionBDD == "Nouvelle vide")
    {
        if (!model->setMultiQueryShowErr(sDDLNouvelle))
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
