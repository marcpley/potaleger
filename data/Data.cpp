#include "data/Data.h"
#include "PotaUtils.h"
#include "mainwindow.h"
#include "qcolor.h"
#include <QSqlTableModel>
#include <QtSql/QSqlQueryModel>
#include <QObject>
#include "potawidget.h"

bool AcceptReturns(const QString sFieldName) {
    return sFieldName=="Notes" or
           sFieldName.startsWith("N_")or
           sFieldName.startsWith("Adresse")or
           sFieldName.startsWith("Conflit_")or
           sFieldName.startsWith("Description");
}

int DefColWidth(QSqlDatabase *db, const QString sTableName,const QString sFieldName) {
    QString sType = DataType(db, sTableName,sFieldName);
    if (sType=="DATE")
        return 100;
    else if (sType.startsWith("BOOL"))
        return 50;
    else if (sType.startsWith("INT"))
        return 40;
    else if (sType=="REAL")
        return 50;

    else if (sTableName.startsWith("ITP") and (sFieldName.startsWith("Déb_") or sFieldName.startsWith("Fin_")))
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

QString FkFilter(QSqlDatabase *db, const QString sTableName,const QString sFieldName, const QModelIndex &index,bool countFk){
    QString filter="";
    if (countFk){ //Test if there is possibles Fk values.
        PotaQuery query(*db);
        QString queryTest;
        if (sTableName=="Consommations")
            queryTest="SELECT count(*) FROM Espèces WHERE Conservation NOTNULL";
        else if (sTableName=="ITP")
            queryTest="SELECT count(*) FROM Espèces";
        else if (sTableName=="Récoltes")
            queryTest="SELECT count(*) FROM Repartir_Recolte_sur('*',NULL)";
        if (sTableName=="Rotations_détails")
            queryTest="SELECT count(*) FROM Rotations";

        if (!queryTest.isEmpty() and query.Selec0ShowErr(queryTest)==0)
            filter="NoFk";
    } else { //Set the filter depending of row data.
        const PotaTableModel *model = qobject_cast<const PotaTableModel *>(index.model());
        if (model) {
            if (sTableName=="Cultures") {
                if (sFieldName=="Variété")
                    filter="Espèce=(SELECT Espèce FROM ITP I WHERE I.IT_plante='"+
                           StrReplace(index.siblingAtColumn(model->fieldIndex("IT_plante")).data().toString(),"'","''")+
                           "')";
                // else if (sFieldName=="IT_plante")
                //     filter="Espèce IN (SELECT E.Espèce FROM Espèces E WHERE E.A_planifier NOTNULL)";
            } else if (sTableName=="Récoltes") {
                if (sFieldName=="Culture")
                    filter="Culture IN (SELECT Culture FROM Repartir_Recolte_sur('*','"+StrReplace(index.siblingAtColumn(model->fieldIndex("Espèce")).data().toString(),"'","''")+"'))";
                else if (sFieldName=="Espèce")
                    filter="Espèce IN (SELECT Espèce FROM Repartir_Recolte_sur('*',NULL))";
                qDebug() << "FkFilter: " << filter;
            } else if (sTableName=="Consommations") {
                if (sFieldName=="Destination")
                    filter="Active NOTNULL";
                else if (sFieldName=="Espèce")
                    filter="Conservation NOTNULL";
                qDebug() << "FkFilter: " << filter;
            }
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
    else if (sTableName=="Cultures__Tempo")
        return 1;//ITP
    else if (sTableName=="IT_rotations_manquants")
        return 1;//ITP
    else
        return 0;
}

QString NoData(const QString sTableName){
    if (sTableName=="Consommations")
        return QObject::tr("Saisissez au moins une espèce marquée 'Conservation' avant de saisir des sorties de stock.");
    else if (sTableName=="Cult_planif")
        return QObject::tr("Saisissez au moins une rotation de cultures (menu Assolement) pour que la planification puisse générer des cultures.");
    else if (sTableName=="ITP")
        return QObject::tr("Vous devez d'abort saisir au moins une espèce de plante (menu 'Espèces') avant de saisir des itinéraires techniques.");
    else if (sTableName=="IT_rotations_manquants")
        return QObject::tr("Aucune !\nToutes les espèces marquées 'A planifier' sont incluses dans les rotations.");
    else if (sTableName=="Récoltes")
        return QObject::tr("Il n'y a aucune culture à récolter pour le moment.");
    else if (sTableName=="Rotations_détails")
        return QObject::tr("Vous devez d'abort saisir au moins une entête de rotation (menu 'Rotations').");
    else
        return QObject::tr("Aucune donnée pour le moment.");
}

bool ReadOnly(QSqlDatabase *db, const QString sTableName,const QString sFieldName) {
    bool bReadOnly=false;
    PotaQuery query(*db);

    //Tables
    if (sTableName=="Cultures"){
        bReadOnly = (((sFieldName=="Culture")and(query.Selec0ShowErr("SELECT Valeur!='Oui' FROM Params WHERE Paramètre='C_modif_N_culture'").toBool())) or
                     sFieldName=="Type" or
                     sFieldName=="Saison" or
                     sFieldName=="Etat");

    } else if (sTableName=="ITP"){
        bReadOnly = (sFieldName=="Type_culture");

    } else if (sTableName=="Params"){
        bReadOnly = (sFieldName!="Valeur");

    } else if (false) {//Views
    } else if (sTableName=="Consommations__Saisies"){
        bReadOnly = (sFieldName=="Stock" or
                     sFieldName=="Sorties");
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
                      sFieldName=="Récolte_com" or
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
                      sFieldName=="Récolte_com" or
                      sFieldName=="Fin_récolte" or
                      sFieldName=="Récolte_faite" or
                      sFieldName=="Terminée" or
                      sFieldName=="Notes");
    } else if (sTableName=="Cultures__à_terminer"){
        bReadOnly = !(sFieldName=="Date_semis" or
                      sFieldName=="Date_plantation" or
                      sFieldName=="Début_récolte" or
                      sFieldName=="Récolte_com" or
                      sFieldName=="Fin_récolte" or
                      sFieldName=="Récolte_faite" or
                      sFieldName=="Terminée" or
                      sFieldName=="Notes");
    } else if (sTableName=="Destinations__conso"){
        bReadOnly = (sFieldName=="Consommation" or
                     sFieldName=="Valeur");

    } else if (sTableName=="Espèces__inventaire"){
        bReadOnly = !(sFieldName=="Date_inv" or
                      sFieldName=="Inventaire" or
                      sFieldName=="Prix_kg" or
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
        bReadOnly = (sFieldName=="Planche" or
                     sFieldName=="Variété" or
                     sFieldName=="Qté_réc");
    } else if (sTableName=="Rotations"){
        bReadOnly = (sFieldName=="Nb_années");
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
        query.ExecShowErr("PRAGMA table_list("+sTableName+")");
        query.next();
        bReadOnly=(query.value(2).toString()!="table");
    }

    return bReadOnly;
}

QColor RowColor(QString sValue){

    QColor c;

    //Cultures
    if (sValue=="Prévue")
        c=cPrevue;
    else if (sValue=="Sous abris")
        c=cSousAbris;
    else if (sValue=="En place")
        c=cEnPlace;
    else if (sValue=="Récolte")
        c=cRecolte;
    else if (sValue=="A terminer")
        c=cATerminer;
    else if (sValue=="Terminée")
        c=cTerminee;
    //Params
    else if (sValue.startsWith("Ilot"))
        c=cPlanche;
    else if (sValue.startsWith("C_"))
        c=cEnPlace;
    else if (sValue.startsWith("Conso_"))
        c=cEspece;
    else
        return QColor();
    if (isDarkTheme())
        c.setAlpha(60);
    else
        c.setAlpha(80);

    return c;
}

QString RowSummary(QString TableName, const QSqlRecord &rec){
    int col=0;
    QString result="";
    if (rec.isNull(0))
        return "";
    else if (TableName=="Apports")
        result=rec.value(rec.indexOf("Apport")).toString()+" - "+
               rec.value(rec.indexOf("Description")).toString()+" - "+
               iif(!rec.value(rec.indexOf("Poids_m²")).isNull(),
                   rec.value(rec.indexOf("Poids_m²")).toString()+"kg/m²","").toString();
    else if (TableName=="Consommations__Saisies")
        result=rec.value(rec.indexOf("Date")).toString()+" - "+
                 rec.value(rec.indexOf("Espèce")).toString()+" - "+
                 iif(!rec.value(rec.indexOf("Quantité")).isNull(),
                     rec.value(rec.indexOf("Quantité")).toString()+"kg - ","").toString()+
                 iif(!rec.value(rec.indexOf("Prix")).isNull(),
                     rec.value(rec.indexOf("Prix")).toString()+QObject::tr("€")+" - ","").toString()+
                 rec.value(rec.indexOf("Destination")).toString();
    else if (TableName=="Cult_planif")
        result=rec.value(rec.indexOf("Planche")).toString()+" - "+
               rec.value(rec.indexOf("IT_plante")).toString()+" - "+
               iif(rec.value(rec.indexOf("Date_semis")).isNull(),
                   rec.value(rec.indexOf("Date_Plantation")).toString(),
                   rec.value(rec.indexOf("Date_semis")).toString()).toString();
    else if (TableName.startsWith("Cultures"))
        result=rec.value(rec.indexOf("Culture")).toString()+" - "+
               rec.value(rec.indexOf("Planche")).toString()+" - "+
               iif(rec.value(rec.indexOf("Variété")).isNull(),
                   rec.value(rec.indexOf("IT_plante")).toString(),
                   rec.value(rec.indexOf("Variété")).toString()).toString()+" - "+
               rec.value(rec.indexOf("Type")).toString()+" - "+
               rec.value(rec.indexOf("Etat")).toString()+" - "+
               iif(rec.value(rec.indexOf("Date_semis")).isNull(),
                   "Plant "+rec.value(rec.indexOf("Date_Plantation")).toString(),
                   "Semis "+rec.value(rec.indexOf("Date_semis")).toString()).toString();
    else if (TableName.startsWith("Destinations"))
        result=rec.value(rec.indexOf("Destination")).toString()+" - "+
               rec.value(rec.indexOf("Date_RAZ")).toString()+" - "+
               str(rec.value(rec.indexOf("Consommation")).toFloat())+"kg - "+
               str(rec.value(rec.indexOf("Valeur")).toFloat())+"€";
    else if (TableName=="Espèces")
        result=rec.value(rec.indexOf("Espèce")).toString()+" ("+
                 rec.value(rec.indexOf("Famille")).toString()+")";
    else if (TableName.startsWith("Espèces__"))
        result=rec.value(rec.indexOf("Espèce")).toString()+" - "+
               rec.value(rec.indexOf("Date_inv")).toString()+" - "+
               str(rec.value(rec.indexOf("Stock")).toFloat())+"kg"+" - "+
               str(rec.value(rec.indexOf("Valeur")).toFloat())+"€";
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
    if (StrLast(result,3)==" - ")
        result=StrFirst(result,result.length()-3);

    return result;
}

QColor TableColor(QString sTName,QString sFName)
{
    //Use sTName.toUpper().startsWith(...) to apply color to the corresponding views.

    //Color by field name for all tables or views.
    if (sFName=="Culture" or
        sFName.startsWith("Nb_cu")or
        sFName=="Min_semis" or sFName=="Max_semis" or
        sFName=="Min_plantation" or sFName=="Max_plantation" or
        sFName=="Min_recolte" or sFName=="Max_recolte" or
        sFName=="Qté_réc_moy")
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
             sFName=="Fin_semis" or
             sFName=="Déb_plantation" or
             sFName=="Fin_plantation" or
             sFName=="Déb_récolte" or
             sFName=="Fin_récolte_ITP" or
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
    else if ((sFName=="Valeur" and sTName.toUpper().startsWith("PARAMS")) or
             sFName=="Année_à_planifier")
        return cParam;
    else if (sFName=="TEMPO")
        return QColor();//Drawed column

    //Color by table name.
    else if (sTName.toUpper().startsWith("APPORTS"))
        return cBase;
    else if (sTName.toUpper().startsWith("CONSOMMATIONS"))
        return cEspece;
    else if (sTName.toUpper().startsWith("CULTURES"))
        return QColor();//Color by row
    else if (sTName.toUpper().startsWith("DESTINATIONS"))
        return cBase;
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

QPixmap TablePixmap(QString sTName, QString text) {
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent); // Fond transparent
    QColor c=QColor();
    QColor cBrush=QColor();
    QColor cPen=QColor();
    c=TableColor(sTName,"");
    if (sTName.startsWith("Cultures"))
        c=cCulture;
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
    if (sTableName=="Apports"){

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
            sToolTip=QObject::tr("Date réelle ou prévue.");
            if (sFieldName=="Début_récolte" or sFieldName=="Fin_récolte")
                sToolTip+="\n"+QObject::tr("Va être mis à jour lors de la saisie des récoltes.");
        }

    } else if (sTableName.startsWith("Destinations")){
        if (sFieldName=="Valeur")
            sToolTip=QObject::tr("Valeur des sorties de stock (€) depuis Date_RAZ (incluse).");

    } else if (sTableName=="Espèces"){
        if (sFieldName=="Dose_semis")
            sToolTip=QObject::tr("Quantité de semence (g/m²).")+"\n"+QObject::tr("Valeur par défaut pour les itinéraires techniques.");
        else if (sFieldName=="Nb_graines_g")
            sToolTip=QObject::tr("Nombre de graines par gramme.")+"\n"+QObject::tr("Valeur par défaut pour les variétés.");

    } else if (sTableName.startsWith("Espèces__")){
        if (sFieldName=="Stock")
            sToolTip=QObject::tr("Quantité totale en stock (kg).");
        else if (sFieldName=="Valeur")
            sToolTip=QObject::tr("Valeur théorique du stock (€).");

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
        if (sFieldName=="Date")
            sToolTip=QObject::tr("Date de récolte.\n"
                                 "Laisser vide pour avoir automatiquement la date de fin de récolte de la culture, ou la date du jour si la date de fin de récolte est dans le futur.");
        else if (sFieldName=="Espèce")
            sToolTip=QObject::tr(  "Les espèces possibles pour saisir une récolte sont celles pour qui il existe des cultures en cours de récolte.\n"
                                   "Voir infobulle 'Culture'.");
        else if (sFieldName=="Culture")
            sToolTip=QObject::tr(  "Les cultures possibles pour saisir une récolte sont celles qui:\n"
                                   "- ont des dates de début et fin de récolte (réelles ou prévues)\n"
                                   "- Début de récolte (Début_récolte) <= date du jour plus avance de saisie de récolte (paramètre 'C_avance_saisie_récolte')\n"
                                   "- Fin de récolte (Fin_récolte) >= date du jour moins délai de saisie de récolte (paramètre 'C_retard_saisie_récolte')\n\n"
                                   "Le paramètre 'C_avance_saisie_récolte' permet de saisir des récoltes faites avant leur date prévue.\n"
                                   "Le paramètre 'C_retard_saisie_récolte' permet de saisir les récoltes après que celles-ci aient été faites.");
        else if (sFieldName=="Quantité")
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
        else if (sFieldName=="Planches_Ilots")
            sToolTip=QObject::tr(   "Regroupement de planches\n"
                                    "(voir le paramètre 'Ilot_nb_car').");
        else if (sFieldName=="Qté_réc")
            sToolTip=QObject::tr("Quantité totale déja récoltée (kg), à cette date.");
        else if (sFieldName=="Rotation")
            sToolTip=QObject::tr("Ensemble de cultures qui se succèdent sur un ensemble de planches.");
        //Apports
        else if (sFieldName=="Apport")
            sToolTip=QObject::tr("Amendement nécessaire avant culture.");
        else if (sFieldName=="Poids_m²")
            sToolTip=QObject::tr("Quantité d'amendement avant culture (kg/m²).");
        //Cultures
        else if (sFieldName=="Culture")
            sToolTip=QObject::tr("Numéro unique de la culture\n"
                                 "(pas de remise à zéro tous les ans).");
        else if (sFieldName=="D_planif")
            sToolTip=QObject::tr("Date de calcul des dates de semis, plantation et récolte.");
        else if (sFieldName=="Récolte_com")
            sToolTip=QObject::tr("Récolte commencée.")+"\n"+
                     QObject::tr("Va être mis à jour lors de la saisie des récoltes.");
        else if (sFieldName=="Terminée")
            sToolTip=QObject::tr("La planche est fermée, disponible pour la culture suivante.\nSaisissez 'NS' (non significative) si la culture ne doit pas être prise en compte dans les analyses.");
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
            sToolTip=QObject::tr("Quantité de récolte en stock (kg).");
        else if (sFieldName=="Date_inv")
            sToolTip=QObject::tr("Date d'inventaire.\nLes espèces ayant une date d'inventaire apparaissent dans l'onglet Stock.");
        else if (sFieldName=="Conservation")
            sToolTip=QObject::tr("Espèce pouvant se conserver. Elles apparaissent dans l'onglet Stock.");
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
            sToolTip=QObject::tr("Pourcentage d'occupation des planches (1 à 100%).")+"\n"+QObject::tr("Plusieurs cultures peuvent partager une planche.");
        else if (sFieldName=="Fi_planches")
            sToolTip=QObject::tr("Filtre de sélection de planches.")+"\n"+QObject::tr("Si filtre = 'AD', seules les planches se terminant par A ou D seront utilisées.");
        else if (sFieldName=="Mise_en_place")
            sToolTip=QObject::tr("Période de mise en place sur la planche (mois-jour).")+"\n"+
                     QObject::tr("* : culture précédente peut-être pas encore récoltée.")+"\n"+
                     QObject::tr("- : culture précédente récoltée depuis %1 mois (automne) à %2 mois (printemps).").arg("4").arg("8");
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
            sToolTip=QObject::tr(  "Nombre de cultures à venir (AV) sur la planche.\n"
                                   "Ces cultures ne sont pour le moment ni semées (semis direct) ni plantées.");
        else if (sFieldName=="Nb_cu_EP")
            sToolTip=QObject::tr(  "Nombre de cultures en place (EP) sur la planche.\n"
                                   "Ces cultures ne sont pas terminées.");
        else if (sFieldName=="Nb_cu_NT")
            sToolTip=QObject::tr(  "Nombre de cultures NON terminées utilisant cet itinéraire technique.");
        else if (sFieldName=="Nb_cu_T")
            sToolTip=QObject::tr(  "Nombre de cultures terminées utilisant cet itinéraire technique.");
        else if (sFieldName=="Nb_cu_TS")
            sToolTip=QObject::tr(  "Nombre de cultures terminées et significatives (champ Terminée différent de 'NS').");
        else if (sFieldName=="Qté_nécess")
            sToolTip=QObject::tr(   "Semence nécessaire (g) pour les cultures non encore semées.");
        else if (sFieldName=="Qté_manquante")
            sToolTip=QObject::tr(   "Qté nécessaire moins Qté en stock moins Qté commandée.");
        else if (sFieldName=="Répartir")
            sToolTip=QObject::tr(  "Pour répartir la quantité récoltée sur plusieurs cultures de l'espèce sélectionnée,\n"
                                   "saisir le début du nom des planches concernées\n"
                                   "ou saisir '*' pour répartir sur toutes les cultures possibles.\n"
                                   "Vide: pas de répartition.\n\n"
                                   "La répartition se fait au prorata des longueurs de planche.\n"
                                   "Attention, la liste des cultures possibles dépend des paramètres 'C_avance_saisie_récolte' et 'C_retard_saisie_récolte'.");
    }

    if (!sDataType.isEmpty()){
        if (sDataType.startsWith("BOOL"))//todo : not used in create table
            sToolTip+=iif(sToolTip=="","","\n\n").toString()+
                      QObject::tr( "Champ Oui/Non (BOOL))\n"
                                   "Vide = Non (ou faux)\n"
                                   "Saisie quelconque = Oui (ou vrai).\n"
                                   "'X', 'Oui', 'Non', '0' ou n'importe quoi veulent dire OUI.");
        else
            sToolTip+=iif(sToolTip=="","","\n\n").toString()+
                      QObject::tr("Format attendu : %1").arg(sDataType);
    }
    return sFieldName+" ("+QObject::tr("table ")+sTableName+")\n"+
           sToolTip;
}

QString ToolTipTable(const QString sTableName) {
    QString sToolTip="";

    if (sTableName=="")
        sToolTip="";

    //Tables
    else if (sTableName=="Apports")
        sToolTip=QObject::tr("Amendement devant être apporté à une planche avant d'y cultiver une espèce de plante.");
    else if (sTableName=="Consommations")
        sToolTip=QObject::tr("Quantités de légume sorties du stock.");
    else if (sTableName=="Cultures")
        sToolTip=QObject::tr("Une culture c'est une plante (variété+itinéraire technique) sur une planche.\nSi la même plante est présente sur plusieurs planches, il y a une culture (numérotée) par planche.");
    else if (sTableName=="Destinations")
        sToolTip=QObject::tr("Destinations des légumes sortis du stock.");
    else if (sTableName=="Espèces")
        sToolTip=QObject::tr("Plante pouvant se reproduire et engendrer une descendance viable et féconde.\nPermet d'enregistrer:\n- Les caractéristiques des graines.\n- L'amendement nécessaire.");
    else if (sTableName=="Familles")
        sToolTip=QObject::tr("Espèces ayant une certaine proximité phylogénétique.\nPermet de d'enregistrer l'intervale de temps minimum entre 2 cultures d'une même famille.");
    else if (sTableName=="Fournisseurs")
        sToolTip=QObject::tr("Fournisseurs des semences.");
    else if (sTableName=="ITP")
        sToolTip=QObject::tr(   "Itinéraires techniques de plantes.\n"
                                "Une ITP c'est Une espèce de plante cultivée d'une certaine manière:\n"
                                "Hative ou tardive, sous serre ou en extérieur, etc.\n"
                                "Chaque ITP à une période de semis, de plantation et de récolte.\n"
                                "Pour chaque culture, il faudra prendre une variété adaptée à l'itinéraire technique voulu.");
    else if (sTableName=="Planches")
        sToolTip=QObject::tr(   "Planches de cultures.\n"
                                "Les 1ers caractères (nb paramétrable) indiquent l'ilot dans lequel se trouve la planche.\n"
                                "Les planches doivent être affectées à des rotations pour que la planification puisse générer les cultures.\n"
                                "Si vous créez vos cultures manuellement, il n'est pas nécessaire d'affecter les planches.\n"
                                "Le nom d'une planche doit avoir la forme <Ilot><unité de production><planche>,\n"
                                "par exemple 'Sud1A' pour la planche A de l'unité de production 1 de l'ilot 'Sud'.");
    else if (sTableName=="Rotations")
        sToolTip=QObject::tr(   "Ensemble d'ITP qui vont se succéder sur un groupe de planches (ilot).\n"
                                "Ici vous ne saisissez que l'entête avec un type de planche et une année de départ.\n"
                                "La liste des ITP qui constituent la rotation est à saisir dans l'onglet 'Détails des rotations'.");
    else if (sTableName=="Rotations_détails")
        sToolTip=QObject::tr(   "Les ITP (donc les espèces cultivées) vont être déplacés chaque année\n"
                                "sur une nouvelle unité de production (UdP).\n"
                                "Si Pc_planches = 100%, les cultures occuperont la totalité des planches de l'UdP.\n"
                                "Si Fi_planches est vide, les cultures occupperont toutes les planches de l'UdP\n"
                                "Si Pc_planches = 50% et Fi_planches = A, les cultures occuperont\n"
                                "la moitié de chaque planche se terminant par A.");
    else if (sTableName=="Récoltes")
        sToolTip=QObject::tr("Quantités de légume récoltées.");
    else if (sTableName=="Types_planche")
        sToolTip=QObject::tr("Permet de caractériser les planches de culture.")+"\n"+QObject::tr("Pour information, non utilisé pour le moment.");
    else if (sTableName=="Variétés")
        sToolTip=QObject::tr("Variété cultivée (cultivar) qui a été sélectionné et choisi pour certaines de ses caractéristiques.\nPermet de gérer les stocks et commandes de semence.");

    if (!sToolTip.isEmpty()) {
        sToolTip+="\n\n"+QObject::tr("Ces informations sont enregistrées dans une table (T).")+"\n"+
                  QObject::tr("Il est possible d'importer dans une table des données extérieures à %1.").arg("Potaléger");
        return sTableName+":\n"+sToolTip;
    }

    //Views
    if (sTableName=="Cult_planif")
        sToolTip=QObject::tr(  "Cultures qui vont être créées lors de la prochaîne planification.\n"
                               "La variété choisie pour chaque culture est celle dont\n"
                               "la quantité de semence en stock est la plus importante.")+"\n"+
                   QObject::tr(  "Liste non directement modifiable, déduite des 'Rotations'.");
    else if (sTableName=="Cultures__Tempo")
        sToolTip=QObject::tr(  "Cultures terminées et significatives (champ Terminée différent de 'NS').");
    else if (sTableName=="IT_rotations_manquants")
        sToolTip=QObject::tr(   "Espèces marquées 'A planifier' et qui ne sont\n"
                                "pourtant pas incluses dans une rotation.\n"
                                "La planification ne générera donc aucune culture pour ces espèces.");
    else if (sTableName=="IT_rotations" or sTableName=="IT_rotations_Ilots")
        sToolTip=QObject::tr(  "Espèces pour lesquelles des cultures vont être créées\n"
                               "lors de la prochaîne planification.")+"\n"+
                   QObject::tr(  "Liste non directement modifiable, déduite des 'Rotations'.");
    else if (sTableName=="Planches_Ilots")
        sToolTip=QObject::tr(  "Les ilots sont des regroupements de planches.\n"
                               "Ils ne sont pas saisis mais déduits des planches saisies.\n"
                               "Le débuts du nom des planches indique leur ilot (voir le paramètre 'Ilot_nb_car').");
    else if (sTableName=="Variétés__inv_et_cde")
        sToolTip=QObject::tr(  "Inventaire des semences pour chaque variété de plante.");

    if (!sToolTip.isEmpty())
        sToolTip+="\n\n";
    sToolTip+=QObject::tr("Ces informations constituent une vue et sont calculées à partir de plusieurs tables.")+"\n"+
              QObject::tr("Il n'est pas possible d'importer dans une vue des données extérieures à %1.").arg("Potaléger")+"\n"+
              QObject::tr("Nom de la vue : ")+sTableName;


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
        //(sFieldName=="Mise_en_place" and sTableName!="Rotations_détails__Tempo") or
        sFieldName.startsWith("Date_"))
        return true;
    else
        return false;
}
