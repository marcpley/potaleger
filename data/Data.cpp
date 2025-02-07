#include "data/Data.h"
#include "PotaUtils.h"
#include "mainwindow.h"
#include "qcolor.h"
#include <QSqlTableModel>
#include <QtSql/QSqlQueryModel>
#include <QObject>
#include "potawidget.h"

int DefColWidth(const QString sTableName,const QString sFieldName) {
    QString sType = DataType(sTableName,sFieldName);
    if (sType=="DATE")
        return 100;
    else if (sType.startsWith("BOOL"))
        return 50;
    else if (sType.startsWith("INT"))
        return 40;
    else if (sType=="REAL")
        return 50;

    else if (sFieldName.startsWith("Déb_") or sFieldName.startsWith("Fin_"))
        return 45;
    else if (sFieldName=="Fi_planches")
        return 40;
    else if (sFieldName=="Mise_en_place")
        return 55;
    else if (sFieldName=="Notes" or sFieldName.startsWith("N_"))
        return 150;
    else if (sFieldName.startsWith("Pc_"))
        return 40;
    else if (sFieldName=="TEMPO")
        return 400;
    else if (sFieldName=="Type_culture")
        return 100;
    else if (sFieldName=="Type_planche")
        return 65;
    else
        return -1;
}

QString DynDDL(QString sQuery)
{
    sQuery=StrReplace(sQuery,"#FmtPlanif#","IN('','01-01','01-15','02-01','02-15','03-01','03-15','04-01','04-15','05-01','05-15','06-01','06-15','07-01','07-15','08-01','08-15','09-01','09-15','10-01','10-15','11-01','11-15','12-01','12-15')");
    sQuery=StrReplace(sQuery,"#DbVer#",DbVersion);
    return sQuery;
}

QString FkFilter(const QString sTableName,const QString sFieldName, const QModelIndex &index){
    QString filter="";
    const PotaTableModel *model = qobject_cast<const PotaTableModel *>(index.model());
    if (model) {
        if (sTableName=="Cultures") {
            if (sFieldName=="Variété")
                filter="Espèce=(SELECT Espèce FROM ITP I WHERE I.IT_plante='"+
                       index.siblingAtColumn(model->fieldIndex("IT_plante")).data().toString()+
                       "')";
            // else if (sFieldName=="IT_plante")
            //     filter="Espèce IN (SELECT E.Espèce FROM Espèces E WHERE E.A_planifier NOTNULL)";
        } else if (sTableName=="Récoltes") {
            if (sFieldName=="Culture")
                filter="Culture IN (SELECT Culture FROM C_en_place JOIN ITP I USING(IT_plante) WHERE I.Espèce='"+
                         index.siblingAtColumn(model->fieldIndex("Espèce")).data().toString()+
                         "')";//index.model()->index(index.row(),index.model()->ind+
            else if (sFieldName=="Espèce")
                filter="Espèce IN (SELECT I.Espèce FROM C_en_place JOIN ITP I USING(IT_plante))";
        }
    }
    return filter;
}

QString GeneratedFielnameForDummyFilter(const QString sTableName) {
    if (sTableName=="Cultures")
        return "Etat";
    else if (sTableName=="ITP")
        return "Type_culture";
    else
        return "";
}

