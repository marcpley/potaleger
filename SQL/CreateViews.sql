-- CREATE VIEW Associations AS SELECT --- Associations de plantes, regroupées en une ligne par association.
--     A.Association, --- Nom de l'association. Les caractères de fin indiquent l'association est bénéfique.
--     count() Nb, --- Nombre d'espèce dans l'association. --UNIT(espèces)
--     count(Requise) Nb_requises, --- Nombre d'espèces requises pour que l'assocition fasse son effet. --UNIT(espèces requises)
--     group_concat( --CASE WHEN (Requise ISNULL)OR((SELECT count(A2.Requise) FROM Associations_détails A2 WHERE A2.Association=A.Association )!=1)
--                       -- THEN coalesce(A.Espèce,A.Groupe,A.Famille)
--                       -- ELSE NULL -- Ne pas mettre l'unique espèce requise.
--                       -- END
--                  coalesce(A.Espèce,A.Groupe,A.Famille),', ') Espèces ---
--         ---multiline
-- FROM Associations_détails A
-- GROUP BY A.Association --,A.Effet
-- ORDER BY A.Association ---
-- ;

-- CREATE VIEW Associations_détails_aap AS SELECT -- Annuelles, associations possibles, sans éclater les familles en espèces.
--     AD.Association,
--     coalesce(AD.Espèce,AD.Groupe,AD.Famille) Espèce_ou_famille,
--     AD.Requise
-- FROM Associations_détails AD
-- LEFT JOIN Espèces E USING(Espèce)
-- WHERE ((AD.Espèce NOTNULL)AND(E.Vivace ISNULL)AND(E.A_planifier NOTNULL))OR
--       ((AD.Espèce ISNULL)AND(AD.Famille IN(SELECT E2.Famille  FROM Espèces E2 WHERE (E2.Vivace ISNULL)AND(E2.A_planifier NOTNULL))));

CREATE VIEW Associations_détails__Espèces AS SELECT -- Associations avec familles remplacées par leurs espèces.
    AD1.Association,
    AD1.Espèce,
    AD1.Requise
FROM Associations_détails AD1
WHERE AD1.Espèce NOTNULL -- Espèce spécifiée dans la rotation.
UNION
SELECT
    AD2.Association,
    E.Espèce,
    AD2.Requise
FROM Associations_détails AD2
JOIN Espèces E ON E.Espèce LIKE AD2.Groupe||'%'
WHERE (AD2.Espèce ISNULL)AND(AD2.Groupe NOTNULL) -- Espèces du groupe
UNION
SELECT
    AD3.Association,
    E.Espèce,
    AD3.Requise
FROM Associations_détails AD3
JOIN Espèces E USING(Famille)
WHERE (AD3.Espèce ISNULL)AND(AD3.Groupe ISNULL)AND(AD3.Famille NOTNULL); -- Famille spécifiée dans la rotation.

CREATE VIEW Associations_détails__Espèces_a AS SELECT
    *
FROM Associations_détails__Espèces
JOIN Espèces USING(Espèce)
WHERE Vivace ISNULL;

CREATE VIEW Associations_détails__Saisies AS SELECT ---
    ---no_data_text Saisissez au moins 3 espèces avant de saisir des associations.
    ---row_summary RS
    A.IdxAsReEsGrFa, --- Nécessaire pour mise à jour de la table via trigger INSTEAD OF UPDATE.
        ---hidden
    A.Association, ---
    A.Espèce, ---
    A.Groupe, ---
    A.Famille, ---
    A.Requise, ---
    CAST(coalesce((SELECT E.Effet FROM Espèces E WHERE  (E.Espèce=A.Espèce)OR(E.Espèce LIKE A.Groupe||'%')),
                  (SELECT F.Effet FROM Familles F WHERE  (F.Famille=A.Famille))) AS TEXT) Effet, ---
    A.Notes, ---
    CAST((SELECT group_concat(AD.Association,', ')||iif(substr(AD.Association,-1,1)!=substr(A.Association,-1,1),' !','') FROM Associations_détails AD
          WHERE NOT(A.Association LIKE '%'||coalesce(A.Espèce,A.Groupe)||'%')AND
                (AD.Association LIKE '%'||coalesce(A.Espèce,A.Groupe)||'%')AND
                (A.Association LIKE '%'||coalesce(AD.Espèce,AD.Groupe)||'%')) AS TEXT) Doublon, ---
    A.Association||' : '||iif(A.Espèce NOTNULL,A.Espèce||' ('||A.Famille||')',iif(A.Groupe NOTNULL,A.Groupe||' ('||A.Famille||')',A.Famille))||iif(A.Requise NOTNULL,' - Requise','') RS ---
        ---hidden
FROM Associations_détails A
ORDER BY A.IdxAsReEsGrFa ---
;

CREATE VIEW Associations__Espèces AS SELECT ---
    A.Association, ---
    -- A.Effet,
    count() Nb, --- --UNIT(espèces)
    count(A.Requise) Nb_requises, --- ::Associations.Nb_requises --UNIT(espèces requises)
    CAST(group_concat( -- CASE WHEN (A.Requise ISNULL)OR((SELECT count(A2.Requise) FROM Associations_détails A2 WHERE A2.Association=A.Association)!=1)
                       -- THEN A.Espèce
                       -- ELSE NULL -- Ne pas mettre l'unique espèce requise.
                       -- END
                      A.Espèce,', ') AS TEXT) Espèces ---
FROM Associations_détails__Espèces A
GROUP BY A.Association --,A.Effet
ORDER BY A.Association ---
;

CREATE VIEW Associations__présentes AS SELECT --- Associations de cultures entre les annuelles et les vivaces en place sur la même planche ou sur une planche proche ('Planches_influencées' dans onglet 'Planches').
    P.Planche, ---
    P.Association, ---
    P.Cultures, ---
        ---multiline
    P.Nb_cult, --- --UNIT(cultures)
    P.Nb_espèces, --- --UNIT(espèces)
    --P.Nb_vivaces, --- --UNIT(espèces)
    P.Espèces Espèces_de_l_association ---
FROM Cu_asso6 P
ORDER BY P.Planche,P.Association;

CREATE VIEW Consommations__Saisies AS SELECT ---
    ---goto_last
    ---no_data_text Saisissez au moins une espèce marquée 'Conservation' avant de saisir des sorties de stock.
    ---row_summary Date,Espèce,Quantité,Prix,Destination
    Co.ID, --- Nécessaire pour mise à jour de la table via trigger INSTEAD OF UPDATE.
        ---hidden
    Co.Date, ---
    Co.Espèce, ---
    Co.Quantité, ---
    Co.Prix, ---
    Co.Destination, ---
    CAST(round(E.Inventaire+coalesce((SELECT sum(Quantité) FROM Récoltes Re
                                      WHERE (Re.Espèce=Co.Espèce)AND(Re.Date BETWEEN E.Date_inv AND Co.Date)),0)
                           -coalesce((SELECT sum(Quantité) FROM Consommations Co2
                                      WHERE (Co2.Espèce=Co.Espèce)AND(Co2.Date BETWEEN E.Date_inv AND Co.Date)),0),3)AS REAL) Stock, --- Quantité restante en stock, à cette date.--UNIT(kg)
    CAST(round((SELECT sum(Quantité) FROM Consommations Co3
                WHERE (Co3.Espèce=Co.Espèce)AND
                      (Co3.Destination=Co.Destination)AND
                      (Co3.Date BETWEEN D.Date_RAZ AND Co.Date)),3)AS REAL) Sorties, --- Somme à cette date des sorties pour cette espèce et cette destination, depuis Date_RAZ de la destination (incluse).--UNIT(kg)
    Co.Notes ---
