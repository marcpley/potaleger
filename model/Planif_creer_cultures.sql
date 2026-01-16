NbCultPlanif=(SELECT count() FROM Planif_planches);
NbCultPlanifValid=(SELECT count() FROM Planif_planches WHERE (Validée NOTNULL));

if (NbCultPlanifValid==0) {
    if (NbCultPlanif==0) {
        messageDialog('Planification','Aucune culture à planifier:<br><br>'||
                                      '- Créez des rotations<br>'||
                                      '- Vérifiez que le paramètre ''Planifier_planches'' n''exclut pas toutes les planches.');
    } else {
        messageDialog('Planification','Aucune culture à planifier validée.<br><br>'||
                                      'Allez dans l''onglet ''Planification/Cultures prévues par planche'' et renseignez la colonne ''Validée''.');
    }
    return;
}

NbCultPlanifRetard=(SELECT count() FROM Planif_planches WHERE (coalesce(Date_semis,Date_plantation)<DATE('now'))AND(Validée NOTNULL));
NbCultPlanifConflitCreer=(SELECT count() FROM Planif_planches WHERE (Déjà_en_place NOTNULL)AND(Validée NOTNULL));
NbCultPlanifNonValidees=(SELECT count() FROM Planif_planches WHERE (Validée ISNULL));
NbCultAVenir=(SELECT count() FROM Cu_non_commencées);
NDerCult=(SELECT max(Culture) FROM Cultures);

if(NbCultAVenir>0) {
    icon=sp_Warning;
    CultAVenir='<br><br>'||'Il y a déjà '||NbCultAVenir||' cultures ni semées ni plantés.<br>';
    if (NbCultAVenir>NbCultPlanifValid*0.9) {
        CultAVenir=CultAVenir||'Peut-être avez-vous déjà généré les prochaines cultures.<br>'||
                               'Si c''est le cas, vous devriez les supprimer avant d''aller plus loin.';
    }
} else {
    icon=sp_Question;
    CultAVenir="";
}

confirm='La saison courante est : '||(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')||'<br><br>'||
                   '<b>Créer les cultures de la saison suivante ('||(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture')||') à partir des plans de rotation ?</b><br><br>'||
                   'La saison courante peut être modifiée dans les paramètres (menu ''Edition'', paramétre ''Année_culture'').<br><br>'||
                   '<b>'||NbCultPlanifValid||' cultures vont être créées.</b><br>'||
                   NbCultPlanifConflitCreer||' cultures vont être en conflit avec des cultures déjà existantes.<br>'||
                   NbCultPlanifNonValidees||' cultures ne vont pas être créées car non validée.<br>'||
                   'Id de la dernière culture: '||NDerCult||
                   CultAVenir;
                   
if (okCancelDialog(confirm,icon,600)) {
    if (NbCultPlanifRetard>0){
        if (NbCultPlanifRetard<NbCultPlanifValid/10) {def=1} else {def=0}
        choice=radioButtonDialog('Parmis les '||NbCultPlanifValid||' cultures à créer, '||
                                 'il y en a '||NbCultPlanifRetard||' dont la date de la 1ère opération (semis ou plantation) est déjà passée.',
                                 'Ne pas créer ces cultures en retard|Créer aussi ces cultures en retard',def,'',sp_Warning);
        if (choice<0) {return;}
    } else {
        choice=1;
    }
    IdCult1=SELECT max(Culture) FROM Cultures;
    
    if (choice==0) {
        INSERT INTO Cultures (Espèce,IT_plante,Variété,Fournisseur,Planche,D_planif,Longueur,Nb_rangs,Espacement,Notes)
                       SELECT Espèce,IT_plante,NULL,NULL,Planche,(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'),
                              Longueur,Nb_rangs,Espacement,Notes||iif(Validée!='x',x'0a0a'||Validée,'')
                       FROM Planif_planches
                       WHERE coalesce(Date_semis,Date_plantation)>=DATE('now')AND(Validée NOTNULL);
    } else {
        INSERT INTO Cultures (Espèce,IT_plante,Variété,Fournisseur,Planche,D_planif,
                              Date_semis,Date_plantation,Début_récolte,Fin_Récolte,
                              Longueur,Nb_rangs,Espacement,Notes)
                       SELECT Espèce,IT_plante,NULL,NULL,Planche,(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'),
                              Date_semis,Date_plantation,Début_récolte,Fin_Récolte,
                              Longueur,Nb_rangs,Espacement,iif(Validée!='x',coalesce(Notes||x'0a0a','')||Validée,Notes)
                       FROM Planif_planches
                       WHERE (Validée NOTNULL);
    }
    if (insert_Cultures>0) {
        DELETE FROM Planif_validations
        WHERE (SELECT (PP.Déjà_créée NOTNULL) FROM Planif_pl_date2 PP WHERE PP.IdxIdPl=Planif_validations.IdxIdPl);
        NbValidNonCrees=(SELECT count() FROM Planif_validations WHERE Validée NOTNULL);
        IdCult2=(SELECT min(Culture) FROM Cultures WHERE Culture>IdCult1);
        IdCult3=(SELECT max(Culture) FROM Cultures);
        nbCult=IdCult3-IdCult2+1;
        mess=nbCult||' cultures créées sur '||NbCultPlanifValid||' cultures prévues.<br><br>'||
             'Id culture: '||IdCult2||' > '||IdCult3||
              iif(NbValidNonCrees>0,'<br>'||NbValidNonCrees||' culture validées mais non crées.','');
        messageDialog(mess,"",iif(IdCult3>IdCult2 and IdCult2>IdCult1 and NbValidNonCrees==0,sp_Information,sp_Warning));
    } else {
        messageDialog('Aucune culture créée.',"",sp_Information);
    }
}