int NaturalSortCol(const QString sTableName){
    if (sTableName=="Params" or
        sTableName=="Rotations_détails__Tempo")
        return -1;
    else if (sTableName=="Cult_planif")
        return 5;//Date_semis
    else if (sTableName=="Cultures__Semis_à_faire")
        return 8;//Date_semis
    else if (sTableName=="Cultures__Plantations_à_faire")
        return 10;//Date_plantation
    else if (sTableName=="Cultures__Récoltes_à_faire")
        return 9;//Début_récolte
    else if (sTableName=="Cultures__à_terminer")
        return 8;//Fin_récolte
    else if (sTableName=="Cultures__Tempo_Espèce")
        return 1;//ITP
    else if (sTableName=="IT_rotations_manquants")
        return 1;//ITP
    else
        return 0;
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
    } else if (sTableName=="Récoltes__Saisies"){
        bReadOnly = (sFieldName=="Planche");
    } else if (sTableName=="Rotations"){
        bReadOnly = !(sFieldName=="Nb_Années");

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

QString RowSummary(QString TableName, const QSqlRecord &rec){
    int col=0;
    QString result="";
    if (TableName=="Apports")
        result=rec.value(rec.indexOf("Apport")).toString()+" - "+
               rec.value(rec.indexOf("Description")).toString()+" - "+
               iif(!rec.value(rec.indexOf("Poids_m²")).isNull(),
                   rec.value(rec.indexOf("Poids_m²")).toString()+"kg/m²","").toString();
    else if (TableName=="Cult_planif")
        result=rec.value(rec.indexOf("Planche")).toString()+" - "+
               rec.value(rec.indexOf("IT_plante")).toString()+" - "+
               iif(rec.value(rec.indexOf("Date_semis")).isNull(),
                   rec.value(rec.indexOf("Date_Plantation")).toString(),
                   rec.value(rec.indexOf("Date_semis")).toString()).toString();
    else if (TableName.startsWith("Cultures"))
        result=rec.value(rec.indexOf("Culture")).toString()+" - "+
               rec.value(rec.indexOf("Planche")).toString()+" - "+
               rec.value(rec.indexOf("IT_plante")).toString()+" - "+
               rec.value(rec.indexOf("Type")).toString()+" - "+
               rec.value(rec.indexOf("Etat")).toString()+" - "+
               iif(rec.value(rec.indexOf("Date_semis")).isNull(),
                   rec.value(rec.indexOf("Date_Plantation")).toString(),
                   rec.value(rec.indexOf("Date_semis")).toString()).toString();
    else if (TableName.startsWith("Familles"))
        result=rec.value(rec.indexOf("Famille")).toString()+" - "+
               iif(!rec.value(rec.indexOf("Intervalle")).isNull(),
                   rec.value(rec.indexOf("Intervalle")).toString()+" "+QObject::tr("ans"),"").toString();
    else if (TableName.startsWith("Fournisseurs"))
        result=rec.value(rec.indexOf("Fournisseur")).toString()+" - "+
               rec.value(rec.indexOf("Site_web")).toString();
    else if (TableName.startsWith("ITP"))
        result=rec.value(rec.indexOf("IT_plante")).toString()+" - "+
               rec.value(rec.indexOf("Type_planche")).toString()+" - "+
               rec.value(rec.indexOf("Type_culture")).toString();
    else if (TableName=="IT_rotations")
        result=rec.value(rec.indexOf("IT_plante")).toString()+" - "+
               rec.value(rec.indexOf("Nb_planches")).toString()+" "+QObject::tr("planches")+" - "+
               rec.value(rec.indexOf("Longueur")).toString()+"m";
    else if (TableName=="IT_rotations_ilots")
        result=rec.value(rec.indexOf("IT_plante")).toString()+" - "+
               QObject::tr("ilot ")+rec.value(rec.indexOf("Ilot")).toString()+" - "+
               rec.value(rec.indexOf("Nb_planches")).toString()+" "+QObject::tr("planches")+" - "+
               rec.value(rec.indexOf("Longueur")).toString()+"m";
    else if (TableName=="Params")
        result=rec.value(rec.indexOf("Section")).toString()+" · "+
                 rec.value(rec.indexOf("Paramètre")).toString()+" : "+
                 rec.value(rec.indexOf("Valeur")).toString()+" "+
                 rec.value(rec.indexOf("Unité")).toString();
    else if (TableName=="Planches")
        result=rec.value(rec.indexOf("Planche")).toString()+" - "+
                 rec.value(rec.indexOf("Type")).toString()+" - "+
                 rec.value(rec.indexOf("Longueur")).toString()+"m x "+
                 rec.value(rec.indexOf("Largeur")).toString()+"m";
    else if (TableName=="Récoltes__Saisies")
        result=rec.value(rec.indexOf("Date")).toString()+" - "+
                 rec.value(rec.indexOf("Espèce")).toString()+" - "+
                 rec.value(rec.indexOf("Culture")).toString()+" - "+
                 iif(!rec.value(rec.indexOf("Quantité")).isNull(),
                     rec.value(rec.indexOf("Quantité")).toString()+QObject::tr("kg"),"").toString()+" - "+
                 rec.value(rec.indexOf("Planche")).toString();
    else if (TableName=="Rotations")
        result=rec.value(rec.indexOf("Rotation")).toString()+" - "+
               rec.value(rec.indexOf("Type_planche")).toString()+" - "+
               iif(!rec.value(rec.indexOf("Nb_années")).isNull(),
                   rec.value(rec.indexOf("Nb_années")).toString()+" "+QObject::tr("ans"),"").toString();
    else if (TableName=="Rotations_détails__Tempo")
        result=rec.value(rec.indexOf("Rotation")).toString()+" - "+
               QObject::tr("Année ")+rec.value(rec.indexOf("Année")).toString()+" - "+
               rec.value(rec.indexOf("IT_plante")).toString();
    else if (TableName=="Successions_par_planche")
        result=rec.value(rec.indexOf("Planche")).toString()+" : "+
               rec.value(rec.indexOf("ITP_en_place")).toString()+" ("+
               rec.value(rec.indexOf("Libre_le")).toString()+") -> "+
               rec.value(rec.indexOf("ITP_à_venir")).toString()+" ("+
               rec.value(rec.indexOf("En_place_le")).toString()+")";
    else if (TableName=="Variétés__inv_et_cde")
        result=rec.value(rec.indexOf("Variété")).toString()+" - "+
               iif(!rec.value(rec.indexOf("Qté_nécess")).isNull(),QObject::tr("Nécessaire ")+rec.value(rec.indexOf("Qté_nécess")).toString()+"g - ","").toString()+
               iif(!rec.value(rec.indexOf("Qté_stock")).isNull(),QObject::tr("Stock ")+rec.value(rec.indexOf("Qté_stock")).toString()+"g - ","").toString()+
               iif(!rec.value(rec.indexOf("Qté_cde")).isNull(),QObject::tr("Cde ")+rec.value(rec.indexOf("Qté_cde")).toString()+"g - ","").toString()+
               iif(!rec.value(rec.indexOf("Qté_manquante")).isNull(),QObject::tr("Manquant ")+rec.value(rec.indexOf("Qté_manquante")).toString()+"g","").toString()+" - "+
               rec.value(rec.indexOf("Fournisseur")).toString();

    if (result=="")
        result=rec.value(col).toString();

    result=StrReplace(result,"-  -","-");
    if (result.last(3)==" - ")
        result=StrFirst(result,result.length()-3);

    return result;
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

    } else if (sTableName=="Cultures" or sTableName.startsWith("Cultures__")){
        if (sFieldName=="Type")
            sToolTip=QObject::tr("Automatique, en fonction des date de semis, plantation et récolte.");
        else if (sFieldName=="Etat")
            sToolTip=QObject::tr("Automatique, en fonction des semis, plantation et récolte faites ou pas.\n"
                                   "Donne la couleur de la ligne dans les tableaux de données.");
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

    } else if (sTableName=="ITP" or sTableName.startsWith("ITP__")){
        if (sFieldName=="Dose_semis")
            sToolTip=QObject::tr("Quantité de semence (g/m²).")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire SI ESPACEMENT = 0.");
        else if (sFieldName.startsWith("J_"))
            sToolTip=QObject::tr("Nombre de jours pour...")+"\n"+
                     QObject::tr("La date de prévue pour les cultures sera le début de période.");
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
            sToolTip=QObject::tr(   "Automatique, en fonction des détails de la rotation.\n"
                                    "3 à 6 ans en général.");

    } else if (sTableName=="Rotations_détails"){
        if (sFieldName=="Année")
            sToolTip=QObject::tr(   "Année de culture dans la rotation.\n"
                                    "Nombre entier entre 1 et 5 si la rotation est sur 5 ans.");
        else if (sFieldName=="Pc_planches")
            sToolTip=QObject::tr(   "Pourcentage d'occupation des planches de l'UdP.\n"
                                    "Exemple: planche de 10m de long occupée à 40% -> la culture sur cette planche fera 4m.");
        else if (sFieldName=="Fi_planches")
            sToolTip=QObject::tr(  "Filtrage des planches de l'UdP.\n"
                                   "Exemple: l'ilot AA contient une UdP (1) de 4 planches (A, B, C et D)\n"
                                   "Les planches sont: AA1A, AA1B, AA1C et AA1D\n"
                                   "Fi_planches = AC -> seule les planches AA1A et AA1C seront occupées par les cultures.");
        else if (sFieldName=="Mise_en_place")
            sToolTip=QObject::tr(  "Momment où la culture sera mise en place sur la planche.\n"
                                   "* indique un chevauchement avec la culture précédente (pas encore récoltée).\n"
                                   "- indique u temps important d'inoccupation de la planche.");
    } else if (sTableName=="Récoltes" or sTableName.startsWith("Récoltes__")){
        if (sFieldName=="Quantité")
            sToolTip=QObject::tr("Quantité récoltée sur la planche (kg).");

    } else if (sTableName=="Types_planche"){
        if (sFieldName=="Type")
            sToolTip=QObject::tr("Par exemple 'Serre', 'Extérieur'...");

    } else if (sTableName=="Variétés"){
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
        else if (sFieldName=="Planches_Ilots")
            sToolTip=QObject::tr(   "Regroupement de planches\n"
                                    "(voir le paramètre 'Ilot_nb_car').");
        //Apports
        else if (sFieldName=="Apport")
            sToolTip=QObject::tr("Ammendement nécessaire avant culture.");
        else if (sFieldName=="Poids_m²")
            sToolTip=QObject::tr("Quantité d'ammendement avant culture (kg/m²).");
        //Cultures
        else if (sFieldName=="Culture")
            sToolTip=QObject::tr("Numéro unique de la culture\n"
                                 "(pas de remise à zéro tous les ans).");
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
        // else if (sFieldName=="Inventaire")
        //     sToolTip=QObject::tr("Quantité de récolte en stock (kg).");
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
        else if (sFieldName=="Nb_graines_g")
            sToolTip=QObject::tr("Nombre de graines par gramme.")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        else if (sFieldName=="Qté_stock")
            sToolTip=QObject::tr("Semence en stock (g).");
        else if (sFieldName=="Qté_cde")
            sToolTip=QObject::tr("Semence commandée (g).")+"\n"+QObject::tr("A réception, mettre à 0 et ajouter la quantité à la quantité en stock.");

        //Views
        else if (sFieldName=="Année_à_planifier")
            sToolTip=QObject::tr("Paramétrable: 'Année_planif'.");
        else if (sFieldName=="C_à_venir")
            sToolTip=QObject::tr(   "Cultures prévues mais pas encore en place sur leur planche.\n"
                                    "Sont incluses les cultures déjà semées sous abris.");
        else if (sFieldName=="C_en_place")
            sToolTip=QObject::tr(   "Cultures en place sur leur planche: semées (semis direct) ou plantées.\n"
                                    "Ne sont pas incluses les cultures semées sous abris mais non plantées.");
        else if (sFieldName=="C_non_commencées")
            sToolTip=QObject::tr(   "Cultures prévues mais ni semées ni plantées.");
        else if (sFieldName=="En_place_le")
            sToolTip=QObject::tr("Plus petite date de semis (direct) ou de plantation parmis les cultures à venir.");
        else if (sFieldName=="Libre_le")
            sToolTip=QObject::tr("Plus grande date de fin de récolte parmis les cultures en place.");
        else if (sFieldName=="Nb_cu_AV")
            sToolTip=QObject::tr(   "Nombre de cultures à venir (AV) sur la planche.\n"
                                   "Ces cultures ne sont pour le moment ni semées (semis direct) ni plantées.");
        else if (sFieldName=="Nb_cu_EP")
            sToolTip=QObject::tr(   "Nombre de cultures en place (EP) sur la planche.\n"
                                   "Ces cultures ne sont pas terminées.");
        else if (sFieldName=="Qté_nécess")
            sToolTip=QObject::tr(   "Semence nécessaire (g) pour les cultures non encore semées.");
        else if (sFieldName=="Qté_manquante")
            sToolTip=QObject::tr(   "Qté nécessaire moins Qté en stock moins Qté commandée.");
    }

    if (sToolTip==""){
        if (DataType(sTableName,sFieldName).startsWith("BOOL"))
            sToolTip=QObject::tr(  "Champ Oui/Non\n"
                                   "Vide = Non (ou faux)\n"
                                   "Saisie quelconque = Oui (ou vrai).\n"
                                   "'X', 'Oui', 'Non', '0' ou n'importe quoi veulent dire OUI.");
    }
    return sToolTip;
}

