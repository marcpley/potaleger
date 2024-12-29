#include "qcolor.h"
#include <QtSql/QSqlQueryModel>

QColor TableColor(QString sTFName)
{
    sTFName=sTFName+"                          ";//to avoid first out of size.
    if (sTFName.toUpper().first(7)=="CULTURE")//Vert
        return QColor("#00ff00");
    else if (sTFName.toUpper().first(6)=="ESPÈCE")//Bleu
        return QColor("#002bff");
    else if (sTFName.toUpper().first(3)=="ITP")//Rouge
        return QColor("#ff0000");
    else if (sTFName.toUpper().first(7)=="PLANCHE")//Orange
        return QColor("#ff8100");
    else if (sTFName.toUpper().first(7)=="VARIÉTÉ")//Jaune
        return QColor("#b7b202");
    else
        return QColor();


    // if (TableName=="Apports")
    //     return QColor("#799e26");
    // //else if (TableName=="Cultures")//Pas de couleur unique, couleur en fonction des données.
    // //    return QColor("#00ff00");
    // else if (TableName=="Espèces")//Vert
    //     return QColor("#00ff00");
    // else if (TableName=="Familles")//Bleu
    //     return QColor("#002bff");
    // else if (TableName=="Fournisseurs")
    //     return QColor("#799e26");
    // else if (TableName=="ITP")//Rouge
    //     return QColor("#ff0000");
    // else if (TableName=="Niveaux")
    //     return QColor("#799e26");
    // else if (TableName=="Planches")//Orange
    //     return QColor("#ff8100");
    // else if (TableName=="Rotations")
    //     return QColor("#799e26");
    // else if (TableName=="Rotations_détails")
    //     return QColor("#799e26");
    // else if (TableName=="Récoltes")//Violet
    //     return QColor("#ec00ff");
    // else if (TableName=="Types_planche")
    //     return QColor("#799e26");
    // else if (TableName=="Variétés")//Jaune
    //     return QColor("#b7b202");
    // else
    //     return QColor();
}
