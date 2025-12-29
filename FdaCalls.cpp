#include "data/Data.h"
#include "PotaUtils.h"
#include "qcolor.h"
#include <QSqlTableModel>
#include <QtSql/QSqlQueryModel>
#include <QObject>
#include "potawidget.h"

QString ComboValues(QSqlDatabase *db, const QString sTableName,const QString sFieldName) {
    PotaQuery query(*db);
    QString result=query.Select0ShowErr("SELECT combo FROM fda_f_schema "
                                        "WHERE (name='"+sTableName+"')AND"
                                          "(field_name='"+sFieldName+"')").toString();
    if (!result.contains("|"))
        result=query.Select0ShowErr("SELECT Valeur FROM Params WHERE Paramètre='"+result+"'").toString();

    return result;
}

int DefColWidth(QSqlDatabase *db, const QString sTableName,const QString sFieldName) {
    PotaQuery query(*db);
    int result=query.Select0ShowErr("SELECT col_width FROM fda_f_schema "
                                    "WHERE (name='"+sTableName+"')AND"
                                          "(field_name='"+sFieldName+"')").toInt();
    if (result>0)
        return result;

    QString sType=DataType(db, sTableName,sFieldName);
    if (sType=="DATE")
        return 80;
    else if (sType.startsWith("BOOL"))
        return 50;
    else if (sType.startsWith("INT"))
        return 40;
    else if (sType=="REAL")
        return 50;
    else
        return -1;
}

QString DynDDL(QString sQuery)
{
    //sQuery=StrReplace(sQuery,"#DbVer#",DbVersion);
    // sQuery=StrReplace(sQuery,"#NoAccent(","replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(replace(");
    // sQuery=StrReplace(sQuery,")NoAccent#",",'â','a'),'à','a'),'ä','a')"
    //                                       ",'é','e'),'è','e'),'ê','e'),'ë','e')"
    //                                       ",'î','i'),'ï','i')"
    //                                       ",'ô','o'),'ö','o')"
    //                                       ",'û','u'),'ù','u'),'ü','u')"
    //                                       ",'ç','c')");
    return sQuery;
}

bool FieldIsMoney(QSqlDatabase *db, const QString sTableName, const QString sFieldName){
    PotaQuery query(*db);
    return query.Select0ShowErr("SELECT money NOTNULL FROM fda_f_schema "
                                "WHERE (name='"+sTableName+"')AND"
                                      "(field_name='"+sFieldName+"')").toBool();
}

bool canOpenTab(QSqlDatabase *db, const QString sTableName) {
    PotaQuery query(*db);
    QString queryTest=query.Select0ShowErr("SELECT can_open_tab FROM fda_t_schema "
                                           "WHERE (name='"+sTableName+"')").toString();

    return queryTest.isEmpty() or query.Select0ShowErr(queryTest).toBool();
}

QString FkFilter(QSqlDatabase *db, const QString sTableName, const QString sFieldName, const QModelIndex &index){
    if (index.row()==-1)
        return "";

    //Set the filter depending of row data.
    PotaQuery query(*db);
    QString filter=query.Select0ShowErr("SELECT fk_filter FROM fda_f_schema "
                                        "WHERE (name='"+sTableName+"')AND"
                                              "(field_name='"+sFieldName+"')").toString();


    for (int i=0;i<index.model()->columnCount();i++) {
        filter=filter.replace("::"+index.model()->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),
                              EscapeSQL(index.model()->index(index.row(),i).data(Qt::EditRole).toString()));
    }
    return filter;
}

QString FkSortCol(QSqlDatabase *db, const QString sTableName,const QString sFieldName){
    PotaQuery query(*db);
    return query.Select0ShowErr("SELECT fk_sort_field FROM fda_f_schema "
                                "WHERE (name='"+sTableName+"')AND"
                                      "(field_name='"+sFieldName+"')").toString();
}

bool hiddenField(QSqlDatabase *db, QString sTName,QString sFName)
{
    PotaQuery query(*db);
    return query.Select0ShowErr("SELECT hidden NOTNULL FROM fda_f_schema "
                                "WHERE (name='"+sTName+"')AND"
                                      "(field_name='"+sFName+"')").toBool();
}