QString ToolTipTable(const QString sTableName) {
    QString sToolTip="";

    if (sTableName=="")
        sToolTip="";

    //Tables
    else if (sTableName=="Apports")
        sToolTip=QObject::tr("Ammendement devant être apporté à une planche avant d'y cultiver une espèce de plante.");
    else if (sTableName=="Cultures")
        sToolTip=QObject::tr("Une culture c'est une plante (variété+itinéraire technique) sur une planche.\nSi la même plante est présente sur plusieurs planches, il y a une culture (numérotée) par planche.");
    else if (sTableName=="Espèces")
        sToolTip=QObject::tr("");
    else if (sTableName=="Familles")
        sToolTip=QObject::tr("");
    else if (sTableName=="Fournisseurs")
        sToolTip=QObject::tr("");
    else if (sTableName=="ITP")
        sToolTip=QObject::tr(   "Itinéraires techniques de plantes.\n"
                                "Une ITP c'est Une espèce de plante cultivée d'une certaine manière:\n"
                                "Hative ou tardive, sous serre ou en extérieur, etc.\n"
                                "Chaque ITP à une période de semis, de plantation et de récolte.\n"
                                "Pour chaque culture, il faudra prendre une variété adaptée à l'itinéraire technique voulu.");
    else if (sTableName=="Planches")
        sToolTip=QObject::tr(   "Planches de cultures.\n"
                                "Les 1ers caractères (nb paramétrable) indiquent l'ilot dans lequel se trouve la planche.\n"
                                "Pour ");
    else if (sTableName=="Rotations")
        sToolTip=QObject::tr(   "Ensemble d'ITP qui vont se succéder sur un groupe de planches (ilot)\n"
                                "en respectant les débuts et fin de chaque culture\n"
                                "et le nombre d'année minimum nécessaire entre 2 cultures d'une même famille (intervale).");
    else if (sTableName=="Rotations_détails")
        sToolTip=QObject::tr(   "Les ITP (donc les espèces cultivées) vont être déplacés chaque année\n"
                                "sur une nouvelle unité de production (UdP).\n"
                                "Si Pc_planches = 100%, les cultures occuperont la totalité des planches de l'UdP.\n"
                                "Si Fi_planches est vide, les cultures occupperont toutes les planches de l'UdP\n"
                                "Si Pc_planches = 50% et Fi_planches = A, les cultures occuperont\n"
                                "la moitié de chaque planche se terminant par A.");
    else if (sTableName=="Récoltes")
        sToolTip=QObject::tr("");
    else if (sTableName=="Types_planche")
        sToolTip=QObject::tr("");
    else if (sTableName=="Variétés")
        sToolTip=QObject::tr("");

    //Views
    else if (sTableName=="Cult_planif")
        sToolTip=QObject::tr(  "Cultures qui vont être créées lors de la prochaîne planification.\n"
                               "La variété choisie pour chaque culture est celle dont\n"
                               "la quantité de semence en stock est la plus importante.")+"\n"+
                   QObject::tr(  "Liste non directement modifiable, déduite des 'Rotations'.");
    else if (sTableName=="IT_rotations" or sTableName=="IT_rotations_Ilots")
        sToolTip=QObject::tr(  "Espèces pour lesquelles des cultures vont être créées\n"
                               "lors de la prochaîne planification.")+"\n"+
                 QObject::tr(  "Liste non directement modifiable, déduite des 'Rotations'.");
    else if (sTableName=="IT_rotations_manquants")
        sToolTip=QObject::tr(   "Espèces marquées 'A planifier' et qui ne sont\n"
                                "pourtant pas incluses dans une rotation.\n"
                                "La planification ne générera donc aucune culture pour ces espèces.");
    else if (sTableName=="Planches_Ilots")
        sToolTip=QObject::tr(  "Les ilots sont des regroupements de planches.\n"
                               "Ils ne sont pas saisis mais déduits des planches saisies.\n"
                               "Le débuts du nom des planches indique leur ilot (voir le paramètre 'Ilot_nb_car').");
    else if (sTableName=="Variétés__inv_et_cde")
        sToolTip=QObject::tr(  "Inventaire des semences pour chaque variété de plante.");

    return sToolTip;
}

bool ViewFieldIsDate(const QString sFieldName){
    if (sFieldName=="Libre_le" or
        sFieldName=="En_place_le" or
        sFieldName=="Min_semis" or
        sFieldName=="Max_semis" or
        sFieldName=="Min_plantation" or
        sFieldName=="Max_plantation" or
        sFieldName=="Min_recolte" or
        sFieldName=="Max_recolte" or
        sFieldName=="Mise_en_place" or
        sFieldName=="Date_MEP" or
        sFieldName=="Date_Ferm")
        return true;
    else
        return false;
}
