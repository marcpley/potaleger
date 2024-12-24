#include "mainwindow.h"
#include "potawidget.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlQueryModel>

bool MainWindow::InfosBDD()
{
    PotaQueryModel model;
    model.lErr = ui->lDBErr;
    if (model.setQueryShowErr("SELECT * FROM Info_Potaléger"))
    {
        ui->tbInfoDB->clear();
        for (int i = 0; i < 10; i++)
            ui->tbInfoDB->append(model.data(model.index(i,1)).toString()+": "+
                                 model.data(model.index(i,2)).toString());
        return true;
    }
    else
    {
        ui->tbInfoDB->append(tr("Impossible de lire la vue 'Info_Potaléger'."));
        return false;
    }
}