bool lastRow(QSqlDatabase *db, const QString sTableName){
    PotaQuery query(*db);
    return query.Select0ShowErr("SELECT goto_last NOTNULL FROM fda_t_schema "
                                "WHERE (name='"+sTableName+"')").toBool();
}

QString NaturalSortCol(QSqlDatabase *db, const QString sTableName){
    PotaQuery query(*db);
    QString result=query.Select0ShowErr("SELECT group_concat(field_name,',') FROM (SELECT field_name FROM fda_f_schema "
                                                                                  "WHERE (name='"+sTableName+"')AND(natural_sort NOTNULL)"
                                                                                  "ORDER BY name,natural_sort)").toString();
    return result;
}

bool multiline(QSqlDatabase *db, const QString sTableName,const QString sFieldName) {
    PotaQuery query(*db);
    return sFieldName=="sql" or
           query.Select0ShowErr("SELECT multiline NOTNULL FROM fda_f_schema "
                                "WHERE (name='"+sTableName+"')AND"
                                      "(field_name='"+sFieldName+"')").toBool();
}

QString NoData(QSqlDatabase *db, const QString sTableName){

    PotaQuery query(*db);
    QString result=query.Select0ShowErr("SELECT no_data_text FROM fda_t_schema "
                                        "WHERE (name='"+sTableName+"')").toString();

    if (!result.isEmpty())
        result=result.replace("\n\n","\n");
    else
        result=QObject::tr("Aucune donnée pour le moment.");

    return result+"\n\n"+ToolTipTable(db,sTableName);
}

bool ReadOnly(QSqlDatabase *db, const QString sTableName,const QString sFieldName) {
    PotaQuery query(*db);
    if (query.Select0ShowErr("SELECT count() FROM fda_f_schema WHERE (name='"+sTableName+"')AND(field_name='"+sFieldName+"')")!=1 or // fda definition does'nt exists.
        query.Select0ShowErr("SELECT (readonly NOTNULL) FROM fda_f_schema WHERE (name='"+sTableName+"')AND(field_name='"+sFieldName+"')").toBool())
        return true;
    else
        return false;
}

QString RowSummary(QSqlDatabase *db, const QString sTableName, const QString rowSummaryModel, const QSqlTableModel *model, const int row){
    QString result="";
    if (rowSummaryModel.isEmpty()) {
        for (int i=0;i<min(3,model->columnCount());i++) {
            if (!model->index(row,i).data(Qt::EditRole).isNull()) {// rec.value(i)
                if (model->index(row,i).data(Qt::DisplayRole).toString().contains("\n")) {
                    result+="####";
                } else {
                    QString dataType=DataType(model->index(row,i).data(Qt::EditRole).toString());
                    if (dataType=="DATE")
                        result+=model->index(row,i).data(Qt::EditRole).toDate().toString("dd/MM/yyyy")+" - ";
                    else if (dataType=="INT")
                        result+=QString::number(model->index(row,i).data(Qt::EditRole).toInt())+Unit(db,sTableName,model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),true)+" - "; //rec.fieldName(i)
                    else if (dataType=="REAL")
                        result+=QString::number(model->index(row,i).data(Qt::EditRole).toDouble())+Unit(db,sTableName,model->headerData(i,Qt::Horizontal,Qt::EditRole).toString(),true)+" - ";
                    else
                        result+=model->index(row,i).data(Qt::DisplayRole).toString()+" - ";
                }
            }
        }
    } else {
        QStringList fieldNames=rowSummaryModel.split(",");
        QString fieldName,format;
        for (int i=0;i<fieldNames.count();i++) {
            if (fieldNames[i].contains("|")) {
                fieldName=fieldNames[i].split("|")[0];
                format=fieldNames[i].mid(fieldNames[i].indexOf("|")+1);
            } else {
                fieldName=fieldNames[i];
                format=":: - ";
            }
            int fieldIndex=model->fieldIndex(fieldName); //rec.indexOf(fieldName);
            if (fieldIndex>-1) {
                if (!model->index(row,fieldIndex).data(Qt::EditRole).isNull()) { //rec.value(fieldIndex)
                    //QVariant sValue=rec.value(fieldIndex);
                    if (model->index(row,fieldIndex).data(Qt::DisplayRole).toString().contains("\n")) {
                        result+=format.replace("::","####");
                    } else {
                        QString dataType=DataType(model->index(row,fieldIndex).data(Qt::EditRole).toString());
                        if (dataType=="DATE")
                            result+=format.replace("::",model->index(row,fieldIndex).data(Qt::EditRole).toDate().toString("dd/MM/yyyy"));
                        else if (dataType=="INT")
                            result+=format.replace("::",QString::number(model->index(row,fieldIndex).data(Qt::EditRole).toInt())+Unit(db,sTableName,fieldName,true));
                        else if (dataType=="REAL")
                            result+=format.replace("::",QString::number(model->index(row,fieldIndex).data(Qt::EditRole).toDouble())+Unit(db,sTableName,fieldName,true));
                        else
                            result+=format.replace("::",model->index(row,fieldIndex).data(Qt::DisplayRole).toString());
                    }
                }
            } else {
                result+=fieldName+"? ";
            }
        }
    }

    while (result.length()>0 and(result.last(1)==" " or result.last(1)=="-"))
        result=result.removeLast();

    return result;
}

