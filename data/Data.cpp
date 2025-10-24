#include "data/Data.h"
#include "PotaUtils.h"
#include "mainwindow.h"
#include "qcolor.h"
#include <QSqlTableModel>
#include <QtSql/QSqlQueryModel>
#include <QObject>
#include "potawidget.h"

bool AcceptReturns(const QString sFieldName) {
    return sFieldName=="A_faire" or
           sFieldName.startsWith("Adresse")or
           sFieldName=="Analyse_sol" or
           sFieldName=="Associations" or
           sFieldName.startsWith("Conflit_")or
           sFieldName.startsWith("Cultures")or
           sFieldName.startsWith("Dates_")or
           sFieldName=="Défavorable" or
           sFieldName.startsWith("Description")or
           sFieldName.startsWith("Espèces") or
           sFieldName=="Etats" or
           sFieldName=="Favorable" or
           sFieldName=="Fertilisants" or
           sFieldName=="Fonction" or
           sFieldName=="Interprétation" or
           sFieldName.startsWith("N_")or
           sFieldName=="Notes" or
           sFieldName.startsWith("Planches")or
           sFieldName.startsWith("Rangs_espacement") or
           sFieldName=="sql" or
           sFieldName=="Taille" or
           sFieldName=="Texte" or
           sFieldName=="Utilisation" or
           sFieldName=="Variétés_ou_It_plante";
}

QString ComboField(const QString sTableName,const QString sFieldName) {
    //Valable uniquement pour les champs modifiables.
    if (sFieldName=="Type" or
        //sFieldName=="Effet" or
        sFieldName=="Irrig")
        return sTableName+"_"+sFieldName;
    else if (sFieldName=="Type_planche")
        return "Planches_Type";
    else return "";
}

int DefColWidth(QSqlDatabase *db, const QString sTableName,const QString sFieldName) {
    QString sType=DataType(db, sTableName,sFieldName);
    if (sType=="DATE")
        return 80;
    else if (sType.startsWith("BOOL"))
        return 50;
    else if (sType.startsWith("INT"))
        return 40;
    else if (sType=="REAL")
        return 50;

    else if (sFieldName=="Fi_planches")
        return 40;
    else if (sFieldName=="Notes" or sFieldName.startsWith("N_"))
        return 150;
    else if (sFieldName.startsWith("Pc_"))
        return 40;
    else if (sFieldName=="TEMPO")
        return 400;
    else if (sFieldName.startsWith("TEMPO_"))
        return 367;
    else if (sFieldName=="Type_culture")
        return 100;
    else if (sFieldName=="Type_planche")
        return 65;
    else
        return -1;
}

QString DynDDL(QString sQuery)
{
    sQuery=StrReplace(sQuery,"#DbVer#",DbVersion);
    sQuery=StrReplace(sQuery,"#NoAccent(","replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(");
    sQuery=StrReplace(sQuery,")NoAccent#",",'â','a'),'à','a'),'ä','a')"
                                          ",'é','e'),'è','e'),'ê','e'),'ë','e')"
                                          ",'î','i'),'ï','i')"
                                          ",'ô','o'),'ö','o')"
                                          ",'û','u'),'ù','u'),'ü','u')"
                                          ",'ç','c')");
    return sQuery;
}

bool FieldIsMoney(const QString sFieldName){
    if (sFieldName.startsWith("Prix") or
        sFieldName.startsWith("Valeur"))
        return true;
    else
        return false;
}

QString FkFilter(QSqlDatabase *db, const QString sTableName, const QString sFieldName, const QString sPageFilter, const QModelIndex &index, bool countFk){
    QString filter="";
    if (countFk){ //Test if there is possibles Fk values.
        PotaQuery query(*db);
        QString queryTest;
        if (sTableName=="Associations_détails")
            queryTest="SELECT count(*)-2 FROM Espèces";
        else if (sTableName=="Consommations")
            queryTest="SELECT count(*) FROM Espèces WHERE Conservation NOTNULL";
        else if (sTableName=="Cultures")
            queryTest="SELECT count(*) FROM Espèces";
        else if (sTableName=="Fertilisations")
            queryTest="SELECT count(*) FROM Cu_répartir_fertilisation"
                      "WHERE (DATE('now') BETWEEN C.Début_fertilisation_possible AND "
                                                 "C.Fin_fertilisation_possible)";
        else if (sTableName=="ITP")
            queryTest="SELECT count(*) FROM Espèces";
        else if (sTableName=="Récoltes")
            //queryTest="SELECT count(*) FROM Cu_répartir_récolte WHERE DATE('now') BETWEEN Début_récolte_possible AND Fin_récolte_possible";
            queryTest="SELECT count(*) FROM Cultures__à_récolter";
        else if (sTableName=="Rotations_détails")
            queryTest="SELECT (SELECT count(*) FROM Rotations)*(SELECT count(*) FROM ITP JOIN Espèces E USING(Espèce) WHERE E.Vivace ISNULL)";

        if (!queryTest.isEmpty() and query.Selec0ShowErr(queryTest).toInt()<=0)
            filter="NoFk";
    } else { //Set the filter depending of row data.
        const PotaTableModel *model=qobject_cast<const PotaTableModel *>(index.model());
        if (model) {
            if (sTableName=="Associations_détails") {
                if (sFieldName=="Famille") {
                    if (!index.siblingAtColumn(model->fieldIndex("Espèce")).data().isNull())
                        filter="Famille=(SELECT Famille FROM Espèces "
                                        "WHERE Espèce='"+StrReplace(index.siblingAtColumn(model->fieldIndex("Espèce")).data().toString(),"'","''")+"')";
                }
            } else if (sTableName=="Consommations") {
                if (sFieldName=="Destination")
                    filter="Active NOTNULL";
                else if (sFieldName=="Espèce")
                    filter="Conservation NOTNULL";
            } else if (sTableName=="Cultures") {
                if (sFieldName=="Variété")
                    filter="Espèce='"+StrReplace(index.siblingAtColumn(model->fieldIndex("Espèce")).data().toString(),"'","''")+"'";
                else if (sFieldName=="IT_plante")
                    filter="(Espèce ISNULL)OR"
                           "(Espèce='"+StrReplace(index.siblingAtColumn(model->fieldIndex("Espèce")).data().toString(),"'","''")+"')";
                else if ((sFieldName=="Espèce")and sPageFilter.contains("Vivace ISNULL"))
                    filter="(Vivace ISNULL)";
                else if ((sFieldName=="Espèce")and sPageFilter.contains("Vivace NOTNULL"))
                    filter="(Vivace NOTNULL)";
            } else if (sTableName=="Fertilisations") {
                if (sFieldName=="Culture")
                    filter="Espèce IN (SELECT Espèce FROM Cu_répartir_fertilisation "
                                      "WHERE (Espèce='"+StrReplace(index.siblingAtColumn(model->fieldIndex("Espèce")).data().toString(),"'","''")+"')AND"+
                                            "(DATE('"+iif(index.siblingAtColumn(model->fieldIndex("Date")).data(Qt::EditRole).isNull(),
                                                          "now",
                                                          index.siblingAtColumn(model->fieldIndex("Date")).data(Qt::EditRole).toString()).toString()+"') BETWEEN Début_fertilisation_possible AND Fin_fertilisation_possible))";
                else if (sFieldName=="Espèce")
                    filter="Espèce IN (SELECT Espèce FROM Cu_répartir_fertilisation "
                                      "WHERE DATE('"+iif(index.siblingAtColumn(model->fieldIndex("Date")).data(Qt::EditRole).isNull(),
                                                         "now",
                                                         index.siblingAtColumn(model->fieldIndex("Date")).data(Qt::EditRole).toString()).toString()+"') BETWEEN Début_fertilisation_possible AND Fin_fertilisation_possible)";
            } else if (sTableName=="ITP") {
                if ((sFieldName=="Espèce")and sPageFilter.contains("Vivace ISNULL"))
                    filter="(Vivace ISNULL)";
                else if ((sFieldName=="Espèce")and sPageFilter.contains("Vivace NOTNULL"))
                    filter="(Vivace NOTNULL)";
            } else if (sTableName=="Planches") {
                if (sFieldName=="Rotation")
                    filter="Type_planche='"+StrReplace(index.siblingAtColumn(model->fieldIndex("Type")).data().toString(),"'","''")+"'";
            } else if (sTableName=="Récoltes") {
                if (sFieldName=="Culture")
                    filter="Culture IN (SELECT Culture FROM Cu_répartir_récolte "
                                       "WHERE (Espèce='"+StrReplace(index.siblingAtColumn(model->fieldIndex("Espèce")).data().toString(),"'","''")+"')AND"+
                                             "(DATE('"+iif(index.siblingAtColumn(model->fieldIndex("Date")).data(Qt::EditRole).isNull(),
                                                           "now",
                                                           index.siblingAtColumn(model->fieldIndex("Date")).data(Qt::EditRole).toString()).toString()+"') BETWEEN Début_récolte_possible AND Fin_récolte_possible))";
                else if (sFieldName=="Espèce")
                    filter="Espèce IN (SELECT Espèce FROM Cu_répartir_récolte "
                                      "WHERE DATE('"+iif(index.siblingAtColumn(model->fieldIndex("Date")).data(Qt::EditRole).isNull(),
                                                         "now",
                                                         index.siblingAtColumn(model->fieldIndex("Date")).data(Qt::EditRole).toString()).toString()+"') BETWEEN Début_récolte_possible AND Fin_récolte_possible)";
            } else if (sTableName=="Rotations_détails") {
                if (sFieldName=="IT_plante")
                    filter="Espèce IN(SELECT E.Espèce FROM Espèces E WHERE E.Vivace ISNULL)";
            } else if (sTableName=="Variétés") {
                if (sFieldName=="IT_plante")
                    filter="(Espèce='"+StrReplace(index.siblingAtColumn(model->fieldIndex("Espèce")).data().toString(),"'","''")+"')OR"+
                           "(Espèce ISNULL)";
                else if ((sFieldName=="Espèce")and sPageFilter.contains("Vivace ISNULL"))
                    filter="(Vivace ISNULL)";
                else if ((sFieldName=="Espèce")and sPageFilter.contains("Vivace NOTNULL"))
                    filter="(Vivace NOTNULL)";
            }
            qDebug() << "FkFilter: " << filter;
        }
    }
    return filter;
}

QString FkSortCol(const QString sTableName,const QString sFieldName){
    if (sTableName=="Récoltes"){
        if (sFieldName=="Culture")
            return "Planche";
    } else if (sTableName=="Fertilisations"){
        if (sFieldName=="Culture")
            return "Planche";
    }
    return "";
}

QString GeneratedFielnameForDummyFilter(const QString sTableName) {
    if (sTableName=="Cultures")
        return "Etat";
    else if (sTableName=="ITP")
        return "Type_culture";
    else
        return "";
}

bool lastRow(const QString sTableName){
    if (sTableName=="Récoltes__Saisies")
        return true;
    else
        return false;
}

int NaturalSortCol(const QString sTableName){
    if (sTableName=="Params" or
        sTableName=="Rotations_détails__Tempo" or
        sTableName=="sqlite_schema" or
        sTableName=="Temp_UserSQL")
        return -1;
    else if (sTableName=="Associations_détails__Saisies")
        return 1;//Association
    else if (sTableName=="Planif_planches")
        return 6;//Date_plantation
    else if (sTableName=="Cultures__à_semer")
        return 9;//Date_semis
    else if (sTableName=="Cultures__à_semer_pep")
        return 8;//Date_semis
    else if (sTableName=="Cultures__à_semer_EP")
        return 9;//Date_semis
    else if (sTableName=="Cultures__à_planter")
        return 11;//Date_plantation
    else if (sTableName=="Cultures__à_récolter")
        return 9;//Début_récolte
    else if (sTableName=="Cultures__à_terminer")
        return 8;//Fin_récolte
    else if (sTableName=="Cultures__à_fertiliser")
        return 6;//Date_MEP
    else if (sTableName=="Cultures__analyse")
        return 1;//ITP
    else
        return 0;
}

