#include "data/Data.h"
#include "PotaUtils.h"
#include "qcolor.h"
#include <QSqlTableModel>
#include <QtSql/QSqlQueryModel>
#include <QObject>

QString DynDDL(QString sQuery)
{
    sQuery=StrReplace(sQuery,"#FmtPlanif#","IN('','01-01','01-15','02-01','02-15','03-01','03-15','04-01','04-15','05-01','05-15','06-01','06-15','07-01','07-15','08-01','08-15','09-01','09-15','10-01','10-15','11-01','11-15','12-01','12-15')");
    return sQuery;
}

QString GeneratedFielnameForDummyFilter(const QString sTableName) {
    if (sTableName=="Cultures")
        return "Etat";
    else if (sTableName=="ITP")
        return "Type_culture";
    else
        return "";
}

bool ReadOnly(const QString sTableName,const QString sFieldName)
{
    bool bReadOnly=false;

    //Tables: explicit readonly
    if (sTableName=="Cultures"){
        bReadOnly = (sFieldName=="Culture" or sFieldName=="Type" or sFieldName=="Etat");

    } else if (sTableName=="ITP"){
        bReadOnly = (sFieldName=="Type_culture");

    } else if (sTableName=="Params"){
        bReadOnly = (sFieldName!="Valeur");

    //Views: explicit editable.
    } else if (sTableName=="Cultures__non_terminées"){
        bReadOnly = !(sFieldName=="Planche" or
                      sFieldName=="IT_plante" or
                      sFieldName=="Variété" or
                      sFieldName=="Fournisseur" or
                      sFieldName=="D_planif" or
                      sFieldName=="Date_semis" or
                      sFieldName=="Semis_fait" or
                      sFieldName=="Date_plantation" or
                      sFieldName=="Plantation_faite" or
                      sFieldName=="Début_récolte" or
                      sFieldName=="Fin_récolte" or
                      sFieldName=="Récolte_faite" or
                      sFieldName=="Terminée" or
                      sFieldName=="Longueur" or
                      sFieldName=="Nb_rangs" or
                      sFieldName=="Espacement" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__Semis_à_faire"){
        bReadOnly = !(sFieldName=="Planche" or
                      sFieldName=="Variété" or
                      sFieldName=="Fournisseur" or
                      sFieldName=="D_planif" or
                      sFieldName=="Date_semis" or
                      sFieldName=="Semis_fait" or
                      sFieldName=="Longueur" or
                      sFieldName=="Nb_rangs" or
                      sFieldName=="Espacement" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__Plantations_à_faire"){
        bReadOnly = !(sFieldName=="Planche" or
                      sFieldName=="Variété" or
                      sFieldName=="Fournisseur" or
                      sFieldName=="D_planif" or
                      sFieldName=="Date_semis" or
                      sFieldName=="Semis_fait" or
                      sFieldName=="Date_plantation" or
                      sFieldName=="Plantation_faite" or
                      sFieldName=="Longueur" or
                      sFieldName=="Nb_rangs" or
                      sFieldName=="Espacement" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__Récoltes_à_faire"){
        bReadOnly = !(sFieldName=="Date_semis" or
                      sFieldName=="Semis_fait" or
                      sFieldName=="Date_plantation" or
                      sFieldName=="Plantation_faite" or
                      sFieldName=="Début_récolte" or
                      sFieldName=="Fin_récolte" or
                      sFieldName=="Récolte_faite" or
                      sFieldName=="Terminée" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__à_terminer"){
        bReadOnly = !(sFieldName=="Date_semis" or
                      sFieldName=="Date_plantation" or
                      sFieldName=="Début_récolte" or
                      sFieldName=="Fin_récolte" or
                      sFieldName=="Récolte_faite" or
                      sFieldName=="Terminée" or
                      sFieldName=="Notes");
    } else if (sTableName=="ITP__Tempo"){
        bReadOnly = !(sFieldName=="IT_plante" or
                      sFieldName=="Espèce" or
                      sFieldName=="Type_planche" or
                      sFieldName=="Déb_semis" or
                      sFieldName=="Fin_semis" or
                      sFieldName=="Déb_plantation" or
                      sFieldName=="Fin_plantation" or
                      sFieldName=="Déb_récolte" or
                      sFieldName=="Fin_récolte" or
                      sFieldName=="Nb_rangs" or
                      sFieldName=="Espacement" or
                      sFieldName=="Nb_graines_trou" or
                      sFieldName=="Dose_semis" or
                      sFieldName=="Notes" or
                      sFieldName=="N_espèce");

    } else if (sTableName=="Rotations_détails__Tempo"){
        bReadOnly = !(sFieldName=="Rotation" or
                      sFieldName=="Année" or
                      sFieldName=="IT_plante" or
                      sFieldName=="Pc_planches" or
                      sFieldName=="Fi_planches" or
                      sFieldName=="Notes");

    } else if (sTableName=="Variétés__inv_et_cde"){
        bReadOnly = !(sFieldName=="Qté_stock" or
                      sFieldName=="Qté_cde" or
                      sFieldName=="Fournisseur" or
                      sFieldName=="Nb_graines_g" or
                      sFieldName=="FG" or
                      sFieldName=="Notes" or
                      sFieldName=="N_espèce" or
                      sFieldName=="N_famille");

    } else {
        QSqlQuery query("PRAGMA table_list("+sTableName+")");
        query.next();
        bReadOnly=(query.value(2).toString()!="table");
    }

    return bReadOnly;
}

QColor RowColor(QString sValue){

    QColor c;

    if (sValue=="Prévue")
        c=cPrevue;
    else if (sValue=="Sous abris")
        c=cSousAbris;
    else if (sValue=="En place")
        c=cEnPlace;
    else if (sValue=="A terminer")
        c=cATerminer;
    else if (sValue=="Terminée")
        c=cTerminee;
    else
        return QColor();
    c.setAlpha(60);
    return c;
}

QString RowSummary(QString TableName,const QModelIndex &index){
    int col=0;
    if (TableName=="Cult_planif")
        return index.siblingAtColumn(0).data(Qt::DisplayRole).toString()+" - "+
               index.siblingAtColumn(3).data(Qt::DisplayRole).toString();
    else if (TableName=="Cultures")
        return index.siblingAtColumn(0).data(Qt::DisplayRole).toString()+" - "+
               index.siblingAtColumn(1).data(Qt::DisplayRole).toString()+" - "+
               index.siblingAtColumn(4).data(Qt::DisplayRole).toString();
    else if (TableName.startsWith("Cultures__"))
        return index.siblingAtColumn(1).data(Qt::DisplayRole).toString()+" - "+
               index.siblingAtColumn(0).data(Qt::DisplayRole).toString()+" - "+
               index.siblingAtColumn(2).data(Qt::DisplayRole).toString();
    else if (TableName=="IT_rotations")
        col=1;
    else if (TableName=="IT_rotations_ilots")
        return index.siblingAtColumn(1).data(Qt::DisplayRole).toString()+" - "+
               QObject::tr("ilot ")+index.siblingAtColumn(2).data(Qt::DisplayRole).toString();
    else if (TableName=="Params")
        return index.model()->headerData(col,Qt::Horizontal,Qt::DisplayRole).toString()+" : "+
               index.siblingAtColumn(0).data(Qt::DisplayRole).toString()+" - "+
               index.siblingAtColumn(1).data(Qt::DisplayRole).toString();
    else if (TableName=="Rotations_détails__Tempo")
        return index.siblingAtColumn(1).data(Qt::DisplayRole).toString()+" - "+
               QObject::tr("année ")+index.siblingAtColumn(2).data(Qt::DisplayRole).toString()+" - "+
               index.siblingAtColumn(3).data(Qt::DisplayRole).toString();
    else if (TableName=="Variétés__inv_et_cde")
        col=2;

    return index.model()->headerData(col,Qt::Horizontal,Qt::DisplayRole).toString()+" : "+
           index.siblingAtColumn(col).data(Qt::DisplayRole).toString();
}

QColor TableColor(QString sTName,QString sFName)
{
    const QColor cBase=QColor("#a17dc2");//Violet gris
    const QColor cEspece=QColor("#002bff");//Bleu
    const QColor cFamille=QColor("#0085c4");//Bleu gris
    const QColor cITP=QColor("#ff0000");//Rouge
    const QColor cPlanche=QColor("#ff8100");//Orange
    const QColor cRotation=QColor("#ce9462");//Orange gris
    const QColor cVariete=QColor("#b7b202");//Jaune
    const QColor cParam=QColor("#7f7f7f");//Gris

    //Use sTName.toUpper().startsWith(...) to apply color to the corresponding views.

    //Color by field name for all tables or views.
    if (sFName=="Culture" or
        sFName.startsWith("Nb_cu"))
        return cCulture;
    else if (sFName=="Espèce" or
        sFName=="Rendement" or
        sFName=="Niveau" or
        sFName=="FG" or
        (sFName=="Apport" and !sTName.toUpper().startsWith("APPORTS")) or
        sFName=="A_planifier" or
        sFName=="N_espèce")
        return cEspece;
    else if (sFName=="Famille" or
             sFName=="N_famille")
        return cFamille;
    else if (sFName=="IT_plante" or
             sFName.startsWith("ITP") or
             (sFName=="Type_planche" and !sTName.toUpper().startsWith("ROTATIONS")) or
             (sFName=="Type_culture" and !sTName.toUpper().startsWith("ROTATIONS")) or
             sFName=="Déb_semis" or
             sFName=="Déb_plantation" or
             sFName=="Déb_récolte" or
             (sFName=="Fin_récolte" and !sTName.toUpper().startsWith("CULTURES"))or
             sFName=="Nb_graines_trou"  or
             (sFName=="Dose_semis" and !sTName.toUpper().startsWith("ESPÈCES")) or
             sFName=="N_IT_plante" )
        return cITP;
    else if (sFName=="Planche" or
             sFName=="Ilot" or
             sFName=="Largeur" or
             sFName=="N_Planche")
        return cPlanche;
    else if (sFName=="Variété" or
             sFName=="Qté_stock")
        return cVariete;
    else if (sFName=="Valeur" or
             sFName=="Année_à_planifier")
        return cParam;
    else if (sFName=="TEMPO")
        return QColor();//Drawed column

    //Color by table name.
    else if (sTName.toUpper().startsWith("APPORTS"))
        return cBase;
    else if (sTName.toUpper().startsWith("CULTURES"))
        return QColor();//Color by row
    else if (sTName.toUpper().startsWith("ESPÈCES"))
        return cEspece;
    else if (sTName.toUpper().startsWith("FAMILLES"))
        return cFamille;
    else if (sTName.toUpper().startsWith("FOURNISSEURS"))
        return cBase;
    else if (sTName.toUpper().startsWith("ITP"))
        return cITP;
    else if (sTName.toUpper().startsWith("PLANCHES"))
        return cPlanche;
    else if (sTName.toUpper().startsWith("RÉCOLTES"))
        return cCulture;
    else if (sTName.toUpper().startsWith("ROTATIONS"))
        return cRotation;
    else if (sTName.toUpper().startsWith("TYPES_PLANCHE"))
        return cBase;
    else if (sTName.toUpper().startsWith("VARIÉTÉS"))
        return cVariete;
    else
        return QColor();
}

QString ToolTipField(const QString sTableName,const QString sFieldName)
{
    QString sToolTip="";

    //Champs présents dans plusieurs tables avec des significations différentes.
    if (sTableName=="Apports"){

    } else if (sTableName=="Cultures"){
        if (sFieldName=="Type")
            sToolTip=QObject::tr("Automatique, en fonction des date de semis, plantation et récolte.");
        else if (sFieldName=="Nb_rangs")
            sToolTip=QObject::tr("Nombre de rangs cultivés sur la planche.")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        else if (sFieldName=="Longueur")
            sToolTip=QObject::tr("Longueur de la culture sur la planche (m).")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        else if (sFieldName=="Date_semis" or sFieldName=="Date_plantation" or sFieldName=="Début_récolte" or sFieldName=="Fin_récolte")
            sToolTip=QObject::tr("Date réelle ou prévue.");

    } else if (sTableName=="Espèces"){
        if (sFieldName=="Dose_semis")
            sToolTip=QObject::tr("Quantité de semence (g/m²).")+"\n"+QObject::tr("Valeur par défaut pour les itinéraires techniques.");
        else if (sFieldName=="Nb_graines_g")
            sToolTip=QObject::tr("Nombre de graines par gramme.")+"\n"+QObject::tr("Valeur par défaut pour les variétés.");
    } else if (sTableName=="Familles"){
        if (sFieldName=="Intervalle")
            sToolTip=QObject::tr("Nombre d'années nécessaires entre 2 cultures consécutives sur la même planche.");

    } else if (sTableName=="Fournisseurs"){
        if (sFieldName=="Priorité")
            sToolTip=QObject::tr("Chez qui commander en priorité.");

    } else if (sTableName=="ITP"){
        if (sFieldName=="Dose_semis")
            sToolTip=QObject::tr("Quantité de semence (g/m²).")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire SI ESPACEMENT = 0.");
        else if (sFieldName=="Nb_rangs")
            sToolTip=QObject::tr("Nombre de rangs cultivés sur une planche.")+
                                 "\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.")+
                                 "\n"+QObject::tr("Valeur par défaut pour les cultures.");

    } else if (sTableName=="Planches"){
        if (sFieldName=="Longueur")
            sToolTip=QObject::tr("Longueur de la planche (m).");
        else if (sFieldName=="Rotation")
            sToolTip=QObject::tr("Rotation de cultures qui utilise cette planche.");

    } else if (sTableName=="Rotations"){
        if (sFieldName=="Nb_années")
            sToolTip=QObject::tr("Automatique, en fonction des détails de la rotation.\n3 à 6 ans en général.");

    } else if (sTableName=="Rotations_détails"){
        if (sFieldName=="Année")
            sToolTip=QObject::tr("Année de culture dans la rotation.")+"\n"+QObject::tr("Nombre entier entre 1 et 5 si la rotation est sur 5 ans.");
        else if (sFieldName=="Nb_planches")
            sToolTip=QObject::tr("Nombre de planches occupées.")+"\n"+QObject::tr("Souvent un ilot de production entier.");

    } else if (sTableName=="Récoltes"){
        if (sFieldName=="Quantité")
            sToolTip=QObject::tr("En kg.");

    } else if (sTableName=="Types_planche"){
        if (sFieldName=="Type")
            sToolTip=QObject::tr("Par exemple 'Serre', 'Extérieur'...");

    } else if (sTableName=="Variétés"){
        if (sFieldName=="Nb_graines_g")
            sToolTip=QObject::tr("Nombre de graines par gramme.")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
    }

    //Champs ayant la même signification partout.
    if (sToolTip==""){

        //Champs existant dans plusieurs tables
        if (sFieldName=="")
            sToolTip=QObject::tr("");
        else if (sFieldName=="Espacement")
            sToolTip=QObject::tr("Espacement entre 2 plants dans un rang de culture (cm)")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        else if (sFieldName=="IT_plante")
            sToolTip=QObject::tr("Itinéraire technique: une espèce de plante et une manière de la cultiver.");
        else if (sFieldName=="Rotation")
            sToolTip=QObject::tr("Ensemble de cultures qui se succèdent sur un ensemble de planches.");

        //Apports
        else if (sFieldName=="Apport")
            sToolTip=QObject::tr("Ammendement nécessaire avant culture.");
        else if (sFieldName=="Poids_m²")
            sToolTip=QObject::tr("Quantité d'ammendement avant culture (kg/m²).");
        //Cultures
        else if (sFieldName=="D_planif")
            sToolTip=QObject::tr("Date de calcul des dates de semis, plantation et récolte.");
        //Espèces
        else if (sFieldName=="Niveau")
            sToolTip=QObject::tr("Niveau de difficulté.")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
        else if (sFieldName=="FG")
            sToolTip=QObject::tr("Faculté germinative.")+"\n"+QObject::tr("Durée de conservation des graines (années).")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
        else if (sFieldName=="T_germ")
            sToolTip=QObject::tr("Température de germination (min-max °C).")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
        else if (sFieldName=="Levée")
            sToolTip=QObject::tr("Temps de levée (jours).")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
        else if (sFieldName=="Inventaire")
            sToolTip=QObject::tr("Quantité de récolte en stock (kg).");
        else if (sFieldName=="A_planifier")
            sToolTip=QObject::tr("Espèce à cultiver l'année prochaine.");
        else if (sFieldName=="Rendement")
            sToolTip=QObject::tr("Production moyenne (kg/m²).")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
        //ITP
        else if (sFieldName=="Type_culture")
            sToolTip=QObject::tr("Automatique, en fonction des débuts de période de semis, plantation et récolte.");
        else if (sFieldName.startsWith("Déb_") or sFieldName.startsWith("Fin_"))
            sToolTip=QObject::tr("Début de mois (ex: 04-01 pour début avril)\nou milieu du mois (ex: 04-15 pour mi-avril).");
        else if (sFieldName=="Nb_graines_trou")
            sToolTip=QObject::tr("Nombre de graines par trou.")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        //Planches
        else if (sFieldName=="Largeur")
            sToolTip=QObject::tr("Largeur de la planche (m).");
        //Rotations
        else if (sFieldName=="Année_1")
            sToolTip=QObject::tr("Année de début de la rotation.")+"\n"+QObject::tr("Cette valeur ne doit pas être changée d'une année sur l'autre tant que la rotation se poursuit.");
        //Rotations_détails
        else if (sFieldName=="Pc_planches")
            sToolTip=QObject::tr("Pourcentage d'occupation des planches (1 à 100%).")+"\n"+QObject::tr("Plusieurs cultures peuvent partager une planches.");
        else if (sFieldName=="Fi_planches")
            sToolTip=QObject::tr("Filtre de sélection de planches.")+"\n"+QObject::tr("Si filtre = 'AD', seules les planches se terminant par A ou D seront utilisées.");
        //Variétés
        else if (sFieldName=="Qté_stock")
            sToolTip=QObject::tr("Semence en stock (g).");
        else if (sFieldName=="Qté_cde")
            sToolTip=QObject::tr("Semence commandée (g).")+"\n"+QObject::tr("A réception, mettre à 0 et ajouter la quantité à la quantité en stock.");
    }
    return sToolTip;
}

QString ToolTipTable(const QString sTableName) {
    QString sToolTip="";

    if (sTableName==""){

    } else if (sTableName=="Cultures"){
        sToolTip=QObject::tr("Une culture c'est une plante (variété+itinéraire technique) sur une planche.\nSi la même plante est présente sur plusieurs planches, il y a une culture (numérotée) par planche.");

    } else if (sTableName=="Espèces"){
        sToolTip=QObject::tr("");
    } else if (sTableName=="Familles"){
        sToolTip=QObject::tr("");
    } else if (sTableName=="Fournisseurs"){
        sToolTip=QObject::tr("");
    } else if (sTableName=="ITP"){
        sToolTip=QObject::tr(   "Itinéraires techniques de plantes.\n"
                                "Une ITP c'est Une espèce de plante cultivée d'une certaine manière:\n"
                                "Hative ou tardive, sous serre ou en extérieur, etc.\n"
                                "Chaque ITP à une période de semis, de plantation et de récolte.\n"
                                "Pour chaque culture, il faudra prendre une variété adaptée à l'itinéraire technique voulu.");
    } else if (sTableName=="Planches"){
        sToolTip=QObject::tr("Planches de cultures.");
    } else if (sTableName=="Rotations"){
        sToolTip=QObject::tr(   "Ensemble d'ITP qui vont se succéder sur un groupe de planches (ilot) en respectant les débuts et fin de chaque culture et le nombre d'année minimum nécessaire entre 2 cultures d'une même famille (intervale).");
    } else if (sTableName=="Rotations_détails"){
        sToolTip=QObject::tr("");
    } else if (sTableName=="Récoltes"){
        sToolTip=QObject::tr("");
    } else if (sTableName=="Types_planche"){
        sToolTip=QObject::tr("");
    } else if (sTableName=="Variétés"){
        sToolTip=QObject::tr("");
    }
    return sToolTip;
}