FROM Consommations Co
JOIN Espèces E USING(Espèce)
LEFT JOIN Destinations D USING(Destination)
WHERE (Co.Date>DATE('now','-'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Conso_historique')||' days'))OR
      (DATE(Co.Date) ISNULL) -- Détection de date incorecte
ORDER BY Co.Date,E.Espèce,D.Destination ---
;

CREATE VIEW Cu_ITP_analyse_a AS SELECT
    Culture,
    IT_plante,
    Date_semis,
    Date_plantation,
    Début_récolte,
    Fin_récolte
FROM Cultures
WHERE (Terminée NOT LIKE '%NS')AND -- Cultures significative
      (Terminée NOTNULL)AND(Terminée NOT LIKE 'v%'); -- Annuelles terminées


CREATE VIEW Cu_ITP_analyse_v AS SELECT
    Culture,
    IT_plante,
    Date_semis,
    Date_plantation,
    Début_récolte,
    Fin_récolte,
    Terminée
FROM Cultures
WHERE (coalesce(Terminée,'') NOT LIKE '%NS')AND -- Cultures significative
      (Terminée LIKE 'v%'); -- Vivace terminée ou pas.

CREATE VIEW Cu_dates_théo AS SELECT -- Dates théoriques au plus tôt.
    C.Culture,
    DATE(C.A_planif||'-01-01',(C.S_semis*7-6-1)||' days') Date_semis,
    DATE(C.A_planif||'-01-01',(C.S_plantation*7-6-1)||' days') Date_plantation,
    iif(C.D_récolte NOTNULL,DATE(C.A_planif||'-01-01',(C.S_récolte*7-6-1)||' days'),NULL) Début_récolte,
    iif(C.D_récolte NOTNULL,DATE(C.A_planif||'-01-01',((C.S_récolte+C.D_récolte)*7-6-1)||' days'),
    DATE(C.A_planif||'-01-01',(C.S_récolte*7-6-1)||' days')) Fin_récolte,
    C.PJ,
    C.Terminée
FROM Cu_dates_théo2 C;

CREATE VIEW Cu_dates_théo2 AS SELECT -- Semaines corrigées: pas de n° semaine inférieur au n° semaine opération précédente.
    C.Culture,
    C.A_planif,
    iif(C.S_semis>coalesce(C.S_plantation,C.S_semis),C.S_semis-52,C.S_semis) S_semis,
    C.S_plantation,
    iif(C.S_récolte<coalesce(C.S_plantation,C.S_semis),C.S_récolte+52,C.S_récolte) S_récolte,
    C.D_récolte,
    C.PJ,
    C.Terminée
FROM Cu_dates_théo3 C;

CREATE VIEW Cu_dates_théo3 AS SELECT -- Avec année de planification et semaines théoriques (ITP et Variétés)
    C.Culture,
    coalesce(iif(C.terminée LIKE 'v',coalesce(substr(C.Début_récolte,1,4),substr(C.Fin_récolte,1,4)),NULL),
             iif((length(C.D_planif)=4)AND(CAST(C.D_planif AS INTEGER) BETWEEN 2000 AND 2100),C.D_planif,NULL), -- Année demandée par utilisateur.
             substr(C.Date_plantation,1,4),substr(C.Date_semis,1,4), -- Année de mise en place de la culture.
             (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture')) A_planif, -- Année après la saison courante.
    I.S_semis,
    I.S_plantation,
    coalesce(V.S_récolte,I.S_récolte) S_récolte,
    coalesce(V.D_récolte,I.D_récolte,0) D_récolte,
    V.PJ,
    C.Terminée
FROM Cu_non_ter C
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Variétés V USING(Variété);

CREATE VIEW Cu_en_place AS SELECT --- Cultures en place sur leur planche: semées en place ou plantées.
                                  --- Ne sont pas incluses les cultures semées en pépinière mais non plantées.
    *
FROM Cu_non_ter
WHERE ((Semis_fait NOTNULL AND Date_plantation ISNULL) OR -- SD semé
       Plantation_faite NOTNULL)-- Plant ou SPep planté
ORDER BY Planche;

CREATE VIEW Cu_esp_prod_an AS SELECT
    C.Saison,
    C.Culture,
    C.Planche,
    C.Espèce,
    C.Longueur,
    C.Longueur*P.Largeur Surface,
    E.Rendement,
    E.Niveau,
    CAST((CASE WHEN C.Saison=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture') THEN E.Obj_annuel ELSE NULL END)AS REAL) Obj_annuel,
    CAST(round(C.Longueur*P.Largeur*E.Rendement,3)AS REAL) Qté_prév,
    CAST(round((SELECT total(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc,
    C.Terminée,
    E.Vivace,
    E.Notes N_espèce
FROM Cultures C
JOIN Planches P USING(Planche)
JOIN Espèces E USING(Espèce)
WHERE (coalesce(C.Terminée,'') NOT LIKE 'v%')AND(E.Vivace ISNULL); -- AND(E.A_planifier NOTNULL)

CREATE VIEW Cu_esp_prod_vi AS SELECT
    R.Saison,
    C.Culture,
    C.Planche,
    C.Espèce,
    C.Longueur,
    C.Longueur*P.Largeur Surface,
    E.Rendement,
    E.Niveau,
    CAST((CASE WHEN R.Saison=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture') THEN E.Obj_annuel ELSE NULL END)AS REAL) Obj_annuel,
    CAST(round(C.Longueur*P.Largeur*E.Rendement,3)AS REAL) Qté_prév,
    CAST(round(total(R.Quantité),3)AS REAL) Qté_réc,
    C.Terminée,
    E.Vivace,
    E.Notes N_espèce
FROM Cultures C
JOIN Planches P USING(Planche)
JOIN Espèces E USING(Espèce)
JOIN Récoltes_saison R USING(Culture)
WHERE (C.Terminée LIKE 'v%')OR(E.Vivace NOTNULL)
GROUP BY R.Saison,C.Culture;

CREATE VIEW Cu_inc_dates AS SELECT
    Culture,
    CAST(iif(Inc_semisD>0,'Semis trop tôt: '||Inc_semisD||' j',
         iif(Inc_semisF>0,'Semis trop tard: '||Inc_semisF||' j',
         iif(Inc_plantD>0,'Plant. trop tôt: '||Inc_plantD||' j',
         iif(Inc_plantF>0,'Plant. trop tard: '||Inc_plantF||' j',
         iif(Inc_RecD>0,'Récolte trop tôt: '||Inc_RecD||' j',
         iif(Inc_RecF>0,'Récolte trop tard: '||Inc_RecF||' j',
         NULL)))))) AS TEXT) Incohérence
FROM Cu_inc_dates2;

CREATE VIEW Cu_inc_dates2 AS SELECT
    C.Culture,
    CAST((julianday(CDT.Date_semis)
          -julianday(C.Date_semis)
          -(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_A_semis'))AS INTEGER) Inc_semisD,
    CAST((julianday(C.Date_semis)
          -julianday(CDT.Date_semis)
          -(coalesce(I.Décal_max,0)+1)*7-(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_R_semis'))AS INTEGER) Inc_semisF,
    CAST((julianday(CDT.Date_plantation)
          -julianday(C.Date_plantation)
          -(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_A_plantation'))AS INTEGER) Inc_plantD,
    CAST((julianday(C.Date_plantation)
          -julianday(CDT.Date_plantation)
          -(coalesce(I.Décal_max,0)+1)*7
          -(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_R_plantation'))AS INTEGER) Inc_plantF,
    CAST((julianday(CDT.Début_récolte)
          -julianday(C.Début_récolte)
          -(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_A_récolte'))AS INTEGER) Inc_RecD,
    CAST((julianday(C.Fin_récolte)
          -julianday(CDT.Fin_récolte)
          -(coalesce(I.Décal_max,0)+1)*7
          -(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_R_récolte'))AS INTEGER) Inc_RecF
FROM Cultures C
LEFT JOIN Cu_dates_théo CDT USING(Culture)
LEFT JOIN ITP I USING(IT_plante)
WHERE (coalesce(C.Terminée,'') NOT LIKE '%NS');

CREATE VIEW Cu_non_commencées AS SELECT --- Cultures prévues mais ni semées (en place ou en pépinière) ni plantées.
    *
FROM Cu_non_ter
WHERE Semis_fait ISNULL AND Plantation_faite ISNULL
ORDER BY Planche;

CREATE VIEW Cu_non_ter AS SELECT
    *
FROM Cultures WHERE NOT((Terminée NOTNULL)AND(Terminée!='v')AND(Terminée!='V'));

CREATE VIEW Cu_asso1 AS SELECT -- Cultures annuelles et vivaces non terminées
    C.Planche,
    C.Culture,
    C.Espèce,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
    C.Fin_récolte,
    iif(C.Terminée NOTNULL,'x',NULL) Vivace
    --NULL Planche_influente
FROM Cu_non_ter C
UNION
SELECT -- Vivaces non terminées sur leurs planches_influencées
    PI.Planche,
    CI.Culture,
    CI.Espèce,
    coalesce(CI.Date_plantation,CI.Date_semis),
    CI.Fin_récolte, --DATE('2100-0-01') todo: impossible de forcer une date sans perdre le type de champ DATE. Il faut utiliser le field_type FDA.
    'x'
    --CI.Planche Planche_influente
FROM Cultures CI
JOIN Planches P USING(Planche)
JOIN Planches PI ON P.Planches_influencées||',' LIKE '%'||PI.Planche||',%'
WHERE CI.Terminée='v' OR CI.Terminée='V';

CREATE VIEW Cu_asso2 AS SELECT -- Cultures annuelles avec ses associations possibles.
    C.*,
    A.Association,
    A.Requise
FROM Cu_asso1 C
JOIN Associations_détails__Espèces A USING(Espèce);

CREATE VIEW Cu_asso3 AS SELECT -- Cultures annuelles avec pour chaque association possible les cultures de la même asso qui la croisent (y compris elle même).
    C.*,
    C2.Culture Culture_A,
    C2.Espèce Espèce_A,
    C2.Date_MEP Date_MEP_A,
    C2.Fin_récolte Fin_récolte_A,
    C2.Vivace Vivace_A,
    C2.Requise Requise_A
FROM Cu_asso2 C
JOIN Cu_asso2 C2 USING(Association)
WHERE (C2.Planche=C.Planche)AND(C2.Association=C.Association)AND
      (julianday(C2.Date_MEP)+(SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Nb_sem_croisement')*7<julianday(C.Fin_récolte))AND
      (julianday(C2.Fin_récolte)-(SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Nb_sem_croisement')*7>julianday(C.Date_MEP));

CREATE VIEW Cu_asso4 AS SELECT -- Regroupement des cultures qui se croisent en associations-espèces.
    C.Planche,
    C.Culture,
    C.Association,
    group_concat(C.Culture_A||' - '||C.Espèce_A||' - MEP '||strftime('%d/%m/%Y',C.Date_MEP_A)||' - Fin réc. '||strftime('%d/%m/%Y',C.Fin_récolte_A),x'0a0a') Cultures,
    count() Nb_cult,
    count(C.Requise_A) Nb_requises
FROM Cu_asso3 C
GROUP BY Culture,Association,Espèce_A;

CREATE VIEW Cu_asso5 AS SELECT DISTINCT -- Regroupement des cultures qui se croisent en associations.
    C.Planche,
    C.Association,
    group_concat(C.Cultures,x'0a0a') Cultures,
    sum(C.Nb_cult) Nb_cult,
    count() Nb_espèces,
    sum(C.Nb_requises) Nb_requises
FROM Cu_asso4 C
GROUP BY Culture,Association;

CREATE VIEW Cu_asso6 AS SELECT -- Associations réelles.
    C.Planche,
    C.Association,
    C.Cultures,
    C.Nb_cult,
    C.Nb_espèces,
    A.Espèces
FROM Cu_asso5 C
JOIN Associations__Espèces A USING(Association)
WHERE (C.Nb_cult>1)AND(C.Nb_requises>0)AND(Nb_espèces>1);

CREATE VIEW Cu_non_ter_asso AS SELECT -- Cultures annuelles et vivaces non terminées
    C.Planche,
    C.Culture,
    C.Espèce,
    C.IT_plante,
    C.Variété,
    C.Type,
    C.Etat,
    C.Date_semis,
    C.Date_plantation,
    C.Début_récolte,
    C.Fin_récolte,
    C.Terminée,
    NULL Planche_influente
FROM Cultures C WHERE NOT((C.Terminée NOTNULL)AND(C.Terminée!='v')AND(C.Terminée!='V'))
UNION
SELECT -- Vivaces non terminées sur leurs planches_influencées
    PI.Planche,
    CI.Culture,
    CI.Espèce,
    CI.IT_plante,
    CI.Variété,
    CI.Type,
    CI.Etat,
    CI.Date_semis,
    CI.Date_plantation,
    CI.Début_récolte,
    CI.Fin_récolte,
    CI.Terminée,
    CI.Planche Planche_influente
FROM Cultures CI
JOIN Planches P USING(Planche)
JOIN Planches PI ON P.Planches_influencées||',' LIKE '%'||PI.Planche||',%'
WHERE CI.Terminée='v' OR CI.Terminée='V';

CREATE VIEW Cu_planif AS SELECT -- Dates théoriques au plus tôt, tenant compte des dates déjà existantes.
    CDT.Culture,
    CDT.Date_semis,
    DATE(CDT.Date_plantation,coalesce((julianday(C.Date_semis)-julianday(CDT.Date_semis)),0)||' days') Date_plantation,
    iif(CDT.Terminée ISNULL,
    DATE(CDT.Début_récolte,coalesce((julianday(C.Date_plantation)-julianday(CDT.Date_plantation)), -- Anuelle, décaler date de récolte avec retard de MEP.
                                    (julianday(C.Date_semis)-julianday(CDT.Date_semis)),0)||' days'),
    DATE(CDT.Début_récolte,
         max(coalesce(CDT.PJ,0)-(substr(CDT.Début_récolte,1,4)-substr(coalesce(C.Date_plantation,C.Date_semis),1,4)) -- Nb année en place
             ,0)||' years')) Début_récolte, -- Vivace, décaler date de récolte avec PJ.
    iif(CDT.Terminée ISNULL,
        DATE(CDT.Fin_récolte,coalesce((julianday(C.Date_plantation)-julianday(CDT.Date_plantation)), -- Anuelle, décaler date de récolte avec retard de MEP.
                                      (julianday(C.Date_semis)-julianday(CDT.Date_semis)),0)||' days'),
        DATE(CDT.Fin_récolte,
             max(coalesce(CDT.PJ,0)-(substr(CDT.Début_récolte,1,4)-substr(coalesce(C.Date_plantation,C.Date_semis),1,4)) -- Nb année en place
                 ,0)||' years')) Fin_récolte -- Vivace, décaler date de récolte avec PJ.
FROM Cu_dates_théo CDT
LEFT JOIN Cultures C USING(Culture);

CREATE VIEW Cu_récolte AS SELECT
    C.Culture,
    count(*) Nb_réc,
    min(R.Date) Date_min,
    max(R.Date) Date_max,
    sum(R.Quantité) Quantité,
    iif(count(R.Réc_ter)>0,'x',NULL) Réc_ter
FROM Cultures C
JOIN Récoltes_saison R USING(Culture)
WHERE R.Saison=C.Saison
GROUP BY Culture
ORDER BY Culture;

-- CREATE VIEW Cu_répartir_fertilisation AS SELECT
--     C.Culture,C.Espèce,C.Planche,C.Longueur*P.Largeur Surface,
--     DATE(coalesce(C.Date_plantation,C.Date_semis),
--          '-'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='Ferti_avance')||' days') Début_fertilisation_possible,
--     DATE(C.Début_récolte,
--          '+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='Ferti_retard')||' days') Fin_fertilisation_possible
-- FROM Cultures C
-- JOIN Planches P USING(Planche);

CREATE VIEW Cu_répartir_récolte AS SELECT
    C.Culture,C.Espèce,C.Planche,C.Longueur,
    max(DATE(C.Début_récolte,
             '-'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_avance')||' days'),
    DATE(coalesce(C.Date_plantation,C.Date_semis),
         '+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_après_MEP')||' days')) Début_récolte_possible,
    DATE(C.Fin_récolte,
         '+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_prolongation')||' days') Fin_récolte_possible,
    C.Récolte_faite,
    C.Terminée
FROM Cultures C
WHERE ((Semis_fait NOTNULL AND Date_plantation ISNULL)OR -- SD semé
       Plantation_faite NOTNULL); -- Plant ou SPep planté

CREATE VIEW Cu_ter AS SELECT
    *
FROM Cultures
WHERE (Terminée NOTNULL)AND(Terminée!='v')AND(Terminée!='V');

-- CREATE VIEW Cu_à_fertiliser AS SELECT
--     substr(C.Planche,1,(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Ilot_nb_car')) Ilot,
--     C.Planche,
--     C.Culture,
--     E.Espèce,
--     coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante,
--     C.Type,
--     C.Etat,
--     C.Color,
--     coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
--     C.Début_récolte,
--     C.Fin_récolte,
--     (SELECT max(CEP2.Fin_récolte) FROM Cu_en_place CEP2 WHERE (CEP2.Planche=C.Planche)AND --Cultures sur la même planche
--                                                               (CEP2.Début_récolte<coalesce(C.Date_plantation,C.Date_semis)) -- dont la récolte commence avant la MEP de la culture courante
--                                                               ) Pl_libre_le,
--     C.Longueur,
--     C.Longueur*PL.Largeur Surface,
--     coalesce(CASE WHEN C.Espacement>0 THEN round(1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100)
--                   ELSE round(I.Dose_semis/E.Dose_semis*100)
--                   END,100) Densité_pc,
--     E.N N_esp,
--     E.★N ★N_esp,
--     E.P P_esp,
--     E.★P ★P_esp,
--     E.K K_esp,
--     E.★K ★K_esp,
--     PL.Analyse,
--     (SELECT sum(N) FROM Fertilisations F WHERE F.Culture=C.Culture) N_fert,
--     (SELECT sum(P) FROM Fertilisations F WHERE F.Culture=C.Culture) P_fert,
--     (SELECT sum(K) FROM Fertilisations F WHERE F.Culture=C.Culture) K_fert,
--     -- (SELECT CF.Fertilisant FROM Cu_Fertilisants CF WHERE CF.Culture=C.Culture) Fertilisant,
--     -- (SELECT CF.Quantité FROM Cu_Fertilisants CF WHERE CF.Culture=C.Culture) Quantité,
--     C.A_faire,
--     C.Notes,
--     PL.Notes N_Planche,
--     E.Notes N_espèce
-- FROM Cultures C
-- LEFT JOIN Espèces E USING (Espèce)
-- LEFT JOIN ITP I USING (IT_plante)
-- LEFT JOIN Planches PL USING (Planche)
-- WHERE (E.N+E.P+E.K>0)AND
--       -- Cultures prévues
--       (((C.Culture IN(SELECT CAV.Culture FROM Cu_à_venir CAV))AND
--         (coalesce(C.Date_plantation,C.Date_semis) < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_fertiliser')||' days')))OR
--       -- Cultures en place dont la récolte n'est pas commencée.
--        ((C.Culture IN(SELECT CEP3.Culture FROM Cu_en_place CEP3))AND
--         (C.Récolte_faite ISNULL))) -- coalesce(C.Récolte_faite,'') NOT LIKE 'x%'
-- ORDER BY coalesce(Date_plantation,Date_semis),C.Espèce,C.IT_plante;

CREATE VIEW Cu_à_irriguer AS SELECT
    C.Planche,
    C.Culture
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
WHERE  (E.Irrig NOTNULL)AND
       (coalesce(C.Date_plantation,C.Date_semis) < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_Irrig_avant_MEP')||' days'))AND
       (coalesce(C.Date_plantation,C.Date_semis) > DATE('now','-'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_Irrig_après_MEP')||' days'));

CREATE VIEW Cu_à_venir AS SELECT --- Cultures prévues mais pas encore en place sur leur planche.
                                 --- Sont incluses les cultures déjà semées en pépinière.
    *
FROM Cu_non_ter
WHERE NOT ((Semis_fait NOTNULL AND Date_plantation ISNULL) OR -- SD semé
           Plantation_faite NOTNULL) -- Plant ou SPep planté
ORDER BY Planche;

CREATE VIEW Cultures__A_faire AS SELECT --- Cultures dont le champ 'A faire' n'est pas vide.
                                        --- Les cultures dont vous effacez le champ 'A faire' restent visibles jusqu'à la fermeture de l'onglet.
    ---row_summary Culture,Planche,Variété_ou_It_plante,Type,Etat,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    C.Planche, ---
    C.Culture, ---
    C.Espèce, ---
    CAST(coalesce(C.Variété,C.IT_plante) AS TEXT) Variété_ou_It_plante, --- ::Cultures__Succ_planches.Variété_ou_It_plante
    C.Type, ---
    C.Etat, ---
    C.color, ---
    C.A_faire, ---
    C.Date_semis, ---
    C.Semis_fait, ---
    C.Date_plantation, ---
    C.Plantation_faite, ---
    C.Début_récolte, ---
    C.Fin_récolte, ---
    C.Récolte_faite, ---
    C.Terminée, ---
    C.Longueur, ---
    C.Nb_rangs, ---
    C.Espacement, ---
    C.Notes, ---
    I.Notes N_IT_plante, ---
        ---multiline
    PL.Notes N_Planche, ---
        ---multiline
    E.Notes N_espèce ---
        ---multiline
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (C.A_faire NOTNULL)
ORDER BY C.Planche,C.Espèce,C.IT_plante ---
;

CREATE VIEW Cultures__Succ_planches AS SELECT --- Planches et leurs cultures semées ou plantées dans la période.
    ---row_summary Planche,Culture,Variété_ou_It_plante,Saison
    Planche, ---
    Culture, ---
    Variété_ou_It_plante, --- Variété, à défaut l'itinéraire technique.

    (SELECT value FROM Prm_draw WHERE name='GraphBarresVert36Mois')||
    Graph||'offset('||CAST(CAST(strftime('%j',DATE('now')) AS INT)+365 AS TEXT)||')'||
           iif(Irrig NOTNULL, -- Ligne bleu pour date du jour
               'rect(0,0,2,25,#007AFF,255)'||
                    iif(Planche!=(lag(Planche) OVER(ORDER BY Planche,Date_MEP)),'text('||Irrig||',5,,#007AFF,255,b)',''),
               'line(0,,,,#ff0000,255)') -- Ligne rouge pour date du jour
        Graph, --- Graphique calculé à partir des dates de semis, plantation et récolte.
            ---draw GraphTitres36Mois

    -- CAST(TEMPO_NP || CASE WHEN true THEN coalesce(Irrig,'') ELSE iif(Irrig NOTNULL,'x','') END AS TEXT) TEMPO_NP, --- --Planche!=Lag
    -- CAST(TEMPO_N  || CASE WHEN true THEN coalesce(Irrig,'') ELSE iif(Irrig NOTNULL,'x','') END AS TEXT) TEMPO_N, ---
    -- CAST(TEMPO_NN || CASE WHEN true THEN coalesce(Irrig,'') ELSE iif(Irrig NOTNULL,'x','') END AS TEXT) TEMPO_NN, ---

    Saison, ---
    Longueur, ---
    Nb_rangs, ---
    Surface, --- --UNIT(m²)
    A_faire, ---
    Notes ---
FROM Cultures__Succ_planches2
ORDER BY Planche,Date_MEP ---
;

CREATE VIEW Cultures__Succ_planches2 AS SELECT
    C.Planche,
    C.Culture,
    CAST(coalesce(C.Variété,C.IT_plante,C.Espèce) AS TEXT) Variété_ou_It_plante,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,

    iif(substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+1)AS TEXT),
        'offset(730)','')||
    iif(substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture'))AS TEXT),
        'offset(365)','')||
    C.Graph Graph,

    -- CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')-1)AS TEXT)
    --      THEN TEMPO
    --      ELSE ':::::::::::'
    --      END TEMPO_NP,
    -- CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
    --      THEN TEMPO
    --      ELSE ':::::::::::'
    --      END TEMPO_N,
    -- CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+1)AS TEXT)
    --      THEN TEMPO
    --      ELSE ':::::::::::'
    --      END TEMPO_NN,

    C.Saison,
    C.Longueur,
    C.Nb_rangs,
    CAST(C.Longueur*PL.Largeur AS REAL) Surface,
    PL.Irrig,
    E.Irrig Irrig_E,
    A_faire,
    C.Notes
FROM Cultures__Tempo C
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Planches PL USING(Planche)
WHERE (Planche NOTNULL)AND
      (coalesce(Date_plantation,Date_semis) BETWEEN ((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')-1)||'-01-01' AND
                                                    ((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+1)||'-12-31');

CREATE VIEW Cultures__Tempo AS SELECT
    *,

    --(SELECT value FROM Prm_draw WHERE name='GraphBarresVert24Mois')||
    replace(replace(replace(replace('rect(xRec,0,wRec,5,cRec,aRec)', -- semis
        'xRec',J_semis),
        'wRec',NJ_semis),
        'cRec',iif(Date_plantation NOTNULL,'#ff6000','#76c801')),
        'aRec',iif(Semis_fait NOTNULL,255,130))||
    replace(replace(replace(replace('rect(xRec,0,wRec,5,cRec,100,gRec)', -- Attente plantation.
        'xRec',J_semis+NJ_semis),
        'wRec',iif(Date_semis NOTNULL AND Date_plantation NOTNULL,J_plantation-J_semis-NJ_semis,0)),
        'cRec',iif(Date_plantation NOTNULL,'#ff6000','#76c801')),
        'gRec',iif(Terminée NOTNULL AND Date_semis NOTNULL AND Date_plantation NOTNULL AND Début_récolte NOTNULL AND Fin_récolte NOTNULL AND
                   Semis_fait NOTNULL AND Plantation_faite ISNULL AND Récolte_faite ISNULL,'h+',''))|| -- Dépérissement en pépinière
    replace(replace(replace('rect(xRec,3,wRec,10,#76c801,aRec)', -- plantation
        'xRec',J_plantation),
        'wRec',NJ_plantation),
        'aRec',iif(Plantation_faite NOTNULL,255,180))||
    replace(replace(replace('rect(xRec,3,wRec,10,#76c801,150,gRec)', -- Attente récolte.
        'xRec',iif(Date_plantation NOTNULL,J_plantation+NJ_plantation,J_semis+NJ_semis)),
        'wRec',iif(Terminée ISNULL OR (Semis_fait NOTNULL AND Date_plantation ISNULL) OR Plantation_faite NOTNULL,
                   iif(Date_plantation NOTNULL,J_récolte-J_plantation-NJ_plantation,J_récolte-J_semis-NJ_semis),
                   0)),
        'gRec',iif(Terminée NOTNULL AND (Date_semis NOTNULL OR Date_plantation NOTNULL) AND
                   (Semis_fait NOTNULL OR Plantation_faite NOTNULL) AND
                   (Début_récolte NOTNULL OR -- Dépérissement en place (récolte pas faite)
                    (Début_récolte ISNULL AND Fin_récolte ISNULL)) AND -- Date de fin culture inconnue
                   Récolte_faite ISNULL,'h+',''))||
    replace(replace(replace(replace('rect(xRec,8,wRec,6,#bf00ff,aRec,gRec)', -- récolte
        'xRec',J_récolte),
        'wRec',NJ_récolte),
        'aRec',iif(Récolte_faite ISNULL,100,255)),
        'gRec',iif(Récolte_faite NOTNULL AND coalesce(Récolte_faite,'') NOT LIKE 'x%','h+,,100',''))||
    'text('||Espèce||','||iif(Date_plantation ISNULL,J_semis+NJ_semis+3,J_plantation+NJ_plantation+3)||',2)'
    Graph

    -- CAST(iif(Date_semis ISNULL,0,CAST(jdS-jd0 AS INTEGER))||':'||
    --      iif(Date_semis ISNULL,0,CAST(jdS-jd0 AS INTEGER)+4)||':'||
    --      iif(Date_plantation ISNULL,0,CAST(jdP-jd0 AS INTEGER))||':'||
    --      iif(Date_plantation ISNULL,0,CAST(jdP-jd0 AS INTEGER)+4)||':'||
    --      iif(Début_récolte ISNULL,0,CAST(jdDR-jd0 AS INTEGER))||':'||
    --      iif(Fin_récolte ISNULL,0,CAST(jdFR-jd0 AS INTEGER)+iif(Début_récolte=Fin_récolte,4,0))||':'||
    --      Espèce||':'||
    --      coalesce(Semis_fait,'')||':'||
    --      coalesce(Plantation_faite,'')||':'||
    --      coalesce(Récolte_faite,'')||':'||
    --      coalesce(Terminée,'')||':' AS TEXT) TEMPO
FROM Cultures__Tempo2;

CREATE VIEW Cultures__Tempo2 AS SELECT
    *,

    coalesce(CAST(julianday(Date_semis)-julianday(substr(coalesce(Date_plantation,Date_semis),1,4)||'-01-01') AS INT),0) J_semis,
    iif(Semis_fait NOTNULL,4,iif(Terminée ISNULL AND Date_semis NOTNULL,
                                   coalesce((SELECT coalesce(I.Décal_max,1)*7 FROM ITP I WHERE I.IT_plante=Cultures.IT_plante),0),
                                   0)) NJ_semis,
    coalesce(CAST(julianday(Date_plantation)-julianday(substr(coalesce(Date_plantation,Date_semis),1,4)||'-01-01') AS INT),0) J_plantation,
    iif(Plantation_faite NOTNULL,4,iif(Terminée ISNULL AND Date_plantation NOTNULL,
                                         coalesce((SELECT coalesce(I.Décal_max,1)*7 FROM ITP I WHERE I.IT_plante=Cultures.IT_plante),0),
                                         0)) NJ_plantation,
    coalesce(CAST(julianday(coalesce(Début_récolte,Fin_récolte))-julianday(substr(coalesce(Date_plantation,Date_semis),1,4)||'-01-01') AS INT),365) J_récolte,
    iif(Terminée ISNULL OR Récolte_faite LIKE 'x%',coalesce(max(CAST(julianday(Fin_récolte)-julianday(Début_récolte) AS INT),4),0),0) NJ_récolte

    -- julianday(substr(coalesce(Date_plantation,Date_semis),1,4)||'-01-01') jd0,
    -- julianday(Date_semis) jdS,
    -- julianday(Date_plantation) jdP,
    -- julianday(Début_récolte) jdDR,
    -- julianday(Fin_récolte) jdFR
FROM Cultures
WHERE (coalesce(Terminée,'') NOT LIKE 'v%'); -- Pas pour les vivaces qui vivent plusieurs années.

CREATE VIEW Cultures__analyse AS SELECT --- Cultures annuelles récoltées, terminées et significatives (champ 'Terminée' différent de '...NS').
                                        --- Les engrais vert, compagnes et vivaces ne sont pas analysées.
    ---no_data_text Il n'y a aucune culture annuelle terminée et significative (champ 'Terminée' différent de '...NS') pour le moment.
    ---row_summary Culture,Planche,IT_plante,Variété,Type,Etat,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    C.Culture, ---
    C.Espèce, ---
    C.IT_plante, ---
    C.Variété, ---
    C.Planche, ---
    C.Type Type_culture, ---
    C.Saison, ---
    iif(C.Semis_fait NOTNULL,C.Date_semis,NULL) Date_semis, ---
    iif(C.Plantation_faite NOTNULL,C.Date_plantation,NULL) Date_plantation, ---
    iif(C.Récolte_faite NOTNULL,C.Début_récolte,NULL) Début_récolte, ---
    iif(C.Récolte_faite NOTNULL,C.Fin_récolte,NULL) Fin_récolte, ---
    CAST(C.Longueur*PL.Largeur AS REAL) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
    CAST(round((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc, --- ::Cultures__non_terminées.Qté_réc
        ---unit kg
    CAST(round(((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture)/(C.Longueur*PL.Largeur)),3)AS REAL) Rendement_C, --- Production de cette culture.
        ---unit kg/m²
    E.Rendement, ---
        ---unit kg/m²
    CAST(CASE WHEN E.Rendement NOTNULL THEN ((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture)/(C.Longueur*PL.Largeur))/E.Rendement*100
              ELSE NULL
              END AS INT) Couv_pc, --- Pourcentage de couverture du rendement théorique de l'espèce.

    (SELECT value FROM Prm_draw WHERE name='GraphBarresVert24Mois')||Graph Graph, --- Graphique calculé à partir des semaines de semis, plantation et récolte.
        ---draw GraphTitres24Mois

    -- TEMPO, ---
    PL.Type Type_planche, ---
    C.Notes ---
FROM Cultures__Tempo C
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Planches PL USING(Planche)
WHERE (C.Type IN('Semis pépinière','Plant','Semis en place')) AND C.Terminée NOTNULL AND (C.Terminée NOT LIKE '%NS')
ORDER BY C.Espèce,C.IT_plante,coalesce(C.Date_semis,C.Date_plantation) ---
;

CREATE VIEW Espèces__analyse AS SELECT ---
       ---row_summary Espèce,Nb_cult
       Espèce, ---
       count(*) Nb_cult, --- --UNIT(cultures)
       sum(Surface) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
       sum(Qté_réc) Qté_réc, --- ::Cultures__non_terminées.Qté_réc
        ---unit kg
       round(sum(Qté_réc)/sum(Surface),2) Rend_réel, --- Rendement réel.
        ---unit kg/m²
       Rendement, ---
       round(sum(Qté_réc)/sum(Surface)/Rendement*100) Couv_pc ---
FROM Cultures__analyse
WHERE Espèce NOT NULL
GROUP BY Espèce,Rendement;

CREATE VIEW Espèces__analyse_saison AS SELECT ---
       ---row_summary Espèce,Saison,Nb_cult
       Espèce, ---
       Saison, ---
       count(*) Nb_cult, --- --UNIT(cultures)
       sum(Surface) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
       sum(Qté_réc) Qté_réc, --- ::Cultures__non_terminées.Qté_réc
        ---unit kg
       round(sum(Qté_réc)/sum(Surface),2) Rend_réel, --- Rendement réel.
        ---unit kg/m²
       Rendement, ---
       round(sum(Qté_réc)/sum(Surface)/Rendement*100) Couv_pc ---
FROM Cultures__analyse
WHERE Espèce NOT NULL
GROUP BY Espèce,Saison,Rendement
ORDER BY Espèce,Saison ---
;

CREATE VIEW Espèces__analyse_type_planche AS SELECT ---
       ---row_summary Espèce,Type_planche,Nb_cult
       Espèce, ---
       Type_planche, ---
       count(*) Nb_cult, --- --UNIT(cultures)
       sum(Surface) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
       sum(Qté_réc) Qté_réc, --- ::Cultures__non_terminées.Qté_réc
        ---unit kg
       round(sum(Qté_réc)/sum(Surface),2) Rend_réel, --- Rendement réel.
        ---unit kg/m²
       Rendement, ---
       round(sum(Qté_réc)/sum(Surface)/Rendement*100) Couv_pc ---
FROM Cultures__analyse
WHERE Espèce NOT NULL
GROUP BY Espèce,Type_planche,Rendement
ORDER BY Espèce,Type_planche ---
;

CREATE VIEW ITP__analyse AS SELECT ---
       IT_plante, ---
       count(*) Nb_cult, --- --UNIT(cultures)
       sum(Surface) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
       sum(Qté_réc) Qté_réc, --- ::Cultures__non_terminées.Qté_réc
        ---unit kg
       round(sum(Qté_réc)/sum(Surface),2) Rendement_ITP, ---
        ---unit kg/m²
       Rendement, ---
        ---unit kg/m²
       round(sum(Qté_réc)/sum(Surface)/Rendement*100) Couv_pc ---
FROM Cultures__analyse
WHERE IT_plante NOT NULL
GROUP BY IT_plante,Rendement
ORDER BY IT_plante ---
;

CREATE VIEW Variétés__analyse AS SELECT ---
       Variété, ---
       count(*) Nb_cult, --- --UNIT(cultures)
       round(sum(Surface),2) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²) -- round et non CAST pour pas avoir l'erreur de décimale dans rowsumary
       round(sum(Qté_réc),3) Qté_réc, --- ::Cultures__non_terminées.Qté_réc
        ---unit kg
       round(sum(Qté_réc)/sum(Surface),2) Rendement_V, ---
        ---unit kg/m²
       Rendement, ---
        ---unit kg/m²
       round(sum(Qté_réc)/sum(Surface)/Rendement*100) Couv_pc ---
FROM Cultures__analyse
WHERE Variété NOT NULL
GROUP BY Variété,Rendement
ORDER BY Variété ---
;

CREATE VIEW Cultures__inc_dates AS SELECT --- Cultures non terminées ou terminées significatives (champ 'Terminée' différent de '...NS') dont une date (semis, plantation, début récolte, fin récolte) est trop éloignée de la période prévue sur l'ITP (paramètre 'Tolérance_...').
    ---row_summary Culture,Planche,Variétés_ou_Espèce,Type,Etat,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    C.Culture, ---
    C.IT_plante, ---
    CAST(coalesce(C.Variété,C.Espèce) AS TEXT) Variétés_ou_Espèce, ---
    C.Planche, ---
    C.Type, ---
    C.Etat, ---
    C.color, ---
    CID.Incohérence, ---
    I.S_semis, ---
    C.Date_semis, ---
    C.Semis_fait, ---
    I.S_plantation, ---
    C.Date_plantation, ---
    C.Plantation_faite, ---
    coalesce(I.S_récolte,V.S_récolte) S_récolte, ---
    coalesce(I.D_récolte,V.D_récolte) D_récolte, ---
    I.Décal_max, ---
    C.Début_récolte, ---
    C.Fin_récolte, ---
    C.Récolte_faite, ---
    C.Terminée, ---
    C.A_faire, ---
    C.Notes ---
FROM Cultures C
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Variétés V USING(Variété)
LEFT JOIN Cu_inc_dates CID USING(Culture)
WHERE (coalesce(C.Terminée,'') NOT LIKE '%NS')AND((C.Type LIKE '%!')OR(CID.Incohérence NOTNULL))
ORDER BY Culture ---
;

CREATE VIEW Cultures__non_terminées AS SELECT --- Cultures dont le champ 'Terminé' est vide.
                                              ---
                                              --- Une 'culture' c'est une plante (espèce,variété) sur une planche avec un itinéraire technique.
                                              --- Si la même plante est présente sur plusieurs planches, il y a une culture (numérotée) par planche.
    ---row_summary Culture,Planche,IT_plante,Variété,Type,Etat,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    C.Planche, ---
    C.Culture, ---
    C.Espèce, ---
    C.IT_plante, ---
    C.Variété, ---
    C.Fournisseur, ---
    C.Type, ---
    C.Saison, ---
    CAST(CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                   ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!= (SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété))
              THEN 'Err. variété !'
              ELSE C.Etat
              END AS TEXT) Etat, ---
    C.color, ---
    C.D_planif, ---
    C.Date_semis, ---
    C.Semis_fait, ---
    C.Date_plantation, ---
    C.Plantation_faite, ---
    C.Début_récolte, ---
    C.Fin_récolte, ---
    C.Récolte_faite, ---
    CAST(round((SELECT Quantité FROM Cu_récolte R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc, --- Quantité totale récoltée.
        ---unit kg
    C.Terminée, ---
    C.Longueur, ---
    C.Nb_rangs, ---
    C.Espacement, ---
    I.Nb_graines_plant, ---
        ---unit graines/plant
    I.Dose_semis, ---
        ---unit g/m²
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100)AS INTEGER) Nb_plants, --- --UNIT(plants)
    CAST(CASE WHEN C.Espacement>0 THEN 1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100
              ELSE I.Dose_semis/E.Dose_semis*100
              END AS INT) Densité_pc, --- Densité réelle, relative à la densité de référence noté sur l'espèce.
    C.A_faire, ---
    C.Notes, ---
    I.Notes N_IT_plante, ---
        ---multiline
    PL.Notes N_Planche ---
        ---multiline
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
ORDER BY C.Planche,coalesce(C.Date_semis,C.Date_plantation),C.Espèce,C.IT_plante ---
;

CREATE VIEW Cultures__vivaces AS SELECT --- Cultures vivaces (champ 'Terminée' commence par 'v' ou espèce vivace).
                                        --- Pendant la période de semis/plantation, la culture d'une vivace est gérée comme une annuelle (champ 'Terminée' vide).
    ---row_summary Culture,Planche,Variété_ou_It_plante,Type,Etat,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    C.Planche, ---
    C.Culture, ---
    coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante, --- ::Cultures__Succ_planches.Variété_ou_It_plante
    C.Etat, ---
    C.color, ---
    C.D_planif, ---
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP, --- Date de mise en place sur la planche de culture: date de semis (en place) ou de plantation.
    C.Début_récolte, ---
    C.Fin_récolte, ---
    C.Récolte_faite, ---
    E.S_taille, ---
    C.A_faire, ---
    C.Terminée, ---
        ---hidden
    C.Notes, ---
    I.Notes N_IT_plante, ---
        ---multiline
    PL.Notes N_Planche, ---
        ---multiline
    E.Irrig Irrig_espèce, --- Irrigation nécessaire pour l'espèce cultivée.
    E.Besoins, ---
        ---multiline
    E.Notes N_espèce ---
        ---multiline
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE  (C.Terminée LIKE 'v%')OR(E.Vivace NOTNULL)
ORDER BY C.Planche,coalesce(C.Variété,C.IT_plante) ---
;

-- CREATE VIEW Cultures__à_fertiliser AS SELECT --- Bilan de fertilisation pour les cultures prévues (paramètre 'C_horizon_fertiliser') ou en place.
--     ---row_summary Planches,Espèce,Type,Etat
--     CAST(group_concat(C.Planche,x'0a0a') AS TEXT) Planches, ---
--         ---multiline
--     CAST(group_concat(C.Culture||' ',x'0a0a') AS TEXT) Cultures, ---
--         ---multiline
--     C.Espèce, ---
--     CAST(group_concat(C.Variété_ou_It_plante,x'0a0a') AS TEXT) Variétés_ou_It_plante, --- ::Cultures__Succ_planches.Variété_ou_It_plante
--         ---multiline
--     Type, ---
--     CAST(CASE WHEN C.Etat='Prévue' OR C.Etat='Pépinière' THEN 'A venir' ELSE C.Etat END AS TEXT) Etat, ---
--     CAST(CASE WHEN C.Etat='Prévue' OR C.Etat='Pépinière' THEN NULL ELSE C.color END AS TEXT) color, ---
--     max(C.Pl_libre_le) Pl_libre_le, --- Plus grande date de fin de récolte des cultures précédentes sur ces planches.
--     C.Date_MEP, --- ::Cultures__vivaces.Date_MEP
--     min(C.Début_récolte) Début_récolte, ---
--     max(C.Fin_récolte) Fin_récolte, ---
--     CAST(sum(C.Surface) AS INT) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
--     CAST(min(sum(coalesce(C.N_fert,0))/total(C.N_esp*C.Surface*C.Densité_pc/100),
--              sum(coalesce(C.P_fert,0))/total(C.P_esp*C.Surface*C.Densité_pc/100),
--              sum(coalesce(C.K_fert,0))/total(C.K_esp*C.Surface*C.Densité_pc/100))*100 AS INTEGER) Fert_pc, --- Pourcentage de fertilisation déjà effectué pour l'élément N, P ou K le plus déficitaire.
--     CAST(CAST(sum(C.N_esp*C.Surface*C.Densité_pc/100) AS INTEGER)||'-'||
--          CAST(sum(C.P_esp*C.Surface*C.Densité_pc/100) AS INTEGER)||'-'||
--          CAST(sum(C.K_esp*C.Surface*C.Densité_pc/100) AS INTEGER) AS TEXT) Besoins_NPK, --- Besoins en Azote - Phosphore - Potassium, pour les planches.
--                                                                                         --- Besoin de l'espèce x densité réelle x surface de planche.
--         ---unit g
--     C.★N_esp, --- Besoin qualitatif de l'espèce.
--         ---cond_formats "::"=="Elevé" #darkRed# #lightRed#,"::"=="Faible" #darkGreen# #lightGreen#
--     C.★P_esp, --- ::Cultures__à_fertiliser.★N_esp
--         ---cond_formats "::"=="Elevé" #darkRed# #lightRed#,"::"=="Faible" #darkGreen# #lightGreen#
--     C.★K_esp, --- ::Cultures__à_fertiliser.★N_esp
--         ---cond_formats "::"=="Elevé" #darkRed# #lightRed#,"::"=="Faible" #darkGreen# #lightGreen#
--     CAST(A.N||'-'||A.P||'-'||A.K||' ('||A.Analyse||')'||x'0a0a'||
--          coalesce(A.Interprétation,'Pas d''interprétation') AS TEXT) Analyse_sol, --- Teneur NPK (g/kg) et interprétation de l'analyse de sol de la planche.
--         ---multiline
--     A.☆N ☆N_sol, --- Teneur qualitative du sol.
--         ---cond_formats "::"=="Faible" #darkRed# #lightRed#,"::"=="Elevé" #darkGreen# #lightGreen#
--     A.☆P ☆P_sol, --- ::Cultures__à_fertiliser.☆N_sol
--         ---cond_formats "::"=="Faible" #darkRed# #lightRed#,"::"=="Elevé" #darkGreen# #lightGreen#
--     A.☆K ☆K_sol, --- ::Cultures__à_fertiliser.☆N_sol
--         ---cond_formats "::"=="Faible" #darkRed# #lightRed#,"::"=="Elevé" #darkGreen# #lightGreen#
--     CAST(CAST(sum(C.N_fert) AS INTEGER)||'-'|| CAST(sum(C.P_fert) AS INTEGER)||'-'|| CAST(sum(C.K_fert) AS INTEGER) AS TEXT) Apports_NPK, --- Sommes des fertilisations (Azote - Phosphore - Potassium) déjà faites sur les planches.
--         ---unit g
--     CAST(sum(C.N_esp*C.Surface*C.Densité_pc/100)-sum(coalesce(C.N_fert,0)) AS INT) N_manq, --- Quantité d'azote manquante.
--         ---unit g
--         ---cond_formats ::>0 #darkRed# #lightRed#,::<0 #darkGreen# #lightGreen#
--     CAST(sum(C.P_esp*C.Surface*C.Densité_pc/100)-sum(coalesce(C.P_fert,0)) AS INT) P_manq, --- Quantité de phosphore manquante.
--         ---unit g
--         ---cond_formats ::>0 #darkRed# #lightRed#,::<0 #darkGreen# #lightGreen#
--     CAST(sum(C.K_esp*C.Surface*C.Densité_pc/100)-sum(coalesce(C.K_fert,0)) AS INT) K_manq, --- Quantité de potassium manquante.
--         ---unit g
--         ---cond_formats ::>0 #darkRed# #lightRed#,::<0 #darkGreen# #lightGreen#
--     C.A_faire, ---
--     C.Notes, ---
--     C.N_Planche, ---
--         ---multiline
--     C.N_espèce ---
--         ---multiline
-- FROM Cu_à_fertiliser C
-- LEFT JOIN Analyses_de_sol A USING(Analyse)
-- GROUP BY C.Ilot,C.Espèce,C.Type,C.Etat,C.Date_MEP,Analyse_sol,C.A_faire,C.Notes,C.N_Planche,C.N_espèce
-- ORDER BY Date_MEP ---
-- ;

CREATE VIEW Cultures__à_irriguer AS SELECT --- Planches:
                                           --- - sans irrigation et ayant des cultures qui nécessitent irrigation
                                           --- - avec irrigation en place mais pas de culture en ayant besoin (paramètres 'C_Irrig_avant_MEP' et 'C_Irrig_après_MEP').
                                           --- Cultures annuelles non terminées et cultures de vivace avant leur 1ère récolte (champ 'Terminée' vide).
    ---row_summary Planche,Culture,Variété_ou_It_plante,Saison
    CaI.Planche, ---
    CaI.Culture, ---
    CaI.Variété_ou_It_plante, --- ::Cultures__Succ_planches.Variété_ou_It_plante
    CaI.Irrig_planche, --- Irrigation en place sur la planche.
    CaI.Irrig_espèce, --- Irrigation nécessaire pour l'espèce cultivée.

    iif(CAST(strftime('%j',DATE('now')) AS INT)>365/2,
        (SELECT value FROM Prm_draw WHERE name='GraphBarresVert24Mois')||'offset(-365)',
        (SELECT value FROM Prm_draw WHERE name='GraphBarresVert36Mois'))||
    CaI.Graph||'offset('||CAST(CAST(strftime('%j',DATE('now')) AS INT)+iif(CAST(strftime('%j',DATE('now')) AS INT)>365/2,0,365) AS TEXT)||')'||
               iif(Irrig_planche NOTNULL, -- Ligne bleu pour date du jour
                   'rect(0,0,2,25,#007AFF,255)'||
                        iif(Planche!=(lag(Planche) OVER(ORDER BY CaI.Planche,CaI.Date_MEP,CaI.Variété_ou_It_plante)),'text('||Irrig_planche||',5,,#007AFF,255,b)',''),
                   'line(0,,,,#ff0000,255)') -- Ligne rouge pour date du jour
        Graph, --- Graphique calculé à partir des dates de semis, plantation et récolte.
        ---draw GraphTitres36Mois

    -- CaI.TEMPO_NP, ---
    -- CAST(CaI.TEMPO_N  || iif(CaI.Irrig_planche NOTNULL,'x','') AS TEXT) TEMPO_N, ---
    -- CaI.TEMPO_NN, ---

    CaI.Saison, ---
    CaI.Longueur, ---
    CaI.Nb_rangs, ---
    CaI.Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
    CaI.A_faire, ---
    CaI.Notes ---
FROM Cultures__à_irriguer2 CaI
WHERE  ((CaI.Planche IN (SELECT CaI1.Planche FROM Cu_à_irriguer CaI1))AND(CaI.Irrig_planche ISNULL))OR
       ((NOT (CaI.Planche IN (SELECT CaI2.Planche FROM Cu_à_irriguer CaI2)))AND(CaI.Irrig_planche NOTNULL))
ORDER BY CaI.Planche,CaI.Date_MEP,CaI.Variété_ou_It_plante ---
;

CREATE VIEW Cultures__à_irriguer2 AS SELECT
    C.Planche,
    C.Culture,
    CAST(coalesce(C.Variété,C.IT_plante,C.Espèce) AS TEXT) Variété_ou_It_plante,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,

    iif(substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+1)AS TEXT),
        'offset(+730)','')||
    iif(substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture'))AS TEXT),
        'offset(+365)','')||
    C.Graph Graph,

    -- CAST(CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')-1)AS TEXT)
    --          THEN TEMPO
    --          ELSE ':::::::::::'
    --          END AS TEXT) TEMPO_NP,
    -- CAST(CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
    --          THEN TEMPO
    --          ELSE ':::::::::::'
    --          END AS TEXT) TEMPO_N,
    -- CAST(CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+1)AS TEXT)
    --          THEN TEMPO
    --          ELSE ':::::::::::'
    --          END AS TEXT) TEMPO_NN,

    C.Saison,
    C.Longueur,
    C.Nb_rangs,
    CAST(C.Longueur*PL.Largeur AS REAL) Surface,
    PL.Irrig Irrig_planche,
    E.Irrig Irrig_espèce,
    C.A_faire,
    C.Notes
FROM Cultures__Tempo C
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Planches PL USING(Planche)
WHERE (Planche NOTNULL)AND
      (C.Terminée ISNULL)AND -- Que les annuelles non terminée.
      ((coalesce(Date_plantation,Date_semis) BETWEEN ((SELECT CAST(Valeur AS INT) FROM Params
                                                       WHERE Paramètre='Année_culture')-1)||'-01-01' AND
                                                     ((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+1)||'-12-31'));

CREATE VIEW Cultures__à_planter AS SELECT --- Cultures non terminées, déjà semées (ou à partir de plants), dont le champ 'Plantation_faite' est vide et dont la date de plantation est proche (paramètre 'C_horizon_plantation') ou passée.
    ---row_summary Culture,Planche,IT_plante,Variété,Type,Etat,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    C.Planche, ---
    C.Culture, ---
    C.Espèce, ---
    C.IT_plante, ---
    C.Variété, ---
    C.Fournisseur, ---
    C.Type, ---
    CAST(CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                   ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!= (SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété))
              THEN 'Err. variété !'
              ELSE C.Etat
              END AS TEXT) Etat, ---
    C.color, ---
    C.D_planif, ---
    C.Date_semis, ---
    C.Semis_fait, ---
    C.Date_plantation, ---
    C.Plantation_faite, ---
    C.Début_récolte, ---
    C.Longueur, ---
    PL.Largeur, ---
        ---unit m
    C.Nb_rangs, ---
    C.Espacement, ---
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100)AS INTEGER) Nb_plants, --- --UNIT(plants)
    CAST(1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100 AS INT) Densité_pc, --- ::Cultures__non_terminées.Densité_pc
    C.A_faire, ---
    C.Notes, ---
    I.Notes N_IT_plante, ---
        ---multiline
    PL.Notes N_planche, ---
        ---multiline
    E.Notes N_espèce ---
        ---multiline
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (Date_plantation < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_plantation')||' days'))AND
      (coalesce(Plantation_faite,'') NOT LIKE 'x%')AND
      ((Semis_fait NOTNULL)OR(Date_semis ISNULL))
ORDER BY C.Date_plantation,C.Planche,C.Espèce,C.IT_plante ---
;

CREATE VIEW Cultures__à_récolter AS SELECT --- Cultures non terminées, déjà semées/plantées, dont le champ 'Récolte_faite' ne commence pas par 'x' et dont la date de début de récolte est proche (paramètre 'C_horizon_récolte') ou passée.
    ---row_summary Culture,Planche,Variété_ou_It_plante,Type,Etat,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    C.Planche, ---
    C.Culture, ---
    C.Espèce, ---
    CAST(coalesce(C.Variété,C.IT_plante) AS TEXT) Variété_ou_It_plante, --- ::Cultures__Succ_planches.Variété_ou_It_plante
    C.Type, ---
    C.Etat, ---
    C.color, ---
    C.Date_semis, ---
    C.Semis_fait, ---
    C.Date_plantation, ---
    C.Plantation_faite, ---
    C.Début_récolte, ---
    C.Fin_récolte, ---
    C.Récolte_faite, ---
    CAST(round((SELECT Quantité FROM Cu_récolte R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc, --- ::Cultures__non_terminées.Qté_réc
        ---unit kg
    C.Terminée, ---
    C.Longueur, ---
    C.Nb_rangs, ---
    C.Espacement, ---
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100)AS INTEGER) Nb_plants, --- --UNIT(plants)
    C.A_faire, ---
    C.Notes, ---
    I.Notes N_IT_plante, ---
        ---multiline
    PL.Notes N_Planche, ---
        ---multiline
    E.Notes N_espèce ---
        ---multiline
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (Début_récolte < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_récolte')||' days'))AND
      (coalesce(Récolte_faite,'') NOT LIKE 'x%')AND
      ((Semis_fait NOTNULL)OR(Date_semis ISNULL))AND
      ((Plantation_faite NOTNULL)OR(Date_plantation ISNULL))
ORDER BY C.Début_récolte,C.Planche,C.Espèce,C.IT_plante ---
;

CREATE VIEW Cultures__à_semer AS SELECT --- Cultures non terminées dont le champ 'Semis_fait' est vide et dont la date de semis est proche (paramètre 'C_horizon_semis') ou passée.
    ---row_summary Culture,Planche,IT_plante,Variété,Type,Etat,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    C.Planche, ---
    C.Culture, ---
    C.Espèce, ---
    C.IT_plante, ---
    C.Variété, ---
    C.Fournisseur, ---
    C.Type, ---
    CAST(CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                   ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!= (SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété))
              THEN 'Err. variété !'
              ELSE C.Etat
              END AS TEXT) Etat, ---
    C.color, ---
    C.D_planif, ---
    C.Date_semis, ---
    C.Semis_fait, ---
    C.Date_plantation, ---
    C.Début_récolte, ---
    C.Longueur, ---
    PL.Largeur, ---
        ---unit m
    C.Nb_rangs, ---
    C.Espacement, ---
    I.Nb_graines_plant, ---
        ---unit graines/plant
    I.Dose_semis, ---
        ---unit g/m²
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100)AS INTEGER) Nb_plants, --- --UNIT(plants)
    CAST(CASE WHEN C.Espacement>0 THEN 1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100
              ELSE I.Dose_semis/E.Dose_semis*100
              END AS INT) Densité_pc, --- ::Cultures__non_terminées.Densité_pc
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant)AS INTEGER) Nb_graines, --- --UNIT(graines)
    CAST(round((CASE WHEN C.Espacement > 0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant / E.Nb_graines_g
                     ELSE C.Longueur * PL.Largeur * I.Dose_semis
                     END),2)AS REAL) Poids_graines, ---
        ---unit g
    C.A_faire, ---
    C.Notes, ---
    I.Notes N_IT_plante, ---
        ---multiline
    PL.Notes N_planche, ---
        ---multiline
    E.Notes N_espèce ---
        ---multiline
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (Date_semis < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_semis')||' days'))AND
      (coalesce(Semis_fait,'') NOT LIKE 'x%')
ORDER BY C.Date_semis,C.Date_plantation,C.Planche,C.Espèce,C.IT_plante ---
;

CREATE VIEW Cultures__à_semer_EP AS SELECT --- Cultures non terminées, à semer en place ('Date_plantation' vide), dont le champ 'Semis_fait' est vide et dont la date de semis est proche (paramètre 'C_horizon_semis') ou passée.
    ---row_summary Culture,Planche,IT_plante,Variété,Type,Etat,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    C.Planche, ---
    C.Culture, ---
    C.Espèce, ---
    C.IT_plante, ---
    C.Variété, ---
    C.Fournisseur, ---
    C.Type, ---
    CAST(CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                   ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!=(SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété))
              THEN 'Err. variété !'
              ELSE C.Etat
              END AS TEXT) Etat, ---
    C.color, ---
    C.D_planif, ---
    C.Date_semis, ---
    C.Semis_fait, ---
    C.Début_récolte, ---
    C.Longueur, ---
    PL.Largeur, ---
        ---unit m
    C.Nb_rangs, ---
    C.Espacement, ---
    I.Nb_graines_plant, ---
        ---unit graines/plant
    I.Dose_semis, ---
        ---unit g/m²
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100)AS INTEGER) Nb_plants, --- --UNIT(plants)
    CAST(CASE WHEN C.Espacement>0 THEN 1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100
             ELSE I.Dose_semis/E.Dose_semis*100
             END AS INT) Densité_pc, --- ::Cultures__non_terminées.Densité_pc
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant)AS INTEGER) Nb_graines, --- --UNIT(graines)
    CAST(round((CASE WHEN C.Espacement > 0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant/E.Nb_graines_g
                     ELSE C.Longueur*PL.Largeur*I.Dose_semis
                     END),2)AS REAL) Poids_graines, ---
        ---unit g
    C.A_faire, ---
    C.Notes, ---
    I.Notes N_IT_plante, ---
        ---multiline
    PL.Notes N_planche, ---
        ---multiline
    E.Notes N_espèce ---
        ---multiline
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (Date_semis < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_semis')||' days'))AND
      (coalesce(Semis_fait,'') NOT LIKE 'x%')AND
      (Date_plantation ISNULL)
ORDER BY C.Date_semis,C.Planche,C.Espèce,C.IT_plante ---
;

CREATE VIEW Cultures__à_semer_pep AS SELECT --- Cultures non terminées, semis en pépinière ('Date_plantation' non vide), dont le champ 'Semis_fait' est vide et dont la date de semis est proche (paramètre 'C_horizon_semis') ou passée.
    ---row_summary Cultures,Planches,IT_plante,Variété,Type,Etat,Date_semis|semis: :: -
    CAST(min(C.Culture)AS INTEGER) Culture,  --- Nécessaire pour surlignage des cellules modifiées.
        ---hidden
    CAST(group_concat(C.Planche,x'0a0a') AS TEXT) Planches, ---
        ---multiline
    CAST(group_concat(' '||C.Culture||' ',x'0a0a') AS TEXT) Cultures, ---
        ---multiline
    C.Espèce, ---
    C.IT_plante, ---
    C.Variété, ---
    C.Type, ---
    C.Etat, ---
    C.color, ---
    C.Date_semis, ---
    CAST(CASE WHEN sum(C.Semis_fait)>0 THEN sum(C.Semis_fait) ELSE min(C.Semis_fait) END AS BOOL) Semis_fait, ---
    CAST(CASE WHEN min(C.Date_plantation)=max(C.Date_plantation) THEN strftime('%d/%m/%Y',min(C.Date_plantation))
              ELSE group_concat(strftime('%d/%m/%Y',C.Date_plantation),x'0a0a')
              END AS TEXT) Dates_plantation, ---
        ---multiline
    I.Nb_graines_plant, ---
        ---unit graines/plant
    I.Dose_semis, ---
        ---unit g/m²
    CAST(group_concat(C.Longueur||'m x '||PL.Largeur||'m - '||
                      C.Nb_rangs||'rg, esp'||C.Espacement||'cm',x'0a0a') AS TEXT) Rangs_espacement, ---
        ---multiline
        ---unit cm
    CAST(sum((C.Longueur * C.Nb_rangs / C.Espacement * 100))AS INTEGER) Nb_plants, --- --UNIT(plants)
    CAST(sum((C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_plant))AS INTEGER) Nb_graines, --- --UNIT(graines)
    CAST(round(sum((CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_plant / E.Nb_graines_g
                         ELSE C.Longueur * PL.Largeur * I.Dose_semis
                         END)),2)AS REAL) Poids_graines, ---
        ---unit g
    CAST(min(C.A_faire) AS TEXT) A_faire, ---
    CAST(min(C.Notes) AS TEXT) Notes, ---
    I.Notes N_IT_plante, ---
        ---multiline
    E.Notes N_espèce ---
        ---multiline
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (Date_semis < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_semis')||' days'))AND
      (coalesce(Semis_fait,'') NOT LIKE 'x%')AND
      (Date_plantation NOTNULL)
GROUP BY C.Espèce,C.IT_plante,C.Variété,C.Type,C.Etat,C.Date_semis
ORDER BY C.Date_semis,C.Date_plantation,C.Planche,C.Espèce,C.IT_plante ---
;

CREATE VIEW Cultures__à_terminer AS SELECT --- Cultures non terminées, déjà semées/plantées/récoltées (voir paramètre 'C_horizon_terminer').
                                           --- Pour compagne et engrais vert ('Début_récolte' vide), sont incluses les cultures dont la date de destruction est proche ou passée.
    ---row_summary Culture,Planche,Variété_ou_It_plante,Type,Etat,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    C.Planche, ---
    C.Culture, ---
    CAST(coalesce(C.Variété,C.IT_plante,C.Espèce) AS TEXT) Variété_ou_It_plante, --- ::Cultures__Succ_planches.Variété_ou_It_plante
    C.Type, ---
    C.Etat, ---
    C.color, ---
    C.Date_semis, ---
    C.Date_plantation, ---
    C.Début_récolte, ---
    (SELECT R.Date FROM Récoltes R WHERE R.Culture=C.Culture ORDER BY Date DESC) Der_récolte, --- Dernière récolte saisie.
    C.Fin_récolte, ---
    C.Récolte_faite, ---
    C.Terminée, ---
    C.Longueur, ---
    C.A_faire, ---
    CAST((SELECT group_concat(Culture_suivante,x'0a0a')
          FROM (SELECT CS.Culture||' - '||coalesce(CS.Variété,CS.IT_plante,CS.Espèce)||' - '||
                       'MEP '||coalesce(strftime('%d/%m/%Y',CS.Date_plantation),strftime('%d/%m/%Y',CS.Date_semis),'?')||' - '||
                       CS.Etat||coalesce(' - '||CS.Longueur||'m','') Culture_suivante
                FROM Cultures CS
                WHERE (CS.Culture!=C.Culture)AND(CS.Planche=C.Planche)AND -- Autres cultures sur la même planche.
                      (coalesce(CS.Date_plantation,CS.Date_semis)>coalesce(C.Date_plantation,C.Date_semis))AND -- Mises en place après.
                      ((CS.Semis_fait NOTNULL AND CS.Date_plantation ISNULL) OR -- SD semé
                       (CS.Plantation_faite NOTNULL)OR -- Plant ou SPep planté
                       ((CS.Terminée NOTNULL)AND NOT(CS.terminée LIKE 'v%')) -- Annuelle terminée.
                       )
                ORDER BY coalesce(CS.Date_plantation,CS.Date_semis) DESC)
          ) AS TEXT) Cultures_suivantes, --- Cultures suivantes déjà en place sur la même planche.
        ---multiline
    C.Notes, ---
    I.Notes N_IT_plante, ---
        ---multiline
    PL.Notes N_Planche, ---
        ---multiline
    E.Notes N_espèce ---
        ---multiline
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE C.Terminée ISNULL AND -- Les vivaces ne sont jamais à terminer.
      ((C.Fin_récolte ISNULL)OR(C.Fin_récolte < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_terminer')||' days')))AND
      ((Semis_fait NOTNULL)OR(Date_semis ISNULL))AND
      ((Plantation_faite NOTNULL)OR(Date_plantation ISNULL))AND
      ((Récolte_faite NOTNULL)OR(Début_récolte ISNULL))
ORDER BY C.Fin_récolte,C.Planche,C.Espèce,C.IT_plante ---
;

CREATE VIEW Destinations__conso AS SELECT ---
    ---row_summary Destination,Type,Date_RAZ
    D.Destination, ---
    D.Type, ---
    D.Adresse, ---
    D.Site_web, ---
    D.Date_RAZ, ---
    CAST((SELECT sum(Quantité) FROM Consommations Co WHERE (Co.Destination=D.Destination)AND(Co.Date >= D.Date_RAZ)) AS INT) Consommation, --- Somme des sorties de stock depuis Date_RAZ (incluse).
        ---unit kg
    CAST((SELECT sum(Prix) FROM Consommations Co WHERE (Co.Destination=D.Destination)AND(Co.Date >= D.Date_RAZ)) AS REAL) Valeur, --- Valeur des sorties de stock depuis Date_RAZ (incluse).
        ---money
    D.Active, ---
    D.Interne, ---
    D.Notes ---
FROM Destinations D;

CREATE VIEW Espèces__a AS SELECT ---
    ---row_summary Espèce|:: ,Famille|(::) ,Catégories
    Espèce, ---
    Famille, ---
    Catégories, ---
    Rendement, ---
    Niveau, ---
    Besoins, ---
    Densité, ---
    Dose_semis, ---
        ---unit g/m²
    Nb_graines_g, ---
    FG, ---
    T_germ, ---
    Levée, ---
    Irrig, ---
    Conservation, ---
    A_planifier, ---
    Obj_annuel, ---
    N, ---
    ★N, ---
    P, ---
    ★P, ---
    K, ---
    ★K, ---
    Effet, ---
    Notes ---
FROM Espèces
WHERE Vivace ISNULL;

CREATE VIEW Bilans_annuels AS SELECT ---
    B.Saison, ---
    CAST(sum(B.Obj_annuel) AS INT) Obj_annuel, ---
        ---unit kg
    CAST(total(B.Qté_prév) AS INT) Qté_prév, --- Quantités prévues (rendement espèce * surface culture).
        ---unit kg
    CAST(total(B.Qté_prév)/total(B.Obj_annuel)*100 AS INT) Couv_prév_pc, --- ::Espèces__Bilans_annuels.Couv_prév_pc
    CAST(total(B.Qté_réc) AS INT) Qté_réc, --- ::Cultures__non_terminées.Qté_réc
        ---unit kg
    CAST(total(B.Qté_réc)/total(B.Qté_prév)*100 AS INT) Couv_réc_pc, --- ::Espèces__Bilans_annuels.Couv_réc_pc
        ---cond_formats ::<80 #darkRed# #lightRed#,::>120 #darkGreen# #lightGreen#
    CAST(total(B.Qté_réc)/total(B.Obj_annuel)*100 AS INT) Couv_obj_pc, --- Quantités déjà récoltées par rapport aux objectifs.
        ---cond_formats ::<80 #darkRed# #lightRed#,::>120 #darkGreen# #lightGreen#
    CAST(total(B.Reste_à_réc) AS INT) Reste_à_réc, ---
        ---unit kg
    CAST(total(B.Nb_cu) AS INT) Nb_cu, --- --UNIT(cultures)
    CAST((SELECT count() FROM Planches P WHERE P.Planche IN(SELECT C.Planche FROM Cultures C WHERE C.Saison=B.Saison)) AS INT) Nb_planches, --- --UNIT(planches)
    CAST((SELECT total(Longueur) FROM Planches P WHERE P.Planche IN(SELECT C.Planche FROM Cultures C WHERE C.Saison=B.Saison)) AS INT) Longueur, ---
        ---unit m
    CAST((SELECT total(Longueur*Largeur) FROM Planches P WHERE P.Planche IN(SELECT C.Planche FROM Cultures C WHERE C.Saison=B.Saison)) AS INT) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
    CAST(sum(Qté_réc_saison) AS INT) Qté_réc_saison, --- Quantités récoltées cette année (cultures de la saison courante et de la précédente).
        ---unit kg
    CAST(sum(Qté_exp) AS INT) Qté_exp, --- Quantités exportées cette année.
        ---unit kg
    CAST(sum(Valeur) AS REAL) Valeur, ---
        ---money
    CAST(sum(Qté_exp)/total(Qté_réc_saison)*100 AS INT) Export_pc --- Quantités exportées par rapport aux quantités récoltées cette année.
        ---cond_formats ::>100 #darkRed# #lightRed#
FROM Espèces__Bilans_annuels B
GROUP BY Saison;

CREATE VIEW Espèces__Bilans_annuels AS SELECT --- Comparatif saison par saison des objectifs de production. -- pour les espèces annuelles marquées 'A planifier'
                                              --- L'objectif annuel n'est valable que pour la saison courante.
                                              --- Prévue=rendement de l'espèce x surface de culture.
                                              ---
                                              --- Attention, c'est la date de mise en place (plantation ou semis en place) qui détermine la saison d'une culture.
                                              --- Des récoltes année N+1 peuvent donc compter pour la saison N si la mise en place est en fin d'année N.
    ---row_summary Espèce,Saison
    C.Espèce, ---
    C.Saison, ---
    C.Rendement, ---
    C.Niveau, ---
    C.Obj_annuel, ---
    CAST(sum(C.Qté_prév) AS INT) Qté_prév, --- Quantités prévues (rendement espèce * surface culture).
        ---unit kg
    CAST(sum(C.Qté_prév)/C.Obj_annuel*100 AS INT) Couv_prév_pc, --- Quantités prévues (rendement x surface) par rapport aux objectifs.
    CAST(sum(C.Qté_réc) AS INT) Qté_réc, --- Quantités récoltées pour les cultures mises en place cette saison.
        ---unit kg
    CAST(sum(C.Qté_réc)/total(C.Qté_prév)*100 AS INT) Couv_réc_pc, --- Quantités déjà récoltées par rapport aux quantités prévues (rendement x surface).
        ---unit kg
        ---cond_formats ::<80 #darkRed# #lightRed#,::>120 #darkGreen# #lightGreen#
    CAST(sum(C.Qté_réc)/C.Obj_annuel*100 AS INT) Couv_obj_pc, --- Quantités récoltées par rapport aux objectifs.
        ---cond_formats ::<80 #darkRed# #lightRed#,::>120 #darkGreen# #lightGreen#
    CAST(sum(iif(C.Terminée ISNULL,max(C.Qté_prév-C.Qté_réc,0),0)) AS INT) Reste_à_réc, --- Estimation des quantités restant à récolter pour les cultures mises en place cette saison et non terminées (qté prévu moins déja réc).
        ---unit kg
    -- CAST(sum(C.Qté_réc+iif(C.Terminée ISNULL,max(C.Qté_prév-C.Qté_réc,0),0))/
    --      ((SELECT Prod_possible FROM Planif_espèces P WHERE P.Espèce=C.Espèce))*100 AS INT) Couv_pla_pc, --- Quantités récoltées + restant à récolter par rapport à la planification.
    CAST(count() AS INT) Nb_cu, --- --UNIT(cultures)
    CAST((SELECT count()
          FROM (SELECT DISTINCT C2.Planche
                FROM Cultures C2 WHERE (C2.Saison=C.Saison)AND(C2.Espèce=C.Espèce)AND(coalesce(C2.Terminée,'') NOT LIKE 'v%'))) AS INT) Nb_planches, --- --UNIT(planches)
    CAST((SELECT group_concat(UP||' ('||(SELECT count()
                                         FROM (SELECT DISTINCT C3.Planche
                                               FROM Cultures C3
                                               WHERE (C3.Saison=C.Saison)AND(C3.Espèce=C.Espèce)AND(C3.Planche LIKE UP||'%')AND
                                                     (coalesce(C3.Terminée,'') NOT LIKE 'v%')))||' pl)',', ') --,x'0a0a'
          FROM (SELECT DISTINCT substr(C2.Planche,1,(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Ilot_nb_car')+
                                                    (SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='UP_nb_car')) UP
                FROM Cultures C2 WHERE (C2.Saison=C.Saison)AND(C2.Espèce=C.Espèce)AND(coalesce(C2.Terminée,'') NOT LIKE 'v%'))) AS INT) Unités_prod, ---
        ---multiline
    CAST(sum(C.Longueur) AS INT) Longueur, ---
        ---unit m
    CAST(sum(C.Surface) AS INT) Surface, --- --UNIT(m²)
    CAST((SELECT sum(R.Quantité) FROM Récoltes R WHERE (R.Espèce=C.Espèce)AND(substr(R.Date,1,4)=C.Saison)) AS INT) Qté_réc_saison, --- Quantités récoltées cette année (cultures de la saison courante et de la précédente).
        ---unit kg
    CAST((SELECT sum(CO.Quantité) FROM Consommations CO LEFT JOIN Destinations D USING(Destination)
          WHERE (CO.Espèce=C.Espèce)AND(substr(CO.Date,1,4)=C.Saison)AND(D.Interne ISNULL)) AS INT) Qté_exp, --- Quantités exportées cette année.
        ---unit kg
    CAST((SELECT sum(CO.Prix) FROM Consommations CO LEFT JOIN Destinations D USING(Destination)
          WHERE (CO.Espèce=C.Espèce)AND(substr(CO.Date,1,4)=C.Saison)AND(D.Interne ISNULL)) AS REAL) Valeur, ---
        ---money
    CAST((SELECT sum(CO.Quantité) FROM Consommations CO LEFT JOIN Destinations D USING(Destination)
          WHERE (CO.Espèce=C.Espèce)AND(substr(CO.Date,1,4)=C.Saison)AND(D.Interne ISNULL))/
         (SELECT sum(R.Quantité) FROM Récoltes R WHERE (R.Espèce=C.Espèce)AND(substr(R.Date,1,4)=C.Saison))*100 AS INT) Export_pc, --- Quantités exportées par rapport aux quantités récoltées cette année.
        ---cond_formats ::>100 #darkRed# #lightRed#
    C.Vivace, ---
    C.N_espèce Notes ---
FROM (SELECT * FROM Cu_esp_prod_an UNION SELECT * FROM Cu_esp_prod_vi) C
WHERE (C.Obj_annuel>0)OR(C.Qté_réc>0)
GROUP BY Espèce,Saison
ORDER BY Espèce,Saison ---
;

CREATE VIEW Espèces__inventaire AS SELECT --- Inventaire des légumes produits/consommés pour chaque espèce dont le champ 'Conservation' est non vide.
    ---row_summary Espèce,Date_inv,Stock,Valeur
    E.Espèce, ---
    E.Date_inv, ---
    E.Inventaire, ---
    CAST(round((SELECT sum(Quantité) FROM Récoltes Re WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),3)AS REAL) Entrées, --- Quantité récolté totale depuis 'Date_inv'.
        ---unit kg
    CAST(round((SELECT sum(Quantité) FROM Consommations Re WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),3)AS REAL) Sorties, --- Quantité consommée totale depuis 'Date_inv'.
        ---unit kg
    CAST(round(E.Inventaire +coalesce((SELECT sum(Quantité) FROM Récoltes Re
                                       WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0)
                            -coalesce((SELECT sum(Quantité) FROM Consommations Re
                                       WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0),3)AS REAL) Stock, --- Quantité totale en stock.
        ---unit kg
    E.Prix_kg, ---
    CAST(round((E.Inventaire +coalesce((SELECT sum(Quantité) FROM Récoltes Re
                                        WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0)
                             -coalesce((SELECT sum(Quantité) FROM Consommations Re
                                        WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0))*E.Prix_kg,3)AS REAL) Valeur, --- Valeur théorique du stock.
        ---money
    Notes ---
FROM Espèces E
WHERE E.Conservation NOT NULL;

CREATE VIEW Espèces__manquantes AS SELECT --- Espèces marquées 'A planifier' et qui ne sont pourtant pas suffisamment incluses dans les rotations.
                                          --- La planification ne générera donc pas assez de cultures pour ces espèces.
    ---no_data_text Aucune !
    ---no_data_text Toutes les espèces marquées 'A planifier' sont suffisamment incluses dans les rotations.
    ---no_data_text
    ---no_data_text L'onglet 'Planification/Cultures prévues par espèce' donne la production prévue par espèce.
    ---row_summary Espèce|:: ,Famille|(::)
    E.Espèce, ---
    E.Famille, ---
    E.Rendement, ---
    E.Niveau, ---
    E.Obj_annuel, ---
    CAST((SELECT sum(Prod_possible) FROM Planif_espèces C WHERE C.Espèce=E.Espèce)AS INTEGER) Prod_possible, --- Production possible pour les cultures prévues.
        ---unit kg
                                                                                                             --- Rendement x surface
    CAST((SELECT sum(Prod_possible) FROM Planif_espèces C WHERE C.Espèce=E.Espèce)/E.Obj_annuel*100 AS INT) Couv_pc, --- Pourcentage de couverture des objectifs.
    CAST((SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture')AS INTEGER) Saison_à_planifier --- Paramétre 'Année_culture' + 1.
FROM Espèces E
WHERE (E.A_planifier NOTNULL)AND
      (NOT(Espèce IN(SELECT Espèce FROM Planif_espèces))OR
       ((SELECT sum(Prod_possible) FROM Planif_espèces C WHERE C.Espèce=E.Espèce)<E.Obj_annuel));

CREATE VIEW Espèces__v AS SELECT ---
    ---row_summary Espèce|:: ,Famille|(::) ,Catégories
    Espèce, ---
    Famille, ---
    Catégories, ---
    Rendement, ---
    Besoins, ---
    S_taille, ---
    Effet, ---
    Usages, ---
    Irrig, ---
    Conservation, ---
    Obj_annuel, ---
    N, ---
    ★N, ---
    P, ---
    ★P, ---
    K, ---
    ★K, ---
    Notes ---
FROM Espèces
WHERE Vivace NOTNULL;

CREATE VIEW Fam_actives AS SELECT
    *
FROM Familles F
WHERE (SELECT count() FROM ITP LEFT JOIN Espèces USING(Espèce) WHERE Espèces.Famille=F.Famille)>0;

CREATE VIEW Fertilisants__inventaire AS SELECT ---
    F.Fertilisant, ---
    F.Date_inv, ---
    F.Inventaire, ---
    CAST(round((SELECT sum(Quantité) FROM Fertilisations Fe WHERE (Fe.Fertilisant=F.Fertilisant)AND(Fe.Date >= F.Date_inv)),3)AS REAL) Sorties, --- Quantité consommée totale depuis 'Date_inv'.
        ---unit kg
    CAST(round(F.Inventaire -coalesce((SELECT sum(Quantité) FROM Fertilisations Fe
                                       WHERE (Fe.Fertilisant=F.Fertilisant)AND(Fe.Date >= F.Date_inv)),0),3)AS REAL) Stock, --- Quantité totale en stock.
        ---unit kg
    F.Prix_kg, ---
        ---money
    CAST(round((F.Inventaire -coalesce((SELECT sum(Quantité) FROM Fertilisations Fe
                                        WHERE (Fe.Fertilisant=F.Fertilisant)AND(Fe.Date >= F.Date_inv)),0))*F.Prix_kg,3)AS REAL) Valeur, --- Valeur théorique du stock.
        ---money
    Notes ---
FROM Fertilisants F;

-- CREATE VIEW Cultures_fertilisation AS SELECT --- total des fertilisations par cultures.
--     FC.Culture, ---
--     FC.Planche, ---
--     min(FC.Date) Début_fert, ---
--     max(FC.Date) Fin_fert, ---
--     FC.N/total(FC.Surface_c)*FC.Surface_c, --- faux
--     FC.P/total(FC.Surface_c)*FC.Surface_c, ---
--     FC.K/total(FC.Surface_c)*FC.Surface_c ---
-- FROM Fertilisations_Cultures FC
-- GROUP BY Culture
-- ORDER BY FC.Culture,FC.Date;

CREATE VIEW Fertilisations_Cultures AS SELECT -- Fertilisations avec les cultures concernées.
    F.ID,
    -- F.Date,
    -- F.Planche,
    -- F.Fertilisant,
    -- F.Quantité,
    -- F.N,
    -- F.P,
    -- F.K,
    -- P.Longueur,
    -- P.Longueur*P.Largeur Surface,
    -- round(F.Quantité/(P.Longueur*P.Largeur),3) Qté_m²,
    C.Culture,
    Culture||' - '||coalesce(C.Variété,C.IT_plante,C.Espèce)||' - '||Etat||' - '||
    coalesce('Plant '||strftime('%d/%m/%Y',C.Date_plantation),'Semis '||strftime('%d/%m/%Y',C.Date_semis)) Culture_texte
    -- C.Longueur*P.Largeur Surface_c
    -- F.Notes
FROM Fertilisations F
JOIN Cultures C USING(Planche)
-- JOIN Planches P USING(Planche)
JOIN Fertilisants Fe USING(Fertilisant)
WHERE ((coalesce(C.Terminée,'') NOT LIKE 'v%')AND -- Annuelle
       (DATE(F.Date,((coalesce(Fe.Délai,1)+1)*7)||' days')<C.Fin_récolte)AND -- Fertilisation+délai+1semaine avant fin de récolte.
       (DATE(F.Date,((coalesce(Fe.Délai,1)+coalesce(Fe.Durée,1)-1)*7)||' days')>(coalesce(C.Date_plantation,C.Date_semis))))OR -- Fertilisation+délai+durée-1semaine après MEP.
      ((C.Terminée LIKE 'v%')AND -- Vivace
       ((C.Terminée LIKE 'v')OR
        (DATE(F.Date,((coalesce(Fe.Délai,1)+1)*7)||' days')<C.Fin_récolte))AND -- Fertilisation+délai+1semaine avant fin de DERNIERE récolte.
       (DATE(F.Date,((coalesce(Fe.Délai,1)+coalesce(Fe.Durée,1)-1)*7)||' days')>coalesce(C.Date_plantation,C.Date_semis))); -- Fertilisation+délai+durée-1semaine après MEP.

CREATE VIEW Fertilisations__Saisies AS SELECT ---
    ---goto_last
    -- ---no_data_text Il n'y a aucune culture à fertiliser pour le moment.
    -- ---no_data_text
    -- ---no_data_text Les fertilisations peuvent être saisies avant la mise en place de la culture sur la planche et jusqu'au début de récolte en modifiant les paramètres 'Ferti_avance' et 'Ferti_retard'.
    ---row_summary Date,Planche,Fertilisant,Quantité
    F.ID, --- Nécessaire pour mise à jour de la table via trigger INSTEAD OF UPDATE.
        ---hidden
    F.Date, ---
    F.Planche, ---
    F.Fertilisant, ---
    F.Quantité, ---
    F.N, ---
    F.P, ---
    F.K, ---
    P.Longueur, ---UNIT m
    P.Longueur*P.Largeur Surface, ---UNIT m²
    round(F.Quantité/(P.Longueur*P.Largeur),3) Qté_m², ---UNIT kg/m²
    Fe.Délai, --- ::Fertilisants.Délai
        ---unit semaines
    Fe.Durée,--- ::Fertilisants.Durée
        ---unit semaines
    (SELECT group_concat(FC.Culture_texte,x'0a0a')
     FROM Fertilisations_Cultures FC
     WHERE FC.ID=F.ID) Cultures, --- Cultures concernées: en place ou à venir sur cette planche à la date de fertilisattion.
                                 --- Fin_récolte au moins 1 mois après la fertilisation.
                                 --- Mise en place maximum 6 mois après la fertilisation.
    F.Notes ---
FROM Fertilisations F
LEFT JOIN Planches P USING(Planche)
LEFT JOIN Fertilisants Fe USING(Fertilisant)
WHERE (F.Date>DATE('now','-'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Ferti_historique')||' days'))OR
      (DATE(F.Date) ISNULL) -- Détection de date incorecte
ORDER BY F.Date,F.Planche,F.Fertilisant ---
;

CREATE VIEW ITP__Tempo AS SELECT --- Itinéraires techniques de plantes
                                 --- Une ITP c'est Une espèce de plante cultivée d'une certaine manière:
                                 --- Hative ou tardive, sous serre ou en extérieur, etc.
                                 --- Chaque ITP à une période de semis, de plantation et de récolte.
                                 --- Pour chaque culture, il faudra prendre une variété adaptée à l'itinéraire technique voulu.
    ---no_data_text Vous devez d'abord saisir au moins une espèce de plante (menus 'Espèces') avant de saisir des itinéraires techniques.
    I.IT_plante, ---
    I.Espèce, ---
    I.Type_planche, ---
    CAST(CASE WHEN I.Espèce NOTNULL AND I.Type_culture='Vivace' AND (SELECT E.Vivace ISNULL FROM Espèces E WHERE E.Espèce=I.Espèce)
              THEN '!Espèce vivace ?'
              ELSE I.Type_culture
              END AS TEXT) Type_culture, ---
    I.S_semis, ---
    I.S_plantation, ---
    coalesce(I.S_plantation,I.S_semis) S_MEP, --- Semaine de mise en place de la culture sur la planche.
    I.S_récolte, ---
    I.D_récolte, ---
    I.Décal_max, ---
    E.Catégories, ---

    (SELECT value FROM Prm_draw WHERE name='GraphBarresVert24Mois')|| -- Barres verticales pour les mois
    replace(replace(replace('rect(xRec,0,wRec,5,cRec,255)', -- Semis
        'xRec',G.J_semis),
        'wRec',G.NJ_semis),
        'cRec',iif(G.S_plantation NOTNULL,'#ff6000','#76c801'))||
    replace(replace(replace('rect(xRec,0,wRec,5,cRec,100)', -- Attente plantation.
        'xRec',G.J_semis+G.NJ_semis),
        'wRec',iif(G.S_semis NOTNULL AND G.S_plantation NOTNULL,G.J_plantation-G.J_semis-G.NJ_semis,0)),
        'cRec',iif(G.S_plantation NOTNULL,'#ff6000','#76c801'))||
    replace(replace('rect(xRec,3,wRec,10,#76c801,255)', -- Plantation
        'xRec',G.J_plantation),
        'wRec',G.NJ_plantation)||
    replace(replace(replace('rect(xRec,3,wRec,10,#76c801,150,gRec)', -- Attente récolte.
        'xRec',iif(G.S_plantation NOTNULL,G.J_plantation+G.NJ_plantation,G.J_semis+G.NJ_semis)),
        'wRec',iif(G.S_plantation NOTNULL,G.J_récolte-G.J_plantation-G.NJ_plantation,G.J_récolte-G.J_semis-G.NJ_semis)),
        'gRec',iif(G.S_récolte ISNULL AND G.D_récolte ISNULL,'h+',''))|| -- Dépérissement en place
    replace(replace('rect(xRec,8,wRec,6,#bf00ff,255)', -- Récolte
        'xRec',G.J_récolte),
        'wRec',G.NJ_récolte)
    Graph, --- Graphique calculé à partir des semaines de semis, plantation et récolte.
        ---draw GraphTitres24Mois

    -- CAST(coalesce((I.S_semis*7-6),0)||':'||
    --      coalesce(((I.S_semis+coalesce(I.Décal_max,0.5))*7-6),0)||':'||
    --      coalesce((I.S_plantation*7-6),0)||':'||
    --      coalesce(((I.S_plantation+coalesce(I.Décal_max,0.5))*7-6),0)||':'||
    --      coalesce((I.S_récolte*7-6),0)||':'||
    --      coalesce(((I.S_récolte+coalesce(I.D_récolte+coalesce(I.Décal_max,0),0))*7-6),0) -- Si pas de récolte (D_récolte NULL) ne pas ajouter le décalage.
    --     AS TEXT) TEMPO, ---

    CAST(CASE WHEN I.S_semis NOTNULL AND I.S_plantation NOTNULL
              THEN iif(I.S_plantation-I.S_semis<0,I.S_plantation-I.S_semis+52,I.S_plantation-I.S_semis)*7
              END AS INT) J_pép, --- Nombre de jours entre semis et plantation.
                                 --- La date prévue pour les cultures sera le début de période.
    CAST(CASE WHEN coalesce(I.S_plantation,I.S_semis) NOTNULL AND I.S_récolte NOTNULL
              THEN iif(I.S_récolte-coalesce(I.S_plantation,I.S_semis)<0,I.S_récolte-coalesce(I.S_plantation,I.S_semis)+52,
                       I.S_récolte-coalesce(I.S_plantation,I.S_semis))*7
              END AS INT) J_en_pl, --- Nombre de jours entre plantation et récolte.
                                   --- La date prévue pour les cultures sera le début de période.
    I.Espacement, ---
    I.Esp_rangs, ---
        ---unit cm
    I.Nb_graines_plant, ---
        ---unit graines/plant
    I.Dose_semis, ---
        ---unit g/m²
    CAST(CASE WHEN I.Espacement>0 THEN 1.0/I.Espacement*100/I.Esp_rangs*100/E.Densité*100
              ELSE I.Dose_semis/E.Dose_semis*100 END AS INT) Densité_pc, --- ::Cultures__non_terminées.Densité_pc
    I.Notes, ---
    E.Notes N_espèce, ---
        ---multiline
    E.Famille ---
FROM ITP I
JOIN ITP_Graph G USING(IT_plante)
LEFT JOIN Espèces E USING(Espèce);

CREATE VIEW ITP_Graph AS SELECT
    I.IT_plante,
    I.S_semis,
    coalesce(I.S_semis*7-6,0) J_semis,
    iif(I.S_semis NOTNULL,coalesce(I.Décal_max,0.5)*7,0) NJ_semis,
    I.S_plantation,
    coalesce(I.S_plantation*7-6+iif(I.S_plantation<I.S_semis,365,0),0) J_plantation,
    iif(I.S_plantation NOTNULL,coalesce(I.Décal_max,0.5)*7,0) NJ_plantation,
    I.S_récolte,
    I.D_récolte,
    coalesce(I.S_récolte*7-6+iif(I.S_semis NOTNULL AND I.S_plantation NOTNULL AND (I.S_plantation<I.S_semis),365,0)+
                             iif(I.S_semis NOTNULL AND I.S_plantation NOTNULL AND (I.S_récolte<I.S_plantation),365,0)+
                             iif(I.S_semis NOTNULL AND I.S_plantation ISNULL AND (I.S_récolte<I.S_semis),365,0)+
                             iif(I.S_semis ISNULL AND I.S_plantation NOTNULL AND (I.S_récolte<I.S_plantation),365,0),
            iif(I.S_semis NOTNULL OR I.S_plantation NOTNULL,365,0)) J_récolte,
    iif(I.S_récolte NOTNULL,coalesce(I.D_récolte+coalesce(I.Décal_max,0),0)*7,0) NJ_récolte -- Si pas de récolte (D_récolte NULL) ne pas ajouter le décalage.
FROM ITP I;

CREATE VIEW ITP__analyse_a AS SELECT --- Pour chaque itinéraire technique d'annuelle, les cultures significatives (champ 'Terminée' différent de '...NS') sont analysées.
    I.IT_plante, ---
    I.Type_culture, ---
    I.S_semis, ---
    I.S_plantation, ---
    I.S_récolte, ---
    CAST((SELECT count() FROM Cultures C WHERE (C.IT_plante=I.IT_plante) AND C.Terminée ISNULL)AS INTEGER) Nb_cu_NT, --- Nombre de cultures NON terminées utilisant cet itinéraire technique. --UNIT(cultures non terminées)
    CAST((SELECT count() FROM Cultures C WHERE (C.IT_plante=I.IT_plante) AND C.Terminée NOTNULL)AS INTEGER) Nb_cu_T, --- Nombre de cultures terminées utilisant cet itinéraire technique. --UNIT(cultures terminées)
    CAST((SELECT count() FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_TS, --- Nombre de cultures terminées et significatives (champ 'Terminée' différent de '...NS' et de 'v...'). --UNIT(cultures terminées et significatives)
    CAST((SELECT min(strftime('%U',C.Date_semis)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_semis_min, ---
    CAST((SELECT max(strftime('%U',C.Date_semis)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_semis_max, ---
    CAST((SELECT min(strftime('%U',C.Date_plantation)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_plant_min, ---
    CAST((SELECT max(strftime('%U',C.Date_plantation)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_plant_max, ---
    CAST((SELECT min(strftime('%U',C.Début_récolte)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_récolte_min, ---
    CAST((SELECT max(strftime('%U',C.Fin_récolte)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_récolte_max, ---
    CAST(round((SELECT sum(R.Quantité) FROM Cu_ITP_analyse_a C LEFT JOIN Récoltes R USING(Culture) WHERE C.IT_plante=I.IT_plante)/
               (SELECT count() FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante),3)AS REAL) Qté_réc_moy --- --UNIT(kg/culture)
FROM ITP I
WHERE (SELECT count() FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante)>0
ORDER BY IT_plante ---
;

CREATE VIEW ITP__analyse_v AS SELECT --- Pour chaque itinéraire technique de vivace, les cultures significatives (champ 'Terminée' différent de '...NS') sont analysées.
    I.IT_plante, ---
    I.Type_culture, ---
    I.S_semis, ---
    I.S_plantation, ---
    I.S_récolte, ---
    CAST((SELECT count() FROM Cu_non_ter C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_NT, --- ::ITP__analyse_a.Nb_cu_NT --UNIT(cultures non terminées)
    CAST((SELECT count() FROM Cu_ter C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_T, --- ::ITP__analyse_a.Nb_cu_T --UNIT(cultures terminées)
    CAST((SELECT count() FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_S, --- Nombre de cultures significatives (champ 'Terminée' différent de '...NS'). --UNIT(cultures significatives)
    CAST((SELECT min(strftime('%U',C.Date_semis)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_semis_min, ---
    CAST((SELECT max(strftime('%U',C.Date_semis)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_semis_max, ---
    CAST((SELECT min(strftime('%U',C.Date_plantation)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_plant_min, ---
    CAST((SELECT max(strftime('%U',C.Date_plantation)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_plant_max, ---
    CAST((SELECT min(strftime('%U',C.Début_récolte)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_récolte_min, ---
    CAST((SELECT max(strftime('%U',C.Fin_récolte)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_récolte_max, ---
    CAST((SELECT sum(ceil((julianday(min(C.Fin_récolte,DATE('now',coalesce(I.D_récolte*7,0)||' days')))
                          -julianday((SELECT min(R.Date) FROM Récoltes R WHERE R.Culture=C.Culture)))/365))
          FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) Nb_récoltes, --- --UNIT(récoltes)
    CAST(round((SELECT sum(R.Quantité) FROM Cu_ITP_analyse_v C LEFT JOIN Récoltes R USING(Culture) WHERE C.IT_plante=I.IT_plante)/
               (SELECT sum(ceil((julianday(min(C.Fin_récolte,DATE('now',coalesce(I.D_récolte*7,0)||' days')))
                                -julianday((SELECT min(R.Date) FROM Récoltes R WHERE R.Culture=C.Culture)))/365))
                FROM Cu_ITP_analyse_v C
                WHERE C.IT_plante=I.IT_plante),3)AS REAL) Qté_réc_moy --- --UNIT(kg/culture)
FROM ITP I
WHERE (SELECT count() FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante)>0
ORDER BY IT_plante ---
;

CREATE VIEW ITP_planif AS SELECT
    I.IT_plante,
    I.Espèce,
    I.S_semis,
    I.S_plantation,
    I.S_récolte,
    I.D_récolte,
    DATE(I.A_planif||'-01-01',(I.S_semis*7-6-1)||' days') Date_semis,
    DATE(I.A_planif||'-01-01',(I.S_plantation*7-6-1)||' days') Date_plantation,
    iif(I.D_récolte NOTNULL,DATE(I.A_planif||'-01-01',(I.S_récolte*7-6-1)||' days'),NULL) Début_récolte,
    iif(I.D_récolte NOTNULL,DATE(I.A_planif||'-01-01',((I.S_récolte+I.D_récolte)*7-6-1)||' days'),
    DATE(I.A_planif||'-01-01',(I.S_récolte*7-6-1)||' days')) Fin_récolte,
    1 Coef_semis,
    1 Coef_plantation,
    1 Coef_début_récolte,
    1 Coef_fin_récolte,
    I.Esp_rangs,
    I.Espacement
FROM ITP_sem_corrigées I;

CREATE VIEW ITP_sem_corrigées AS SELECT
    I.IT_plante,
    I.Espèce,
    (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture') A_planif,
    iif(I.S_semis>I.S_plantation,I.S_semis-52,I.S_semis) S_semis,
    I.S_plantation,
    iif(I.S_récolte<coalesce(I.S_plantation,I.S_semis),I.S_récolte+52,I.S_récolte) S_récolte,
    I.D_récolte,
    I.Esp_rangs,
    I.Espacement
FROM ITP I;

CREATE VIEW Info_Potaléger AS SELECT
    1 N,
    'Version de la BDD' Info,
    '2025-11-28' Valeur;

CREATE VIEW Planche_asso_esp_présentes AS SELECT -- Pour chaque associations possible, compter les cultures d'espèces identiques présentes sur la planche.
    C.Planche,
    min(C.Date_MEP) Date_MEP, -- min car si plusieurs culture d'une même espèce, une seule est nécessaire.
    max(C.Fin_récolte) Fin_récolte, -- max, idem.
    C.Association,
    C.Espèce,
    C.Requise,
    count() Nb, -- Nb cultures même planche, asso et espèce.
    count(Vivace) Nb_vivaces,
    group_concat(C.Cultures,x'0a0a') Cultures
FROM Planche_asso_possibles C
GROUP BY C.Planche,C.Association,C.Espèce,C.Requise;

-- CREATE VIEW Planche_asso_esp_requises_présentes AS SELECT
--     P.Planche,
--     max(iif(P.Requise NOTNULL,P.Date_MEP,DATE('2000-01-01'))) Date_MEP, -- max car les cultures requises doivent être présentes en même temps.
--     min(iif(P.Requise NOTNULL,P.Fin_récolte,DATE('2100-01-01'))) Fin_récolte, -- min, idem.
--     P.Association,
--     CAST(count() AS INT) Nb_distinct, -- nb d'espèces même planche et asso.
--     count(P.Requise) Nb_requises, -- nb d'espèces requises même planche et asso.
--     CAST(sum(P.Nb) AS INT) Nb, -- Nb cultures même planche et asso.
--     CAST(sum(Nb_vivaces) AS INT) Nb_vivaces,
--     CAST(group_concat(P.Cultures,x'0a0a') AS TEXT) Cultures
-- FROM Planche_asso_esp_présentes P
-- WHERE (P.Requise NOTNULL)OR -- Espèces requises
--       -- Espèces non requises qui croisent les requises.
--       ((julianday(P.Fin_récolte)-(SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Nb_sem_croisement')*7>
--         (SELECT max(julianday(P2.Date_MEP)) FROM Planche_asso_esp_présentes P2
--          WHERE (P2.Planche=P.Planche)AND(P2.Association=P.Association)AND(P2.Requise NOTNULL)))AND
--        (julianday(P.Date_MEP)+(SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Nb_sem_croisement')*7<
--         (SELECT min(julianday(P2.Fin_récolte)) FROM Planche_asso_esp_présentes P2
--          WHERE (P2.Planche=P.Planche)AND(P2.Association=P.Association)AND(P2.Requise NOTNULL))))
-- GROUP BY P.Planche,P.Association;

CREATE VIEW Planche_asso_possibles AS SELECT -- Liste des associations possibles pour chaque espèce des cultures non terminées.
    C.Planche,
    C.Culture,
    C.Espèce,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
    C.Fin_récolte,
    iif(C.Terminée LIKE 'v%','x',NULL) Vivace,
    C.Culture||coalesce(' ('||Planche_influente||')','')||' - '|| -- Indiquer la planche réelle de la culture.
    coalesce(C.Variété,C.IT_plante,C.Espèce)||' - '||C.Type||' - '||C.Etat||' - '||
    coalesce('Plant '||strftime('%d/%m/%Y',C.Date_plantation),'Semis '||strftime('%d/%m/%Y',C.Date_semis),'')||
    ' - Réc '||coalesce(strftime('%d/%m',C.Début_récolte),strftime('%d/%m',C.Fin_récolte),'') Cultures,
    A.Association,
    A.Espèce Espèce_A,
    A.Requise
FROM Cu_non_ter_asso C
-- LEFT JOIN Espèces E USING(Espèce)
JOIN Associations_détails__Espèces A USING(Espèce)
ORDER BY C.Planche,C.Culture,Association;

CREATE VIEW Planif_associations AS SELECT --- Associations de cultures entre les annuelles planifiées la prochaine saison et les vivaces en place sur la même planche ou sur une planche proche ('Planches_influencées' dans onglet 'Planches').
    ---no_data_text Saisissez au moins une rotation de cultures (menu Assolement) pour que la planification puisse générer des cultures.
    -- PA.IdxIdPl, ---hidden
    P.Rotation, ---
    PA.Planche, ---
    PA.Association, ---
    PA.Cultures, ---
        ---multiline
    PA.Nb Nb_cultures, --- --UNIT(cultures)
    PA.Nb_distinct Nb_espèces, --- --UNIT(espèces)
    PA.Nb_vivaces, --- --UNIT(vivaces)
    A.Espèces Espèces_de_l_association ---
        ---multiline
FROM Planif_asso_esp_requises_présentes PA
LEFT JOIN Planches P USING(Planche)
JOIN Associations__Espèces A USING(Association)
WHERE (PA.Nb>PA.Nb_vivaces)AND -- Associations qui inclue des annuelles.
      ((A.Nb>1)AND(PA.Nb_distinct>1)AND(PA.Nb_requises=A.Nb_requises))AND -- Association de plusieurs espèces et toutes les requises sont présentes.
      (julianday(PA.Fin_récolte)-julianday(PA.Date_MEP)>=
       (SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Nb_sem_croisement')*7) -- Croisement mini pour les cultures.
ORDER BY Planche,Association ---
;

CREATE VIEW Planif_asso_esp_présentes AS SELECT -- Pour chaque associations possible, compter les cultures d'espèces identiques présentes sur la planche.
    --C.IdxIdPl,
    C.Planche,
    min(C.Date_MEP) Date_MEP, -- min car si plusieurs culture d'une même espèce, une seule est nécessaire.
    max(C.Fin_récolte) Fin_récolte, -- max, idem.
    C.Association,
    C.Espèce,
    C.Requise,
    count() Nb, -- Nb cultures même planche, asso et espèce.
    count(Vivace) Nb_vivaces,
    group_concat(C.Cultures,x'0a0a') Cultures
FROM Planif_asso_possibles C
GROUP BY C.Planche,C.Association,C.Espèce,C.Requise;

CREATE VIEW Planif_asso_esp_requises_présentes AS SELECT
    -- P.IdxIdPl,
    P.Planche,
    max(iif(P.Requise NOTNULL,P.Date_MEP,DATE('2000-01-01'))) Date_MEP, -- max car les cultures requises doivent être présentes en même temps.
    min(iif(P.Requise NOTNULL,P.Fin_récolte,DATE('2100-01-01'))) Fin_récolte, -- min, idem.
    P.Association,
    CAST(count() AS INT) Nb_distinct, -- nb d'espèces même planche et asso.
    count(P.Requise) Nb_requises, -- nb d'espèces requises même planche et asso.
    CAST(sum(P.Nb) AS INT) Nb, -- Nb cultures même planche et asso.
    CAST(sum(P.Nb_vivaces) AS INT) Nb_vivaces,
    CAST(group_concat(P.Cultures,x'0a0a') AS TEXT) Cultures
FROM Planif_asso_esp_présentes P
WHERE (P.Requise NOTNULL)OR -- Espèces requises
      -- Espèces non requises qui croisent les requises.
      ((julianday(P.Fin_récolte)-(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Nb_sem_croisement')*7> --todo: les non requises ne passent jamais
        (SELECT max(julianday(P2.Date_MEP)) FROM Planif_asso_esp_présentes P2
         WHERE (P2.Planche=P.Planche)AND(P2.Association=P.Association)AND(P2.Requise NOTNULL)))AND
       (julianday(P.Date_MEP)+(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Nb_sem_croisement')*7<
        (SELECT min(julianday(P2.Fin_récolte)) FROM Planif_asso_esp_présentes P2
         WHERE (P2.Planche=P.Planche)AND(P2.Association=P.Association)AND(P2.Requise NOTNULL))))
GROUP BY P.Planche,P.Association;

CREATE VIEW Planif_asso_possibles AS SELECT -- Liste des associations possibles pour chaque espèce des cultures planifiées.
    -- C.IdxIdPl,
    C.Planche,
    C.Espèce,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
    C.Fin_récolte,
    coalesce(C.IT_plante,C.Espèce)||
    coalesce(' ('||C.Culture||' - '||C.Planche_influente||')',' (planif)')||' - '|| -- Indiquer la planche réelle de la culture de vivace.
    coalesce('Plant '||strftime('%d/%m/%Y',C.Date_plantation),'Semis '||strftime('%d/%m/%Y',C.Date_semis),'')||
    ' - Réc '||coalesce(strftime('%d/%m',C.Début_récolte),strftime('%d/%m',C.Fin_récolte),'') Cultures,
    C.Vivace,
    A.Association,
    A.Espèce Espèce_A,
    A.Requise
FROM Planif_planches_asso C
JOIN Associations_détails__Espèces A USING(Espèce)
ORDER BY C.Planche,Association;

CREATE VIEW Pl_Décalées AS SELECT
    PL.Planche,
    PL.Rotation,
    PL.Année +(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+1-R.Année_1
             -(R.Nb_années*floor(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+0-R.Année_1+PL.Année)/R.Nb_années)) Année,
    PL.Longueur,
    PL.Largeur
FROM Planches PL
JOIN Rotations R USING(Rotation)
WHERE R.Active NOTNULL;

CREATE VIEW Pl_bilan_fert AS SELECT
    C.Saison,
    P.Planche,
    C.Culture,
    coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante,
    C.Etat,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
    coalesce(CASE WHEN C.Espacement>0 THEN round(1.0/C.Espacement*100*C.Nb_rangs/P.Largeur/E.Densité*100)
             ELSE round(I.Dose_semis/E.Dose_semis*100) END,100) Densité_pc,
    E.N*C.Longueur*P.Largeur N_esp,
    E.P*C.Longueur*P.Largeur P_esp,
    E.K*C.Longueur*P.Largeur K_esp,
    NULL N_fert,
    NULL P_fert,
    NULL K_fert,
    NULL Fertilisation
FROM Planches P
LEFT JOIN Cultures C USING(Planche)
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN ITP I USING(IT_plante)
WHERE coalesce(C.Terminée,'') NOT LIKE 'v%'
UNION
SELECT
    substr(DATE(F.Date,(coalesce(Fe.Délai,1)*7)||' days'),1,4) Saison,
    P.Planche,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    F.N N_fert,
    F.P P_fert,
    F.K K_fert,
    F.Fertilisant||' '||CAST(F.Quantité AS TEXT)||'kg - '||strftime('%d/%m/%Y',F.Date) Fertilisation
FROM Planches P
LEFT JOIN Fertilisations F USING(Planche)
JOIN Fertilisants Fe USING(Fertilisant)
-- GROUP BY Saison,Planche
-- ORDER BY Planche,Date_MEP;

CREATE VIEW Assolement_Ilots AS SELECT --- Les ilots sont des regroupements de planches.
                                     --- Ils ne sont pas saisis mais déduits des planches saisies.
                                     --- Le débuts du nom des planches indique leur ilot (voir le paramètre 'Ilot_nb_car').
    CAST(substr(P.Planche,1,(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Ilot_nb_car')) AS TEXT) Ilot, ---
    P.Type, ---
    CAST(count() AS INTEGER) Nb_planches, --- --UNIT(planches)
    CAST(round(sum(P.Longueur),2) AS REAL) Longueur, ---
        ---unit m
    CAST(round(sum(P.Longueur*P.Largeur),2)AS REAL) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
    P.Rotation --- ::Rotations.Rotation
FROM Planches P
GROUP BY Ilot,Type,Rotation
ORDER BY Ilot,Type,Rotation ---
;

CREATE VIEW Assolement_Unités_prod AS SELECT --- Les unités de production sont des regroupements de planches.
                                             --- Elles ne sont pas saisies mais déduites des planches saisies.
                                             --- Le débuts du nom des planches indique leur ilot (paramètre 'Ilot_nb_car') puis leur unité de production (paramètre 'UP_nb_car').
    CAST(substr(P.Planche,1,(SELECT CAST(Valeur AS INT)
                             FROM Params WHERE Paramètre='Ilot_nb_car')+
                            (SELECT CAST(Valeur AS INT)
                             FROM Params WHERE Paramètre='UP_nb_car')) AS TEXT) Unité_prod, ---
    P.Type, ---
    CAST(count() AS INTEGER) Nb_planches, --- --UNIT(planches)
    CAST(round(sum(P.Longueur),2) AS REAL) Longueur, ---
        ---unit m
    CAST(round(sum(P.Longueur*P.Largeur),2)AS REAL) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
    P.Rotation ---
FROM Planches P
GROUP BY Unité_prod,Type,Rotation
ORDER BY Unité_prod,Type,Rotation ---
;

CREATE VIEW Planches__bilan_fert AS SELECT --- Bilan de fertilisation par planche sur une saison.
                                           --- Besoins=somme des besoins des cultures annuelles de la saison.
                                           --- Apports=somme des fertilisations des cultures annuelles de la saison.
    P.Saison, ---
        ---hidden
    P.Planche, ---
    PL.Type, ---
    PL.Longueur*PL.Largeur Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
    CAST(count(P.Culture) AS INT) Nb_cu, --- --UNIT(cultures)
    CAST(group_concat(P.Culture||' - '||P.Variété_ou_It_plante||' - '||P.Etat||' - '||strftime('%d/%m/%Y',P.Date_MEP),x'0a0a') AS TEXT) Cultures, ---
        ---multiline
    CAST(CAST(sum(P.N_esp*P.Densité_pc/100) AS INTEGER)||'-'||
         CAST(sum(P.P_esp*P.Densité_pc/100) AS INTEGER)||'-'||
         CAST(sum(P.K_esp*P.Densité_pc/100) AS INTEGER) AS TEXT) Besoins_NPK, --- Besoins en Azote - Phosphore - Potassium, pour les planches.
--                                                                            --- Besoin de l'espèce x densité réelle x surface de planche.
        ---unit g
    CAST(A.N||'-'||A.P||'-'||A.K||' ('||A.Analyse||')'||x'0a0a'||
         coalesce(A.Interprétation,'Pas d''interprétation') AS TEXT) Analyse_sol, --- Teneur NPK (g/kg) et interprétation de l'analyse de sol de la planche.
        ---multiline
    CAST(group_concat(P.Fertilisation,x'0a0a') AS TEXT) Fertilisants, ---
        ---multiline
    CAST(CAST(sum(P.N_fert) AS INTEGER)||'-'||CAST(sum(P.P_fert) AS INTEGER)||'-'||CAST(sum(P.K_fert) AS INTEGER) AS TEXT) Apports_NPK, --- Sommes des fertilisations (Azote - Phosphore - Potassium) déjà faites sur les planches.
        ---unit g
    CAST(round(min(sum(coalesce(P.N_fert,0))/total(P.N_esp*P.Densité_pc/100),
                   sum(coalesce(P.P_fert,0))/total(P.P_esp*P.Densité_pc/100),
                   sum(coalesce(P.K_fert,0))/total(P.K_esp*P.Densité_pc/100))*100) AS INTEGER) Fert_pc, --- Pourcentage de fertilisation déjà effectué pour l'élément N, P ou K le plus déficitaire.
    CAST(sum(P.N_esp*P.Densité_pc/100)-sum(P.N_fert) AS INT) N_manq, ---
        ---unit g
        ---cond_formats ::>0 #darkRed# #lightRed#,::<0 #darkGreen# #lightGreen#
    CAST(sum(P.P_esp*P.Densité_pc/100)-sum(P.P_fert) AS INT) P_manq, ---
        ---unit g
        ---cond_formats ::>0 #darkRed# #lightRed#,::<0 #darkGreen# #lightGreen#
    CAST(sum(P.K_esp*P.Densité_pc/100)-sum(P.K_fert) AS INT) K_manq, ---
        ---unit g
        ---cond_formats ::>0 #darkRed# #lightRed#,::<0 #darkGreen# #lightGreen#
    PL.Notes ---
FROM Pl_bilan_fert P
JOIN Planches PL USING(Planche)
LEFT JOIN Analyses_de_sol A USING(Analyse)
GROUP BY P.Saison,P.Planche;

CREATE VIEW Planches__deficit_fert AS SELECT --- Planches dont le bilan de fertilisation est faible (paramètre 'Déficit_fert') sur une des 2 saisons précédente (N-1 ou N-2).
    P.Planche, ---
    P.Type, ---
    CAST((SELECT group_concat(CAST(C.Culture AS TEXT)||' - '||coalesce(C.Variété,C.IT_plante,C.Espèce)||' - '||C.Etat,x'0a0a') FROM Cu_non_ter C
          WHERE C.Planche=P.Planche) AS TEXT) Cultures, ---
        ---multiline
    (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
     WHERE (PBF1.Planche=P.Planche)AND(CAST(PBF1.Saison AS INT)=(SELECT Valeur-3 FROM Params WHERE Paramètre='Année_culture'))) Fert_Nm3_pc, --- Fertilisation année N-3.
        ---dyn_header SELECT 'Fert '||(Valeur-3) FROM Params WHERE Paramètre='Année_culture'
        ---col_width 100
        ---cond_formats ::<50 #darkRed# #lightRed#,::>100 #darkGreen# #lightGreen#
    (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
     WHERE (PBF1.Planche=P.Planche)AND(CAST(PBF1.Saison AS INT)=(SELECT Valeur-2 FROM Params WHERE Paramètre='Année_culture'))) Fert_Nm2_pc, --- Fertilisation année N-2.
        ---dyn_header SELECT 'Fert '||(Valeur-2) FROM Params WHERE Paramètre='Année_culture'
        ---col_width 100
        ---cond_formats ::<50 #darkRed# #lightRed#,::>100 #darkGreen# #lightGreen#
    (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
     WHERE (PBF1.Planche=P.Planche)AND(CAST(PBF1.Saison AS INT)=(SELECT Valeur-1 FROM Params WHERE Paramètre='Année_culture'))) Fert_Nm1_pc, --- Fertilisation année précédente.
        ---dyn_header SELECT 'Fert '||(Valeur-1) FROM Params WHERE Paramètre='Année_culture'
        ---col_width 100
        ---cond_formats ::<50 #darkRed# #lightRed#,::>100 #darkGreen# #lightGreen#
    (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
     WHERE (PBF1.Planche=P.Planche)AND(CAST(PBF1.Saison AS INT)=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture'))) Fert_N_pc --- Fertilisation année de culture.
                                                                                                                           --- Paramètre 'Année_culture'.
        ---dyn_header SELECT 'Fert '||Valeur FROM Params WHERE Paramètre='Année_culture'
        ---col_width 100
        ---cond_formats ::<50 #darkRed# #lightRed#,::>100 #darkGreen# #lightGreen#
FROM Planches P
WHERE (P.Planche IN (SELECT PBF.Planche FROM Planches__bilan_fert PBF
                     WHERE (PBF.Fert_pc<(SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Déficit_fert'))AND
                           (CAST(PBF.Saison AS INT) BETWEEN (SELECT Valeur-2 FROM Params WHERE Paramètre='Année_culture')AND
                                                            (SELECT Valeur-1 FROM Params WHERE Paramètre='Année_culture'))));

CREATE VIEW Planif_espèces AS SELECT --- Espèces pour lesquelles des cultures vont être créées lors de la prochaîne planification.
                                     --- Liste non directement modifiable, déduite des 'Rotations'.
    ---no_data_text Saisissez au moins une rotation de cultures (menu Assolement) pour que la planification puisse générer des cultures.
    ---row_summary Espèce,Nb_planches,Longueur
    C.Espèce, ---
    E.Famille, ---
    CAST(count() AS INTEGER) Nb_planches, --- --UNIT(planches)
    CAST(sum(C.Longueur)AS REAL) Longueur, ---
        ---unit m
    CAST(sum(C.Longueur*C.Nb_rangs)AS REAL) Long_rang, ---
        ---unit m
    CAST(sum(C.Longueur*C.Nb_rangs/C.Espacement*100) AS INTEGER) Nb_plants, --- --UNIT(plants)
    CAST(sum(C.Surface)AS REAL) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
    E.Rendement, ---
        ---unit kg/m²
    E.Obj_annuel, ---
        ---unit kg
    CAST(sum(C.Prod_possible)AS REAL) Prod_possible, --- ::Espèces__manquantes.Prod_possible
        ---unit kg
    CAST(sum(C.Prod_possible)/E.Obj_annuel*100 AS INT) Couv_pc, --- ::Espèces__manquantes.Couv_pc
        ---cond_formats ::<50 #darkRed# #lightRed#,::>200 #darkRed# #lightRed#,::<90,::>150,true #darkGreen# #lightGreen#
    C.Saison ---
FROM Planif_pl_date C
LEFT JOIN Espèces E USING(Espèce)
GROUP BY Espèce
ORDER BY Espèce ---
;

CREATE VIEW Planif_ilots AS SELECT --- Espèces pour lesquelles des cultures vont être créées lors de la prochaîne planification.
                                   --- Liste non directement modifiable, déduite des 'Rotations'.
    ---no_data_text Saisissez au moins une rotation de cultures (menu Assolement) pour que la planification puisse générer des cultures.
    ---row_summary Ilot|Ilot :: - ,Nb_planches,Longueur
    CAST(substr(C.Planche,1,(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Ilot_nb_car')) AS TEXT) Ilot, ---
    C.Espèce, ---
    min(Date_semis) Date_semis, ---
    min(Date_plantation) Date_plantation, ---
    CAST(count() AS INTEGER) Nb_planches, --- --UNIT(planches)
    CAST(sum(C.Longueur)AS REAL) Longueur, ---
        ---unit m
    CAST(sum(C.Longueur*C.Nb_rangs)AS REAL) Long_rang, ---
        ---unit m
    CAST(sum(C.Longueur*C.Nb_rangs/C.Espacement*100) AS INTEGER) Nb_plants, --- --UNIT(plants)
    CAST(sum(C.Surface)AS REAL) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
    CAST(sum(C.Prod_possible)AS REAL) Prod_possible, --- ::Espèces__manquantes.Prod_possible
        ---unit kg
    C.Saison ---
FROM Planif_pl_date C
GROUP BY Ilot,Espèce
ORDER BY Ilot,Espèce ---
;

CREATE VIEW Planif_planches AS SELECT --- Cultures à valider pour qu'elles soient créées lors de la prochaîne planification.
                                      -- --- La variété choisie pour chaque culture est celle dont la quantité de semence en stock est la plus importante.
                                      --- La liste peut être limitée à certaines planches (paramètre 'Planifier_planches').
                                      --- Liste déduite des 'Rotations'.
    ---no_data_text Saisissez au moins une rotation de cultures (menu Assolement) pour que la planification puisse générer des cultures.
    ---row_summary Planche,IT_plante,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    PP.ID, --- Nécessaire pour mise à jour de la table Rotations_détails via trigger INSTEAD OF UPDATE.
        ---hidden
    PP.IdxIdPl, --- Nécessaire pour la mise à jour de la table Planif_validations via trigger INSTEAD OF UPDATE.
        ---hidden
    PP.Rotation, ---
    PP.Planche, ---
    PP.Espèce, ---
    PP.IT_plante, --- ::Cultures.IT_plante
    -- (SELECT V.Variété FROM Variétés V WHERE V.Espèce=PP.Espèce ORDER BY V.Qté_stock DESC) Variété, --- -- Les IT des plans de rotation ont toujours une Espèce.
    -- (SELECT V.Fournisseur FROM Variétés V WHERE V.Espèce=PP.Espèce ORDER BY V.Qté_stock DESC) Fournisseur, ---
    PP.Date_semis, ---
    PP.Date_plantation, ---
    PP.Début_récolte, ---
    PP.Fin_récolte, ---
    CASE WHEN (PP.Déjà_créée NOTNULL)
         THEN PP.Déjà_créée||' - '||(SELECT CDC.Etat FROM Cultures CDC WHERE CDC.Culture=PP.Déjà_créée)||
                -- Différences entre la culture trouvée et les données de planif.
                CASE WHEN (SELECT (CDC.Date_semis!=PP.Date_semis)OR(CDC.Date_plantation!=PP.Date_plantation)OR
                                  (CDC.Longueur!=PP.Longueur)OR(CDC.Espacement!=PP.Espacement)OR(CDC.Nb_rangs!=PP.Nb_rangs) FROM Cultures CDC WHERE CDC.Culture=PP.Déjà_créée)
                     THEN (SELECT x'0a0a'||'Différences :'||
                                  iif(CDC.Date_semis!=PP.Date_semis,x'0a0a'||'Semis '||coalesce(strftime('%d/%m/%Y',CDC.Date_semis),'vide'),'')||
                                  iif(CDC.Date_plantation!=PP.Date_plantation,x'0a0a'||'Plantation '||coalesce(strftime('%d/%m/%Y',CDC.Date_plantation),'vide'),'')||
                                  iif(CDC.Longueur!=PP.Longueur,x'0a0a'||'Longueur '||coalesce(CDC.Longueur||' m','vide'),'')||
                                  iif(CDC.Espacement!=PP.Espacement,x'0a0a'||'Espacement '||coalesce(CDC.Espacement||' cm','vide'),'')||
                                  iif(CDC.Nb_rangs!=PP.Nb_rangs,x'0a0a'||coalesce(CAST(CDC.Nb_rangs AS INT)||' rang(s)',''),'')||
                                  '!' --Affichage en rouge car il y a des différences
                           FROM Cultures CDC WHERE CDC.Culture=PP.Déjà_créée)
                     ELSE ''
                END
         END Déjà_créée, --- Une culture a déjà été créée avec des dates de semis/plantation de plus ou moins 30 jours par rapport aux dates de la planification.
        ---multiline
    (SELECT group_concat(Cultures,x'0a0a')
     FROM (SELECT C.Culture||' - '||coalesce(C.Variété,C.Espèce)||' - '||C.Etat||
                  ' - '||strftime('%d/%m/%Y',coalesce(C.Date_plantation,C.Date_semis))||' - '||strftime('%d/%m/%Y',C.Fin_récolte) Cultures
           FROM Cultures C
           WHERE (C.Terminée ISNULL)AND(C.Planche=PP.Planche)AND(C.Fin_récolte NOTNULL)AND
                 (C.Fin_récolte>coalesce(PP.Date_plantation,PP.Date_semis))
           ORDER BY C.Culture DESC))||
     iif(PP.Déjà_créée ISNULL,'!','') Déjà_en_place, --- Cultures de la saison précédente qui ne seront pas terminées lors de la mise en place de cette nouvelle culture,
                                                     --- ou culture déjà créées pour la saison à planifier.
                                                     ---
                                                     --- La 1ère date est la date de mise en place (plantation ou semis direct).
                                                     --- La 2ème date est la date de fin de récolte.
        ---multiline
    PV.Validée, --- Indiquez si cette culture doit être créée lors de la prochaine planification.
                --- Si la valeur saisie est un texte (et pas simplement un 'x' ou ✔), il sera copié dans les notes de la culture.
    PP.Longueur, ---
        ---unit m
    PP.Nb_rangs, ---
        ---unit rangs
    PP.Espacement, --- ::Cultures.Espacement
        ---unit cm
    PP.Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
    PP.Prod_possible, --- ::Espèces__manquantes.Prod_possible
        ---unit kg
    PP.Notes, ---
        ---multiline
    PP.Saison ---
FROM Planif_pl_date2 PP
LEFT JOIN Planif_validations PV USING(IdxIdPl)
ORDER BY coalesce(PP.Date_plantation,PP.Date_semis) ---
;

-- CREATE VIEW Planif_pl_date3 AS SELECT -- Suppression des n° de culture "Déjà créée" si déjà utilisé précedememnt. TROP LENT.
--     PP.ID,
--     PP.IdxIdPl,
--     PP.Rotation,
--     PP.Planche,
--     PP.Espèce,
--     PP.IT_plante,
--     PP.Date_semis,
--     PP.Date_plantation,
--     PP.Début_récolte,
--     PP.Fin_récolte,
--     CASE WHEN (PP.Déjà_créée NOTNULL)AND
--               NOT (PP.Déjà_créée IN (SELECT PP2.Déjà_créée FROM Planif_pl_date2 PP2
--                                      WHERE coalesce(PP2.Date_plantation,PP2.Date_Semis)<coalesce(PP.Date_plantation,PP.Date_Semis)))
--          THEN PP.Déjà_créée
--          END Déjà_créée,
--     PP.Longueur,
--     PP.Nb_rangs,
--     PP.Espacement,
--     PP.Surface,
--     PP.Prod_possible,
--     PP.Notes,
--     PP.Saison
-- FROM Planif_pl_date2 PP;

CREATE VIEW Planif_pl_date2 AS SELECT
    PP.ID,
    PP.IdxIdPl,
    PP.Rotation,
    PP.Planche,
    PP.Espèce,
    PP.IT_plante,
    PP.Date_semis,
    PP.Date_plantation,
    PP.Début_récolte,
    PP.Fin_récolte,
    (SELECT Culture FROM
        (SELECT C.Culture,
                abs(iif(C.Date_semis NOTNULL AND PP.Date_semis NOTNULL,CAST((julianday(C.Date_semis)-julianday(PP.Date_semis)) AS INT),0))
               +abs(iif(C.Date_plantation NOTNULL AND PP.Date_plantation NOTNULL,CAST((julianday(C.Date_plantation)-julianday(PP.Date_plantation)) AS INT),0)) Proximité
         FROM Cultures C
         WHERE (C.Terminée ISNULL)AND
               (C.Espèce=PP.Espèce)AND(C.IT_plante=PP.IT_plante)AND(C.Planche=PP.Planche)AND
                (((C.Date_semis ISNULL)AND(PP.Date_semis ISNULL))OR
                 (C.Date_semis BETWEEN DATE(PP.Date_semis,'-30 days') AND (DATE(PP.Date_semis,'+30 days'))))AND
                (((C.Date_plantation ISNULL)AND(PP.Date_plantation ISNULL))OR
                 (C.Date_plantation BETWEEN DATE(PP.Date_plantation,'-30 days') AND (DATE(PP.Date_plantation,'+30 days')))))
     ORDER BY Proximité) Déjà_créée,
    PP.Longueur,
    PP.Nb_rangs,
    PP.Espacement,
    PP.Surface,
    PP.Prod_possible,
    PP.Notes,
    PP.Saison
FROM Planif_pl_date PP
ORDER BY coalesce(PP.Date_plantation,PP.Date_semis);

CREATE VIEW Planif_pl_date AS SELECT
    RD.ID,
    RD.ID||'-'||PL.Planche IdxIdPl,
    RD.Rotation||' - an.'||RD.Année Rotation,
    PL.Planche,
    E.Espèce,
    RD.IT_plante,
    DATE(I.Date_semis,(coalesce(RD.Décalage*I.Coef_semis,0)*7)||' days') Date_semis,
    DATE(I.Date_plantation,(coalesce(RD.Décalage*I.Coef_plantation,0)*7)||' days') Date_plantation,
    DATE(I.Début_récolte,(coalesce(RD.Décalage*I.Coef_début_récolte,0)*7)||' days') Début_récolte,
    DATE(I.Fin_récolte,(coalesce(RD.Décalage*I.Coef_fin_récolte,0)*7)||' days') Fin_récolte,
    CAST(CASE WHEN RD.Occupation='R' THEN PL.Longueur
              WHEN RD.Occupation='E' THEN PL.Longueur
              ELSE round((RD.Pc_planches/100*PL.Longueur),2)
              END AS REAL) Longueur,
    CAST(CASE WHEN RD.Occupation='R' THEN max(round(RD.Pc_planches/100*PL.Largeur*100/I.Esp_rangs),1)
              WHEN RD.Occupation='E' THEN max(round(PL.Largeur*100/I.Esp_rangs),1)
              ELSE max(round(PL.Largeur*100/I.Esp_rangs),1)
              END AS REAL) Nb_rangs,
    CAST(CASE WHEN RD.Occupation='R' THEN I.Espacement
              WHEN RD.Occupation='E' THEN round(I.Espacement/RD.Pc_planches*100)
              ELSE I.Espacement
              END AS REAL) Espacement,
    CAST(round(RD.Pc_planches/100*PL.Longueur*PL.Largeur,2)AS REAL) Surface,
    CAST(round(RD.Pc_planches/100*PL.Longueur*PL.Largeur*E.Rendement,2)AS REAL) Prod_possible,
    RD.Notes,
    CAST((SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture')AS INTEGER) Saison
FROM Rotations_détails RD
JOIN Rotations R USING(Rotation)
LEFT JOIN Pl_Décalées PL USING(Rotation,Année)
JOIN ITP_planif I USING(IT_plante)
LEFT JOIN Espèces E USING(Espèce)
WHERE (R.Active NOTNULL)AND
      ((PL.Planche ISNULL)OR
       (SELECT (Valeur ISNULL) FROM Params WHERE Paramètre='Planifier_planches')OR
       (PL.Planche LIKE (SELECT Valeur FROM Params WHERE Paramètre='Planifier_planches')||'%'))AND
      ((RD.Fi_planches ISNULL) OR (Fi_planches LIKE '%'||substr(PL.Planche,-1,1) ||'%'));

CREATE VIEW Planif_planches_asso AS SELECT -- Cultures annulles et vivaces non terminées
    -- C.IdxIdPl,
    C.Planche,
    NULL Culture,
    C.Espèce,
    C.IT_plante,
    C.Date_semis,
    C.Date_plantation,
    C.Début_récolte,
    C.Fin_récolte,
    NULL Vivace,
    NULL Planche_influente
FROM Planif_pl_date C
UNION
SELECT -- Vivaces non terminées sur leurs planches réelles
    -- NULL,
    CI.Planche,
    CI.Culture,
    CI.Espèce,
    CI.IT_plante,
    CI.Date_semis,
    CI.Date_plantation,
    CI.Début_récolte,
    CI.Fin_récolte,
    'x' Vivace,
    CI.Planche Planche_influente
FROM Cultures CI
WHERE CI.Terminée='v' OR CI.Terminée='V'
UNION
SELECT -- Vivaces non terminées sur leurs planches influencées
    -- NULL,
    PI.Planche,
    CI.Culture,
    CI.Espèce,
    CI.IT_plante,
    CI.Date_semis,
    CI.Date_plantation,
    CI.Début_récolte,
    CI.Fin_récolte,
    'x' Vivace,
    CI.Planche Planche_influente
FROM Cultures CI
JOIN Planches P USING(Planche)
JOIN Planches PI ON P.Planches_influencées||',' LIKE '%'||PI.Planche||',%'
WHERE CI.Terminée='v' OR CI.Terminée='V';

CREATE VIEW Planif_récoltes_m AS
WITH RECURSIVE Mois AS (
    SELECT Planche||Espèce||Début_récolte Id,
        Espèce,
        Début_récolte Date_jour,
        Fin_récolte,
        Prod_possible,
        0 Offset
    FROM Planif_pl_date
    WHERE (Début_récolte NOTNULL)AND(Fin_récolte NOTNULL)
    UNION ALL
    SELECT Id,
        Espèce,
        date(Date_jour,'+1 month') Date_jour,
        Fin_récolte,
        Prod_possible,
        Offset + 1
    FROM Mois
    WHERE Date_jour < Fin_récolte ),
Durées AS (
    SELECT Id,
        count() Nb_Mois
    FROM Mois GROUP BY Id )

SELECT M.Id,
    M.Espèce,
    M.Date_jour Date,
    CAST(CASE WHEN D.Nb_Mois=1 THEN M.Prod_possible
              WHEN D.Nb_Mois=2 THEN M.Prod_possible/2
              WHEN (M.Offset=0)OR(M.Offset=D.Nb_Mois-1) THEN M.Prod_possible / (D.Nb_Mois-1) / 2
              ELSE M.Prod_possible / (D.Nb_Mois-1)
              END AS REAL) Qté_réc,
    E.Prix_kg,
    CAST(((CASE WHEN D.Nb_Mois=1 THEN M.Prod_possible
                WHEN D.Nb_Mois=2 THEN M.Prod_possible/2
                WHEN (M.Offset=0)OR(M.Offset=D.Nb_Mois-1) THEN M.Prod_possible / (D.Nb_Mois-1) / 2
                ELSE M.Prod_possible / (D.Nb_Mois-1)
                END)*E.Prix_kg) AS REAL) Valeur
FROM Mois M
JOIN Durées D USING(Id)
JOIN Espèces E USING(Espèce)
ORDER BY M.Date_jour;

CREATE VIEW Planif_récoltes_s AS
WITH RECURSIVE Semaines AS (
SELECT Planche||Espèce||Début_récolte Id,
    Espèce,
    Début_récolte Date_jour,
    Fin_récolte,
    Prod_possible,
    0 Offset
FROM Planif_pl_date
WHERE (Début_récolte NOTNULL)AND(Fin_récolte NOTNULL)
UNION ALL
SELECT Id,
    Espèce,
    date(Date_jour,'+7 day') Date_jour,
    Fin_récolte,
    Prod_possible,
    Offset + 1
FROM Semaines
WHERE Date_jour < Fin_récolte ),
Durées AS (
SELECT Id,
    count() Nb_semaines
FROM Semaines GROUP BY Id )

SELECT S.Id,
    S.Espèce,
    S.Date_jour Date,
    CAST(CASE WHEN D.Nb_semaines=1 THEN S.Prod_possible
              WHEN D.Nb_semaines=2 THEN S.Prod_possible/2
              WHEN (S.Offset=0)OR(S.Offset=D.Nb_semaines-1) THEN S.Prod_possible / (D.Nb_semaines-1) / 2
              ELSE S.Prod_possible / (D.Nb_semaines-1)
              END AS REAL) Qté_réc,
    E.Prix_kg,
    CAST(((CASE WHEN D.Nb_semaines=1 THEN S.Prod_possible
                WHEN D.Nb_semaines=2 THEN S.Prod_possible/2
                WHEN (S.Offset=0)OR(S.Offset=D.Nb_semaines-1) THEN S.Prod_possible / (D.Nb_semaines-1) / 2
                ELSE S.Prod_possible / (D.Nb_semaines-1)
                END)*E.Prix_kg) AS REAL) Valeur
FROM Semaines S
JOIN Durées D USING(Id)
JOIN Espèces E USING(Espèce)
ORDER BY S.Date_jour;

CREATE VIEW Prm_NbMaxSemPlancheLibreSMeP AS SELECT -- nb max de semaines de planche libre avant, en fonction de la semaine de MEP.
    1 Semaine,
    23 Nb_sem
UNION
VALUES (2,23),(3,24),(4,24),(5,25), -- janv
       (6,25),(7,26),(8,27),(9,28),
       (10,29),(11,30),(12,31),(13,32),
       (14,33),(15,34),(16,34),(17,35),
       (18,35),(19,35),(20,34),(21,34),(22,33),
       (23,32),(24,31),(25,30),(26,29),
       (27,28),(28,27),(29,26),(30,25),(31,24),
       (32,23),(33,22),(34,21),(35,20),
       (36,20),(37,19),(38,19),(39,18),
       (40,18),(41,17),(42,17),(43,17),(44,18),
       (45,18),(46,19),(47,19),(48,20),
       (49,20),(50,21),(51,21),(52,22),(53,22); -- déc

CREATE VIEW Prm_draw AS SELECT --Ensembles prédéfinis de commandes de dessin.
    'Vide' name,
    '' value
UNION
VALUES
    ('GraphBarresVert12Mois',
     'color(#838383) line(31) line(59) line(90) line(120) line(151) line(181) line(212) line(243) line(273) line(304) line(334) line(364) line(366) color()'),
    ('GraphBarresVert24Mois',
     'color(#838383) line(31) line(59) line(90) line(120) line(151) line(181) line(212) line(243) line(273) line(304) line(334) line(364) line(366) '||
     'offset(+365)   line(31) line(59) line(90) line(120) line(151) line(181) line(212) line(243) line(273) line(304) line(334) line(364) line(366) color() offset(-365)'),
    ('GraphBarresVert36Mois',
     'color(#838383) line(31) line(59) line(90) line(120) line(151) line(181) line(212) line(243) line(273) line(304) line(334) line(364) line(366) '||
     'offset(+365)   line(31) line(59) line(90) line(120) line(151) line(181) line(212) line(243) line(273) line(304) line(334) line(364) line(366) '||
     'offset(+365)   line(31) line(59) line(90) line(120) line(151) line(181) line(212) line(243) line(273) line(304) line(334) line(364) line(366) color() offset(-730)'),
    ('GraphTitres12Mois',
     'offset(+0,5) text(jan,5) text(fév,36) text(mar,64) text(avr,96) text(mai,125) text(juin,156) text(juil,188) text(aoû,217) text(sep,248) text(oct,278) text(nov,309) text(déc,339)'),
    ('GraphTitres24Mois',
     'offset(+0,5) text(jan,5) text(fév,36) text(mar,64) text(avr,96) text(mai,125) text(juin,156) text(juil,188) text(aoû,217) text(sep,248) text(oct,278) text(nov,309) text(déc,339) '||
     'offset(+365) text(jan,5) text(fév,36) text(mar,64) text(avr,96) text(mai,125) text(juin,156) text(juil,188) text(aoû,217) text(sep,248) text(oct,278) text(nov,309) text(déc,339)'),
    ('GraphTitres36Mois',
     'offset(+0,5) text(jan,5) text(fév,36) text(mar,64) text(avr,96) text(mai,125) text(juin,156) text(juil,188) text(aoû,217) text(sep,248) text(oct,278) text(nov,309) text(déc,339) '||
     'offset(+365) text(jan,5) text(fév,36) text(mar,64) text(avr,96) text(mai,125) text(juin,156) text(juil,188) text(aoû,217) text(sep,248) text(oct,278) text(nov,309) text(déc,339) '||
     'offset(+365) text(jan,5) text(fév,36) text(mar,64) text(avr,96) text(mai,125) text(juin,156) text(juil,188) text(aoû,217) text(sep,248) text(oct,278) text(nov,309) text(déc,339)');

-- CREATE VIEW RD_asso_esp_présentes AS SELECT -- Pour chaque associations possible, compter les cultures d'espèces différentes présentes dans la rotation.
--     R.IdxRotAnFp,
--     min(R.S_MEP) S_MEP, -- min car si plusieurs culture d'une même espèce, une seule est nécessaire.
--     max(R.S_Fin_récolte) S_Fin_récolte, -- max, idem.
--     R.Association,
--     R.Espèce,
--     R.Requise,
--     count() Nb
-- FROM RD_asso_possibles R
-- GROUP BY R.IdxRotAnFp,R.Association,R.Espèce,R.Requise;

-- CREATE VIEW RD_asso_esp_requises_présentes AS SELECT
--     R.IdxRotAnFp,
--     max(iif(R.Requise NOTNULL,R.S_MEP,0)) S_MEP, -- max car les cultures requises doivent être présentes en même temps.
--     min(iif(R.Requise NOTNULL,R.S_Fin_récolte,1000)) S_Fin_récolte, -- min, idem.
--     R.Association,
--     count() Nb_distinct,
--     count(R.Requise) Nb_requises,
--     sum(R.Nb) Nb,
--     group_concat(R.Espèce) Espèces_présentes
-- FROM RD_asso_esp_présentes R
-- WHERE (R.Requise NOTNULL)OR -- Espèces requises
--       -- Espèces non requises qui croisent les requises.
--       ((R.S_Fin_récolte-(SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Nb_sem_croisement')>
--         (SELECT max(R2.S_MEP) FROM RD_asso_esp_présentes R2
--          WHERE (R2.IdxRotAnFp=R.IdxRotAnFp)AND
--                (R2.Association=R.Association)AND(R2.Requise NOTNULL)))AND
--        (R.S_MEP+(SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Nb_sem_croisement')<
--         (SELECT min(R2.S_Fin_récolte) FROM RD_asso_esp_présentes R2
--          WHERE (R2.IdxRotAnFp=R.IdxRotAnFp)AND
--                (R2.Association=R.Association)AND(R2.Requise NOTNULL))))
-- GROUP BY R.IdxRotAnFp,R.Association;

-- CREATE VIEW RD_asso_possibles AS SELECT -- Liste des associations possibles pour chaque espèce d'une rotation.
--     R.IdxRotAnFp,
--     R.Espèce,
--     R.S_MEP,
--     R.S_Fin_récolte,
--     A.Association,
--     A.Espèce Espèce_A,
--     A.Requise
-- FROM RD_espèces R
-- JOIN Associations_détails__Espèces_a A USING(Espèce)
-- ORDER BY R.IdxRotAnFp,R.Espèce,A.Association;

-- CREATE VIEW RD_asso_possibles_cult AS SELECT -- Liste des associations possibles pour chaque espèce d'une rotation.
--     R.ID,
--     A.Association,
--     A.Requise
-- FROM Rotations_détails R
-- JOIN RD_ITP RI USING(ID)
-- JOIN Associations_détails__Espèces_a A ON (A.Espèce=RI.Espèce);

-- CREATE VIEW RD_asso_présentes AS SELECT -- Sélectionner les associations présentes dans la rotation.
--     R.*,
--     A.Nb Nb_asso,
--     A.Espèces
-- FROM RD_asso_esp_requises_présentes R
-- JOIN Associations__Espèces A USING(Association)
-- WHERE ((A.Nb>1)AND(R.Nb_distinct>1)AND(R.Nb_requises=A.Nb_requises))AND -- Association de plusieurs espèces/familles et les requises sont présentes.
--       ((R.S_Fin_récolte-R.S_MEP)>=(SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Nb_sem_croisement')); -- Croisement mini pour les cultures.

-- CREATE VIEW RD_espèces AS -- Liste des espèces d'une rotation.
-- SELECT -- Espèce trouvées sur les rotations
--     DISTINCT R.Rotation||'-'||R.Année||'-'||coalesce(R.Fi_planches,'') IdxRotAnFp,
--     I.Espèce,
--     max(coalesce(I.S_plantation+coalesce(R.décalage,0),I.S_semis+coalesce(R.décalage,0),52)) S_MEP,
--     min(coalesce(I.S_récolte+coalesce(I.D_récolte,0)+coalesce(R.décalage,0),52)) S_Fin_récolte
-- FROM Rotations_détails R
-- JOIN Rotations Rot USING(Rotation)
-- LEFT JOIN ITP_sem_corrigées I USING(IT_plante)
-- LEFT JOIN Espèces E USING(Espèce)
-- WHERE Rot.Active NOTNULL
-- GROUP BY IdxRotAnFp,Espèce
-- UNION
-- SELECT -- Espèce sans Fi_planches, les reporter sur toutes les planches spécifiées pour d'autre espèces.
--     DISTINCT R.Rotation||'-'||R.Année||'-'||R2.Fi_planches IdxRotAnFp,
--     I.Espèce,
--     max(coalesce(I.S_plantation+coalesce(R.décalage,0),I.S_semis+coalesce(R.décalage,0),52)) S_MEP,
--     min(coalesce(I.S_récolte+coalesce(I.D_récolte/2,0)+coalesce(R.décalage,0),52)) S_récolte
-- FROM Rotations_détails R
-- JOIN Rotations Rot USING(Rotation)
-- JOIN Rotations_détails R2 USING(Rotation,Année)
-- LEFT JOIN ITP_sem_corrigées I USING(IT_plante)
-- LEFT JOIN Espèces E USING(Espèce)
-- WHERE (Rot.Active NOTNULL)AND(R.Fi_planches ISNULL)AND(R2.Fi_planches NOTNULL)
-- GROUP BY IdxRotAnFp,Espèce
-- UNION
-- SELECT -- Espèce avec Fi_planches, les reporter sur toutes les planches si d'autre espèces n'ont pas de Fi_planches.
--     DISTINCT R.Rotation||'-'||R.Année||'-' IdxRotAnFp,
--     I.Espèce,
--     max(coalesce(I.S_plantation+coalesce(R.décalage,0),I.S_semis+coalesce(R.décalage,0),52)) S_MEP,
--     min(coalesce(I.S_récolte+coalesce(I.D_récolte/2,0)+coalesce(R.décalage,0),52)) S_récolte
-- FROM Rotations_détails R
-- JOIN Rotations Rot USING(Rotation)
-- JOIN Rotations_détails R2 USING(Rotation,Année)
-- LEFT JOIN ITP_sem_corrigées I USING(IT_plante)
-- LEFT JOIN Espèces E USING(Espèce)
-- WHERE (Rot.Active NOTNULL)AND(R.Fi_planches NOTNULL)AND(R2.Fi_planches ISNULL)
-- GROUP BY IdxRotAnFp,Espèce;

CREATE VIEW RD_famille AS SELECT
    RD.ID,
    RD.Rotation,
    RD.Année,
    RD.Fi_planches,
    R.Nb_années,
    F.Famille,
    F.Intervalle
FROM Rotations_détails RD
JOIN Rotations R USING(Rotation)
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Espèces E USING(Espèce)
JOIN Familles F USING(Famille)
WHERE R.Active NOTNULL;

CREATE VIEW RD_ITP AS SELECT
    R.Rotation||CAST(2001+R.Année AS TEXT)||'-'|| -- Rotation, année.
        substr('0'||CAST((coalesce(I.S_plantation,I.S_semis,52)+coalesce(R.Décalage,0)) AS TEXT),-2)||'-'|| -- Semaine de mise en place.
        coalesce(R.Fi_planches,'')||'-'||
        CAST(2001+R.Année+iif(coalesce(I.S_plantation,I.S_semis)>I.S_récolte,1,0)AS TEXT)||'-'|| -- Année de récolte.
        substr('0'||CAST(coalesce(I.S_récolte,52)AS TEXT),-2)||'-'|| -- Semaine de récolte.
        coalesce(I.Espèce,'') IdxRDITP,
    R.ID,
    R.Rotation,
    RO.Nb_années,
    R.Année,
    R.IT_plante,
    R.Pc_planches,
    R.occupation,
    R.Fi_planches,
    coalesce(R.Décalage,0) Décalage,
    I.Type_planche,
    I.Type_culture,
    coalesce(I.S_plantation,I.S_semis)+coalesce(R.Décalage,0) S_MEP,
    coalesce(I.S_récolte+iif(coalesce(I.S_plantation,I.S_semis)>I.S_récolte,52,0),52)+coalesce(I.D_récolte+coalesce(R.Décalage,0),0) S_Ferm, -- Si pas de durée de récolte (EV), ne pas prendre en compte le décalage, l'EV peut être viré quand on veut.
    coalesce(I.S_plantation,I.S_semis)+coalesce(R.Décalage,0)+(R.Année-1)*52 S_MEP_abs,
    coalesce(I.S_récolte+iif(coalesce(I.S_plantation,I.S_semis)>I.S_récolte,52,0),52)+coalesce(I.D_récolte+coalesce(R.Décalage,0),0)
                                                                                     +(R.Année-1)*52 S_Ferm_abs,
    I.S_semis,
    I.S_plantation,
    I.S_récolte,
    I.D_récolte,
    I.Décal_max,

    coalesce((I.S_semis+coalesce(R.Décalage,0))*7-6,0) J_semis,
    iif(I.S_semis NOTNULL,iif(R.Décalage NOTNULL,1,coalesce(I.Décal_max,0.5))*7,0) NJ_semis,
    coalesce((I.S_plantation+coalesce(R.Décalage,0))*7-6+iif(I.S_plantation<I.S_semis,365,0),0) J_plantation,
    iif(I.S_plantation NOTNULL,iif(R.Décalage NOTNULL,1,coalesce(I.Décal_max,0.5))*7,0) NJ_plantation,
    coalesce((I.S_récolte+coalesce(R.Décalage,0))*7-6+
                             iif(I.S_semis NOTNULL AND I.S_plantation NOTNULL AND (I.S_plantation<I.S_semis),365,0)+
                             iif(I.S_semis NOTNULL AND I.S_plantation NOTNULL AND (I.S_récolte<I.S_plantation),365,0)+
                             iif(I.S_semis NOTNULL AND I.S_plantation ISNULL AND (I.S_récolte<I.S_semis),365,0)+
                             iif(I.S_semis ISNULL AND I.S_plantation NOTNULL AND (I.S_récolte<I.S_plantation),365,0),
            iif(I.S_semis NOTNULL OR I.S_plantation NOTNULL,365,0)) J_récolte,
    iif(I.S_récolte NOTNULL,coalesce(I.D_récolte+iif(R.Décalage NOTNULL,0,coalesce(I.Décal_max,0.5)),0)*7,0) NJ_récolte, -- Si pas de récolte (D_récolte NULL) ne pas ajouter le décalage.

    I.Espacement,
    I.Esp_rangs,
    E.Espèce,
    E.Catégories,
    E.A_planifier,
    F.Famille,
    F.Intervalle
FROM Rotations_détails R
JOIN Rotations RO USING(Rotation)
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Familles F USING(Famille)
WHERE RO.Active NOTNULL
ORDER BY IdxRDITP;

CREATE VIEW RD_ITP_2 AS SELECT -- Pour trouver la culture antérieure (si la courante n'est pas la 1ère) ou la dernière culture de la rotation.
    ID,
    Rotation,
    S_MEP_abs,
    S_Ferm_abs,
    Pc_planches,
    Fi_planches
FROM RD_ITP
ORDER BY IdxRDITP DESC;

CREATE VIEW Rotations_détails__Tempo AS SELECT --- Les ITP (donc les espèces cultivées) vont être déplacés chaque année sur une nouvelle unité de production (UdP).
                                               --- Si Pc_planches=100%, les cultures occuperont la totalité des planches de l'UdP.
                                               --- Si Fi_planches est vide, les cultures occupperont toutes les planches de l'UdP.
                                               --- Si Pc_planches=50% et Fi_planches=A, les cultures occuperont la moitié de chaque planche se terminant par A.
    ---no_data_text Vous devez d'abort saisir au moins une entête de rotation (menu 'Rotations') et un itinéraire technique de plante annuelle.
    ---row_summary Rotation,Année|Année :: - ,IT_plante,Fi_planches|Planches ::
    R.ID, --- N° interne et unique.
        ---hidden
    RI.IdxRDITP, --- Colonne de tri par défaut
        ---hidden
    R.Rotation, ---
    R.Année, ---
    R.IT_plante, ---
    -- RI.Nb_rangs,
    RI.Espacement, --- ::Cultures.Espacement
        ---unit cm
    RI.Esp_rangs, ---
        ---unit cm
    R.Pc_planches, ---
    R.Occupation, ---
    R.Fi_planches, ---
    R.Décalage, ---

    -- Conflit temporel, à la fin de S_MEP.
    CAST(RI.S_MEP ||
    (CASE
     -- Vérif que la culture n'est pas mise en place trop tôt par rapport à la culture antérieure dans la rotation.
     WHEN (RI.S_MEP_abs <  -- Mise en place au plus tôt de la culture courante
                          coalesce((SELECT R2.S_Ferm_abs FROM RD_ITP_2 R2
                                    WHERE -- comparaison avec la fermeture au plus tôt de la culture antérieure (si la courante n'est pas la 1ère de la rotation)
                                          (R2.Rotation=R.Rotation)AND(R2.S_MEP_abs<=RI.S_MEP_abs)AND(R2.ID!=RI.ID)AND
                                          -- et filtres de planche identiques ou nuls.
                                          ((RI.Fi_planches ISNULL) OR (R2.Fi_planches ISNULL) OR (RI.Fi_planches=R2.Fi_planches))AND
                                          -- total d'occupation des planches en conflit supérieur à 100.
                                          (RI.Pc_planches+R2.Pc_planches>100)),
                                   (SELECT R3.S_Ferm_abs-52*RI.Nb_années FROM RD_ITP_2 R3
                                    WHERE (R3.Rotation=R.Rotation)AND
                                          ((RI.Fi_planches ISNULL) OR (R3.Fi_planches ISNULL) OR (RI.Fi_planches=R3.Fi_planches))AND
                                          (RI.Pc_planches+R3.Pc_planches>100)),
                                   0))
     THEN '*'
     -- Vérif que la culture n'est pas mise en place trop tard par rapport à la culture antérieure dans la rotation.
     WHEN RI.S_MEP_abs-(SELECT Nb_sem FROM Prm_NbMaxSemPlancheLibreSMeP
                        WHERE Semaine=RI.S_MEP) > -- Mise en place au plus tôt de la culture courante
                                                  coalesce(-- comparaison avec la fermeture au plus tôt de la culture antérieure (si la courante n'est pas la 1ère de la rotation)
                                                           (SELECT R2.S_Ferm_abs FROM RD_ITP_2 R2
                                                            WHERE (R2.Rotation=R.Rotation)AND(R2.S_MEP_abs<=RI.S_MEP_abs)AND(R2.ID!=RI.ID)),
                                                           -- comparaison avec la fermeture au plus tôt de la dernière culture de la rotation
                                                           (SELECT R3.S_Ferm_abs-52*RI.Nb_années FROM RD_ITP_2 R3
                                                            WHERE R3.Rotation=R.Rotation))
     THEN '-'
     ELSE ''
     END) AS TEXT) S_MEP, --- Semaine de mise en place de la culture sur la planche.
                          --- * indique un chevauchement avec la culture précédente (pas encore récoltée).
                          --- - indique que la culture précédente est récoltée depuis 4 mois (automne) à 8 mois (printemps).

    CAST(RI.S_Ferm-iif(RI.S_Ferm>52,52,0) AS INT) S_Ferm, ---
    RI.Catégories, ---

    (SELECT value FROM Prm_draw WHERE name='GraphBarresVert24Mois')|| -- Barres verticales pour les mois
    replace(replace(replace('rect(xRec,0,wRec,5,cRec,255)', -- Semis
        'xRec',RI.J_semis-iif(RI.S_semis>RI.S_plantation,365,0)),
        'wRec',RI.NJ_semis),
        'cRec',iif(RI.S_plantation NOTNULL,'#ff6000','#76c801'))||
    replace(replace(replace('rect(xRec,0,wRec,5,cRec,100)', -- Attente plantation.
        'xRec',RI.J_semis+RI.NJ_semis-iif(RI.S_semis>RI.S_plantation,365,0)),
        'wRec',iif(RI.S_semis NOTNULL AND RI.S_plantation NOTNULL,RI.J_plantation-RI.J_semis-RI.NJ_semis,0)),
        'cRec',iif(RI.S_plantation NOTNULL,'#ff6000','#76c801'))||
    replace(replace('rect(xRec,3,wRec,10,#76c801,255)', -- Plantation
        'xRec',RI.J_plantation-iif(RI.S_semis>RI.S_plantation,365,0)),
        'wRec',RI.NJ_plantation)||
    replace(replace(replace('rect(xRec,3,wRec,10,#76c801,150,gRec)', -- Attente récolte.
        'xRec',iif(RI.S_plantation NOTNULL,RI.J_plantation+RI.NJ_plantation-iif(RI.S_semis>RI.S_plantation,365,0),RI.J_semis+RI.NJ_semis)),
        'wRec',iif(RI.S_plantation NOTNULL,RI.J_récolte-RI.J_plantation-RI.NJ_plantation,RI.J_récolte-RI.J_semis-RI.NJ_semis)),
        'gRec',iif(RI.S_récolte ISNULL AND RI.D_récolte ISNULL,'h+',''))|| -- Pas de récolte ni de fin de culture
    replace(replace('rect(xRec,8,wRec,6,#bf00ff,255)', -- Récolte
        'xRec',RI.J_récolte-iif(RI.S_semis>RI.S_plantation,365,0)),
        'wRec',RI.NJ_récolte)
    Graph, --- Graphique calculé à partir des semaines de semis, plantation et récolte, ainsi que du décalage.
        ---draw GraphTitres24Mois

    -- CAST(
    -- coalesce((RI.S_semis+coalesce(R.Décalage,0)-iif(RI.S_semis>RI.S_plantation,52,0)) -- Semis année N-1.
    --          *7-6,0)||':'|| -- Conversion semaines -> jours puis se caler au lundi.
    -- coalesce((RI.S_semis+iif(R.Décalage ISNULL,coalesce(RI.Décal_max,0)+1, -- Montrer la période de semis.
    --                                                     R.Décalage+1) -- La semaine de semis est fixée, afficher un point (1 semaine).
    --                     -iif(RI.S_semis>RI.S_plantation,52,0)) -- Semis année N-1.
    --          *7-6,0)||':'|| -- Conversion semaines -> jours puis se caler au lundi.
    -- coalesce((RI.S_plantation+coalesce(R.Décalage,0))*7-6,0)||':'||
    -- coalesce((RI.S_plantation+iif(R.Décalage ISNULL,coalesce(RI.Décal_max,0)+1, -- Montrer la période de plantation.
    --                               R.Décalage+1))
    --          *7-6,0)||':'|| -- Conversion semaines -> jours puis se caler au lundi.
    -- coalesce((RI.S_récolte+coalesce(R.Décalage,0))*7-6,0)||':'||
    -- coalesce((RI.S_récolte+coalesce(RI.D_récolte -- Si pas de récolte (D_récolte NULL) ne pas ajouter le décalage (pas de coalesce).
    --                                 +iif(R.Décalage ISNULL,coalesce(RI.Décal_max,0), -- Montrer la période de récolte plus décalage.
    --                                      R.Décalage+0),0)) -- La semaine de début de récolte est fixée, afficher la période de récolte.
    --          *7-6,0)
    -- AS TEXT) TEMPO, ---

    RI.A_planifier, ---

    CAST(CASE WHEN (RI.Type_planche NOTNULL) AND (RI.Type_planche<>(SELECT Type_planche FROM Rotations WHERE Rotation=R.Rotation) )
              THEN RI.Type_planche END AS TEXT) Conflit_type_planche, ---

    -- Conflit familles.
    CAST(
    CASE WHEN R.IT_plante NOTNULL -- Recherche de familles trop proches dans une rotation.
         THEN (SELECT group_concat(result,x'0a0a')
               FROM (SELECT DISTINCT result
                     FROM (SELECT RF.Famille || ' année ' || format('%i',RF.Année) result FROM RD_famille RF
                           WHERE (RF.Rotation=R.Rotation)AND --ITP de la rotation
                                 (RF.Année<>R.Année)AND -- que les ITP des autres années de la rotation
                                 (RF.Famille=RI.Famille)AND -- que les ITP de la même famille
                                 ((RF.Fi_planches ISNULL)OR(R.Fi_planches ISNULL)OR(RF.Fi_planches=R.Fi_planches))AND
                                 ((R.Année BETWEEN RF.Année-RF.Intervalle+1 AND RF.Année+RF.Intervalle-1)OR -- en conflit lors du 1er cycle
                                  (R.Année BETWEEN RF.Année-RF.Intervalle+1+RF.Nb_années AND RF.Année+RF.Intervalle-1+RF.Nb_années))))) || -- en conflit lors du 2ème cycle
              x'0a0a'
         END ||
    'Familles possibles : '||coalesce((SELECT group_concat(Famille,', ')
                                       FROM (SELECT DISTINCT RF.Famille
                                             FROM RD_famille RF
                                             WHERE (RF.Rotation=R.Rotation)AND -- ITP dans la même rotation
                                                   (RF.Année<>R.Année)AND -- ITP des autres années de la rotation
                                                   NOT(R.Année BETWEEN RF.Année-RF.Intervalle+1 AND RF.Année+RF.Intervalle-1)AND -- pas en conflit lors du 1er cycle
                                                   NOT(R.Année BETWEEN RF.Année-RF.Intervalle+1+RF.Nb_années AND RF.Année+RF.Intervalle-1+RF.Nb_années)
                                             UNION
                                             SELECT F.Famille FROM Fam_actives F
                                             WHERE NOT(F.Famille IN(SELECT Famille FROM RD_famille F2 WHERE F2.Rotation=R.Rotation)))),
                                      'aucune')
    AS TEXT) Conflit_famille, --- Retour trop rapide d'une famille de plante sur la planche.
        ---multiline

    -- Associations: désactivé car produit un bug, notamment la ligne SELECT group_concat(RDAP2.Association,x'0a0a').
    -- Bug: Certaines lignes sont non modifiables ni supprimables, le triggers ne trouve pas la ligne àmodifier (WHERE)
    -- CAST(
    -- CASE WHEN R.IT_plante NOTNULL -- AND R.Pc_planches<100 AND R.Occupation IN('R','E')
    --      -- Recheche des associations dans la rotations
    --      THEN (CASE WHEN (SELECT count() FROM RD_asso_présentes RDAP1
    --                       WHERE (R.Rotation||'-'||R.Année||'-'||coalesce(R.Fi_planches,'')=RDAP1.IdxRotAnFp)AND -- Associations trouvées sur ces planches.
    --                             -- (RDAP.Association IN(SELECT RDAPC.Association FROM RD_asso_possibles_cult RDAPC
    --                             --                      WHERE RDAPC.ID=R.ID))AND -- l'espèce fait partie de l'association.
    --                             (RDAP1.Espèces_présentes LIKE '%'||RI.Espèce||'%'))>0 --
    --                 THEN (SELECT group_concat(RDAP2.Association,x'0a0a')
    --                       FROM RD_asso_présentes RDAP2
    --                       WHERE (R.Rotation||'-'||R.Année||'-'||coalesce(R.Fi_planches,'')=RDAP2.IdxRotAnFp)AND
    --                             -- (RDAP.Association IN(SELECT RDAPC.Association FROM RD_asso_possibles_cult RDAPC
    --                             --                      WHERE RDAPC.ID=R.ID))AND
    --                             (RDAP2.Espèces_présentes LIKE '%'||RI.Espèce||'%'))
    --                 WHEN ((SELECT Valeur FROM Params WHERE Paramètre='Rot_Rech_ass_poss')='Oui')OR((R.Pc_planches<100)AND(R.Occupation IN('R','E')))
    --                 THEN 'Ass. possibles : '|| -- Proposer des associations.
    --                      (SELECT group_concat(Espèce_ou_famille,', ')
    --                       FROM (SELECT DISTINCT ADE.Espèce_ou_famille
    --                             FROM RD_asso_possibles_cult RDAPC
    --                             LEFT JOIN Associations_détails_aap ADE USING(Association)
    --                             WHERE (RDAPC.Association LIKE '%'||(SELECT Valeur FROM Params WHERE Paramètre='Asso_bénéfique'))AND
    --                                   (RDAPC.ID=R.ID)AND
    --                                   ((RDAPC.Requise NOTNULL)OR -- Espèce de la RD requise dans l'association
    --                                    (RDAPC.Association LIKE '%'||ADE.Espèce_ou_famille||'%'))AND -- Espèce de la RD incluse dans le nom de l'association
    --                                   NOT(RI.Espèce LIKE ADE.Espèce_ou_famille||'%')
    --                             ORDER BY Espèce_ou_famille))
    --                 END)
    --      END
    -- AS TEXT) Associations, ---
    --     ---multiline

    RI.Famille, ---
    RI.Intervalle, ---
    R.Notes ---
FROM Rotations_détails R
JOIN RD_ITP RI USING(ID)
ORDER BY RI.IdxRDITP ---
;

-- CREATE VIEW Test AS SELECT ---
--     ID, ---
--     Description, ---draw text(jan,9,8) line(31,0,31,22) color(#ff7171,255) text(fév,40,8)
--     Texte ---
-- FROM Notes;

CREATE VIEW Rotations_détails__Tempo_occup AS SELECT R.Rotation, --- Occupation de planche obtenu en superposant toutes les cultures.
        R.Année, ---
        R.Fi_planches, ---
        min(CAST(R.S_MEP AS INT)) S_MEP_min, --- Semaine de mise en place de la 1ère culture de l'année sur ces planches.
        max(R.S_Ferm+iif(R.S_Ferm<CAST(R.S_MEP AS INT),52,0))-iif(max(R.S_Ferm+iif(R.S_Ferm<CAST(R.S_MEP AS INT),52,0))>52,52,0) S_Ferm_max, --- Semaine de fin de récolte de la dernière culture de l'année sur ces planches.

        (SELECT value FROM Prm_draw WHERE name='GraphBarresVert24Mois')|| -- Barres verticales pour les mois
        replace(replace('rect(xRec,3,wRec,10,#76c801,150)',
        'xRec',coalesce(min(CAST(R.S_MEP AS INT))*7-6,0)),
        'wRec',coalesce(max(R.S_Ferm+iif(R.S_Ferm<CAST(R.S_MEP AS INT),52,0))*7-6,0)
              -coalesce(min(CAST(R.S_MEP AS INT))*7-6,0))
            Graph ---draw GraphTitres24Mois
        -- CAST('::'||coalesce(min(CAST(R.S_MEP AS INT))*7-6,0)||':'||
        --     coalesce(min(CAST(R.S_MEP AS INT))*7-6,0)||':'||
        --     coalesce(max(R.S_Ferm+iif(R.S_Ferm<CAST(R.S_MEP AS INT),52,0))*7-6,0)||'::' AS TEXT) TEMPO ---
FROM Rotations_détails__Tempo R
JOIN RD_ITP RI USING(ID)
GROUP BY R.Rotation,R.Année,R.Fi_planches
ORDER BY R.Rotation,R.Année,R.Fi_planches ---
;

CREATE VIEW Récoltes__Saisies AS SELECT --- Quantités de légume récoltées par culture.
    ---goto_last
    ---no_data_text Il n'y a aucune culture à récolter pour le moment.
    ---no_data_text
    ---no_data_text Les récoltes peuvent être saisies avant ou après la période de récolte en modifiant les paramètres 'C_récolte_avance' et 'C_récolte_prolongation'.
    ---row_summary Date,Espèce,Culture,Quantité
    R.ID, --- Nécessaire pour mise à jour de la table via trigger INSTEAD OF UPDATE.
        ---hidden
    R.Date, ---
    R.Espèce, ---
    C.Planche Planche·s, --- Pour répartir la quantité récoltée sur plusieurs cultures de l'espèce sélectionnée, saisir le début du nom des planches concernées ou saisir '*' pour répartir sur toutes les cultures possibles.
                         --- Vide: pas de répartition.
                         ---
                         --- La répartition se fait au prorata des longueurs de planche.
                         --- Attention, la liste des cultures possibles dépend des paramètres 'C_récolte_avance' et 'C_récolte_prolongation'.
    R.Culture, ---
    R.Quantité, ---
    R.Réc_ter, ---
    C.Variété, ---
    CAST(round((SELECT sum(Quantité) FROM Récoltes R2 WHERE (R2.Culture=R.Culture)AND(R2.Date<=R.Date)),3)AS REAL) Total_réc, --- Quantité totale déja récoltée, à cette date pour cette culture.
        ---unit kg
    R.Notes ---
FROM Récoltes R
JOIN Cultures C USING(Culture)
WHERE -- (R.Date>DATE('now','-'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_historique_récolte')||' days'))OR
      (R.Date>=DATE(coalesce((SELECT Valeur FROM Params WHERE Paramètre='C_voir_récolte'),'0000')||'-01-01'))OR
      (DATE(R.Date) ISNULL)
ORDER BY R.Date,C.Espèce,C.Planche,R.Culture ---
;

CREATE VIEW Récoltes_saison AS SELECT --- Récoltes avec calcul de la saison, en fonction de la période normale de récolte.
    CASE WHEN coalesce(C.terminée,'') NOT LIKE 'v%' -- Annuelle
         THEN C.Saison
         ELSE CAST(substr(R.Date,1,4)+ -- Vivace, saison = année de récolte,
              iif(CAST(strftime('%U',R.Date) AS INT)<coalesce(V.S_Récolte,I.S_Récolte,CAST(strftime('%U',C.Début_récolte) AS INT),0)
                                                                   -(52-coalesce(V.D_récolte,I.D_récolte,(C.Fin_récolte-C.Début_récolte)/7,0))/2,
                  -1, -- récolte année N+1 alors que début de récolte de la culture année N, la saison de récolte est N.
                  0) AS TEXT)
         END Saison, --- Saison à laquelle est affectée la récolte.
    -- CAST(strftime('%U',R.Date) AS INT) Sem_rec,
    R.*
FROM Récoltes R
JOIN Cultures C USING(Culture)
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Variétés V USING(Variété);

CREATE VIEW Variétés__cde_plants AS SELECT --- Plants achetés nécessaires pour chaque variété de plante.
                                           --- Cultures prises pris en compte : 'Plant'.
    ---row_summary Variété|::,Famille| (::) - ,Qté_nécess|Qté nécessaire: ::
    E.Famille, ---
    V.Espèce, ---
    V.Variété, ---
    CAST(round((SELECT sum(CASE WHEN C.Espacement>0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100 ELSE NULL END)
                FROM Cu_non_commencées C
                WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL)),2)AS REAL) Qté_nécess, --- Nb de plants nécessaires pour les cultures non encore plantées. --UNIT(plants)
    (SELECT min(C.Date_plantation)
     FROM Cu_non_commencées C
     WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL)) pour_le, --- Plus petite date de plantation.
    V.Fournisseur, ---
    CASt((SELECT count() FROM Cu_non_commencées C WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL))AS INTEGER) Cu_non_commencées, --- Cultures prévues mais ni semées (en place ou en pépinière) ni plantées. --UNIT(cultures non commencées)
    CAST((SELECT sum(C.Longueur) FROM Cu_non_commencées C WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL))AS REAL) Long_planches, ---
        ---unit m
    CAST((SELECT sum(C.Longueur*PL.Largeur) FROM Cu_non_commencées C LEFT JOIN Planches PL USING(Planche)
          WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL))AS REAL) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
    V.Notes, ---
    E.Notes N_espèce, ---
        ---multiline
    F.Notes N_famille ---
        ---multiline
FROM Variétés V
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Familles F USING(Famille)
UNION
SELECT
    E.Famille,
    E.Espèce,
    '..', -- .. rend la valeur invisible et la couleur de cellule grise, pour ne pas avoir le jaune de la table Variétés.
    CAST(round((SELECT sum(CASE WHEN C.Espacement>0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100 ELSE NULL END)
                FROM Cu_non_commencées C
                WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis ISNULL)),2)AS REAL)||'#002bff' Qté_nécess,
    (SELECT min(C.Date_plantation)
     FROM Cu_non_commencées C
     WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis ISNULL)) pour_le,
    '..',
    CASt((SELECT count() FROM Cu_non_commencées C WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis ISNULL))AS INTEGER)||'#002bff' Cu_non_commencées,
    CAST((SELECT sum(C.Longueur) FROM Cu_non_commencées C WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis ISNULL))AS REAL)||'#002bff' Long_planches,
    CAST((SELECT sum(C.Longueur*PL.Largeur) FROM Cu_non_commencées C LEFT JOIN Planches PL USING(Planche)
          WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis ISNULL))AS REAL)||'#002bff' Surface,
    '..',
    E.Notes N_espèce,
    F.Notes N_famille
FROM Espèces E
LEFT JOIN Familles F USING(Famille)
WHERE (SELECT count() FROM Cu_non_commencées C WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_Semis ISNULL))>0
ORDER BY Famille,Espèce,Variété ---
;

CREATE VIEW Variétés__inv_et_cde AS SELECT --- Inventaire des semences pour chaque variété de plante.
                                           --- Cultures prises en compte : 'Semis pépinière', 'Semis en place', 'Compagne' et 'Engrais vert'.
    ---row_summary Variété|::,Famille| (::) - ,Qté_nécess|Qté nécessaire: ::
    E.Famille, ---
    V.Espèce, ---
    V.Variété, ---
    CAST(round((SELECT sum(CASE WHEN C.Espacement>0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant/V.Nb_graines_g
                                ELSE C.Longueur*PL.Largeur*I.Dose_semis END)
                FROM Cu_non_commencées C
                LEFT JOIN ITP I USING(IT_plante)
                LEFT JOIN Planches PL USING(Planche)
                WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL)),2)AS REAL) Qté_nécess, --- Semence nécessaire pour les cultures non encore semées.
        ---unit g
    (SELECT min(C.Date_semis)
     FROM Cu_non_commencées C
     WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL)) pour_le, --- Plus petite date de semis.
    V.Qté_stock, ---
    V.Qté_cde, ---
    CAST(round(max((SELECT sum(CASE WHEN C.Espacement>0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant/V.Nb_graines_g
                                    ELSE C.Longueur*PL.Largeur*I.Dose_semis
                                    END) FROM Cu_non_commencées C
                                         LEFT JOIN ITP I USING(IT_plante)
                                         LEFT JOIN Planches PL USING(Planche)
                    WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))
                   -coalesce(V.Qté_stock,0)-coalesce(V.Qté_cde,0),0),2)AS REAL) Qté_manquante, --- Qté nécessaire moins Qté en stock moins Qté commandée.
        ---unit g
    V.Fournisseur, ---
    V.Nb_graines_g, ---
        ---unit graines/g
    E.FG, ---
    CASt((SELECT count() FROM Cu_non_commencées C WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS INTEGER) Cu_non_commencées, --- Cultures prévues mais ni semées (en place ou en pépinière) ni plantées. --UNIT(cultures non commencées)
    CAST((SELECT sum(C.Longueur) FROM Cu_non_commencées C WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS REAL) Long_planches, ---
        ---unit m
    CAST((SELECT sum(C.Longueur*PL.Largeur) FROM Cu_non_commencées C LEFT JOIN Planches PL USING(Planche)
          WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS REAL) Surface, --- ::Cultures__Succ_planches.Surface --UNIT(m²)
    CAST((SELECT round(sum(C.Longueur*C.Nb_rangs/C.Espacement*100) ) FROM Cu_non_commencées C
          WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS INTEGER) Nb_plants, --- --UNIT(plants)
    V.Notes, ---
    E.Notes N_espèce, ---
        ---multiline
    F.Notes N_famille ---
        ---multiline
FROM Variétés V
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Familles F USING(Famille)
UNION
SELECT
    E.Famille,
    E.Espèce,
    '..' Variété, -- .. rend la valeur invisible et la couleur de cellule grise, pour ne pas avoir le jaune de la table Variétés.
    round((SELECT sum(CASE WHEN C.Espacement>0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant/E.Nb_graines_g
                                ELSE C.Longueur*PL.Largeur*I.Dose_semis END)
                FROM Cu_non_commencées C
                LEFT JOIN ITP I USING(IT_plante)
                LEFT JOIN Planches PL USING(Planche)
                WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis NOTNULL)),2)||'#002bff' Qté_nécess,
    (SELECT min(C.Date_semis)
     FROM Cu_non_commencées C
     WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis NOTNULL)) pour_le,
    '..' Qté_stock,
    '..' Qté_cde,
    '..' Qté_manquante,
    '..' Fournisseur,
    coalesce(E.Nb_graines_g,'')||'#002bff' Nb_graines_g,
    coalesce(E.FG,'')||'#002bff',
    CAST((SELECT count() FROM Cu_non_commencées C WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis NOTNULL))AS INTEGER)||'#002bff' Cu_non_commencées,
    CAST((SELECT sum(C.Longueur) FROM Cu_non_commencées C WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis NOTNULL))AS REAL)||'#002bff' Long_planches,
    CAST((SELECT sum(C.Longueur*PL.Largeur) FROM Cu_non_commencées C LEFT JOIN Planches PL USING(Planche)
          WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis NOTNULL))AS REAL)||'#002bff' Surface,
    CAST((SELECT round(sum(C.Longueur*C.Nb_rangs/C.Espacement*100) ) FROM Cu_non_commencées C
          WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis NOTNULL))AS INTEGER)||'#002bff' Nb_plants,
    '..' Notes,
    E.Notes N_espèce,
    F.Notes N_famille
FROM Espèces E
LEFT JOIN Familles F USING(Famille)
WHERE (SELECT count() FROM Cu_non_commencées C WHERE (C.Espèce=E.Espèce)AND(C.Variété ISNULL)AND(C.Date_semis NOTNULL))>0
ORDER BY Famille,Espèce,Variété ---
;

--------------
-- Couleurs --
--------------

-- Base
UPDATE fada_t_schema SET color='#a17dc2'
WHERE (name LIKE 'Associations%')OR(name LIKE 'Destinations%')OR(name LIKE 'Fournisseurs%');

UPDATE fada_f_schema SET color='#a17dc2'
WHERE (field_name LIKE 'Association%');

-- Espèces
UPDATE fada_t_schema SET color='#002bff'
WHERE (name LIKE 'Espèces%')OR(name LIKE 'Consommations%');

UPDATE fada_f_schema SET color='#002bff'
WHERE (field_name LIKE 'Espèce%')OR(field_name LIKE '%_esp')OR
      (field_name IN('A_planifier','Catégories','Besoins','FG','Groupe',
                     'Irrig_espèce','Niveau','N_espèce','Obj_annuel','S_taille','Usages','Rendement'));

-- Cultures
UPDATE fada_t_schema SET color='#00ff00'
WHERE (name LIKE 'Cultures%')OR(name LIKE 'Récoltes%');

UPDATE fada_f_schema SET color='#00ff00'
WHERE (field_name LIKE 'Culture%')OR(field_name LIKE 'Nb_cu%')OR
      (field_name IN('Min_semis','Max_semis"','Min_plantation','Max_plantation','Min_recolte','Max_recolte','Qté_réc_moy'))OR
      ((field_name LIKE '%_MEP')AND(field_name!='S_MEP'));

-- Familles
UPDATE fada_t_schema SET color='#0085c4'
WHERE (name LIKE 'Familles%');

UPDATE fada_f_schema SET color='#0085c4'
WHERE (field_name LIKE 'Famille%')OR(field_name IN('N_famille,Intervalle'));

-- Fertilisations
UPDATE fada_t_schema SET color='#00A67A'
WHERE (name LIKE 'Fertilisa%')OR(name LIKE 'Analyses_de_sol%');

UPDATE fada_f_schema SET color='#00A67A'
WHERE (field_name LIKE 'Fert%')OR(field_name LIKE '%_sol')OR(field_name LIKE '%_fert')OR
      (field_name IN('Analyse','Fertilisant','Apports_NPK','N_manq','P_manq"','K_manq'));

-- ITP
UPDATE fada_t_schema SET color='#ff0000'
WHERE (name LIKE 'ITP%');

UPDATE fada_f_schema SET color='#ff0000'
WHERE (field_name LIKE 'ITP%')OR
      (field_name IN('IT_plante','S_semis','S_plantation','Décal_max','Esp_rangs','Nb_graines_plant','N_IT_plante'))OR
      ((field_name IN('Type_planche','Type_culture'))AND(name NOT LIKE 'Rotations%'))OR
      ((field_name IN('S_récolte','D_récolte'))AND(name NOT LIKE 'Variétés%'))OR
      ((field_name IN('Espacement'))AND(name NOT LIKE 'Cultures%'))OR
      ((field_name IN('Dose_semis'))AND(name NOT LIKE 'Espèces%'));

-- Assolement
UPDATE fada_t_schema SET color='#ff8100'
WHERE (name LIKE 'Planches%')OR(name LIKE 'Assolement%');

UPDATE fada_f_schema SET color='#ff8100'
WHERE (field_name LIKE 'Planche%')OR
      (field_name IN('Ilot','Largeur','N_Planche'))OR
      ((field_name IN('Longueur','Nb_planches','Surface','Unités_prod'))AND(name LIKE '%Bilans%'));

-- Rotations
UPDATE fada_t_schema SET color='#ce9462'
WHERE (name LIKE 'Rotations%');

-- Variétés
UPDATE fada_t_schema SET color='#b7b202'
WHERE (name LIKE 'Variétés%');

UPDATE fada_f_schema SET color='#b7b202'
WHERE (field_name LIKE 'Variété%')OR
      (field_name IN('Qté_stock'))OR
      ((field_name IN('S_récolte','D_récolte'))AND(name LIKE 'Variétés%'));

-- Params
UPDATE fada_f_schema SET color='#7f7f7f'
WHERE (field_name IN('Saison_à_planifier'));

---------------
-- col width --
---------------
UPDATE fada_f_schema SET col_width=150 WHERE (field_name='Notes')OR(field_name LIKE 'N\_%' ESCAPE '\');
UPDATE fada_f_schema SET col_width=40  WHERE (field_name LIKE 'Pc\_%' ESCAPE '\');
UPDATE fada_f_schema SET col_width=70  WHERE (field_name LIKE 'S\_%' ESCAPE '\');
UPDATE fada_f_schema SET col_width=365*1.5 WHERE (field_name='Graph');
--UPDATE fada_f_schema SET col_width=400 WHERE (field_name='TEMPO');
--UPDATE fada_f_schema SET col_width=367 WHERE (field_name LIKE 'TEMPO\_%' ESCAPE '\');
UPDATE fada_f_schema SET col_width=100 WHERE (field_name='Type_culture');
UPDATE fada_f_schema SET col_width=65 WHERE (field_name='Type_planche');