QString NoData(const QString sTableName){
    if (sTableName=="Associations_détails__Saisies")
        return QObject::tr("Saisissez au moins 3 espèces avant de saisir des associations.");
    else if (sTableName=="Consommations__Saisies")
        return QObject::tr("Saisissez au moins une espèce marquée 'Conservation' avant de saisir des sorties de stock.");
    else if (sTableName=="Cultures")
        return QObject::tr("Saisissez au moins une espèce avant de saisir des cultures.");
    else if (sTableName=="Cultures__analyse")
        return QObject::tr("Il n'y a aucune culture annuelle terminée et significative (champ 'Terminée' différent de '...NS') pour le moment.");
    else if (sTableName=="Fertilisations__Saisies")
        return QObject::tr("Il n'y a aucune culture à fertiliser pour le moment.\n\n"
                           "Les fertilisations peuvent être saisies avant la mise en place de la culture sur la planche et jusqu'au début de récolte en modifiant les paramètres 'Ferti_avance' et 'Ferti_retard'.");
    else if (sTableName=="ITP__Tempo")
        return QObject::tr("Vous devez d'abort saisir au moins une espèce de plante (menus 'Espèces') avant de saisir des itinéraires techniques.");
    else if (sTableName=="Espèces__manquantes")
        return QObject::tr("Aucune !\nToutes les espèces marquées 'A planifier' sont suffisamment incluses dans les rotations.\n\n"
                           "L'onglet 'Planification/Cultures prévues par espèce' donne la production prévue par espèce.");
    else if (sTableName=="Planches__deficit_fert")
        return QObject::tr("Aucune planche dont le bilan de fertilisation est faible (paramètre 'Déficit_fert') sur une des 2 saisons précédente (N-1 ou N-2).");
    else if (sTableName.startsWith("Planif_"))
        return QObject::tr("Saisissez au moins une rotation de cultures (menu Assolement) pour que la planification puisse générer des cultures.");
    else if (sTableName=="Récoltes__Saisies")
        return QObject::tr("Il n'y a aucune culture à récolter pour le moment.\n\n"
                           "Les récoltes peuvent être saisies avant ou après la période de récolte en modifiant les paramètres 'C_récolte_avance' et 'C_récolte_prolongation'.");
    else if (sTableName=="Rotations_détails__Tempo")
        return QObject::tr("Vous devez d'abort saisir au moins une entête de rotation (menu 'Rotations') et un itinéraire technique de plante annuelle.");
    else if (sTableName=="Temp_FK_errors")
        return QObject::tr("Aucune erreur.")+"\n\n"+
               QObject::tr("Une erreur d'intégrité c'est par exemple l'existence de cultures de tomate alors que l'espèce 'Tomate' n'existe pas ou est orthographiée différement.");
    else
        return QObject::tr("Aucune donnée pour le moment.");
}

