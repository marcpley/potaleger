#include "PotaUtils.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlQueryModel>

bool MainWindow::InfosBDD()
{
    PotaQuery pQuery;
    pQuery.lErr = ui->lDBErr;
    if (pQuery.ExecShowErr("SELECT * FROM Info_Potaléger"))
    {
        ui->tbInfoDB->clear();
        while (pQuery.next())
            ui->tbInfoDB->append(pQuery.value(1).toString()+": "+
                                 pQuery.value(2).toString());
        return true;
    }
    else
    {
        ui->tbInfoDB->append(tr("Impossible de lire la vue 'Info_Potaléger'."));
        return false;
    }
}

QColor CouleurTable(QString TableName)
{
    if (TableName=="Apports")
        return QColor("#799e26");
    //else if (TableName=="Cultures")//Pas de couleur unique, couleur en fonction des données.
    //    return QColor("#00ff00");
    else if (TableName=="Espèces")//Vert
        return QColor("#00ff00");
    else if (TableName=="Familles")//Bleu
        return QColor("#002bff");
    else if (TableName=="Fournisseurs")
        return QColor("#799e26");
    else if (TableName=="ITP")//Rouge
        return QColor("#ff0000");
    else if (TableName=="Niveaux")
        return QColor("#799e26");
    else if (TableName=="Planches")//Orange
        return QColor("#ff8100");
    else if (TableName=="Rotations")
        return QColor("#799e26");
    else if (TableName=="Rotations_détails")
        return QColor("#799e26");
    else if (TableName=="Récoltes")//Violet
        return QColor("#ec00ff");
    else if (TableName=="Types_planche")
        return QColor("#799e26");
    else if (TableName=="Variétés")//Jaune
        return QColor("#b7b202");
    else
        return QColor();
}