QString RowSummaryModel(QSqlDatabase *db, QString sTableName) {
    PotaQuery query(*db);
    return query.Select0ShowErr("SELECT row_summary FROM fda_t_schema "
                                "WHERE (name='"+sTableName+"')").toString();
}

QColor TableColor(QSqlDatabase *db, QString sTName,QString sFName)
{
    PotaQuery query(*db);
    if (sFName.isEmpty())
        return QColor(query.Select0ShowErr("SELECT color FROM fda_t_schema "
                                           "WHERE (name='"+sTName+"')").toString());
    else
        return QColor(query.Select0ShowErr("SELECT color FROM fda_f_schema "
                                           "WHERE (name='"+sTName+"')AND"
                                                 "(field_name='"+sFName+"')").toString());
}

QPixmap TablePixmap(QSqlDatabase *db,QString sTName, QString text) {
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent); // Fond transparent
    QColor c=QColor();
    QColor cBrush=QColor();
    QColor cPen=QColor();
    c=TableColor(db,sTName,"");
    cBrush=c;
    cBrush.setAlpha(100);
    if (!isDarkTheme())
        cPen=QColor("#000000");
    else
        cPen=QColor("#ffffff");
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(cBrush);
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

QString ToolTipField(QSqlDatabase *db,const QString sTableName,const QString sFieldName, const QString sDataType, const QString sBaseData) {
    QString sToolTip="";

    PotaQuery query(*db);
    sToolTip=query.Select0ShowErr("SELECT description FROM fda_f_schema "
                                  "WHERE (name='"+sTableName+"')AND"
                                        "(field_name='"+sFieldName+"')").toString();

    if (sToolTip.isEmpty() and sTableName.contains("__")) //Search description of real table field.
        sToolTip=query.Select0ShowErr("SELECT description FROM fda_f_schema "
                                      "WHERE (name='"+sTableName.first(sTableName.indexOf("__"))+"')AND"
                                            "(field_name='"+sFieldName+"')").toString();

    while (sToolTip.startsWith("::")) //Reference to description of another table and field.
        sToolTip=query.Select0ShowErr("SELECT description FROM fda_f_schema "
                                      "WHERE (name='"+sToolTip.mid(2).split(".")[0]+"')AND"
                                            "(field_name='"+sToolTip.mid(2).split(".")[1]+"')").toString();

    if (sToolTip.isEmpty()) {//Search description of master table field.
        QString masterTable=query.Select0ShowErr("SELECT master_table FROM fda_f_schema "
                                                 "WHERE (name='"+sTableName+"')AND"
                                                       "(field_name='"+sFieldName+"')").toString();
        if (!masterTable.isEmpty()) {
            QString masterField=query.Select0ShowErr("SELECT master_field FROM fda_f_schema "
                                                     "WHERE (name='"+sTableName+"')AND"
                                                           "(field_name='"+sFieldName+"')").toString();
            if (!masterField.isEmpty())
                sToolTip=query.Select0ShowErr("SELECT description FROM fda_f_schema "
                                              "WHERE (name='"+masterTable+"')AND"
                                                    "(field_name='"+masterField+"')").toString();
        }
    }

    if (!sToolTip.isEmpty())
        sToolTip=sToolTip.replace("\n\n","\n");

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

    QString unit=Unit(db,sTableName,sFieldName);
    if (!unit.isEmpty())
        sToolTip+=iif(sToolTip=="","","\n\n").toString()+
                  QObject::tr("Unité: ")+unit;

    if (sBaseData=="x")
        sToolTip+=iif(sToolTip=="","","\n\n").toString()+
                  QObject::tr("Ce champ fait partie des données de base 🔺 (fournies avec l'application).\n"
                              "Si vous modifiez les données de base, vous pourrez revenir à leurs valeurs initiales (clic droit).");
    if (multiline(db,sTableName,sFieldName))
        sToolTip+=iif(sToolTip=="","","\n\n").toString()+
                  QObject::tr("Enter pour passer en édition multi-lignes.\n"
                              "Ctrl+Enter pour valider le texte multi-ligne.\n"
                              "Echap pour abandonner les modification du texte multi-ligne.");

    if (!sDataType.isEmpty()) {
        if (sDataType.startsWith("BOOL")) {
            sToolTip+=iif(sToolTip=="","","\n\n").toString()+
                      QObject::tr( "Champ Oui/Non (BOOL))\n"
                                   "Vide=Non (ou faux)");
            if (!sToolTip.contains("'x' ") and !sToolTip.contains("'v' "))
                sToolTip+=QObject::tr( "\n"
                                       "Saisie quelconque=Oui (ou vrai).\n"
                                       "'x', 'Oui', 'Non', '0' ou n'importe quoi veulent dire OUI.\n"
                                       "A l'affichage 'x' est remplacé par ✔️.");
        } else {
            sToolTip+=iif(sToolTip=="","","\n\n").toString()+
                      QObject::tr("Format : %1").arg(sDataType);
            if ((sDataType=="REAL" or sDataType.startsWith("INT"))and !ReadOnly(db,sTableName,sFieldName))
                sToolTip+="\n\n"+
                          QObject::tr("Calculatrice: saisir une formule (ex '=1+1') puis Entrée pour avoir le résultat.");

        }
    }

    if (sFieldName.startsWith("TEMPO")) {
        return "";
    } else {
        if (sTableName.indexOf("__")>1) {
            QString realTableName=SubString(sTableName,0,sTableName.indexOf("__")-1);
            return sFieldName+" ("+QObject::tr("table")+" "+realTableName+", "+QObject::tr("vue")+" "+sTableName+")\n\n"+
                   sToolTip;
        } else {
            return sFieldName+" ("+QObject::tr("table")+" "+sTableName+")\n\n"+
                   sToolTip;
        }
    }
}