bool ReadOnly(QSqlDatabase *db, const QString sTableName,const QString sFieldName) {
    bool bReadOnly=false;
    PotaQuery query(*db);

    if (query.Selec0ShowErr("SELECT count() FROM pragma_table_xinfo('"+sTableName+"') WHERE name='"+sFieldName+"'")==0)
        return true;

    //Tables
    if (sTableName=="Analyses_de_sol"){
        bReadOnly=(sFieldName.startsWith("☆"));
    } else if (sTableName=="Cultures"){
        bReadOnly=(((sFieldName=="Culture")and(query.Selec0ShowErr("SELECT Valeur!='Oui' FROM Params WHERE Paramètre='C_modif_N_culture'").toBool())) or
                     sFieldName=="Type" or
                     sFieldName=="Saison" or
                     sFieldName=="Etat");

    } else if (sTableName=="Espèces"){
        bReadOnly=(sFieldName.startsWith("★"));
    } else if (sTableName=="Fertilisants"){
        bReadOnly=(sFieldName.endsWith("_disp_pc"));

    } else if (sTableName=="ITP"){
        bReadOnly=(sFieldName=="Type_culture");

    } else if (sTableName=="Params"){
        bReadOnly=(sFieldName!="Valeur");

    } else if (false) {//Views
    } else if (sTableName=="Associations_détails__Saisies"){
        bReadOnly=(sFieldName=="Effet");
    } else if (sTableName=="Consommations__Saisies"){
        bReadOnly=(sFieldName=="Stock" or
                     sFieldName=="Sorties");
    } else if (sTableName=="Cultures__non_terminées"){
        bReadOnly=!(((sFieldName=="Culture")and(query.Selec0ShowErr("SELECT Valeur='Oui' FROM Params WHERE Paramètre='C_modif_N_culture'").toBool())) or
                      sFieldName=="Planche" or
                      sFieldName=="Espèce" or
                      sFieldName=="IT_plante" or
                      sFieldName=="Variété" or
                      sFieldName=="Fournisseur" or
                      sFieldName=="D_planif" or
                      sFieldName=="Date_semis" or
                      sFieldName=="Semis_fait" or
                      sFieldName=="Date_plantation" or
                      sFieldName=="Plantation_faite" or
                      sFieldName=="Début_récolte" or
                      // sFieldName=="Récolte_com" or
                      sFieldName=="Fin_récolte" or
                      sFieldName=="Récolte_faite" or
                      sFieldName=="Terminée" or
                      sFieldName=="Longueur" or
                      sFieldName=="Nb_rangs" or
                      sFieldName=="Espacement" or
                      sFieldName=="A_faire" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__à_semer_pep"){
        bReadOnly=!(sFieldName=="Date_semis" or
                      sFieldName=="Semis_fait" or
                      sFieldName=="A_faire" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__à_semer_EP" or sTableName=="Cultures__à_semer"){
        bReadOnly=!(sFieldName=="Planche" or
                      sFieldName=="Variété" or
                      sFieldName=="Fournisseur" or
                      sFieldName=="D_planif" or
                      sFieldName=="Date_semis" or
                      sFieldName=="Semis_fait" or
                      sFieldName=="Longueur" or
                      sFieldName=="Nb_rangs" or
                      sFieldName=="Espacement" or
                      sFieldName=="A_faire" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__à_planter"){
        bReadOnly=!(sFieldName=="Planche" or
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
                      sFieldName=="A_faire" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__à_récolter"){
        bReadOnly=!(sFieldName=="Date_semis" or
                      sFieldName=="Semis_fait" or
                      sFieldName=="Date_plantation" or
                      sFieldName=="Plantation_faite" or
                      sFieldName=="Début_récolte" or
                      // sFieldName=="Récolte_com" or
                      sFieldName=="Fin_récolte" or
                      sFieldName=="Récolte_faite" or
                      sFieldName=="Terminée" or
                      sFieldName=="A_faire" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__à_terminer"){
        bReadOnly=!(sFieldName=="Date_semis" or
                      sFieldName=="Date_plantation" or
                      sFieldName=="Début_récolte" or
                      // sFieldName=="Récolte_com" or
                      sFieldName=="Fin_récolte" or
                      sFieldName=="Récolte_faite" or
                      sFieldName=="Terminée" or
                      sFieldName=="A_faire" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__A_faire"){
        bReadOnly=!(sFieldName=="Date_semis" or
                      sFieldName=="Semis_fait" or
                      sFieldName=="Date_plantation" or
                      sFieldName=="Plantation_faite" or
                      sFieldName=="Début_récolte" or
                      sFieldName=="Fin_récolte" or
                      sFieldName=="Récolte_faite" or
                      sFieldName=="Terminée" or
                      sFieldName=="A_faire" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__vivaces"){
        bReadOnly=!(sFieldName=="D_planif" or
                      sFieldName=="Début_récolte" or
                      sFieldName=="Fin_récolte" or
                      sFieldName=="Récolte_faite" or
                      sFieldName=="Terminée" or
                      sFieldName=="A_faire" or
                      sFieldName=="Notes");
    } else if (sTableName=="Destinations__conso"){
        bReadOnly=(sFieldName=="Consommation" or
                     sFieldName=="Valeur");

    } else if (sTableName=="Espèces__couverture"){
        bReadOnly=!(sFieldName=="Rendement" or
                      sFieldName=="Niveau" or
                      sFieldName=="Obj_annuel" or
                      sFieldName=="Notes");
    } else if (sTableName=="Espèces__inventaire"){
        bReadOnly=!(sFieldName=="Date_inv" or
                      sFieldName=="Inventaire" or
                      sFieldName=="Prix_kg" or
                      sFieldName=="Notes");
    } else if (sTableName.startsWith("Espèces__")){
        bReadOnly=(sFieldName.startsWith("★"));
    } else if (sTableName=="Fertilisants__inventaire"){
        bReadOnly=!(sFieldName=="Date_inv" or
                      sFieldName=="Inventaire" or
                      sFieldName=="Prix_kg" or
                      sFieldName=="Notes");
    } else if (sTableName=="Fertilisations__Saisies"){
        bReadOnly=(sFieldName=="ID" or
                     sFieldName=="N" or
                     sFieldName=="P" or
                     sFieldName=="K" or
                     sFieldName=="Planche" or
                     sFieldName=="Variété");
    } else if (sTableName=="ITP__Tempo"){
        bReadOnly=!(sFieldName=="IT_plante" or
                      sFieldName=="Espèce" or
                      sFieldName=="Type_planche" or
                      sFieldName=="S_semis" or
                      sFieldName=="S_plantation" or
                      sFieldName=="S_récolte" or
                      sFieldName=="D_récolte" or
                      sFieldName=="Décal_max" or
                      sFieldName=="Nb_rangs" or
                      sFieldName=="Espacement" or
                      sFieldName=="Nb_graines_trou" or
                      sFieldName=="Dose_semis" or
                      sFieldName=="Notes" or
                      sFieldName=="N_espèce");
    } else if (sTableName=="Notes"){
        bReadOnly=((sFieldName=="ID" and (query.Selec0ShowErr("SELECT Valeur!='Oui' FROM Params WHERE Paramètre='Notes_Modif_dates'").toBool())) or
                     (sFieldName.startsWith("Date_") and (query.Selec0ShowErr("SELECT Valeur!='Oui' FROM Params WHERE Paramètre='Notes_Modif_dates'").toBool())));
    } else if (sTableName=="Récoltes__Saisies"){
        bReadOnly=(sFieldName=="ID" or
                     sFieldName=="Planche" or
                     sFieldName=="Variété" or
                     sFieldName=="Total_réc");
    } else if (sTableName=="Rotations"){
        bReadOnly=(sFieldName=="Nb_années");
    } else if (sTableName=="Rotations_détails__Tempo"){
        bReadOnly=!(sFieldName=="Rotation" or
                      sFieldName=="Année" or
                      sFieldName=="IT_plante" or
                      sFieldName=="Pc_planches" or
                      sFieldName=="Occupation" or
                      sFieldName=="Fi_planches" or
                      sFieldName=="Décalage" or
                      sFieldName=="Notes");
    } else if (sTableName=="Variétés__inv_et_cde"){
        bReadOnly=!(sFieldName=="Qté_stock" or
                      sFieldName=="Qté_cde" or
                      sFieldName=="Fournisseur" or
                      sFieldName=="Nb_graines_g" or
                      sFieldName=="FG" or
                      sFieldName=="Notes" or
                      sFieldName=="N_espèce" or
                      sFieldName=="N_famille");

    } else {
        query.ExecShowErr("PRAGMA table_list("+sTableName+")");
        query.next();
        bReadOnly=(query.value(2).toString()!="table");
    }

    return bReadOnly;
}

QColor RowColor(QString sValue, QString sTableName){

    QColor c;
    int alpha;
    if (isDarkTheme())
        alpha=60;
    else
        alpha=80;

    if (sTableName=="Cultures__Tempo" or
        sTableName=="Cultures__à_irriguer") {
        if (isDarkTheme())
            alpha=40;
        else
            alpha=60;
        if (sValue.toInt() % 2==0)//Pair
            c=cPlanche;
        else
            return QColor();
    } else if (sTableName.startsWith("Cultures")) {
        if (sValue=="Prévue")
            c=cPrevue;
        else if (sValue=="Pépinière")
            c=cPepiniere;
        else if ((sValue=="En place")and(sTableName!="Cultures__vivaces"))
            c=cEnPlace;
        else if (sValue=="Récolte")
            c=cRecolte;
        else if (sValue=="A terminer")
            c=cATerminer;
        else if (sValue=="Terminée")
            c=cTerminee;
        else
            return QColor();
    } else if (sTableName=="Params") {
        // if (sValue=="Assolement")
        //     c=cPlanche;
        // else if (sValue=="Cultures")
        //     c=cEnPlace;
        // else if (sValue=="Consommations")
        //     c=cEspece;
        // else if (sValue=="Associations" or sValue=="Fournisseurs" or sValue=="Fertilisation")
        //     c=cBase;
        // else
            c=TableColor(sValue,"");
    } else
        return QColor();

    c.setAlpha(alpha);

    return c;
}

QString RowSummary(QString TableName, const QSqlRecord &rec){
    QString result="";
    if (rec.isNull(0))
        return "";
    else if (TableName=="Associations_détails__Saisies")
        result=rec.value(rec.indexOf("Association")).toString()+" - "+
               iif(!rec.value(rec.indexOf("Espèce")).isNull(),
                   rec.value(rec.indexOf("Espèce")).toString()+" ("+rec.value(rec.indexOf("Famille")).toString()+")",
                   rec.value(rec.indexOf("Famille")).toString()).toString()+
               iif(!rec.value(rec.indexOf("Requise")).isNull()," - Requise","").toString();
    else if (TableName=="Analyses_de_sol")
        result=rec.value(rec.indexOf("Analyse")).toString()+" - "+
               rec.value(rec.indexOf("Date")).toDate().toString("dd/MM/yyyy")+" - "+
               rec.value(rec.indexOf("Planche")).toString()+
               iif(rec.value(rec.indexOf("Organisme")).isNull(),""," - "+
                   rec.value(rec.indexOf("Organisme")).toString()).toString();
    else if (TableName=="Consommations__Saisies")
        result=rec.value(rec.indexOf("Date")).toDate().toString("dd/MM/yyyy")+" - "+
                 rec.value(rec.indexOf("Espèce")).toString()+" - "+
                 iif(!rec.value(rec.indexOf("Quantité")).isNull(),
                     rec.value(rec.indexOf("Quantité")).toString()+"kg - ","").toString()+
                 iif(!rec.value(rec.indexOf("Prix")).isNull(),
                     rec.value(rec.indexOf("Prix")).toString()+QObject::tr("€")+" - ","").toString()+
                 rec.value(rec.indexOf("Destination")).toString();
    else if (TableName=="Planif_planches")
        result=rec.value(rec.indexOf("Planche")).toString()+" - "+
               rec.value(rec.indexOf("IT_plante")).toString()+" - "+
               iif(rec.value(rec.indexOf("Date_semis")).isNull(),
                   rec.value(rec.indexOf("Date_plantation")).toDate().toString("dd/MM/yyyy"),
                   rec.value(rec.indexOf("Date_semis")).toDate().toString("dd/MM/yyyy")).toString();
    else if (TableName=="Cultures__à_semer_pep")
        result=iif(!rec.value(rec.indexOf("Cultures")).toString().contains("\n"),
                   rec.value(rec.indexOf("Cultures")).toString().trimmed(),"####").toString()+" - "+
               iif(!rec.value(rec.indexOf("Planches")).toString().contains("\n"),
                   rec.value(rec.indexOf("Planches")).toString(),"####").toString()+" - "+
               iif(rec.value(rec.indexOf("Variété")).isNull(),
                   rec.value(rec.indexOf("IT_plante")).toString(),
                   rec.value(rec.indexOf("Variété")).toString()).toString()+" - "+
               rec.value(rec.indexOf("Type")).toString()+" - "+
               rec.value(rec.indexOf("Etat")).toString()+" - "+
               "Semis "+rec.value(rec.indexOf("Date_semis")).toDate().toString("dd/MM/yyyy");
    else if (TableName=="Cultures__Tempo" or
             TableName=="Cultures__à_irriguer")
        result=rec.value(rec.indexOf("Planche")).toString()+" - "+
                 rec.value(rec.indexOf("Culture")).toString()+" - "+
                 rec.value(rec.indexOf("Variété_ou_It_plante")).toString()+" - "+
                 rec.value(rec.indexOf("Saison")).toString();
    else if (TableName=="Cultures__à_fertiliser")
        result=rec.value(rec.indexOf("Planches")).toString().replace("\n\n","/")+" - "+
               rec.value(rec.indexOf("Espèce")).toString()+" - "+
               rec.value(rec.indexOf("Type")).toString()+" - "+
               rec.value(rec.indexOf("Etat")).toString();
    else if (TableName.startsWith("Cultures"))
        result=rec.value(rec.indexOf("Culture")).toString()+" - "+
               rec.value(rec.indexOf("Planche")).toString()+" - "+
               iif(rec.value(rec.indexOf("Variété")).isNull(),
                   rec.value(rec.indexOf("IT_plante")).toString(),
                   rec.value(rec.indexOf("Variété")).toString()).toString()+" - "+
               rec.value(rec.indexOf("Type")).toString()+" - "+
               rec.value(rec.indexOf("Etat")).toString()+" - "+
               iif(!rec.value(rec.indexOf("Date_plantation")).isNull(),
                   "Plant "+rec.value(rec.indexOf("Date_plantation")).toDate().toString("dd/MM/yyyy"),
                   "Semis "+rec.value(rec.indexOf("Date_semis")).toDate().toString("dd/MM/yyyy")).toString();
    else if (TableName=="Planif_espèces")
        result=rec.value(rec.indexOf("Espèce")).toString()+" - "+
               rec.value(rec.indexOf("Nb_planches")).toString()+" "+QObject::tr("planches")+" - "+
               rec.value(rec.indexOf("Longueur")).toString()+"m";
    else if (TableName=="Planif_ilots")
        result=QObject::tr("Ilot ")+rec.value(rec.indexOf("Ilot")).toString()+" - "+
               rec.value(rec.indexOf("Nb_planches")).toString()+" "+QObject::tr("planches")+" - "+
               rec.value(rec.indexOf("Longueur")).toString()+"m";
    else if (TableName.startsWith("Destinations"))
        result=rec.value(rec.indexOf("Destination")).toString()+" - "+
               rec.value(rec.indexOf("Date_RAZ")).toDate().toString("dd/MM/yyyy")+" - "+
               str(rec.value(rec.indexOf("Consommation")).toFloat())+"kg - "+
               str(rec.value(rec.indexOf("Valeur")).toFloat())+"€";
    else if (TableName.startsWith("Espèces__inv"))
        result=rec.value(rec.indexOf("Espèce")).toString()+" - "+
               rec.value(rec.indexOf("Date_inv")).toDate().toString("dd/MM/yyyy")+" - "+
               str(rec.value(rec.indexOf("Stock")).toFloat())+"kg"+" - "+
               str(rec.value(rec.indexOf("Valeur")).toFloat())+"€";
    else if (TableName.startsWith("Espèces"))
        result=rec.value(rec.indexOf("Espèce")).toString()+" ("+
                 rec.value(rec.indexOf("Famille")).toString()+")";
    else if (TableName.startsWith("Familles"))
        result=rec.value(rec.indexOf("Famille")).toString()+" - "+
                 iif(!rec.value(rec.indexOf("Intervalle")).isNull(),
                     rec.value(rec.indexOf("Intervalle")).toString()+" "+QObject::tr("ans"),"").toString();
    else if (TableName.startsWith("Fertilisants"))
        result=rec.value(rec.indexOf("Fertilisant")).toString()+" - "+
               rec.value(rec.indexOf("Type")).toString()+" - "+
               rec.value(rec.indexOf("N")).toString()+"-"+
               rec.value(rec.indexOf("P")).toString()+"-"+
               rec.value(rec.indexOf("K")).toString();
    else if (TableName=="Fertilisations__Saisies")
        result=rec.value(rec.indexOf("Date")).toDate().toString("dd/MM/yyyy")+" - "+
                 rec.value(rec.indexOf("Espèce")).toString()+" - "+
                 rec.value(rec.indexOf("Culture")).toString()+" - "+
                 rec.value(rec.indexOf("Fertilisant")).toString()+" - "+
                 iif(!rec.value(rec.indexOf("Quantité")).isNull(),
                     rec.value(rec.indexOf("Quantité")).toString()+"kg - ","").toString()+
                 rec.value(rec.indexOf("Planche")).toString();
    else if (TableName.startsWith("Fournisseurs"))
        result=rec.value(rec.indexOf("Fournisseur")).toString()+" - "+
               rec.value(rec.indexOf("Site_web")).toString();
    else if (TableName.startsWith("ITP"))
        result=rec.value(rec.indexOf("IT_plante")).toString()+" - "+
               rec.value(rec.indexOf("Type_planche")).toString()+" - "+
               rec.value(rec.indexOf("Type_culture")).toString();
    else if (TableName=="Notes")
        result=rec.value(rec.indexOf("ID")).toString()+" - "+
               rec.value(rec.indexOf("Type")).toString()+" - "+
               rec.value(rec.indexOf("Description")).toString();
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
        result=rec.value(rec.indexOf("Date")).toDate().toString("dd/MM/yyyy")+" - "+
                 rec.value(rec.indexOf("Espèce")).toString()+" - "+
                 rec.value(rec.indexOf("Culture")).toString()+" - "+
                 iif(!rec.value(rec.indexOf("Quantité")).isNull(),
                     rec.value(rec.indexOf("Quantité")).toString()+"kg - ","").toString()+
                 rec.value(rec.indexOf("Planche")).toString();
    else if (TableName=="Rotations")
        result=rec.value(rec.indexOf("Rotation")).toString()+" - "+
               rec.value(rec.indexOf("Type_planche")).toString()+" - "+
               iif(!rec.value(rec.indexOf("Nb_années")).isNull(),
                   rec.value(rec.indexOf("Nb_années")).toString()+" "+QObject::tr("ans"),"").toString();
    else if (TableName=="Rotations_détails__Tempo")
        result=rec.value(rec.indexOf("Rotation")).toString()+" - "+
               QObject::tr("Année ")+rec.value(rec.indexOf("Année")).toString()+" - "+
               rec.value(rec.indexOf("IT_plante")).toString()+
               iif(rec.value(rec.indexOf("Occupation")).toString()=="R",
                   " - "+QString::number(rec.value(rec.indexOf("Pc_planches")).toDouble()/100*
                                         rec.value(rec.indexOf("Nb_rangs")).toDouble())+QObject::tr(" rang(s)"),"").toString()+
               iif(rec.value(rec.indexOf("Occupation")).toString()=="E",
                   " - "+QObject::tr("Espacement")+" "+QString::number(round(rec.value(rec.indexOf("Espacement")).toDouble()/
                                                          rec.value(rec.indexOf("Pc_planches")).toDouble()*100)),"").toString();
    // else if (TableName=="Successions_par_planche")
    //     result=rec.value(rec.indexOf("Planche")).toString()+" : "+
    //            rec.value(rec.indexOf("ITP_en_place")).toString()+" ("+
    //            rec.value(rec.indexOf("Libre_le")).toString()+") -> "+
    //            rec.value(rec.indexOf("ITP_à_venir")).toString()+" ("+
    //            rec.value(rec.indexOf("En_place_le")).toString()+")";
    else if (TableName=="Variétés__inv_et_cde")
        result=rec.value(rec.indexOf("Variété")).toString()+" - "+
               iif(!rec.value(rec.indexOf("Qté_nécess")).isNull(),QObject::tr("Nécessaire ")+rec.value(rec.indexOf("Qté_nécess")).toString()+"g - ","").toString()+
               iif(!rec.value(rec.indexOf("Qté_stock")).isNull(),QObject::tr("Stock ")+rec.value(rec.indexOf("Qté_stock")).toString()+"g - ","").toString()+
               iif(!rec.value(rec.indexOf("Qté_cde")).isNull(),QObject::tr("Cde ")+rec.value(rec.indexOf("Qté_cde")).toString()+"g - ","").toString()+
               iif(!rec.value(rec.indexOf("Qté_manquante")).isNull(),QObject::tr("Manquant ")+rec.value(rec.indexOf("Qté_manquante")).toString()+"g","").toString()+" - "+
               rec.value(rec.indexOf("Fournisseur")).toString();

    if (result=="") {
        if (!rec.value(0).toString().contains("\n"))
            result=rec.value(0).toString();
        if (rec.count()>1 and !rec.value(1).toString().contains("\n"))
            result+=" - "+rec.value(1).toString();
        if (rec.count()>2 and !rec.value(2).toString().contains("\n"))
            result+=" - "+rec.value(2).toString();
    }

    result=StrReplace(result,"-  -","-");
    if (StrLast(result,3)==" - ")
        result=StrFirst(result,result.length()-3);

    return result;
}

QColor TableColor(QString sTName,QString sFName)
{
    //Use sTName.toUpper().startsWith(...) to apply color to the corresponding views.

    //Color by field name for all tables or views.
    if (sFName.endsWith("_sol"))
        return cBase;
    else if (sFName.startsWith("Association")
        // or sFName=="Effet"
        )
        return cBase;
    else if (sFName.startsWith("Culture") or
        (sFName.endsWith("_MEP")and sFName!="S_MEP") or
        sFName=="Etats" or
        sFName.startsWith("Nb_cu") or
        sFName=="Min_semis" or sFName=="Max_semis" or
        sFName=="Min_plantation" or sFName=="Max_plantation" or
        sFName=="Min_recolte" or sFName=="Max_recolte" or
        sFName=="Qté_réc_moy")
        return cCulture;
    else if (sFName.startsWith("Espèce") or
        sFName.endsWith("_esp") or
        sFName=="A_planifier" or
        sFName=="Défavorable" or
        sFName=="Favorable" or
        sFName=="FG" or
        sFName=="Groupe" or
        sFName=="Irrig_espèce" or
        sFName=="Niveau" or
        sFName=="N_espèce" or
        sFName=="Obj_annuel" or
        sFName=="Taille" or
        sFName=="Rendement")
        return cEspece;
    else if (sFName=="Famille" or
             sFName=="N_famille")
        return cFamille;
    else if (sFName=="Analyse" or
             sFName=="Fertilisant" or
             sFName=="Apports_NPK" or
             sFName.startsWith("Fert") or
             sFName.endsWith("_fert") or
             sFName=="N_manq" or
             sFName=="P_manq" or
             sFName=="K_manq")
        return cFertilisant;
    else if (sFName=="IT_plante" or
             sFName.startsWith("ITP") or
             (sFName=="Type_planche" and !sTName.toUpper().startsWith("ROTATIONS")) or
             (sFName=="Type_culture" and !sTName.toUpper().startsWith("ROTATIONS")) or
             sFName=="S_semis" or
             sFName=="S_plantation" or
             (sFName=="S_récolte" and !sTName.toUpper().startsWith("VARIÉTÉS")) or
             (sFName=="D_récolte" and !sTName.toUpper().startsWith("VARIÉTÉS"))or
             sFName=="Décal_max" or
             (sFName=="Nb_rangs" and !sTName.startsWith("CULTURES",Qt::CaseInsensitive)) or
             (sFName=="Espacement" and !sTName.startsWith("CULTURES",Qt::CaseInsensitive)) or
             sFName=="Nb_graines_trou"  or
             (sFName=="Dose_semis" and !sTName.toUpper().startsWith("ESPÈCES")) or
             sFName=="N_IT_plante" )
        return cITP;
    else if (sFName.startsWith("Planche") or
             sFName=="Ilot" or
             sFName=="Largeur" or
             sFName=="N_Planche")
        return cPlanche;
    else if (sFName=="Variété" or
             (sFName=="S_récolte" and sTName.toUpper().startsWith("VARIÉTÉS")) or
             (sFName=="D_récolte" and sTName.toUpper().startsWith("VARIÉTÉS"))or
             sFName=="Qté_stock")
        return cVariete;
    else if ((sFName=="Valeur" and sTName.toUpper().startsWith("PARAMS")) or
             sFName=="Année_à_planifier")
        return cParam;
    else if (sFName.startsWith("TEMPO") or
             sFName.startsWith("Prod_") or
             sFName.startsWith("Couv_"))
        return QColor();

    //Color by table name.
    else if (sTName.toUpper().startsWith("ASSOCIATIONS"))
        return cBase;
    else if (sTName.toUpper().startsWith("CONSOMMATIONS"))
        return cEspece;
    else if (sTName.toUpper().startsWith("CULTURES"))
        return cCulture;
    else if (sTName.toUpper().startsWith("DESTINATIONS"))
        return cBase;
    else if (sTName.toUpper().startsWith("ESPÈCES"))
        return cEspece;
    else if (sTName.toUpper().startsWith("FAMILLES"))
        return cFamille;
    else if (sTName.toUpper().startsWith("FERTILISA") or
             sTName.toUpper().startsWith("ANALYSES_DE_SOL"))
        return cFertilisant;
    else if (sTName.toUpper().startsWith("FOURNISSEURS"))
        return cBase;
    else if (sTName.toUpper().startsWith("ITP"))
        return cITP;
    else if (sTName.toUpper().startsWith("PLANCHES") or
             sTName.toUpper().startsWith("ASSOLEMENT"))
        return cPlanche;
    else if (sTName.toUpper().startsWith("RÉCOLTES"))
        return cCulture;
    else if (sTName.toUpper().startsWith("ROTATIONS"))
        return cRotation;
    // else if (sTName.toUpper().startsWith("TYPES_PLANCHE"))
    //     return cBase;
    else if (sTName.toUpper().startsWith("VARIÉTÉS"))
        return cVariete;
    else
        return QColor();
}

QPixmap TablePixmap(QString sTName, QString text) {
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent); // Fond transparent
    QColor c=QColor();
    QColor cBrush=QColor();
    QColor cPen=QColor();
    c=TableColor(sTName,"");
    // if (sTName.startsWith("Cultures"))
    //     c=cCulture;
    cBrush=c;
    cBrush.setAlpha(100);
    if (!isDarkTheme())
        cPen=QColor("#000000");
    else
        cPen=QColor("#ffffff");
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(cBrush);
    // if (bTable)
    //     painter.setPen(cPen);
    // else
        painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 16, 16);

    if (!text.isEmpty()) {
        painter.setPen(cPen);
        QFont font( "Arial", 7); //, QFont::Bold
        painter.setFont(font);

        QRect rect(0, 0, 16, 16);
        painter.drawText(rect, Qt::AlignCenter, text);
    }
    return pixmap;
}

QString ToolTipField(const QString sTableName,const QString sFieldName, const QString sDataType) {
    QString sToolTip="";

    //Champs présents dans plusieurs tables avec des significations différentes.

    // if (sTableName=="Apports"){
    //     if (sFieldName=="Type")
    //         sToolTip=QObject::tr("Liste de choix paramétrable (paramétre 'Combo_Apports_Type').");
    // } else
    if (sTableName.startsWith("Associations")){
        if (sFieldName=="Association")
            sToolTip=QObject::tr("Nom de l'association.\n"
                                 "Terminer par ' +' pour signifier que l'association est favorable.\n"
                                 "Terminer par ' -' pour signifier que l'association est défavorable.");
        // else if (sFieldName=="Effet")
        //     sToolTip=QObject::tr("Effet bénéfique ou néfaste de l'association sur la croissance et\n"
        //                          "la production de l'espèce.\n"
        //                          "Liste de choix paramétrable (paramétre 'Combo_Associations_détails_Effet').");
    } else if (sTableName=="Analyses_de_sol"){
        const char* teneur="Teneur en %1 du prélèvement (g/kg).";
        const char* teneurMol="Teneur en %1 du prélèvement (cmol+/kg).\n"
                              "cmol+ : centimole de charge positive.";
        const char* valRef="Valeurs de référence: %1.";
        if (sFieldName=="Sable_grossier")
            sToolTip=QObject::tr("Pourcentage de %1.").arg(QObject::tr("sable grossier")+" (0,2 - 2 mm)");
        else if (sFieldName=="Sable_fin")
            sToolTip=QObject::tr("Pourcentage de %1.").arg(QObject::tr("sable fin")+" (0,05 - 0,2 mm)");
        else if (sFieldName=="Limon_grossier")
            sToolTip=QObject::tr("Pourcentage de %1.").arg(QObject::tr("limon grossier")+" (0,2 - 2 mm)")+"\n"+
                     QObject::tr("Favorise la battance");
        else if (sFieldName=="Limon_fin")
            sToolTip=QObject::tr("Pourcentage de %1.").arg(QObject::tr("limon fin")+" (0,05 - 0,2 mm)")+"\n"+
                     QObject::tr("Favorise la battance");
        else if (sFieldName=="Argile")
            sToolTip=QObject::tr("Pourcentage de %1.").arg(QObject::tr("argile")+" (0,05 - 0,2 mm)");
        else if (sFieldName=="IB")
            sToolTip=QObject::tr("Indice de battance:\n"
                                 "Risque de désagrégation du sol sous l'action de la pluie et de formation d'une croûte superficielle lors du ressuyage.")+"\n"+
                     QObject::tr(valRef).arg("IB < 1,4");
        else if (sFieldName=="pH")
            sToolTip=QObject::tr("Potentiel hydrogène, à l'eau, du prélèvement.")+"\n"+
                     QObject::tr(valRef).arg("6,5 < pH < 7,5");
        else if (sFieldName=="MO")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Matière organique").toLower())+"\n"+
                     QObject::tr(valRef).arg("15 < MO < 20");
        else if (sFieldName=="CEC")
            sToolTip=QObject::tr("Capacité d’Echange Cationique du prélèvement (cmol+/kg).\n"
                                 "Mesure le pouvoir fixateur de cations du sol.")+"\n"+
                     QObject::tr(valRef).arg("10 < CEC < 20");
        else if (sFieldName=="Cations")
            sToolTip=QObject::tr(teneurMol).arg(QObject::tr("Cations").toLower())+"\n"+
                     QObject::tr(valRef).arg("Cations >= CEC");
        else if (sFieldName=="N")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Azote").toLower()+" (Kjeldhal)")+"\n"+
                     QObject::tr(valRef).arg("0,9 < N < 1,1");
        else if (sFieldName=="P")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Phosphore").toLower())+"\n"+
                     QObject::tr(valRef).arg("0,08 < P2O5 < 0,12");
        else if (sFieldName=="K")
            sToolTip=QObject::tr("Potassium")+"\n"+
                     QObject::tr(teneur).arg(QObject::tr("Potasse").toLower())+"\n"+
                     QObject::tr(valRef).arg("0,12 < K2O < 0,15");
        else if (sFieldName=="C")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Carbone").toLower())+"\n"+
                     QObject::tr(valRef).arg("9 < C < 11");
        else if (sFieldName=="CN")
            sToolTip=QObject::tr("Rapport Carbone/Azote.")+"\n"+
                     QObject::tr(valRef).arg("8 < C/N < 11");
        else if (sFieldName=="Ca")
            sToolTip=QObject::tr("Calcium")+"\n"+
                     QObject::tr(teneur).arg(QObject::tr("Chaux").toLower())+"\n"+
                     QObject::tr(valRef).arg("3,7 < CaO < 3,9");
        else if (sFieldName=="Mg")
            sToolTip=QObject::tr("Magnésium")+"\n"+
                     QObject::tr(teneur).arg(QObject::tr("Magnésie").toLower())+"\n"+
                     QObject::tr(valRef).arg("0,09 < MgO < 0,14");
        else if (sFieldName=="Na")
            sToolTip=QObject::tr("Sodium")+"\n"+
                     QObject::tr(teneur).arg(QObject::tr("Oxyde de sodium").toLower())+"\n"+
                     QObject::tr(valRef).arg("0.02 < Na2O < 0,24");
        else if (sFieldName.startsWith("☆"))
            sToolTip=QObject::tr("Teneur qualitative du sol.");
    } else if (sTableName=="Consommations" or sTableName.startsWith("Consommations__")){
        if (sFieldName=="Date")
            sToolTip=QObject::tr("Date de sortie de stock.\n"
                                 "Laisser vide pour avoir automatiquement la date du jour.");
        else if (sFieldName=="Quantité")
            sToolTip=QObject::tr("Quantité sortie de stock (kg).");
        else if (sFieldName=="Stock")
            sToolTip=QObject::tr("Quantité restante en stock (kg), à cette date.");
        else if (sFieldName=="Sorties")
            sToolTip=QObject::tr("Somme à cette date des sorties (kg) pour cette espèce et cette destination, depuis Date_RAZ de la destination (incluse).");
    } else if (sTableName=="Destinations"){
        if (sFieldName=="Type")
            sToolTip=QObject::tr("Liste de choix paramétrable (paramétre 'Combo_Destinations_Type').");
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
        else if (sFieldName=="Date_semis" or sFieldName=="Date_plantation" or sFieldName=="Début_récolte" or sFieldName=="Fin_récolte") {
            sToolTip=QObject::tr("Date réelle ou prévue.\n"
                                 "Laisser la date prévue même si l'opération ne sera pas faite, pour que le type de culture soit correctement calculé.");
            if (sFieldName=="Date_semis")
                sToolTip+="\n"+QObject::tr("Vide pour plant acheté.");
            else if (sFieldName=="Date_plantation")
                sToolTip+="\n"+QObject::tr("Vide pour semis en place et engrais vert.");
            if (sFieldName=="Début_récolte" or sFieldName=="Fin_récolte") {
                sToolTip+="\n"+QObject::tr("Va être mis à jour lors de la saisie des récoltes.");
                if (sFieldName=="Début_récolte")
                    sToolTip+="\n"+QObject::tr("Vide pour compagne et engrais vert.");
                else if (sFieldName=="Fin_récolte")
                    sToolTip+="\n"+QObject::tr("Date de destruction compagne et engrais vert.");
            }
        } else if (sFieldName=="Irrig_planche")
            sToolTip=QObject::tr("Irrigation en place sur la planche.");
        else if (sFieldName=="Irrig_espèce")
            sToolTip=QObject::tr("Irrigation nécessaire pour l'espèce cultivée.");
    } else if (sTableName.startsWith("Destinations")){
        if (sFieldName=="Valeur")
            sToolTip=QObject::tr("Valeur des sorties de stock (€) depuis Date_RAZ (incluse).");

    } else if (sTableName=="Espèces__inventaire" or sTableName=="Fertilisants__inventaire"){
        if (sFieldName=="Stock")
            sToolTip=QObject::tr("Quantité totale en stock (kg).");
        else if (sFieldName=="Valeur")
            sToolTip=QObject::tr("Valeur théorique du stock (€).");

    } else if (sTableName.startsWith("Espèces")){
        const char* valRef="Valeurs de référence: %1.";
        if (sFieldName=="Densité")
            sToolTip=QObject::tr("Nombre de plants par m² si l'espèce est seule sur la planche.\n"
                                 "Les besoins NPK pour une culture tiennent compte de la densité réelle de la culture.");
        else if (sFieldName=="Dose_semis")
            sToolTip=QObject::tr("Quantité de semence (g/m²).")+"\n"+QObject::tr("Valeur par défaut pour les itinéraires techniques.");
        else if (sFieldName=="Nb_graines_g")
            sToolTip=QObject::tr("Nombre de graines par gramme.")+"\n"+QObject::tr("Valeur par défaut pour les variétés.");
        else if (sFieldName=="Irrig")
            sToolTip=QObject::tr("Irrigation nécessaire (paramétre 'Combo_Espèces_Irrig').");
        else if (sFieldName=="N")
            sToolTip=QObject::tr("Besoin en %1 (g/m²).\n"
                                 "Les besoins NPK pour une culture tiennent compte de la densité réelle de la culture.").arg("Azote").toLower();
        else if (sFieldName=="★N")
            sToolTip=QObject::tr("Besoin qualitatif en %1.").arg("Azote").toLower()+"\n"+
                     QObject::tr(valRef).arg("5 < N < 10");
        else if (sFieldName=="P")
            sToolTip=QObject::tr("Besoin en %1 (g/m²).\n"
                                 "Les besoins NPK pour une culture tiennent compte de la densité réelle de la culture.").arg("Phosphore").toLower();
        else if (sFieldName=="★P")
            sToolTip=QObject::tr("Besoin qualitatif en %1.").arg("Phosphore").toLower()+"\n"+
                     QObject::tr(valRef).arg("2.5 < P < 5");
        else if (sFieldName=="K")
            sToolTip=QObject::tr("Besoin en %1 (g/m²).\n"
                                 "Les besoins NPK pour une culture tiennent compte de la densité réelle de la culture.").arg("Potassium").toLower();
        else if (sFieldName=="★K")
            sToolTip=QObject::tr("Besoin qualitatif en %1.").arg("Potassium").toLower()+"\n"+
                     QObject::tr(valRef).arg("7 < K < 12");

    } else if (sTableName=="Familles"){
        if (sFieldName=="Intervalle")
            sToolTip=QObject::tr("Nombre d'années nécessaires entre 2 cultures consécutives sur la même planche.");

    } else if (sTableName=="Fertilisants"){
        const char* teneur="Teneur en %1 du fertilisant (%).";
        const char* coefficient="Coefficient de disponibilité (%) pour l'élément %1 en conditions optimales:\n"
                                "- sol neutre, bien structuré, bien pourvu en matière organique ;\n"
                                "- bonne humidité ;\n"
                                "- bonne activité biologique ;\n"
                                "- apport bien intégré.";
        const char* dispo="Teneur en %1 disponible pour les plantes (% du poids de fertilisant) en conditions optimales:\n"
                                "- sol neutre, bien structuré, bien pourvu en matière organique ;\n"
                                "- bonne humidité ;\n"
                                "- bonne activité biologique ;\n"
                                "- apport bien intégré.\n\n"
                                "Diponible=teneur x coefficient / 100\n"
                                "Un autre coefficient relatif au sol et aux pratiques culturales sera appliqué pour estimer les quantités de fertilisant nécessaires (paramètre 'Ferti_coef_...').";
        const char* valRef="Valeurs de référence: %1.";
        if (sFieldName=="Type")
            sToolTip=QObject::tr("Liste de choix paramétrable (paramétre 'Combo_Fertilisants_Type').");
        else if (sFieldName=="pH")
            sToolTip=QObject::tr("Potentiel hydrogène.\n"
                                 "pH < 7 : acide\n"
                                 "pH > 7 : basique\n\n"
                                 "La disponibilité des nutriments pour les plantes dépend directement du pH du sol.");
        else if (sFieldName=="N")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Azote").toLower());
        else if (sFieldName=="N_coef")
            sToolTip=QObject::tr(coefficient).arg(QObject::tr("Azote"));
        else if (sFieldName=="N_disp_pc")
            sToolTip=QObject::tr(dispo).arg(QObject::tr("Azote"))+"\n"+
                     QObject::tr(valRef).arg("0.2 < N disp < 1");
        else if (sFieldName=="P")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Phosphore").toLower());
        else if (sFieldName=="P_coef")
            sToolTip=QObject::tr(coefficient).arg(QObject::tr("Phosphore"));
        else if (sFieldName=="P_disp_pc")
            sToolTip=QObject::tr(dispo).arg(QObject::tr("Phosphore"))+"\n"+
                     QObject::tr(valRef).arg("0.1 < P disp < 0.5");
        else if (sFieldName=="K")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Potassium").toLower());
        else if (sFieldName=="K_coef")
            sToolTip=QObject::tr(coefficient).arg(QObject::tr("Potassium"));
        else if (sFieldName=="K_disp_pc")
            sToolTip=QObject::tr(dispo).arg(QObject::tr("Potassium"))+"\n"+
                     QObject::tr(valRef).arg("0.4 < K disp < 2");
        else if (sFieldName=="Ca")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Calcium").toLower());
        else if (sFieldName=="Ca_coef")
            sToolTip=QObject::tr(coefficient).arg(QObject::tr("Calcium"));
        else if (sFieldName=="Ca_disp_pc")
            sToolTip=QObject::tr(dispo).arg(QObject::tr("Calcium"))+"\n"+
                     QObject::tr(valRef).arg("0.4 < Ca disp < 2");
        else if (sFieldName=="Fe")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Fer").toLower());
        else if (sFieldName=="Fe_coef")
            sToolTip=QObject::tr(coefficient).arg(QObject::tr("Fer"));
        else if (sFieldName=="Fe_disp_pc")
            sToolTip=QObject::tr(dispo).arg(QObject::tr("Fer"))+"\n"+
                     QObject::tr(valRef).arg("0.02 < Fe disp < 0.1");
        else if (sFieldName=="Mg")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Magnésium").toLower());
        else if (sFieldName=="Mg_coef")
            sToolTip=QObject::tr(coefficient).arg(QObject::tr("Magnésium"));
        else if (sFieldName=="Mg_disp_pc")
            sToolTip=QObject::tr(dispo).arg(QObject::tr("Magnésium"))+"\n"+
                     QObject::tr(valRef).arg("0.2 < Mg disp < 1");
        else if (sFieldName=="Na")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Sodium").toLower());
        else if (sFieldName=="Na_coef")
            sToolTip=QObject::tr(coefficient).arg(QObject::tr("Sodium"));
        else if (sFieldName=="Na_disp_pc")
            sToolTip=QObject::tr(dispo).arg(QObject::tr("Sodium"))+"\n"+
                     QObject::tr(valRef).arg("0.04 < Na disp < 0.2");
        else if (sFieldName=="S")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Souffre").toLower());
        else if (sFieldName=="S_coef")
            sToolTip=QObject::tr(coefficient).arg(QObject::tr("Souffre"));
        else if (sFieldName=="S_disp_pc")
            sToolTip=QObject::tr(dispo).arg(QObject::tr("Souffre"))+"\n"+
                     QObject::tr(valRef).arg("0.04 < S disp < 0.2");
        else if (sFieldName=="Si")
            sToolTip=QObject::tr(teneur).arg(QObject::tr("Silicium").toLower());
        else if (sFieldName=="Si_coef")
            sToolTip=QObject::tr(coefficient).arg(QObject::tr("Silicium"));
        else if (sFieldName=="Si_disp_pc")
            sToolTip=QObject::tr(dispo).arg(QObject::tr("Silicium"))+"\n"+
                     QObject::tr(valRef).arg("0.1 < Si disp < 0.5");

    } else if (sTableName.startsWith("Fertilisations")){
        const char* Apport="Apport en %1 sur la planche (g).\n"
                           "Quantité x %2 disp fertilisant x Coefficient de disponibilité relatif au sol (paramètre 'Ferti_coef_%2')\n"
                           "ATTENTION valeur calculée lors de la saisie et non mise à jour si le coefficient ou la teneur du fertilisant est modifié.";
        if (sFieldName=="Date")
            sToolTip=QObject::tr("Date de fertilisation.\n"
                                 "Laisser vide pour avoir automatiquement la date du jour.");
        else if (sFieldName=="Espèce")
            sToolTip=QObject::tr(  "Les espèces possibles pour saisir une fertilisation sont celles pour qui il existe des cultures à venir ou en place avant récolte.\n"
                                   "Voir infobulle 'Culture'.");
        else if (sFieldName=="Culture")
            sToolTip=QObject::tr(  "Les cultures possibles pour saisir une fertilisation sont celles qui:\n"
                                   "- Date de mise en place (semis en place ou plantation) <= date du jour plus avance de fertilisation (paramètre 'Ferti_avance')\n"
                                   "- Début de récolte (Début_récolte) >= date du jour moins délai de saisie de fertilisation (paramètre 'Ferti_retard')\n\n"
                                   "Le paramètre 'Ferti_avance' permet de saisir des fertilisations avant la date de mise en place de la culture.\n"
                                   "Le paramètre 'Ferti_retard' permet de saisir les fertilisation après le début de récolte (Début_récolte).");
        else if (sFieldName=="Quantité")
            sToolTip=QObject::tr("Quantité apportée sur la planche (kg).");
        else if (sFieldName=="Planche·s")
            sToolTip=QObject::tr(  "Pour répartir la quantité apporté sur plusieurs cultures de l'espèce sélectionnée,\n"
                                   "saisir le début du nom des planches concernées\n"
                                   "ou saisir '*' pour répartir sur toutes les cultures possibles pour l'espèce sélectionnée.\n"
                                   "Si pas d'espèce sélectionnée, la répartition se fait sur TOUTES les cultures possible.\n"
                                   "Vide: pas de répartition.\n\n"
                                   "La répartition se fait au prorata des surfaces de planche.\n"
                                   "Attention, la liste des cultures possibles (en période de fertilisation) dépend des paramètres 'Ferti_avance' et 'Ferti_retard'.");
        else if (sFieldName=="N")
            sToolTip=QObject::tr(Apport).arg(QObject::tr("Azote").toLower()).arg("N");
    } else if (sTableName=="Fournisseurs"){
        if (sFieldName=="Priorité")
            sToolTip=QObject::tr("Chez qui commander en priorité.");
        else if (sFieldName=="Type")
            sToolTip=QObject::tr("Liste de choix paramétrable (paramétre 'Combo_Fournisseurs_Type').");
    } else if (sTableName=="ITP" or sTableName.startsWith("ITP__")){
        if (sFieldName=="Type_planche")
            sToolTip=QObject::tr("Liste de choix paramétrable (paramétre 'Combo_Planches_Type').");
        else if (sFieldName=="Dose_semis")
            sToolTip=QObject::tr("Quantité de semence (g/m²).")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire SI ESPACEMENT=0.");
        else if (sFieldName.startsWith("J_"))
            sToolTip=QObject::tr("Nombre de jours pour...")+"\n"+
                     QObject::tr("La date prévue pour les cultures sera le début de période.");
        else if (sFieldName=="Nb_rangs")
            sToolTip=QObject::tr("Nombre de rangs cultivés sur une planche.")+
                                 "\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.")+
                                 "\n"+QObject::tr("Valeur par défaut pour les cultures.");

    } else if (sTableName=="Notes"){
        if (sFieldName=="Type")
            sToolTip=QObject::tr("Type de note (paramétre 'Combo_Notes_Type').");
        else if (sFieldName=="Notes")
            sToolTip=QObject::tr("Texte de la note (Ctrl + N pour éditer).");
    } else if (sTableName=="Planches"){
        if (sFieldName=="Type")
            sToolTip=QObject::tr("Liste de choix paramétrable (paramétre 'Combo_Planches_Type').");
        else if (sFieldName=="Longueur")
            sToolTip=QObject::tr("Longueur de la planche (m).");
        else if (sFieldName=="Irrig")
            sToolTip=QObject::tr("Irrigation actuellement installée sur la planche (paramétre 'Combo_Planches_Irrig').");
        else if (sFieldName=="Rotation")
            sToolTip=QObject::tr("Rotation de cultures qui utilise cette planche.");

    } else if (sTableName=="Rotations"){
        if (sFieldName=="Type")
            sToolTip=QObject::tr("Liste de choix paramétrable (paramétre 'Combo_Planches_Type').");
        else if (sFieldName=="Nb_années")
            sToolTip=QObject::tr(   "Automatique, en fonction des détails de la rotation.\n"
                                    "3 à 6 ans en général.");

    } else if (sTableName=="Rotations_détails__Tempo"){
        if (sFieldName=="Année")
            sToolTip=QObject::tr(   "Année de culture dans la rotation.\n"
                                    "Nombre entier entre 1 et 5 si la rotation est sur 5 ans.");
        else if (sFieldName=="Pc_planches" or sFieldName=="Occupation")
            sToolTip=QObject::tr(   "Pourcentage d'occupation des planches de l'UdP.\n"
                                    "Exemple: planche de 10m de long occupée à 50%\n"
                                    "- la culture occupera 10m avec un rang sur 2 si 'Occupation' commence par 'R'.\n"
                                    "- la culture occupera 10m avec un espacement entre plants 2 fois plus grand\n"
                                    "si 'Occupation' commence par 'E'.\n"
                                    "- la culture occupera 5m dans les autres cas (pas d'association de plante).\n\n"
                                    "Pour créer des associations de plante, utilisez toute la longueur de planche :\n"
                                    "'Occupation' commence par 'R' (rang) ou 'E' (espacement).");
        else if (sFieldName=="Fi_planches")
            sToolTip=QObject::tr(  "Filtrage des planches de l'UdP sur le dernier caractère du nom de planche.\n"
                                   "Exemple: l'ilot AA contient une UdP (1) de 4 planches (A, B, C et D)\n"
                                   "Les planches sont: AA1A, AA1B, AA1C et AA1D\n"
                                   "Fi_planches=AC -> seule les planches AA1A et AA1C seront occupées par les cultures.");
        else if (sFieldName=="Décalage")
            sToolTip=QObject::tr(  "Décalage (en semaines) de la culture, par rapport aux semaines au plus tôt de l'ITP,\n"
                                   "dans la limte de 'Décal_max' sur l'ITP."
                                   "Si vide, l'affichage montre les périodes possibles de semis, plantation et récolte,\n"
                                   "et les dates de cultures seront au plus tôt.");
        else if (sFieldName=="S_MEP")
            sToolTip=QObject::tr(  "Semaine de mise en place de la culture sur la planche.\n"
                                   "* indique un chevauchement avec la culture précédente (pas encore récoltée).\n"
                                   "- indique que la culture précédente est récoltée depuis %1 mois (automne) à %2 mois (printemps).").arg("4").arg("8");

    } else if (sTableName.startsWith("Récoltes")){
        if (sFieldName=="Date")
            sToolTip=QObject::tr("Date de récolte.\n"
                                 "Laisser vide pour avoir automatiquement la date de fin de récolte de la culture, ou la date du jour si la date de fin de récolte est dans le futur.");
        else if (sFieldName=="Espèce")
            sToolTip=QObject::tr(  "Les espèces possibles pour saisir une récolte sont celles pour qui il existe des cultures en cours de récolte.\n"
                                   "Voir infobulle 'Culture'.");
        else if (sFieldName=="Culture")
            sToolTip=QObject::tr(  "Les cultures possibles pour saisir une récolte sont celles qui:\n"
                                   "- ont des dates de début et fin de récolte (réelles ou prévues)\n"
                                   "- Début récolte <= date du jour plus avance de saisie de récolte (paramètre 'C_récolte_avance')\n"
                                   "- Fin récolte >= date du jour moins délai de saisie de récolte (paramètre 'C_récolte_prolongation')\n\n"
                                   "Le paramètre 'C_récolte_avance' permet de saisir des récoltes faites avant leur date prévue.\n"
                                   "Le paramètre 'C_récolte_prolongation' permet de saisir les récoltes après que celles-ci aient été faites.");
        else if (sFieldName=="Quantité")
            sToolTip=QObject::tr("Quantité récoltée sur la planche (kg).");
        else if (sFieldName=="Planche·s")
            sToolTip=QObject::tr(  "Pour répartir la quantité récoltée sur plusieurs cultures de l'espèce sélectionnée,\n"
                                   "saisir le début du nom des planches concernées\n"
                                   "ou saisir '*' pour répartir sur toutes les cultures possibles.\n"
                                   "Vide: pas de répartition.\n\n"
                                   "La répartition se fait au prorata des longueurs de planche.\n"
                                   "Attention, la liste des cultures possibles dépend des paramètres 'C_récolte_avance' et 'C_récolte_prolongation'.");
        else if (sFieldName=="Total_réc")
             sToolTip=QObject::tr("Quantité totale déja récoltée (kg), à cette date pour cette culture.");

    // } else if (sTableName=="Types_planche"){
    //     if (sFieldName=="Type")
    //         sToolTip=QObject::tr("Par exemple 'Serre', 'Extérieur'...");

    } else if (sTableName=="Variétés"){
        if (sFieldName=="S_récolte")
            sToolTip=QObject::tr("N° de semaine (1 à 52).")+"\n"+
                     QObject::tr("Laisser vide pour que la valeur saisie sur l'itinéraire technique soit utilisée.");
        else if (sFieldName=="D_récolte")
            sToolTip=QObject::tr("Durée de récolte en semaines (1 à 52).")+"\n"+
                     QObject::tr("Laisser vide pour que la valeur saisie sur l'itinéraire technique soit utilisée.");
        else if (sFieldName=="PJ")
            sToolTip=QObject::tr(  "Période de juvénilité: Nombre d'années avant la 1ère récolte.\n"
                                   "0 pour les plantes à récolter moins de 12 mois après la date de plantation.\n"
                                   "1 pour les bisannuelles.");
    } else if (sTableName=="Variétés__inv_et_cde"){
        if (sFieldName=="Qté_nécess")
            sToolTip=QObject::tr("Semence nécessaire (g) pour les cultures non encore semées.");
    } else if (sTableName=="Variétés__cde_plants"){
        if (sFieldName=="Qté_nécess")
            sToolTip=QObject::tr("Nb de plants nécessaires pour les cultures non encore plantées.");
    }

    //Champs ayant la même signification partout.
    if (sToolTip==""){

        //Champs existant dans plusieurs tables
        if (sFieldName=="")
            sToolTip=QObject::tr("");
        else if (sFieldName=="Densité_pc")
            sToolTip=QObject::tr("Densité réelle, relative à la densité de référence noté sur l'espèce.");
        else if (sFieldName=="Espacement")
            sToolTip=QObject::tr("Espacement entre 2 plants dans un rang de culture (cm), après éclairecicement éventuel.")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        else if (sFieldName=="IT_plante")
            sToolTip=QObject::tr("Itinéraire technique: une espèce de plante et une manière de la cultiver.\n"
                                 "Sur une variété, vous pouvez sélectionner parmis les ITP de l'espèce concernée ou parmis les ITP qui n'ont pas d'espèce.\n"
                                 "Dans un plan de rotation, vous pouvez sélectionner parmis les ITP qui ont une espèce (annuelle ou vivace).\n"
                                 "Pour une culture, vous pouvez sélectionner parmis les ITP de l'espèce cultivée ou parmis les ITP qui n'ont pas d'espèce.");
        else if (sFieldName=="Notes")
            sToolTip=QObject::tr("Notes pour cette ligne.\n"
                                 "Ctrl+N pour passer en édition multi-lignes.\n\n"
                                 "Format Markdown: https://www.markdownguide.org\n"
                                 "# Titre\n"
                                 "## Sous-titre\n"
                                 "### ...\n"
                                 "**gras**\n"
                                 "*italique*\n"
                                 "...");
        else if (sFieldName=="Planches_Ilots")
            sToolTip=QObject::tr(   "Regroupement de planches\n"
                                    "(voir le paramètre 'Ilot_nb_car').");
        else if (sFieldName=="Prod_possible")
            sToolTip=QObject::tr("Production possible pour les cultures prévues (kg).\n"
                                 "Rendement x surface");
        else if (sFieldName=="Qté_réc")
            sToolTip=QObject::tr("Quantité totale récoltée (kg).");
        else if (sFieldName=="Rotation")
            sToolTip=QObject::tr("Ensemble de cultures qui se succèdent sur un ensemble de planches.");
        else if (sFieldName=="Variété_ou_It_plante")
            sToolTip=QObject::tr("Variété, à défaut l'itinéraire technique.");
        //Association
        else if (sFieldName=="Groupe")
            sToolTip=QObject::tr("Espèces dont le nom commence par.");
        else if (sFieldName=="Requise")
            sToolTip=QObject::tr("L'espèce doit obligatoirement être présente\n"
                                 "pour que l'association fasse son effet.");
        //Cultures
        else if (sFieldName=="Culture")
            sToolTip=QObject::tr("Numéro unique de la culture\n"
                                 "(pas de remise à zéro tous les ans).");
        else if (sFieldName=="Saison")
            sToolTip=QObject::tr("Annuelle: année de mise en place sur la planche de culture (date de plantation ou de semis en place)\n"
                                 "Vivace: année de début récolte, à défaut année de mise en place.");
        else if (sFieldName=="D_planif")
            sToolTip=QObject::tr("Date de calcul des dates de semis, plantation et récolte (planification).\n"
                                 "La planification des cultures est faite en fonction de l'ITP et éventuellement de la variété pour les dates de récolte.\n"
                                 "Les dates sont calées sur les débuts de période de l'ITP/variété.\n\n"
                                 "Effacer 'D_planif' et valider pour recalculer les dates non renseignées sur la culture.\n"
                                 "Pour planifier la culture sur une saison particulière, saisissez dans 'D_planif' l'année sur 4 chiffres (ex 2025).");
        else if (sFieldName=="Semis_fait")
            sToolTip=QObject::tr("'x' ou commence par 'x' : le semis est réussi.\n"
                                 "Autre valeur : le semis est en cours.");
        else if (sFieldName=="Plantation_faite")
            sToolTip=QObject::tr("'x' ou commence par 'x' : la plantation est réussie.\n"
                                 "Autre valeur : la plantation est en cours.");
        else if (sFieldName=="Récolte_faite")
            sToolTip=QObject::tr("'x' ou commence par 'x' : la récolte est terminée.\n"
                                 "Autre valeur : la récolte est en cours.")+"\n"+
                     QObject::tr("Va être mis à jour lors de la saisie des récoltes.");
        else if (sFieldName=="Terminée")
            sToolTip=QObject::tr("'v' : à récolter aussi les années suivantes (vivace).\n"
                                 "Autre valeur : la planche est fermée et disponible pour la culture suivante.\n\n"
                                 "Commence par 'v' : culture de vivace terminée.\n"
                                 "Le 'v' est automatiquement ajouté lors de la saisie d'une récolte si l'espèce est 'Vivace'.\n\n"
                                 "Ajoutez 'NS' (non significative) à la fin si la culture ne doit pas être prise en compte dans les analyses.");
        else if (sFieldName=="Rendement_C")
            sToolTip=QObject::tr("Production de cette culture (kg/m²).");
        //Destinations
        else if (sFieldName=="Date_RAZ")
            sToolTip=QObject::tr("Date à partir de laquelle sont prises en compte les sorties de stock.");
        else if (sFieldName=="Consommation")
            sToolTip=QObject::tr("Somme des sorties de stock (kg) depuis Date_RAZ (incluse).");
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
            sToolTip=QObject::tr("Quantité en stock (kg) à la date d'inventaire.");
        else if (sFieldName=="Date_inv")
            sToolTip=QObject::tr("Date d'inventaire.");
        else if (sFieldName=="Entrées")
            sToolTip=QObject::tr("Quantité récolté totale depuis 'Date_inv' (kg).");
        else if (sFieldName=="Sorties")
            sToolTip=QObject::tr("Quantité consommée totale depuis 'Date_inv' (kg)");
        else if (sFieldName=="Vivace")
            sToolTip=QObject::tr("Espèce pouvant être récoltée tout les ans, après la période de juvénilité (noté sur la variété).\n"
                                 "Pendant la période de juvénilité, le champ 'Type' des cultures est identique aux cultures annuelles (semis, plant).\n"
                                 "A partir de la 1ère récolte, le champ 'Type' des cultures devient 'Vivace' (le champ 'Terminée'='v').");
        else if (sFieldName=="Conservation")
            sToolTip=QObject::tr("Espèce pouvant se conserver. Elles apparaissent dans l'onglet Stock.");
        else if (sFieldName=="A_planifier")
            sToolTip=QObject::tr("Espèce à cultiver l'année prochaine.");
        else if (sFieldName=="Rendement")
            sToolTip=QObject::tr("Production théorique moyenne de l'espèce (kg/m²).");
        else if (sFieldName=="Obj_annuel")
            sToolTip=QObject::tr("Objectif de production annuel (kg).");
        //ITP
        else if (sFieldName=="Type_culture")
            sToolTip=QObject::tr("Automatique, en fonction des débuts de période de semis, plantation et récolte.");
        else if (sFieldName=="S_semis")
            sToolTip=QObject::tr("N° de semaine (1 à 52) du début de la période de semis.")+"\n"+
                     QObject::tr("Vide pour plant acheté.");
        else if (sFieldName=="S_plantation")
            sToolTip=QObject::tr("N° de semaine (1 à 52) du début de la période de plantation.")+"\n"+
                     QObject::tr("Vide pour semis en place et engrais vert.");
        else if (sFieldName=="S_récolte")
            sToolTip=QObject::tr("N° de semaine (1 à 52) du début de la période de récolte.")+"\n"+
                     QObject::tr("Si 'D_récolte' est vide (compagne, engrais vert), semaine de destruction de la culture.");
        else if (sFieldName=="D_récolte")
            sToolTip=QObject::tr("Durée de récolte en semaines (1 à 52).")+"\n"+
                     QObject::tr("Vide pour compagne et engrais vert.");
        else if (sFieldName=="Décal_max")
            sToolTip=QObject::tr("Décalage maximum possible (en semaines) des semis, plantation et récolte, pour que l'ITP reste viable.");
        else if (sFieldName=="Nb_graines_trou")
            sToolTip=QObject::tr("Nombre de graines par trou.\n"
                                 "Si semis en ligne continue avec éclaircissement, saisir 4 graines pour un éclaircissement de 75%.")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        //Maintenance (plusieurs vues
        else if (sFieldName=="PK_field_name")
            sToolTip=QObject::tr("Champ clé et unique permettant de retrouver l'enregistrement.");
        else if (sFieldName=="FK_field_name")
            sToolTip=QObject::tr("Champ clé étrangère contenant une clé incorrecte.");
        else if (sFieldName=="Field_count")
            sToolTip=QObject::tr("Nombre de champs (colonnes) dans la table (ou vue).");
        else if (sFieldName=="Trigger_count")
            sToolTip=QObject::tr("Nombre de triggers de la table (ou vue).\n"
                                 "Un trigger est un programme SQL qui se déclenche automatiquement lors de la modification des données.");
        else if (sFieldName=="Use_count")
            sToolTip=QObject::tr("Nombre d'appels à cette table (ou vue) par d'autres tables (ou vues).");
        else if (sFieldName=="Rec_count")
            sToolTip=QObject::tr("Nombre d'enregistrements (lignes) dans la table.");
        else if (sFieldName=="Sql")
            sToolTip=QObject::tr("Code SQL de création de la table, vue ou trigger.");
        //Planches
        else if (sFieldName=="Largeur")
            sToolTip=QObject::tr("Largeur de la planche (m).");
        else if (sFieldName=="Planches_influencées")
            sToolTip=QObject::tr("Planches pouvant être influencées par une culture de vivace sur la planche courante (associations).\n"
                                 "Noms de planche séparés par des virgules (%1).").arg(",")+"\n\n"+
                     QObject::tr("%1 pour coller une sélection de cellules dans la cellule courante (en une seule ligne).").arg("Ctrl+Alt+V");
        //Rotations
        else if (sFieldName=="Année_1")
            sToolTip=QObject::tr("Année de début de la rotation.")+"\n"+QObject::tr("Cette valeur ne doit pas être changée d'une année sur l'autre tant que la rotation se poursuit.");
        //Variétés
        else if (sFieldName=="Nb_graines_g")
            sToolTip=QObject::tr("Nombre de graines par gramme.")+"\n"+QObject::tr("Utilisé pour calculer le poids de semence nécessaire.");
        else if (sFieldName=="Qté_stock")
            sToolTip=QObject::tr("Semence en stock (g).");
        else if (sFieldName=="Qté_cde")
            sToolTip=QObject::tr("Semence commandée (g).")+"\n"+QObject::tr("A réception, mettre à 0 et ajouter la quantité à la quantité en stock.");

        //Views
        else if (sFieldName=="Analyse_sol")
            sToolTip=QObject::tr("Teneur NPK (g/kg) et interprétation de l'analyse de sol de la planche.");
        else if (sFieldName=="Année_à_planifier")
            sToolTip=QObject::tr("Paramétre 'Année_culture' + 1.");
        else if (sFieldName=="Apports_NPK")
            sToolTip=QObject::tr("Sommes des fertilisations (Azote - Phosphore - Potassium) déjà faites sur les planches (g).");
        else if (sFieldName=="Besoins_NPK")
            sToolTip=QObject::tr("Besoins en Azote - Phosphore - Potassium, pour les planches (g).\n"
                                 "Besoin de l'espèce x densité réelle x surface de planche.");
        else if (sFieldName=="Couv_prév_pc")
            sToolTip=QObject::tr(  "Quantitées prévues (rendement x surface) par rapport aux objectifs.");
        else if (sFieldName=="Couv_réc_pc")
            sToolTip=QObject::tr(  "Quantitées récoltées par rapport aux quantitées prévues (rendement x surface).");
        else if (sFieldName.startsWith("Couv"))
            sToolTip=QObject::tr("Pourcentage de couverture des objectifs.");
        else if (sFieldName=="Cu_à_venir")
            sToolTip=QObject::tr(   "Cultures prévues mais pas encore en place sur leur planche.\n"
                                    "Sont incluses les cultures déjà semées en pépinière.");
        else if (sFieldName=="Cu_en_place")
            sToolTip=QObject::tr(   "Cultures en place sur leur planche: semées en place ou plantées.\n"
                                    "Ne sont pas incluses les cultures semées en pépinière mais non plantées.");
        else if (sFieldName=="Cu_non_commencées")
            sToolTip=QObject::tr(   "Cultures prévues mais ni semées ni plantées.");
         else if (sFieldName=="Date_MEP")
             sToolTip=QObject::tr("Date de mise en place sur la planche de culture: date de semis (en place) ou de plantation.");
        // else if (sFieldName=="Libre_le")
        //     sToolTip=QObject::tr("Plus grande date de fin de récolte parmis les cultures en place.");
        else if (sFieldName=="Fert_pc")
            sToolTip=QObject::tr(  "Pourcentage de fertilisation déjà effectué pour l'élément N, P ou K le plus déficitaire.");
        else if (sFieldName=="Nb_cu_AV")
            sToolTip=QObject::tr(  "Nombre de cultures à venir (AV) sur la planche.\n"
                                   "Ces cultures ne sont pour le moment ni semées (semis en place) ni plantées.");
        else if (sFieldName=="Nb_cu_EP")
            sToolTip=QObject::tr(  "Nombre de cultures en place (EP) sur la planche.\n"
                                   "Ces cultures ne sont pas terminées.");
        else if (sFieldName=="Nb_cu_NT")
            sToolTip=QObject::tr(  "Nombre de cultures NON terminées utilisant cet itinéraire technique.");
        else if (sFieldName=="Nb_cu_T")
            sToolTip=QObject::tr(  "Nombre de cultures terminées utilisant cet itinéraire technique.");
        else if (sFieldName=="Nb_cu_TS")
            sToolTip=QObject::tr(  "Nombre de cultures terminées et significatives (champ 'Terminée' différent de '...NS' et de 'v...').");
        else if (sFieldName=="Nb_cu_S")
            sToolTip=QObject::tr(  "Nombre de cultures significatives (champ 'Terminée' différent de '...NS').");
        // else if (sFieldName.startsWith("Prod_N"))
        //     sToolTip=QObject::tr(  "Somme des productions (réelles ou possibles) des cultures de la saison (kg).")+"\n"+
        //              QObject::tr(  "Saison=année de mise en place (plantation ou semis en place), la récolte peut se terminer l'année suivante.")+"\n"+
        //              QObject::tr(  "Production réelle (somme des récoltes) pour les cultures terminées.")+"\n"+
        //              QObject::tr(  "Production possible (rendement de l'espèce x surface de la culture) pour les cultures non terminées.");
        else if (sFieldName=="Pl_libre_le")
            sToolTip=QObject::tr(   "Plus grande date de fin de récolte des cultures précédentes sur ces planches.");
        else if (sFieldName=="Qté_manquante")
            sToolTip=QObject::tr(   "Qté nécessaire moins Qté en stock moins Qté commandée.");
        else if (sFieldName=="Surface")
            sToolTip=QObject::tr("En m²");
        //Fertilisation
        else if (sFieldName.startsWith("★") and sFieldName.endsWith("_esp"))
            sToolTip=QObject::tr("Besoin qualitatif de l'espèce.");
        else if (sFieldName.startsWith("☆") and sFieldName.endsWith("_sol"))
            sToolTip=QObject::tr("Teneur qualitative du sol.");
        else if (sFieldName==("N_manq"))
            sToolTip=QObject::tr("Quantité %1 manquante (g).").arg(QObject::tr("Azote").toLower());
        else if (sFieldName==("P_manq"))
            sToolTip=QObject::tr("Quantité %1 manquante (g).").arg(QObject::tr("Phosphore").toLower());
        else if (sFieldName==("K_manq"))
            sToolTip=QObject::tr("Quantité %1 manquante (g).").arg(QObject::tr("Potassium").toLower());
    }

    if (!sDataType.isEmpty()){
        if (sDataType.startsWith("BOOL")){
            sToolTip+=iif(sToolTip=="","","\n\n").toString()+
                      QObject::tr( "Champ Oui/Non (BOOL))\n"
                                   "Vide=Non (ou faux)");
            if (!sToolTip.contains("'x' ") and !sToolTip.contains("'v' "))
                sToolTip+=QObject::tr( "\n"
                                       "Saisie quelconque=Oui (ou vrai).\n"
                                       "'X', 'Oui', 'Non', '0' ou n'importe quoi veulent dire OUI.");
        } else {
            sToolTip+=iif(sToolTip=="","","\n\n").toString()+
                      QObject::tr("Format : %1").arg(sDataType);
            if (sDataType=="REAL" or sDataType.startsWith("INT"))
                sToolTip+="\n\n"+
                          QObject::tr("Calculatrice: saisir une formule (ex '=1+1') puis Entrée pour avoir le résultat.");

        }
    }
    if (sFieldName.startsWith("TEMPO"))
        return "";
    else
        return sFieldName+" ("+QObject::tr("table ")+sTableName+")\n\n"+
               sToolTip;
}

QString ToolTipTable(const QString sTableName) {
    QString sToolTip="";

    if (sTableName=="")
        sToolTip="";

    //Tables
    // else if (sTableName=="Apports")
    //     sToolTip=QObject::tr("Amendement devant être apporté à une planche avant d'y cultiver une espèce de plante.");
    // else
    if (sTableName=="Associations_détails__Saisies")
        sToolTip=QObject::tr("Associations d'espèces ou de familles de plante.\n"
                             "Permettent de détecter les associations entre :\n"
                             "- annuelles des plans de rotation\n"
                             "- annuelles planifiées la prochaine saison et vivaces en place *\n"
                             "- annuelles et vivaces en place *\n"
                             "*: en fonction de l'influence des planches entre elles (onglet 'Planches').");
    else if (sTableName=="Associations__présentes")
        sToolTip=QObject::tr("Associations de cultures entre les annuelles et les vivaces en place\n"
                             "sur la même planche ou sur une planche proche ('Planches_influencées' dans onglet 'Planches').");
    else if (sTableName=="Consommations__Saisies")
        sToolTip=QObject::tr("Quantités de légume sorties du stock.");
    else if (sTableName.startsWith("Cultures__à")or
             (sTableName=="Cultures__non_terminées")or
             (sTableName=="Cultures")) {
        if (sTableName=="Cultures__non_terminées")
            sToolTip=QObject::tr("Cultures dont le champ 'Terminé' est vide.")+"\n\n";
        else if (sTableName=="Cultures__à_semer")
            sToolTip=QObject::tr("Cultures non terminées dont le champ 'Semis_fait' est vide et dont la date de semis est proche (paramètre 'C_horizon_semis') ou passée.")+"\n\n";
        else if (sTableName=="Cultures__à_semer_pep")
            sToolTip=QObject::tr("Cultures non terminées, semis en pépinière ('Date_plantation' non vide), dont le champ 'Semis_fait' est vide et dont la date de semis est proche (paramètre 'C_horizon_semis') ou passée.")+"\n\n";
        else if (sTableName=="Cultures__à_semer_EP")
            sToolTip=QObject::tr("Cultures non terminées, à semer en place ('Date_plantation' vide), dont le champ 'Semis_fait' est vide et dont la date de semis est proche (paramètre 'C_horizon_semis') ou passée.")+"\n\n";
        else if (sTableName=="Cultures__à_planter")
            sToolTip=QObject::tr("Cultures non terminées, déjà semées (ou à partir de plants), dont le champ 'Plantation_faite' est vide et dont la date de plantation est proche (paramètre 'C_horizon_plantation') ou passée.")+"\n\n";
        else if (sTableName=="Cultures__à_irriguer")
            sToolTip=QObject::tr(  "Planches sans irrigation et ayant des cultures qui nécessitent irrigation\n"
                                   "et planches avec irrigation en place mais pas de culture en ayant besoin (paramètres 'C_Irrig_avant_MEP' et 'C_Irrig_après_MEP').\n"
                                   "Cultures annuelles non terminées et cultures de vivace avant leur 1ère récolte (champ 'Terminée' vide).")+"\n\n";
        else if (sTableName=="Cultures__à_récolter")
            sToolTip=QObject::tr("Cultures non terminées, déjà semées/plantées, dont le champ 'Récolte_faite' ne commence pas par 'x' et dont la date de début de récolte est proche (paramètre 'C_horizon_récolte') ou passée.")+"\n\n";
        else if (sTableName=="Cultures__à_terminer")
            sToolTip=QObject::tr("Cultures non terminées, déjà semées/plantées/récoltées (paramètre 'C_horizon_terminer').\n"
                                 "Pour compagne et engrais vert ('Début_récolte' vide), sont incluses les cultures dont la date de destruction est proche ou passée.")+"\n\n";
        else if (sTableName=="Cultures__A_faire")
            sToolTip=QObject::tr("Cultures dont le champ 'A faire' n'est pas vide.\n"
                                 "Les cultures dont vous effacez le champ 'A faire' restent visibles jusqu'à la fermeture de l'onglet.")+"\n\n";
        else if (sTableName=="Cultures__à_fertiliser")
            sToolTip=QObject::tr("Bilan de fertilisation pour les cultures prévues (paramètre 'C_horizon_fertiliser') ou en place.")+"\n\n";
        sToolTip+=QObject::tr("Une 'culture' c'est une plante (variété+itinéraire technique) sur une planche.\nSi la même plante est présente sur plusieurs planches, il y a une culture (numérotée) par planche.");
    }
    else if (sTableName=="Destinations__conso")
        sToolTip=QObject::tr("Destinations des légumes sortis du stock.");
    else if (sTableName=="Espèces__a" or sTableName=="Espèces__v")
        sToolTip=QObject::tr("Plante pouvant se reproduire et engendrer une descendance viable et féconde.\n"
                             "Permet d'enregistrer:\n"
                             "- Les caractéristiques des graines (pour les annuelles).\n"
                             "- L'amendement nécessaire.\n"
                             "- Les objectifs de production.\n"
                             "- Des informations utiles pour les cultures.");
    else if (sTableName=="Familles")
        sToolTip=QObject::tr("Espèces ayant une certaine proximité phylogénétique.\n"
                             "Permet de d'enregistrer l'intervale de temps minimum entre 2 cultures d'une même famille.");
    else if (sTableName=="Fertilisants")
        sToolTip=QObject::tr("Engrais, ammendements et paillages pour la fertilisation des planches de culture.\nLes engrais vert ne sont pas à saisir ici, ils sont gérés comme les autres cultures.");
    else if (sTableName=="Fertilisations__Saisies")
        sToolTip=QObject::tr("Quantités de fertilisant apportés par culture.");
    else if (sTableName=="Fournisseurs")
        sToolTip=QObject::tr("Fournisseurs des semences.");
    else if (sTableName=="ITP__Tempo")
        sToolTip=QObject::tr(   "Itinéraires techniques de plantes.\n"
                                "Une ITP c'est Une espèce de plante cultivée d'une certaine manière:\n"
                                "Hative ou tardive, sous serre ou en extérieur, etc.\n"
                                "Chaque ITP à une période de semis, de plantation et de récolte.\n"
                                "Pour chaque culture, il faudra prendre une variété adaptée à l'itinéraire technique voulu.");
    else if (sTableName=="Notes")
        sToolTip=QObject::tr("Notes utilisateurs.");
    else if (sTableName=="Planches")
        sToolTip=QObject::tr(   "Planches de cultures.\n"
                                "Les 1ers caractères (nb paramétrable) indiquent l'ilot dans lequel se trouve la planche.\n"
                                "Les planches doivent être affectées à des rotations pour que la planification puisse générer les cultures.\n"
                                "Si vous créez vos cultures manuellement, il n'est pas nécessaire d'affecter les planches.\n"
                                "Le nom d'une planche doit avoir la forme <Ilot><unité de production><planche>,\n"
                                "par exemple 'Sud1A' pour la planche A de l'unité de production 1 de l'ilot 'Sud'.");
    else if (sTableName=="Planif_associations")
        sToolTip=QObject::tr("Associations de cultures entre les annuelles planifiées la prochaine saison et\n"
                             "les vivaces en place sur la même planche ou sur une planche proche ('Planches_influencées' dans onglet 'Planches').");
    else if (sTableName=="Rotations")
        sToolTip=QObject::tr(   "Ensemble d'ITP qui vont se succéder sur un groupe de planches (ilot).\n"
                                "Ici vous ne saisissez que l'entête avec un type de planche et une année de départ.\n"
                                "La liste des ITP qui constituent la rotation est à saisir dans l'onglet 'Détails des rotations'.");
    else if (sTableName=="Rotations_détails__Tempo")
        sToolTip=QObject::tr(   "Les ITP (donc les espèces cultivées) vont être déplacés chaque année\n"
                                "sur une nouvelle unité de production (UdP).\n"
                                "Si Pc_planches=100%, les cultures occuperont la totalité des planches de l'UdP.\n"
                                "Si Fi_planches est vide, les cultures occupperont toutes les planches de l'UdP.\n"
                                "Si Pc_planches=50% et Fi_planches=A, les cultures occuperont\n"
                                "la moitié de chaque planche se terminant par A.");
    else if (sTableName=="Récoltes__Saisies")
        sToolTip=QObject::tr("Quantités de légume récoltées par culture.");
    // else if (sTableName=="Types_planche")
    //     sToolTip=QObject::tr("Permet de caractériser les planches de culture.")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
    else if (sTableName=="Variétés")
        sToolTip=QObject::tr("Variété cultivée (cultivar) qui a été sélectionné et choisi pour certaines de ses caractéristiques.\nPermet de gérer les stocks et commandes de semence.");

    if (!sToolTip.isEmpty()) {
        sToolTip+="\n\n"+QObject::tr("Ces informations sont enregistrées dans une table (T).")+"\n"+
                  QObject::tr("Il est possible d'importer dans une table des données extérieures à %1.").arg("Potaléger");
        if (sTableName.contains("__"))
            return QObject::tr("Nom de la table : ")+sTableName.first(sTableName.indexOf("__"))+"\n"+sToolTip;
        else
            return QObject::tr("Nom de la table : ")+sTableName+"\n"+sToolTip;
    }

    if (sTableName=="Params")
        return QObject::tr("Paramètres pour la base de données (BDD) courante.");

    //Views
    if (sTableName=="Planif_planches")
        sToolTip=QObject::tr(  "Cultures qui vont être créées lors de la prochaîne planification.\n"
                               "La variété choisie pour chaque culture est celle dont\n"
                               "la quantité de semence en stock est la plus importante.\n" // todo
                               "La liste peut être limitée à certaines planches (paramètre 'Planifier_planches').")+"\n"+
                   QObject::tr(  "Liste non directement modifiable, déduite des 'Rotations'.");
    else if (sTableName=="Cultures__inc_dates")
        sToolTip=QObject::tr(  "Cultures non terminées ou terminées significatives (champ 'Terminée' différent de '...NS')\n"
                               "dont une date (semis, plantation, début récolte, fin récolte) est trop éloignée de la période prévue sur l'ITP (paramètre 'Tolérance_...').");
    else if (sTableName=="Cultures__analyse")
        sToolTip=QObject::tr(  "Cultures annuelles récoltées, terminées et significatives (champ 'Terminée' différent de '...NS').\n"
                               "Les engrais vert, compagnes et vivaces ne sont pas analysées.");
    else if (sTableName=="Cultures__vivaces")
        sToolTip=QObject::tr(  "Cultures vivaces (champ 'Terminée' commence par 'v' ou espèce vivace).\n"
                               "Pendant la période de semis/plantation, la culture d'une vivace est gérée comme une annuelle (champ 'Terminée' vide).");
    else if (sTableName=="Cultures__Tempo")
        sToolTip=QObject::tr(  "Planches et leurs cultures semées ou plantées dans la période.");
    else if (sTableName=="Espèces__couverture")
        sToolTip=QObject::tr(   "Comparatif saison par saison des objectifs de production pour les espèces annuelles.\n"
                                "L'objectif annuel n'est valable que pour la saison courante.\n"
                                "Prévue=rendement de l'espèce x surface de culture.\n\n"
                                "Attention, c'est la date de mise en place (plantation ou semis en place) qui détermine la saison d'une culture.\n"
                                "Des récoltes année N+1 peuvent donc compter pour la saison N si la mise en place est en fin d'année N.");
    else if (sTableName=="Espèces__inventaire")
        sToolTip=QObject::tr(  "Inventaire des légumes produits/consommés pour chaque espèce dont le champ 'Conservation' est non vide.");
    else if (sTableName=="ITP__analyse_a")
        sToolTip=QObject::tr(  "Pour chaque itinéraire technique (%1), les cultures significatives (champ 'Terminée' différent de '...NS') sont analysées.").arg(QObject::tr("annuelles"));
    else if (sTableName=="ITP__analyse_v")
        sToolTip=QObject::tr(  "Pour chaque itinéraire technique (%1), les cultures significatives (champ 'Terminée' différent de '...NS') sont analysées.").arg(QObject::tr("vivaces"));
    else if (sTableName=="Espèces__manquantes")
        sToolTip=QObject::tr(   "Espèces marquées 'A planifier' et qui ne sont\n"
                                "pourtant pas suffisamment incluses dans les rotations.\n"
                                "La planification ne générera donc pas assez de cultures pour ces espèces.");
    else if (sTableName=="Planif_espèces" or sTableName=="Planif_ilots")
        sToolTip=QObject::tr(  "Espèces pour lesquelles des cultures vont être créées\n"
                               "lors de la prochaîne planification.")+"\n"+
                   QObject::tr(  "Liste non directement modifiable, déduite des 'Rotations'.");
    else if (sTableName=="Planches_Ilots")
        sToolTip=QObject::tr(  "Les ilots sont des regroupements de planches.\n"
                               "Ils ne sont pas saisis mais déduits des planches saisies.\n"
                               "Le débuts du nom des planches indique leur ilot (voir le paramètre 'Ilot_nb_car').");
    else if (sTableName=="Planches__bilan_fert")
        sToolTip=QObject::tr(  "Bilan de fertilisation par planche sur une saison.\n"
                               "Besoins=somme des besoins des cultures annuelles de la saison.\n"
                               "Apports=somme des fertilisations des cultures annuelles de la saison.");
    else if (sTableName=="Planches__deficit_fert")
        sToolTip=QObject::tr(  "Planches dont le bilan de fertilisation est faible (paramètre 'Déficit_fert') sur une des 2 saisons précédente (N-1 ou N-2).");
    else if (sTableName=="sqlite_schema")
        sToolTip=QObject::tr("Liste des requêtes SQL qui ont permi de créer les tables et les vues de la BDD ouverte.")+"\n"+
                 QObject::tr("Ces requêtes peuvent être utilisées pour créer une BDD vide de même structure\n"
                             "dans un autre logiciel de gestion de BDD.");
    else if (sTableName=="Temp_FK_errors")
        sToolTip=QObject::tr("Liste des erreurs d'intégrité.")+"\n"+
                 QObject::tr("Une erreur d'intégrité c'est par exemple l'existence de cultures de tomate alors que l'espèce 'Tomate' n'existe pas ou est orthographiée différement.");
    else if (sTableName=="Temp_Table_list")
        sToolTip=QObject::tr("Liste des tables réellement existantes dans la BDD ouverte.\n"
                             "Toutes les informations que vous saisissez sont enregistrées dans ces tables.")+"\n"+
                 QObject::tr("La mise à jour du schéma de BDD peut modifier cette liste.");
    else if (sTableName=="Temp_View_list")
        sToolTip=QObject::tr("Liste des vues réellement existantes dans la BDD ouverte.\n"
                             "Les vues présentent des informations calculées à partir de celles de plusieurs tables.")+"\n"+
                 QObject::tr("La mise à jour du schéma de BDD peut modifier cette liste.");
    else if (sTableName=="Variétés__inv_et_cde")
        sToolTip=QObject::tr(  "Inventaire des semences pour chaque variété de plante.\n"
                               "Cultures prises en compte : 'Semis pépinière', 'Semis en place', 'Compagne' et 'Engrais vert'.");
    else if (sTableName=="Variétés__cde_plants")
        sToolTip=QObject::tr(  "Plants achetés nécessaires pour chaque variété de plante.\n"
                               "Cultures prises pris en compte : 'Plant'.");

    if (!sToolTip.isEmpty())
        sToolTip+="\n\n";
    if (!sTableName.startsWith("Temp_") and !sTableName.startsWith("sqlite_"))
        sToolTip+=QObject::tr("Ces informations constituent une vue et sont calculées à partir de plusieurs tables.")+"\n"+
                  QObject::tr("Il n'est pas possible d'importer dans une vue des données extérieures à %1.").arg("Potaléger")+"\n";
    sToolTip+=QObject::tr("Nom de la vue : ")+sTableName;


    return sToolTip;
}

