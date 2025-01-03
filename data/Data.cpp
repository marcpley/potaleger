#include "PotaUtils.h"
#include "qcolor.h"
#include <QtSql/QSqlQueryModel>
#include <QObject>


QString DynDDL(QString sQuery)
{
    sQuery=StrReplace(sQuery,"#FmtPlanif#","IN('','01-01','01-15','02-01','02-15','03-01','03-15','04-01','04-15','05-01','05-15','06-01','06-15','07-01','07-15','08-01','08-15','09-01','09-15','10-01','10-15','11-01','11-15','12-01','12-15')");
    return sQuery;
}

bool ReadOnly(const QString sTableName,const QString sFieldName)
{
    bool bReadOnly=false;

    if (sTableName=="Cultures"){
        bReadOnly = (sFieldName=="Culture" or sFieldName=="Type");

    } else if (sTableName=="ITP"){
        bReadOnly = (sFieldName=="Type_culture");

    } else if (sTableName=="Params"){
        bReadOnly = (sFieldName!="Valeur");
    }

    return bReadOnly;
}

QColor TableColor(QString sTName,QString sFName)
{
    const QColor cBase=QColor("#a17dc2");//Violet gris
    const QColor cCulture=QColor("#00ff00");//Vert
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

    //Color by table name.
    else if (sTName.toUpper().startsWith("APPORTS"))
        return cBase;
    else if (sTName.toUpper().startsWith("CULTURES"))
        return cCulture;
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

QString ToolTip(const QString sTableName,const QString sFieldName)
{
    QString sToolTip="";

    if (sTableName=="Apports"){
        if (sFieldName=="Poids_m²")
            sToolTip=QObject::tr("Ammendement en kg par m².");

    } else if (sTableName=="Cultures"){
        if (sFieldName=="TYPE")
            sToolTip=QObject::tr("Automatique, en fonction des date de semis, plantation et récolte.");
        else if (sFieldName=="Longueur")
            sToolTip=QObject::tr("Longueur de la culture sur la planche (m).")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        else if (sFieldName=="D_planif")
            sToolTip=QObject::tr("Date de calcul des date de semis, plantation et récolte.");
        else if (sFieldName=="Date_semis" or sFieldName=="Date_plantation" or sFieldName=="Début_récolte" or sFieldName=="Fin_récolte")
            sToolTip=QObject::tr("Date réelle ou prévue.");

    } else if (sTableName=="Espèces"){
        if (sFieldName=="Rendement")
            sToolTip=QObject::tr("Production moyenne (kg/m²).")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
        else if (sFieldName=="Niveau")
            sToolTip=QObject::tr("Niveau de difficulté.")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
        else if (sFieldName=="Dose_semis")
            sToolTip=QObject::tr("Quantité de semence (g/m²).")+"\n"+QObject::tr("Valeur par défaut pour les itinéraires techniques.");
        else if (sFieldName=="Nb_graines_g")
            sToolTip=QObject::tr("Nombre de graines par gramme.")+"\n"+QObject::tr("Valeur par défaut pour les variétés.");
        else if (sFieldName=="FG")
            sToolTip=QObject::tr("Faculté germinative.")+"\n"+QObject::tr("Durée de conservation des graines (années).")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
        else if (sFieldName=="T_germ")
            sToolTip=QObject::tr("Température de germination (min-max °C).")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
        else if (sFieldName=="Levée")
            sToolTip=QObject::tr("Temps de levée (jours).")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
        else if (sFieldName=="Inventaire")
            sToolTip=QObject::tr("Quantité en stock (kg).");
        else if (sFieldName=="A_planifier")
            sToolTip=QObject::tr("Espèce à cultiver l'année prochaine.");

    } else if (sTableName=="Familles"){
        if (sFieldName=="Intervalle")
            sToolTip=QObject::tr("Nombre d'années nécessaires entre 2 cultures consécutives sur la même planche.");

    } else if (sTableName=="Fournisseurs"){
        if (sFieldName=="Priorité")
            sToolTip=QObject::tr("Chez qui commander en priorité.");

    } else if (sTableName=="ITP"){
        if (sFieldName=="Dose_semis")
            sToolTip=QObject::tr("Quantité de semence (g/m²).")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire SI ESPACEMENT = 0.");
        else if (sFieldName=="TYPE")
            sToolTip=QObject::tr("Automatique, en fonction des début de périodes de semis, plantation et récolte.");
        else if (sFieldName.startsWith("Déb_") or sFieldName.startsWith("Fin_"))
            sToolTip=QObject::tr("Début de mois (ex: 04-01 pour début avril)\nou milieu du mois (ex: 04-15 pour mi-avril).");

    } else if (sTableName=="Planches"){
        if (sFieldName=="Longueur")
            sToolTip=QObject::tr("Longueur de la planche (m).");
        else if (sFieldName=="Largeur")
            sToolTip=QObject::tr("Largeur de la planche (m).");
        else if (sFieldName=="Rotation")
            sToolTip=QObject::tr("Rotation de cultures qui utilise cette planche.");

    } else if (sTableName=="Rotations"){
        if (sFieldName=="Année_1")
            sToolTip=QObject::tr("Année de début de la rotation.")+"\n"+QObject::tr("Cette valeur ne doit pas être changée d'une année sur l'autre tant que la rotation se poursuit.");
        else if (sFieldName=="Nb_années")
            sToolTip=QObject::tr("3 à 6 ans en général.");

    } else if (sTableName=="Rotations_détails"){
        if (sFieldName=="Année")
            sToolTip=QObject::tr("Année de culture dans la rotation.")+"\n"+QObject::tr("Nombre entier entre 1 et 5 si la rotation est sur 5 ans.");
        else if (sFieldName=="Pc_planches")
            sToolTip=QObject::tr("Pourcentage d'occupation des planches (1 à 100%).")+"\n"+QObject::tr("Plusieurs cultures peuvent partager une planches.");
        else if (sFieldName=="Nb_planches")
            sToolTip=QObject::tr("Nombre de planches occupées.")+"\n"+QObject::tr("Souvent un ilot de production entier.");
        else if (sFieldName=="Fi_planches")
            sToolTip=QObject::tr("Filtre de sélection de planches.")+"\n"+QObject::tr("Si filtre = 'AD', seules les planches se terminant par A ou D seront utilisées.");

    } else if (sTableName=="Récoltes"){
        if (sFieldName=="Quantité")
            sToolTip=QObject::tr("En kg.");

    } else if (sTableName=="Types_planche"){
        if (sFieldName=="Type")
            sToolTip=QObject::tr("Par exemple 'Serre', 'Extérieur'...");

    } else if (sTableName=="Variétés"){
        if (sFieldName=="Nb_graines_g")
            sToolTip=QObject::tr("Nombre de graines par gramme.")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        else if (sFieldName=="Qté_stock")
            sToolTip=QObject::tr("Semence en stock (g).");
        else if (sFieldName=="Qté_cde")
            sToolTip=QObject::tr("Semence commandée (g).")+"\n"+QObject::tr("A réception, mettre à 0 et ajouter la quantité à la quantité en stock.");
    }

    if (sToolTip==""){
        if (sFieldName=="")
            sToolTip=QObject::tr("");
        else if (sFieldName=="Apport")
            sToolTip=QObject::tr("Ammendement nécessaire avant culture.");
        else if (sFieldName=="Espacement")
            sToolTip=QObject::tr("Espacement entre 2 plants dans un rang de culture (cm)")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        else if (sFieldName=="IT_plante")
            sToolTip=QObject::tr("Itinéraire technique");
        else if (sFieldName=="Nb_graines_trou")
            sToolTip=QObject::tr("Nombre de graines par trou.")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        else if (sFieldName=="Nb_rangs")
            sToolTip=QObject::tr("Nombre de rangs cultivés sur une planche.")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        else if (sFieldName=="Rotation")
            sToolTip=QObject::tr("Ensemble de cultures qui se succèdent sur un ensemble de planches.");
    }
    return sToolTip;
}