QString ToolTipTable(QSqlDatabase *db,const QString sTableName) {
    QString sToolTip="";

    // if (sTableName=="") ???
    //     sToolTip="";

    PotaQuery query(*db);
    sToolTip=query.Select0ShowErr("SELECT description FROM fda_t_schema WHERE (name='"+sTableName+"')").toString();

    if (sToolTip.isEmpty() and sTableName.contains("__"))
        sToolTip=query.Select0ShowErr("SELECT description FROM fda_t_schema WHERE (name='"+sTableName.first(sTableName.indexOf("__"))+"')").toString();

    if (!sToolTip.isEmpty())
        sToolTip=sToolTip.replace("\n\n","\n");

    if (sToolTip.isEmpty()) {
        if (sTableName.startsWith("fda_"))
            sToolTip=QObject::tr("Informations additionnelles qui déterminent le comportement de l'application.")+"\n"+
                     QObject::tr("Version bêta : incomplet et partiellement utilisé.")+"\n"+
                     QObject::tr("La mise à jour du schéma de BDD peut modifier cette table.")+"\n\n"+
                     QObject::tr("Nom de la table : ")+sTableName;
        else if (sTableName=="Params")
            sToolTip=QObject::tr("Paramètres pour la base de données (BDD) courante.")+"\n"+
                     QObject::tr("La mise à jour du schéma de BDD peut modifier cette table.")+"\n\n"+
                     QObject::tr("Nom de la table : ")+sTableName;
        else if (sTableName=="FK_errors")
            sToolTip=QObject::tr("Liste des erreurs d'intégrité.")+"\n"+
                     QObject::tr("Une erreur d'intégrité c'est par exemple l'existence de cultures de tomate alors que l'espèce 'Tomate' n'existe pas ou est orthographiée différement.")+"\n"+
                     QObject::tr("La mise à jour du schéma de BDD peut modifier cette liste.")+"\n\n"+
                     QObject::tr("Nom de la vue : ")+sTableName;
        else if (sTableName=="sqlite_schema")
            sToolTip=QObject::tr("Liste des requêtes SQL qui ont permi de créer les tables et les vues de la BDD ouverte.")+"\n"+
                     QObject::tr("Ces requêtes peuvent être utilisées pour créer une BDD vide de même structure dans un autre logiciel de gestion de BDD.")+"\n"+
                     QObject::tr("La mise à jour du schéma de BDD peut modifier cette liste.")+"\n\n"+
                     QObject::tr("Nom de la vue : ")+sTableName;
        else if (sTableName=="Table_list")
            sToolTip=QObject::tr("Liste des tables réellement existantes dans la BDD ouverte.\n"
                                 "Toutes les informations que vous saisissez sont enregistrées dans ces tables.")+"\n"+
                     QObject::tr("La mise à jour du schéma de BDD peut modifier cette liste.")+"\n\n"+
                     QObject::tr("Nom de la vue : ")+sTableName;
        else if (sTableName=="View_list")
            sToolTip=QObject::tr("Liste des vues réellement existantes dans la BDD ouverte.\n"
                                 "Les vues présentent des informations calculées à partir de celles de plusieurs tables ou vues.")+"\n"+
                     QObject::tr("La mise à jour du schéma de BDD peut modifier cette liste.")+"\n\n"+
                     QObject::tr("Nom de la vue : ")+sTableName;
    } else {
        QString tblType=query.Select0ShowErr("SELECT tbl_type FROM fda_t_schema WHERE (name='"+sTableName+"')").toString();
        if (tblType=="Table") {
            sToolTip=sToolTip+
                     "\n\n"+QObject::tr("Ces informations sont enregistrées dans une table (T) et peuvent être directement modifiéees.")+"\n"+
                     QObject::tr("Nom de la table : ")+sTableName;
        } else if (tblType=="View as table") {
            sToolTip=sToolTip+
                     "\n\n"+QObject::tr("Ces informations constituent une vue et sont calculées à partir de plusieurs tables ou vues.")+"\n"+
                     QObject::tr("Elles peuvent être modifiéees via les triggers associés à la vue.")+"\n"+
                     QObject::tr("Nom de la vue : ")+sTableName;
             if (sTableName.contains("__"))
                sToolTip+="\n"+
                          QObject::tr("Nom de la table sous-jacente : ")+sTableName.first(sTableName.indexOf("__"));
        } else if (tblType=="View") {
            sToolTip=sToolTip+
                     "\n\n"+QObject::tr("Ces informations constituent une vue et sont calculées à partir de plusieurs tables ou vues.")+"\n"+
                     QObject::tr("Elles peuvent pas être modifiéees.")+"\n"+
                     QObject::tr("Nom de la vue : ")+sTableName;;
             if (sTableName.contains("__"))
                sToolTip+="\n"+
                          QObject::tr("Nom de la table sous-jacente : ")+sTableName.first(sTableName.indexOf("__"));
        }
    }
    return sToolTip;
}

QString Unit(QSqlDatabase *db,const QString sTableName,const QString sFieldName, const bool bSpaceBefore) {
    PotaQuery query(*db);
    QString result=query.Select0ShowErr("SELECT unit FROM fda_f_schema "
                                "WHERE (name='"+sTableName+"')AND"
                                      "(field_name='"+sFieldName+"')").toString();
    if (result.isEmpty() and FieldIsMoney(db,sTableName,sFieldName))
        result=query.Select0ShowErr("SELECT Valeur FROM Params "
                                    "WHERE (Paramètre='Devise')").toString();
    return iif(bSpaceBefore," ","").toString()+result;

}
