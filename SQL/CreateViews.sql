CREATE VIEW Associations AS SELECT --- Associations de plantes, regroupées en une ligne par association.
    A.Association, --- Nom de l'association. Les caractères de fin indiquent l'association est bénéfique.
    -- A.Effet,
    count() Nb, --- Nombre de plantes dans l'association.
    count(Requise) Nb_requises, --- Nombre de plantes requise pour que l'assocition ait son effet.
    group_concat( --CASE WHEN (Requise ISNULL)OR((SELECT count(A2.Requise) FROM Associations_détails A2 WHERE A2.Association=A.Association )!=1)
                      -- THEN coalesce(A.Espèce,A.Groupe,A.Famille)
                      -- ELSE NULL -- Ne pas mettre l'unique espèce requise.
                      -- END
                 coalesce(A.Espèce,A.Groupe,A.Famille),', ') Espèces ---
FROM Associations_détails A
GROUP BY A.Association --,A.Effet
ORDER BY A.Association;

CREATE VIEW Associations_détails_aap AS SELECT -- Annuelles, associations possibles, sans éclater les familles en espèces.
    AD.Association,
    -- AD.Effet,
    coalesce(AD.Espèce,AD.Groupe,AD.Famille) Espèce_ou_famille,
    AD.Requise
FROM Associations_détails AD
LEFT JOIN Espèces E USING(Espèce)
WHERE ((AD.Espèce NOTNULL)AND(E.Vivace ISNULL)AND(E.A_planifier NOTNULL))OR
      ((AD.Espèce ISNULL)AND(AD.Famille IN(SELECT E2.Famille  FROM Espèces E2 WHERE (E2.Vivace ISNULL)AND(E2.A_planifier NOTNULL))));

CREATE VIEW Associations_détails__Espèces AS SELECT -- Associations avec familles remplacées par leurs espèces.
    AD1.Association,
    -- AD1.Effet,
    AD1.Espèce,
    AD1.Requise
FROM Associations_détails AD1
WHERE AD1.Espèce NOTNULL -- Espèce spécifiée dans la rotation.
UNION
SELECT
    AD2.Association,
    -- AD2.Effet,
    E.Espèce,
    AD2.Requise
FROM Associations_détails AD2
JOIN Espèces E ON E.Espèce LIKE AD2.Groupe||'%'
WHERE (AD2.Espèce ISNULL)AND(AD2.Groupe NOTNULL) -- Espèces du groupe
UNION
SELECT
    AD3.Association,
    -- AD3.Effet,
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
    A.IdxAsReEsGrFa, ---
    A.Association, ---
    -- A.Effet,
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
                (A.Association LIKE '%'||coalesce(AD.Espèce,AD.Groupe)||'%')) AS TEXT) Doublon ---
FROM Associations_détails A
ORDER BY A.IdxAsReEsGrFa;
UPDATE fda_schema SET tbl_type='View as table' WHERE (name='Associations_détails__Saisies');

CREATE VIEW Associations__Espèces AS SELECT
    A.Association,
    -- A.Effet,
    count() Nb,
    count(A.Requise) Nb_requises,
    CAST(group_concat( -- CASE WHEN (A.Requise ISNULL)OR((SELECT count(A2.Requise) FROM Associations_détails A2 WHERE A2.Association=A.Association)!=1)
                       -- THEN A.Espèce
                       -- ELSE NULL -- Ne pas mettre l'unique espèce requise.
                       -- END
                      A.Espèce,', ') AS TEXT) Espèces
FROM Associations_détails__Espèces A
GROUP BY A.Association --,A.Effet
ORDER BY A.Association;

CREATE VIEW Associations__présentes AS SELECT -- Sélectionner les associations présentes dans les planches.
    P.Planche,
    P.Association,
    P.Cultures,
    P.Nb Nb_cultures,
    P.Nb_distinct Nb_espèces,
    P.Nb_vivaces,
    A.Espèces Espèces_de_l_association
FROM Planche_asso_esp_requises_présentes P
JOIN Associations__Espèces A USING(Association)
WHERE ((A.Nb>1)AND(P.Nb_distinct>1)AND(P.Nb_requises=A.Nb_requises))AND -- Association de plusieurs espèces et toutes les requises sont présentes.
      (julianday(P.Début_récolte)-julianday(P.Date_MEP)>=(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Nb_sem_croisement')*7); -- Croisement mini pour les cultures.

CREATE VIEW Consommations__Saisies AS SELECT ---
    Co.ID, ---
    Co.Date, ---
    Co.Espèce, ---
    Co.Quantité, ---
    Co.Prix, ---
    Co.Destination, ---
    CAST(round(E.Inventaire+coalesce((SELECT sum(Quantité) FROM Récoltes Re
                                      WHERE (Re.Espèce=Co.Espèce)AND(Re.Date BETWEEN E.Date_inv AND Co.Date)),0)
                           -coalesce((SELECT sum(Quantité) FROM Consommations Co2
                                      WHERE (Co2.Espèce=Co.Espèce)AND(Co2.Date BETWEEN E.Date_inv AND Co.Date)),0),3)AS REAL) Stock, ---
    CAST(round((SELECT sum(Quantité) FROM Consommations Co3
                WHERE (Co3.Espèce=Co.Espèce)AND
                      (Co3.Destination=Co.Destination)AND
                      (Co3.Date BETWEEN D.Date_RAZ AND Co.Date)),3)AS REAL) Sorties, ---
    Co.Notes ---
FROM Consommations Co
JOIN Espèces E USING(Espèce)
LEFT JOIN Destinations D USING(Destination)
WHERE (Co.Date>DATE('now','-'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Conso_historique')||' days'))OR
      (DATE(Co.Date) ISNULL) -- Détection de date incorecte
ORDER BY Co.Date,E.Espèce,D.Destination;
UPDATE fda_schema SET tbl_type='View as table' WHERE (name='Consommations__Saisies');

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
    coalesce(iif((length(C.D_planif)=4)AND(CAST(C.D_planif AS INTEGER) BETWEEN 2000 AND 2100),C.D_planif,NULL), -- Année demandée par utilisateur.
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

CREATE VIEW Cu_en_place AS SELECT --Cultures semées (SD) ou plantées.
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
    Culture,
    CAST((julianday(CP.Date_semis)
          -julianday(C.Date_semis)
          -(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_A_semis'))AS INTEGER) Inc_semisD,
    CAST((julianday(C.Date_semis)
          -julianday(CP.Date_semis)
          -(coalesce(I.Décal_max,0)+1)*7-(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_R_semis'))AS INTEGER) Inc_semisF,
    CAST((julianday(CP.Date_plantation)
          -julianday(C.Date_plantation)
          -(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_A_plantation'))AS INTEGER) Inc_plantD,
    CAST((julianday(C.Date_plantation)
          -julianday(CP.Date_plantation)
          -(coalesce(I.Décal_max,0)+1)*7
          -(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_R_plantation'))AS INTEGER) Inc_plantF,
    CAST((julianday(CP.Début_récolte)
          -julianday(C.Début_récolte)
          -(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_A_récolte'))AS INTEGER) Inc_RecD,
    CAST((julianday(C.Fin_récolte)
          -julianday(CP.Fin_récolte)
          -(coalesce(I.Décal_max,0)+1)*7
          -(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Tolérance_R_récolte'))AS INTEGER) Inc_RecF
FROM Cultures C
LEFT JOIN Cu_dates_théo CP USING(Culture)
LEFT JOIN ITP I USING(IT_plante)
WHERE (coalesce(C.Terminée,'') NOT LIKE '%NS');

CREATE VIEW Cu_non_commencées AS SELECT --Cultures ni semées (SEP ou SPep) ni plantées.
    *
FROM Cu_non_ter
WHERE Semis_fait ISNULL AND Plantation_faite ISNULL
ORDER BY Planche;

CREATE VIEW Cu_non_ter AS SELECT
    *
FROM Cultures WHERE NOT((Terminée NOTNULL)AND(Terminée!='v')AND(Terminée!='V'));

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

CREATE VIEW Cu_répartir_fertilisation AS SELECT
    C.Culture,C.Espèce,C.Planche,C.Longueur*P.Largeur Surface,
    DATE(coalesce(C.Date_plantation,C.Date_semis),
         '-'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='Ferti_avance')||' days') Début_fertilisation_possible,
    DATE(C.Début_récolte,
         '+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='Ferti_retard')||' days') Fin_fertilisation_possible
FROM Cultures C
JOIN Planches P USING(Planche);

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

CREATE VIEW Cu_à_fertiliser AS SELECT
    substr(C.Planche,1,(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Ilot_nb_car')) Ilot,
    C.Planche,
    C.Culture,
    E.Espèce,
    coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante,
    C.Type,
    C.Etat,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
    C.Début_récolte,
    C.Fin_récolte,
    (SELECT max(CEP2.Fin_récolte) FROM Cu_en_place CEP2 WHERE (CEP2.Planche=C.Planche)AND --Cultures sur la même planche
                                                              (CEP2.Début_récolte<coalesce(C.Date_plantation,C.Date_semis)) -- dont la récolte commence avant la MEP de la culture courante
                                                              ) Pl_libre_le,
    C.Longueur,
    C.Longueur*PL.Largeur Surface,
    coalesce(CASE WHEN C.Espacement>0 THEN round(1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100)
                  ELSE round(I.Dose_semis/E.Dose_semis*100)
                  END,100) Densité_pc,
    E.N N_esp,
    E.★N ★N_esp,
    E.P P_esp,
    E.★P ★P_esp,
    E.K K_esp,
    E.★K ★K_esp,
    PL.Analyse,
    (SELECT sum(N) FROM Fertilisations F WHERE F.Culture=C.Culture) N_fert,
    (SELECT sum(P) FROM Fertilisations F WHERE F.Culture=C.Culture) P_fert,
    (SELECT sum(K) FROM Fertilisations F WHERE F.Culture=C.Culture) K_fert,
    -- (SELECT CF.Fertilisant FROM Cu_Fertilisants CF WHERE CF.Culture=C.Culture) Fertilisant,
    -- (SELECT CF.Quantité FROM Cu_Fertilisants CF WHERE CF.Culture=C.Culture) Quantité,
    C.A_faire,
    C.Notes,
    PL.Notes N_Planche,
    E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (E.N+E.P+E.K>0)AND
      -- Cultures prévues
      (((C.Culture IN(SELECT CAV.Culture FROM Cu_à_venir CAV))AND
        (coalesce(C.Date_plantation,C.Date_semis) < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_fertiliser')||' days')))OR
      -- Cultures en place dont la récolte n'est pas commencée.
       ((C.Culture IN(SELECT CEP3.Culture FROM Cu_en_place CEP3))AND
        (C.Récolte_faite ISNULL))) -- coalesce(C.Récolte_faite,'') NOT LIKE 'x%'
ORDER BY coalesce(Date_plantation,Date_semis),C.Espèce,C.IT_plante;

CREATE VIEW Cu_à_irriguer AS SELECT
    C.Planche,
    C.Culture
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
WHERE  (E.Irrig NOTNULL)AND
       (coalesce(C.Date_plantation,C.Date_semis) < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_Irrig_avant_MEP')||' days'))AND
       (coalesce(C.Date_plantation,C.Date_semis) > DATE('now','-'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_Irrig_après_MEP')||' days'));

CREATE VIEW Cu_à_venir AS SELECT --Cultures ni semées (SD) ni plantées.
    *
FROM Cu_non_ter
WHERE NOT ((Semis_fait NOTNULL AND Date_plantation ISNULL) OR -- SD semé
           Plantation_faite NOTNULL) -- Plant ou SPep planté
ORDER BY Planche;

CREATE VIEW Cultures__A_faire AS SELECT
    C.Planche,
    C.Culture,
    C.Espèce,
    CAST(coalesce(C.Variété,C.IT_plante) AS TEXT) Variété_ou_It_plante,
    C.Type,
    C.Etat,
    C.A_faire,
    C.Date_semis,
    C.Semis_fait,
    C.Date_plantation,
    C.Plantation_faite,
    C.Début_récolte,
    C.Fin_récolte,
    C.Récolte_faite,
    C.Terminée,
    C.Longueur,
    C.Nb_rangs,
    C.Espacement,
    C.Notes,
    I.Notes N_IT_plante,
    PL.Notes N_Planche,
    E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (C.A_faire NOTNULL)
ORDER BY C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Cultures__Tempo AS SELECT
    CAST(dense_rank() OVER (ORDER BY Planche) AS INT) num_planche,
    Planche,
    Culture,
    Variété_ou_It_plante,
    CAST(TEMPO_NP || CASE WHEN Planche!=Lag THEN coalesce(Irrig,'') ELSE iif(Irrig NOTNULL,'x','') END AS TEXT) TEMPO_NP,
    CAST(TEMPO_N  || CASE WHEN Planche!=Lag THEN coalesce(Irrig,'') ELSE iif(Irrig NOTNULL,'x','') END AS TEXT) TEMPO_N,
    CAST(TEMPO_NN || CASE WHEN Planche!=Lag THEN coalesce(Irrig,'') ELSE iif(Irrig NOTNULL,'x','') END AS TEXT) TEMPO_NN,
    Saison,
    Longueur,
    Nb_rangs,
    Surface,
    A_faire,
    Notes
FROM Cultures__Tempo2;

CREATE VIEW Cultures__Tempo2 AS SELECT
    C.Planche,
    lag(Planche) OVER (ORDER BY Planche,coalesce(C.Date_plantation,C.Date_semis)) AS Lag,
    C.Culture,
    CAST(coalesce(C.Variété,C.IT_plante,C.Espèce) AS TEXT) Variété_ou_It_plante,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
    CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')-1)AS TEXT)
         THEN TEMPO
         ELSE ':::::::::::'
         END TEMPO_NP,
    CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
         THEN TEMPO
         ELSE ':::::::::::'
         END TEMPO_N,
    CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+1)AS TEXT)
         THEN TEMPO
         ELSE ':::::::::::'
         END TEMPO_NN,
    C.Saison,
    C.Longueur,
    C.Nb_rangs,
    CAST(C.Longueur*PL.Largeur AS REAL) Surface,
    PL.Irrig,
    E.Irrig Irrig_E,
    A_faire,
    C.Notes
FROM Cultures__Tempo3 C
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Planches PL USING(Planche)
WHERE (Planche NOTNULL)AND
      (coalesce(Terminée,'') NOT LIKE 'v%')AND -- Pas de succession pour les vivaces.
      (coalesce(Date_plantation,Date_semis) BETWEEN ((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')-1)||'-01-01' AND
                                                    ((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+1)||'-12-31')
ORDER BY C.Planche,coalesce(C.Date_plantation,C.Date_semis);

CREATE VIEW Cultures__Tempo3 AS SELECT
    *,
    CAST(iif(Date_semis ISNULL,0,CAST(jdS-jd0 AS INTEGER))||':'||
         iif(Date_semis ISNULL,0,CAST(jdS-jd0 AS INTEGER)+4)||':'||
         iif(Date_plantation ISNULL,0,CAST(jdP-jd0 AS INTEGER))||':'||
         iif(Date_plantation ISNULL,0,CAST(jdP-jd0 AS INTEGER)+4)||':'||
         iif(Début_récolte ISNULL,0,CAST(jdDR-jd0 AS INTEGER))||':'||
         iif(Fin_récolte ISNULL,0,CAST(jdFR-jd0 AS INTEGER)+iif(Début_récolte=Fin_récolte,4,0))||':'||
         Espèce||':'||
         coalesce(Semis_fait,'')||':'||
         coalesce(Plantation_faite,'')||':'||
         coalesce(Récolte_faite,'')||':'||
         coalesce(Terminée,'')||':' AS TEXT) TEMPO
FROM Cultures__Tempo4;

CREATE VIEW Cultures__Tempo4 AS SELECT
    *,
    julianday(substr(coalesce(Date_plantation,Date_semis),1,4)||'-01-01') jd0,
    julianday(Date_semis) jdS,
    julianday(Date_plantation) jdP,
    julianday(Début_récolte) jdDR,
    julianday(Fin_récolte) jdFR
FROM Cultures;

CREATE VIEW Cultures__analyse AS SELECT
    C.Culture,
    C.IT_plante,
    CAST(coalesce(C.Variété,C.Espèce) AS TEXT) Variétés_ou_Espèce,
    C.Planche,
    C.Type,
    C.Saison,
    iif(C.Semis_fait NOTNULL,C.Date_semis,NULL) Date_semis,
    iif(C.Plantation_faite NOTNULL,C.Date_plantation,NULL) Date_plantation,
    iif(C.Récolte_faite NOTNULL,C.Début_récolte,NULL) Début_récolte,
    iif(C.Récolte_faite NOTNULL,C.Fin_récolte,NULL) Fin_récolte,
    CAST(C.Longueur*PL.Largeur AS REAL) Surface,
    CAST(round((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc,
    CAST(round(((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture)/(C.Longueur*PL.Largeur)),3)AS REAL) Rendement_C,
    E.Rendement,
    CAST(CASE WHEN E.Rendement NOTNULL THEN ((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture)/(C.Longueur*PL.Largeur))/E.Rendement*100
              ELSE NULL
              END AS INT) Couv_pc,
    TEMPO,
    C.Notes
FROM Cultures__Tempo3 C
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Planches PL USING(Planche)
WHERE (C.Type IN('Semis pépinière','Plant','Semis en place')) AND C.Terminée NOTNULL AND (C.Terminée NOT LIKE '%NS')
ORDER BY C.Espèce,C.IT_plante,coalesce(C.Date_semis,C.Date_plantation);

CREATE VIEW Cultures__inc_dates AS SELECT
    C.Culture,
    C.IT_plante,
    CAST(coalesce(C.Variété,C.Espèce) AS TEXT) Variétés_ou_Espèce,
    C.Planche,
    C.Type,
    C.Etat,
    CID.Incohérence,
    I.S_semis,
    C.Date_semis,
    I.S_plantation,
    C.Date_plantation,
    I.S_récolte,
    I.D_récolte,
    I.Décal_max,
    C.Début_récolte,
    C.Fin_récolte,
    C.A_faire,
    C.Notes
FROM Cultures C
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Variétés V USING(Variété)
LEFT JOIN Cu_inc_dates CID USING(Culture)
WHERE (coalesce(C.Terminée,'') NOT LIKE '%NS')AND ((C.Type LIKE '%?')OR (CID.Incohérence NOTNULL))
ORDER BY Culture;

CREATE VIEW Cultures__non_terminées AS SELECT
    C.Planche,
    C.Culture,
    C.Espèce,
    C.IT_plante,
    C.Variété,
    C.Fournisseur,
    C.Type,
    C.Saison,
    CAST(CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                   ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!= (SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété))
              THEN 'Err. variété !'
              ELSE C.Etat
              END AS TEXT) Etat,
    C.D_planif,
    C.Date_semis,
    C.Semis_fait,
    C.Date_plantation,
    C.Plantation_faite,
    C.Début_récolte,
    C.Fin_récolte,
    C.Récolte_faite,
    CAST(round((SELECT Quantité FROM Cu_récolte R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc,
    C.Terminée,
    C.Longueur,
    C.Nb_rangs,
    C.Espacement,
    I.Nb_graines_plant,
    I.Dose_semis,
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100)AS INTEGER) Nb_plants,
    CAST(CASE WHEN C.Espacement>0 THEN 1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100
              ELSE I.Dose_semis/E.Dose_semis*100
              END AS INT) Densité_pc,
    C.A_faire,
    C.Notes,
    I.Notes N_IT_plante,
    PL.Notes N_Planche
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
ORDER BY C.Planche,coalesce(C.Date_semis,C.Date_plantation),C.Espèce,C.IT_plante;

CREATE VIEW Cultures__vivaces AS SELECT
    C.Planche,
    C.Culture,
    coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante,
    C.Etat,
    C.D_planif,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
    C.Début_récolte,
    C.Fin_récolte,
    C.Récolte_faite,
    E.S_taille,
    C.A_faire,
    C.Terminée,
    C.Notes,
    I.Notes N_IT_plante,
    PL.Notes N_Planche,
    E.Irrig Irrig_espèce,
    E.Besoins,
    E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE  (C.Terminée LIKE 'v%')OR(E.Vivace NOTNULL)
ORDER BY C.Planche,coalesce(C.Variété,C.IT_plante);

CREATE VIEW Cultures__à_fertiliser AS SELECT
    CAST(group_concat(C.Planche,x'0a0a') AS TEXT) Planches,
    CAST(group_concat(C.Culture||' ',x'0a0a') AS TEXT) Cultures,
    C.Espèce,
    CAST(group_concat(C.Variété_ou_It_plante,x'0a0a') AS TEXT) Variétés_ou_It_plante,
    Type,
    CAST(CASE WHEN C.Etat='Prévue' OR C.Etat='Pépinière' THEN 'A venir' ELSE C.Etat END AS TEXT) Etat,
    max(C.Pl_libre_le) Pl_libre_le,
    C.Date_MEP,
    min(C.Début_récolte) Début_récolte,
    max(C.Fin_récolte) Fin_récolte,
    CAST(sum(C.Surface) AS INT) Surface,
    CAST(min(sum(coalesce(C.N_fert,0))/total(C.N_esp*C.Surface*C.Densité_pc/100),
             sum(coalesce(C.P_fert,0))/total(C.P_esp*C.Surface*C.Densité_pc/100),
             sum(coalesce(C.K_fert,0))/total(C.K_esp*C.Surface*C.Densité_pc/100))*100 AS INTEGER) Fert_pc,
    CAST(CAST(sum(C.N_esp*C.Surface*C.Densité_pc/100) AS INTEGER)||'-'||
         CAST(sum(C.P_esp*C.Surface*C.Densité_pc/100) AS INTEGER)||'-'||
         CAST(sum(C.K_esp*C.Surface*C.Densité_pc/100) AS INTEGER) AS TEXT) Besoins_NPK,
    C.★N_esp,
    C.★P_esp,
    C.★K_esp,
    CAST(A.N||'-'||A.P||'-'||A.K||' ('||A.Analyse||')'||x'0a0a'||
         coalesce(A.Interprétation,'Pas d''interprétation') AS TEXT) Analyse_sol,
    A.☆N ☆N_sol,
    A.☆P ☆P_sol,
    A.☆K ☆K_sol,
    CAST(CAST(sum(C.N_fert) AS INTEGER)||'-'|| CAST(sum(C.P_fert) AS INTEGER)||'-'|| CAST(sum(C.K_fert) AS INTEGER) AS TEXT) Apports_NPK,
    CAST(sum(C.N_esp*C.Surface*C.Densité_pc/100)-sum(coalesce(C.N_fert,0)) AS INT) N_manq,
    CAST(sum(C.P_esp*C.Surface*C.Densité_pc/100)-sum(coalesce(C.P_fert,0)) AS INT) P_manq,
    CAST(sum(C.K_esp*C.Surface*C.Densité_pc/100)-sum(coalesce(C.K_fert,0)) AS INT) K_manq,
    C.A_faire,
    C.Notes,
    C.N_Planche,
    C.N_espèce
FROM Cu_à_fertiliser C
LEFT JOIN Analyses_de_sol A USING(Analyse)
GROUP BY C.Ilot,C.Espèce,C.Type,C.Etat,C.Date_MEP,Analyse_sol,C.A_faire,C.Notes,C.N_Planche,C.N_espèce;

CREATE VIEW Cultures__à_irriguer AS SELECT
    CAST(dense_rank() OVER (ORDER BY CaI.Planche) AS INT) num_planche,
    CaI.Planche,
    CaI.Culture,
    CaI.Variété_ou_It_plante,
    CaI.Irrig_planche,
    CaI.Irrig_espèce,
    CaI.TEMPO_NP,
    CAST(CaI.TEMPO_N  || iif(CaI.Irrig_planche NOTNULL,'x','') AS TEXT) TEMPO_N,
    CaI.TEMPO_NN,
    CaI.Saison,
    CaI.Longueur,
    CaI.Nb_rangs,
    CaI.Surface,
    CaI.A_faire,
    CaI.Notes
FROM Cultures__à_irriguer2 CaI
WHERE  ((CaI.Planche IN (SELECT CaI1.Planche FROM Cu_à_irriguer CaI1))AND(CaI.Irrig_planche ISNULL))OR
       ((NOT (CaI.Planche IN (SELECT CaI2.Planche FROM Cu_à_irriguer CaI2)))AND(CaI.Irrig_planche NOTNULL));

CREATE VIEW Cultures__à_irriguer2 AS SELECT
    C.Planche,
    lag(Planche) OVER (ORDER BY Planche,coalesce(C.Date_plantation,C.Date_semis)) AS Lag,
    C.Culture,
    CAST(coalesce(C.Variété,C.IT_plante,C.Espèce) AS TEXT) Variété_ou_It_plante,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
    CAST(CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')-1)AS TEXT)
             THEN TEMPO
             ELSE ':::::::::::'
             END AS TEXT) TEMPO_NP,
    CAST(CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
             THEN TEMPO
             ELSE ':::::::::::'
             END AS TEXT) TEMPO_N,
    CAST(CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+1)AS TEXT)
             THEN TEMPO
             ELSE ':::::::::::'
             END AS TEXT) TEMPO_NN,
    C.Saison,
    C.Longueur,
    C.Nb_rangs,
    CAST(C.Longueur*PL.Largeur AS REAL) Surface,
    PL.Irrig Irrig_planche,
    E.Irrig Irrig_espèce,
    C.A_faire,
    C.Notes
FROM Cultures__Tempo3 C
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Planches PL USING(Planche)
WHERE (Planche NOTNULL)AND
      (C.Terminée ISNULL)AND -- Que les annuelles non terminée.
      ((coalesce(Date_plantation,Date_semis) BETWEEN ((SELECT CAST(Valeur AS INT) FROM Params
                                                       WHERE Paramètre='Année_culture')-1)||'-01-01' AND
                                                     ((SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Année_culture')+1)||'-12-31'))
ORDER BY C.Planche,coalesce(C.Date_plantation,C.Date_semis),C.Espèce,C.IT_plante;

CREATE VIEW Cultures__à_planter AS SELECT
    C.Planche,
    C.Culture,
    C.Espèce,
    C.IT_plante,
    C.Variété,
    C.Fournisseur,
    C.Type,
    CAST(CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                   ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!= (SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété))
              THEN 'Err. variété !'
              ELSE C.Etat
              END AS TEXT) Etat,
    C.D_planif,
    C.Date_semis,
    C.Semis_fait,
    C.Date_plantation,
    C.Plantation_faite,
    C.Début_récolte,
    C.Longueur,
    PL.Largeur,
    C.Nb_rangs,
    C.Espacement,
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100)AS INTEGER) Nb_plants,
    CAST(1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100 AS INT) Densité_pc,
    C.A_faire,
    C.Notes,
    I.Notes N_IT_plante,
    PL.Notes N_planche,
    E.Notes N_espèce
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (Date_plantation < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_plantation')||' days'))AND
      (coalesce(Plantation_faite,'') NOT LIKE 'x%')AND
      ((Semis_fait NOTNULL)OR(Date_semis ISNULL))
ORDER BY C.Date_plantation,C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Cultures__à_récolter AS SELECT
    C.Planche,
    C.Culture,
    C.Espèce,
    CAST(coalesce(C.Variété,C.IT_plante) AS TEXT) Variété_ou_It_plante,
    C.Type,
    C.Etat,
    C.Date_semis,
    C.Semis_fait,
    C.Date_plantation,
    C.Plantation_faite,
    C.Début_récolte,
    C.Fin_récolte,
    C.Récolte_faite,
    CAST(round((SELECT Quantité FROM Cu_récolte R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc,
    C.Terminée,
    C.Longueur,
    C.Nb_rangs,
    C.Espacement,
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100)AS INTEGER) Nb_plants,
    C.A_faire,
    C.Notes,
    I.Notes N_IT_plante,
    PL.Notes N_Planche,
    E.Notes N_espèce
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (Début_récolte < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_récolte')||' days'))AND
      (coalesce(Récolte_faite,'') NOT LIKE 'x%')AND
      ((Semis_fait NOTNULL)OR(Date_semis ISNULL))AND
      ((Plantation_faite NOTNULL)OR(Date_plantation ISNULL))
ORDER BY C.Début_récolte,C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Cultures__à_semer AS SELECT
    C.Planche,
    C.Culture,
    C.Espèce,
    C.IT_plante,
    C.Variété,
    C.Fournisseur,
    C.Type,
    CAST(CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                   ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!= (SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété))
              THEN 'Err. variété !'
              ELSE C.Etat
              END AS TEXT) Etat,
    C.D_planif,
    C.Date_semis,
    C.Semis_fait,
    C.Date_plantation,
    C.Début_récolte,
    C.Longueur,
    PL.Largeur,
    C.Nb_rangs,
    C.Espacement,
    I.Nb_graines_plant,
    I.Dose_semis,
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100)AS INTEGER) Nb_plants,
    CAST(CASE WHEN C.Espacement>0 THEN 1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100
              ELSE I.Dose_semis/E.Dose_semis*100
              END AS INT) Densité_pc,
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant)AS INTEGER) Nb_graines,
    CAST(round((CASE WHEN C.Espacement > 0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant / E.Nb_graines_g
                     ELSE C.Longueur * PL.Largeur * I.Dose_semis
                     END),2)AS REAL) Poids_graines,
    C.A_faire,
    C.Notes,
    I.Notes N_IT_plante,
    PL.Notes N_planche,
    E.Notes N_espèce
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (Date_semis < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_semis')||' days'))AND
      (coalesce(Semis_fait,'') NOT LIKE 'x%')
ORDER BY C.Date_semis,C.Date_plantation,C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Cultures__à_semer_EP AS SELECT
    C.Planche,
    C.Culture,
    C.Espèce,
    C.IT_plante,
    C.Variété,
    C.Fournisseur,
    C.Type,
    CAST(CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                   ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!=(SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété))
              THEN 'Err. variété !'
              ELSE C.Etat
              END AS TEXT) Etat,
    C.D_planif,
    C.Date_semis,
    C.Semis_fait,
    C.Début_récolte,
    C.Longueur,
    PL.Largeur,
    C.Nb_rangs,
    C.Espacement,
    I.Nb_graines_plant,
    I.Dose_semis,
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100)AS INTEGER) Nb_plants,
    CAST(CASE WHEN C.Espacement>0 THEN 1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100
             ELSE I.Dose_semis/E.Dose_semis*100
             END AS INT) Densité_pc,
    CAST((C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant)AS INTEGER) Nb_graines,
    CAST(round((CASE WHEN C.Espacement > 0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant/E.Nb_graines_g
                     ELSE C.Longueur*PL.Largeur*I.Dose_semis
                     END),2)AS REAL) Poids_graines,
    C.A_faire,
    C.Notes,
    I.Notes N_IT_plante,
    PL.Notes N_planche,
    E.Notes N_espèce
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (Date_semis < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_semis')||' days'))AND
      (coalesce(Semis_fait,'') NOT LIKE 'x%')AND
      (Date_plantation ISNULL)
ORDER BY C.Date_semis,C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Cultures__à_semer_pep AS SELECT
    CAST(min(C.Culture)AS INTEGER) Culture, -- Pour servir d'index, colonne à cacher.
    CAST(group_concat(C.Planche,x'0a0a') AS TEXT) Planches,
    CAST(group_concat(' '||C.Culture||' ',x'0a0a') AS TEXT) Cultures,
    C.Espèce,
    C.IT_plante,
    C.Variété,
    C.Type,
    C.Etat,
    C.Date_semis,
    CAST(CASE WHEN sum(C.Semis_fait)>0 THEN sum(C.Semis_fait) ELSE min(C.Semis_fait) END AS BOOL) Semis_fait,
    CAST(CASE WHEN min(C.Date_plantation)=max(C.Date_plantation) THEN strftime('%d/%m/%Y',min(C.Date_plantation))
              ELSE group_concat(strftime('%d/%m/%Y',C.Date_plantation),x'0a0a')
              END AS TEXT) Dates_plantation,
    I.Nb_graines_plant,
    I.Dose_semis,
    CAST(group_concat(C.Longueur||'m x '||PL.Largeur||'m - '||
                      C.Nb_rangs||'rg, esp'||C.Espacement||'cm',x'0a0a') AS TEXT) Rangs_espacement,
    CAST(sum((C.Longueur * C.Nb_rangs / C.Espacement * 100))AS INTEGER) Nb_plants,
    CAST(sum((C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_plant))AS INTEGER) Nb_graines,
    CAST(round(sum((CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_plant / E.Nb_graines_g
                         ELSE C.Longueur * PL.Largeur * I.Dose_semis
                         END)),2)AS REAL) Poids_graines,
    CAST(min(C.A_faire) AS TEXT) A_faire,
    CAST(min(C.Notes) AS TEXT) Notes,
    I.Notes N_IT_plante,
    E.Notes N_espèce
FROM Cu_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE (Date_semis < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_semis')||' days'))AND
      (coalesce(Semis_fait,'') NOT LIKE 'x%')AND
      (Date_plantation NOTNULL)
GROUP BY C.Espèce,C.IT_plante,C.Variété,C.Type,C.Etat,C.Date_semis
ORDER BY C.Date_semis,C.Date_plantation,C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Cultures__à_terminer AS SELECT ---
    C.Planche,
    C.Culture,
    CAST(coalesce(C.Variété,C.IT_plante,C.Espèce) AS TEXT) Variété_ou_It_plante,
    C.Type,
    C.Etat,
    C.Date_semis,
    C.Date_plantation,
    C.Début_récolte,
    (SELECT R.Date FROM Récoltes R WHERE R.Culture=C.Culture ORDER BY Date DESC) Der_récolte, --- Dernière récolte saisie.
    C.Fin_récolte,
    C.Récolte_faite,
    C.Terminée,
    C.Longueur,
    C.A_faire,
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
    C.Notes,
    I.Notes N_IT_plante,
    PL.Notes N_Planche,
    E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE C.Terminée ISNULL AND -- Les vivaces ne sont jamais à terminer.
      ((C.Fin_récolte ISNULL)OR(C.Fin_récolte < DATE('now','+'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_horizon_terminer')||' days')))AND
      ((Semis_fait NOTNULL)OR(Date_semis ISNULL))AND
      ((Plantation_faite NOTNULL)OR(Date_plantation ISNULL))AND
      ((Récolte_faite NOTNULL)OR(Début_récolte ISNULL))
ORDER BY C.Fin_récolte,C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Destinations__conso AS SELECT ---
    D.Destination, ---
    D.Type, ---
    D.Adresse, ---
    D.Site_web, ---
    D.Date_RAZ, ---
    CAST((SELECT sum(Quantité) FROM Consommations Co WHERE (Co.Destination=D.Destination)AND(Co.Date >= D.Date_RAZ)) AS INT) Consommation, ---
    CAST((SELECT sum(Prix) FROM Consommations Co WHERE (Co.Destination=D.Destination)AND(Co.Date >= D.Date_RAZ)) AS REAL) Valeur, ---
    D.Active, ---
    D.Interne, ---
    D.Notes ---
FROM Destinations D;
UPDATE fda_schema SET tbl_type='View as table' WHERE (name='Destinations__conso');

CREATE VIEW Espèces__a AS SELECT ---
    Espèce,
    Famille,
    Catégories, ---
    Rendement,
    Niveau,
    Besoins,
    Densité,
    Dose_semis,
    Nb_graines_g,
    FG,
    T_germ,
    Levée,
    Irrig,
    Conservation,
    A_planifier,
    Obj_annuel,
    N,
    ★N,
    P,
    ★P,
    K,
    ★K,
    Effet,
    Notes
FROM Espèces
WHERE Vivace ISNULL;

CREATE VIEW Bilans_annuels AS SELECT ---
    B.Saison,
    CAST(sum(B.Obj_annuel) AS INT) Obj_annuel,
    -- CAST(total(B.Qté_prév) AS INT) Qté_prév,
    CAST(total(B.Qté_planif) AS INT) Qté_planif,
    -- CAST(total(B.Qté_prév)/total(B.Obj_annuel)*100 AS INT) Couv_prév_pc,
    CAST(total(B.Qté_réc) AS INT) Qté_réc,
    CAST(total(B.Reste_à_réc) AS INT) Reste_à_réc,
    -- CAST(total(B.Qté_réc)/total(B.Qté_prév)*100 AS INT) Couv_réc_pc,
    CAST(total(B.Qté_réc)/total(B.Obj_annuel)*100 AS INT) Couv_obj_pc, --- Quantités déjà récoltées par rapport aux objectifs.
    CAST(total(B.Nb_cu) AS INT) Nb_cu,
    CAST((SELECT count() FROM Planches P WHERE P.Planche IN(SELECT C.Planche FROM Cultures C WHERE C.Saison=B.Saison)) AS INT) Nb_planches,
    CAST((SELECT total(Longueur) FROM Planches P WHERE P.Planche IN(SELECT C.Planche FROM Cultures C WHERE C.Saison=B.Saison)) AS INT) Longueur,
    CAST((SELECT total(Longueur*Largeur) FROM Planches P WHERE P.Planche IN(SELECT C.Planche FROM Cultures C WHERE C.Saison=B.Saison)) AS INT) Surface,
    CAST(sum(Qté_réc_saison) AS INT) Qté_réc_saison, --- Quantités récoltées cette année (cultures de la saison courante et de la précédente).
    CAST(sum(Qté_exp) AS INT) Qté_exp, --- Quantités exportées cette année.
    CAST(sum(Valeur) AS REAL) Valeur,
    CAST(sum(Qté_exp)/total(Qté_réc_saison)*100 AS INT) Export_pc --- Quantités exportées par rapport aux quantités récoltées cette année.
FROM Espèces__Bilans_annuels B
GROUP BY Saison;

CREATE VIEW Espèces__Bilans_annuels AS SELECT ---
    C.Espèce,
    C.Saison,
    C.Rendement,
    C.Niveau,
    C.Obj_annuel,
    -- CAST(sum(C.Qté_prév) AS INT) Qté_prév,
    CAST((SELECT Prod_possible FROM Planif_espèces P WHERE P.Espèce=C.Espèce) AS INT) Qté_planif,
    -- CAST(sum(C.Qté_prév)/C.Obj_annuel*100 AS INT) Couv_prév_pc,
    CAST(sum(C.Qté_réc) AS INT) Qté_réc, --- Quantités récoltées pour les cultures mises en place cette saison.
    CAST(sum(iif(C.Terminée ISNULL,max(C.Qté_prév-C.Qté_réc,0),0)) AS INT) Reste_à_réc, --- Estimation des quantités restant à récolter pour les cultures mises en place cette saison et non terminées (qté prévu moins déja réc).
    -- CAST(sum(C.Qté_réc)/total(C.Qté_prév)*100 AS INT) Couv_réc_pc,
    CAST(sum(C.Qté_réc)/C.Obj_annuel*100 AS INT) Couv_obj_pc, --- Quantités récoltées par rapport aux objectifs.
    CAST(sum(C.Qté_réc+iif(C.Terminée ISNULL,max(C.Qté_prév-C.Qté_réc,0),0))/
         ((SELECT Prod_possible FROM Planif_espèces P WHERE P.Espèce=C.Espèce))*100 AS INT) Couv_pla_pc, --- Quantités récoltées + restant à récolter par rapport à la planification.
    CAST(count() AS INT) Nb_cu,
    CAST((SELECT count()
          FROM (SELECT DISTINCT C2.Planche
                FROM Cultures C2 WHERE (C2.Saison=C.Saison)AND(C2.Espèce=C.Espèce)AND(coalesce(C2.Terminée,'') NOT LIKE 'v%'))) AS INT) Nb_planches,
    CAST((SELECT group_concat(UP||' ('||(SELECT count()
                                         FROM (SELECT DISTINCT C3.Planche
                                               FROM Cultures C3
                                               WHERE (C3.Saison=C.Saison)AND(C3.Espèce=C.Espèce)AND(C3.Planche LIKE UP||'%')AND
                                                     (coalesce(C3.Terminée,'') NOT LIKE 'v%')))||' pl)',', ') --,x'0a0a'
          FROM (SELECT DISTINCT substr(C2.Planche,1,(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Ilot_nb_car')+
                                                    (SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='UP_nb_car')) UP
                FROM Cultures C2 WHERE (C2.Saison=C.Saison)AND(C2.Espèce=C.Espèce)AND(coalesce(C2.Terminée,'') NOT LIKE 'v%'))) AS INT) Unités_prod,
    CAST(sum(C.Longueur) AS INT) Longueur,
    CAST(sum(C.Surface) AS INT) Surface,
    CAST((SELECT sum(R.Quantité) FROM Récoltes R WHERE (R.Espèce=C.Espèce)AND(substr(R.Date,1,4)=C.Saison)) AS INT) Qté_réc_saison, --- Quantités récoltées cette année (cultures de la saison courante et de la précédente).
    CAST((SELECT sum(CO.Quantité) FROM Consommations CO LEFT JOIN Destinations D USING(Destination)
          WHERE (CO.Espèce=C.Espèce)AND(substr(CO.Date,1,4)=C.Saison)AND(D.Interne ISNULL)) AS INT) Qté_exp, --- Quantités exportées cette année.
    CAST((SELECT sum(CO.Prix) FROM Consommations CO LEFT JOIN Destinations D USING(Destination)
          WHERE (CO.Espèce=C.Espèce)AND(substr(CO.Date,1,4)=C.Saison)AND(D.Interne ISNULL)) AS REAL) Valeur,
    CAST((SELECT sum(CO.Quantité) FROM Consommations CO LEFT JOIN Destinations D USING(Destination)
          WHERE (CO.Espèce=C.Espèce)AND(substr(CO.Date,1,4)=C.Saison)AND(D.Interne ISNULL))/
         (SELECT sum(R.Quantité) FROM Récoltes R WHERE (R.Espèce=C.Espèce)AND(substr(R.Date,1,4)=C.Saison))*100 AS INT) Export_pc, --- Quantités exportées par rapport aux quantités récoltées cette année.
    C.Vivace,
    C.N_espèce Notes
FROM (SELECT * FROM Cu_esp_prod_an UNION SELECT * FROM Cu_esp_prod_vi) C
WHERE (C.Obj_annuel>0)OR(C.Qté_réc>0)
GROUP BY Espèce,Saison
ORDER BY Espèce,Saison;

CREATE VIEW Espèces__inventaire AS SELECT
    E.Espèce,
    E.Date_inv,
    E.Inventaire,
    CAST(round((SELECT sum(Quantité) FROM Récoltes Re WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),3)AS REAL) Entrées,
    CAST(round((SELECT sum(Quantité) FROM Consommations Re WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),3)AS REAL) Sorties,
    CAST(round(E.Inventaire +coalesce((SELECT sum(Quantité) FROM Récoltes Re
                                       WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0)
                            -coalesce((SELECT sum(Quantité) FROM Consommations Re
                                       WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0),3)AS REAL) Stock,
    E.Prix_kg,
    CAST(round((E.Inventaire +coalesce((SELECT sum(Quantité) FROM Récoltes Re
                                        WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0)
                             -coalesce((SELECT sum(Quantité) FROM Consommations Re
                                        WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0))*E.Prix_kg,3)AS REAL) Valeur,
    Notes
FROM Espèces E
WHERE E.Conservation NOT NULL;

CREATE VIEW Espèces__manquantes AS SELECT
    E.Espèce,
    E.Rendement,
    E.Niveau,
    E.Obj_annuel,
    CAST((SELECT sum(Prod_possible) FROM Planif_espèces C WHERE C.Espèce=E.Espèce)AS INTEGER) Prod_possible,
    CAST((SELECT sum(Prod_possible) FROM Planif_espèces C WHERE C.Espèce=E.Espèce)/E.Obj_annuel*100 AS INT) Couv_pc,
    CAST((SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture')AS INTEGER) Année_à_planifier
FROM Espèces E
WHERE (E.A_planifier NOTNULL)AND
      (NOT(Espèce IN(SELECT Espèce FROM Planif_espèces))OR
       ((SELECT sum(Prod_possible) FROM Planif_espèces C WHERE C.Espèce=E.Espèce)<E.Obj_annuel));

CREATE VIEW Espèces__v AS SELECT ---
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
    -- A_planifier,
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

CREATE VIEW Fertilisants__inventaire AS SELECT
    F.Fertilisant,
    F.Date_inv,
    F.Inventaire,
    CAST(round((SELECT sum(Quantité) FROM Fertilisations Fe WHERE (Fe.Fertilisant=F.Fertilisant)AND(Fe.Date >= F.Date_inv)),3)AS REAL) Sorties,
    CAST(round(F.Inventaire -coalesce((SELECT sum(Quantité) FROM Fertilisations Fe
                                       WHERE (Fe.Fertilisant=F.Fertilisant)AND(Fe.Date >= F.Date_inv)),0),3)AS REAL) Stock,
    F.Prix_kg,
    CAST(round((F.Inventaire -coalesce((SELECT sum(Quantité) FROM Fertilisations Fe
                                        WHERE (Fe.Fertilisant=F.Fertilisant)AND(Fe.Date >= F.Date_inv)),0))*F.Prix_kg,3)AS REAL) Valeur,
    Notes
FROM Fertilisants F;

CREATE VIEW Fertilisations__Saisies AS SELECT ---
    F.ID, ---
    F.Date, ---
    F.Espèce, ---
    C.Planche Planche·s, ---
    F.Culture, ---
    F.Fertilisant, ---
    F.Quantité, ---
    F.N, ---
    F.P, ---
    F.K, ---
    C.Variété, ---
    F.Notes ---
FROM Fertilisations F
JOIN Cultures C USING(Culture)
WHERE (F.Date>DATE('now','-'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Ferti_historique')||' days'))OR
      (DATE(F.Date) ISNULL) -- Détection de date incorecte
ORDER BY F.Date,F.Espèce,C.Planche,F.Culture;
UPDATE fda_schema SET tbl_type='View as table' WHERE (name='Fertilisations__Saisies');

CREATE VIEW ITP__Tempo AS SELECT ---
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

    CAST(coalesce((I.S_semis*7-6),0)||':'||
         coalesce(((I.S_semis+coalesce(I.Décal_max,0.5))*7-6),0)||':'||
         coalesce((I.S_plantation*7-6),0)||':'||
         coalesce(((I.S_plantation+coalesce(I.Décal_max,0.5))*7-6),0)||':'||
         coalesce((I.S_récolte*7-6),0)||':'||
         coalesce(((I.S_récolte+coalesce(I.D_récolte+coalesce(I.Décal_max,0),0))*7-6),0) -- Si pas de récolte (D_récolte NULL) ne pas ajouter le décalage.
        AS TEXT) TEMPO, ---

    CAST(CASE WHEN I.S_semis NOTNULL AND I.S_plantation NOTNULL
              THEN iif(I.S_plantation-I.S_semis<0,I.S_plantation-I.S_semis+52,I.S_plantation-I.S_semis)*7
              END AS INT) J_pép, ---
    CAST(CASE WHEN coalesce(I.S_plantation,I.S_semis) NOTNULL AND I.S_récolte NOTNULL
              THEN iif(I.S_récolte-coalesce(I.S_plantation,I.S_semis)<0,I.S_récolte-coalesce(I.S_plantation,I.S_semis)+52,
                       I.S_récolte-coalesce(I.S_plantation,I.S_semis))*7
              END AS INT) J_en_pl, ---
    -- I.Nb_rangs,
    I.Espacement, ---
    I.Esp_rangs, ---
    I.Nb_graines_plant, ---
    I.Dose_semis, ---
    CAST(CASE WHEN I.Espacement>0 THEN 1.0/I.Espacement*100/I.Esp_rangs*100/E.Densité*100
              ELSE I.Dose_semis/E.Dose_semis*100 END AS INT) Densité_pc, ---
    I.Notes, ---
    E.Notes N_espèce, ---
    E.Famille ---
FROM ITP I
LEFT JOIN Espèces E USING(Espèce);
UPDATE fda_schema SET tbl_type='View as table' WHERE (name='ITP__Tempo');
UPDATE fda_schema SET readonly='x' WHERE (name='ITP__Tempo')AND(field_name IN('S_MEP','Catégories','N_espèce'));

CREATE VIEW ITP__analyse_a AS SELECT
    I.IT_plante,
    I.Type_culture,
    I.S_semis,
    I.S_plantation,
    I.S_récolte,
    CAST((SELECT count() FROM Cultures C WHERE (C.IT_plante=I.IT_plante) AND C.Terminée ISNULL)AS INTEGER) Nb_cu_NT,
    CAST((SELECT count() FROM Cultures C WHERE (C.IT_plante=I.IT_plante) AND C.Terminée NOTNULL)AS INTEGER) Nb_cu_T,
    CAST((SELECT count() FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_TS,
    CAST((SELECT min(strftime('%U',C.Date_semis)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_semis_min,
    CAST((SELECT max(strftime('%U',C.Date_semis)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_semis_max,
    CAST((SELECT min(strftime('%U',C.Date_plantation)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_plant_min,
    CAST((SELECT max(strftime('%U',C.Date_plantation)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_plant_max,
    CAST((SELECT min(strftime('%U',C.Début_récolte)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_récolte_min,
    CAST((SELECT max(strftime('%U',C.Fin_récolte)+1) FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) AS INT) S_récolte_max,
    CAST(round((SELECT sum(R.Quantité) FROM Cu_ITP_analyse_a C LEFT JOIN Récoltes R USING(Culture) WHERE C.IT_plante=I.IT_plante)/
               (SELECT count() FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante),3)AS REAL) Qté_réc_moy
FROM ITP I
WHERE (SELECT count() FROM Cu_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante)>0
ORDER BY IT_plante;

CREATE VIEW ITP__analyse_v AS SELECT
    I.IT_plante,
    I.Type_culture,
    I.S_semis,
    I.S_plantation,
    I.S_récolte,
    CAST((SELECT count() FROM Cu_non_ter C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_NT,
    CAST((SELECT count() FROM Cu_ter C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_T,
    CAST((SELECT count() FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_S,
    CAST((SELECT min(strftime('%U',C.Date_semis)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_semis_min,
    CAST((SELECT max(strftime('%U',C.Date_semis)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_semis_max,
    CAST((SELECT min(strftime('%U',C.Date_plantation)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_plant_min,
    CAST((SELECT max(strftime('%U',C.Date_plantation)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_plant_max,
    CAST((SELECT min(strftime('%U',C.Début_récolte)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_récolte_min,
    CAST((SELECT max(strftime('%U',C.Fin_récolte)+1) FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) S_récolte_max,
    CAST((SELECT sum(ceil((julianday(min(C.Fin_récolte,DATE('now',coalesce(I.D_récolte*7,0)||' days')))
                          -julianday((SELECT min(R.Date) FROM Récoltes R WHERE R.Culture=C.Culture)))/365))
          FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) AS INT) Nb_récoltes,
    CAST(round((SELECT sum(R.Quantité) FROM Cu_ITP_analyse_v C LEFT JOIN Récoltes R USING(Culture) WHERE C.IT_plante=I.IT_plante)/
               (SELECT sum(ceil((julianday(min(C.Fin_récolte,DATE('now',coalesce(I.D_récolte*7,0)||' days')))
                                -julianday((SELECT min(R.Date) FROM Récoltes R WHERE R.Culture=C.Culture)))/365))
                FROM Cu_ITP_analyse_v C
                WHERE C.IT_plante=I.IT_plante),3)AS REAL) Qté_réc_moy
FROM ITP I
WHERE (SELECT count() FROM Cu_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante)>0
ORDER BY IT_plante;

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
    '2025-09-25' Valeur;

CREATE VIEW Planche_asso_esp_présentes AS SELECT -- Pour chaque associations possible, compter les cultures d'espèces identiques présentes sur la planche.
    C.Planche,
    min(C.Date_MEP) Date_MEP, -- min car si plusieurs culture d'une même espèce, une seule est nécessaire.
    max(C.Début_récolte) Début_récolte, -- max, idem.
    C.Association,
    C.Espèce,
    C.Requise,
    count() Nb, -- Nb cultures même planche, asso et espèce.
    count(Vivace) Nb_vivaces,
    group_concat(C.Cultures,x'0a0a') Cultures
FROM Planche_asso_possibles C
GROUP BY C.Planche,C.Association,C.Espèce,C.Requise;

CREATE VIEW Planche_asso_esp_requises_présentes AS SELECT
    P.Planche,
    max(P.Date_MEP) Date_MEP, -- max car les cultures doivent être présentes en même temps.
    min(P.Début_récolte) Début_récolte, -- min, idem.
    P.Association,
    CAST(count() AS INT) Nb_distinct, -- nb d'espèces même planche et asso.
    count(P.Requise) Nb_requises, -- nb d'espèces requises même planche et asso.
    CAST(sum(P.Nb) AS INT) Nb, -- Nb cultures même planche et asso.
    CAST(sum(Nb_vivaces) AS INT) Nb_vivaces,
    CAST(group_concat(P.Cultures,x'0a0a') AS TEXT) Cultures
FROM Planche_asso_esp_présentes P
GROUP BY P.Planche,P.Association;

CREATE VIEW Planche_asso_possibles AS SELECT -- Liste des associations possibles pour chaque espèce des cultures non terminées.
    C.Planche,
    C.Culture,
    C.Espèce,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
    coalesce(C.Début_récolte,C.Fin_récolte) Début_récolte,
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

CREATE VIEW Planif_associations AS SELECT -- Sélectionner les associations présentes dans la planif.
    P.Planche,
    P.Association,
    P.Cultures,
    P.Nb Nb_cultures,
    P.Nb_distinct Nb_espèces,
    P.Nb_vivaces,
    A.Espèces Espèces_de_l_association
FROM Planif_asso_esp_requises_présentes P
JOIN Associations__Espèces A USING(Association)
WHERE (P.Nb>P.Nb_vivaces)AND -- Associations qui inclue des annuelles.
      ((A.Nb>1)AND(P.Nb_distinct>1)AND(P.Nb_requises=A.Nb_requises))AND -- Association de plusieurs espèces et toutes les requises sont présentes.
      (julianday(P.Début_récolte)-julianday(P.Date_MEP)>=(SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Nb_sem_croisement')*7); -- Croisement mini pour les cultures.

CREATE VIEW Planif_asso_esp_présentes AS SELECT -- Pour chaque associations possible, compter les cultures d'espèces identiques présentes sur la planche.
    C.Planche,
    min(C.Date_MEP) Date_MEP, -- min car si plusieurs culture d'une même espèce, une seule est nécessaire.
    max(C.Début_récolte) Début_récolte, -- max, idem.
    C.Association,
    C.Espèce,
    C.Requise,
    count() Nb, -- Nb cultures même planche, asso et espèce.
    count(Vivace) Nb_vivaces,
    group_concat(C.Cultures,x'0a0a') Cultures
FROM Planif_asso_possibles C
GROUP BY C.Planche,C.Association,C.Espèce,C.Requise;

CREATE VIEW Planif_asso_esp_requises_présentes AS SELECT
    P.Planche,
    max(P.Date_MEP) Date_MEP, -- max car les cultures doivent être présentes en même temps.
    min(P.Début_récolte) Début_récolte, -- min, idem.
    P.Association,
    CAST(count() AS INT) Nb_distinct, -- nb d'espèces même planche et asso.
    count(P.Requise) Nb_requises, -- nb d'espèces requises même planche et asso.
    CAST(sum(P.Nb) AS INT) Nb, -- Nb cultures même planche et asso.
    CAST(sum(P.Nb_vivaces) AS INT) Nb_vivaces,
    CAST(group_concat(P.Cultures,x'0a0a') AS TEXT) Cultures
FROM Planif_asso_esp_présentes P
GROUP BY P.Planche,P.Association;

CREATE VIEW Planif_asso_possibles AS SELECT -- Liste des associations possibles pour chaque espèce des cultures planifiées.
    C.Planche,
    C.Espèce,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
    coalesce(C.Début_récolte,C.Fin_récolte) Début_récolte,
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
JOIN Rotations R USING(Rotation);

CREATE VIEW Pl_bilan_fert AS SELECT
    C.Saison,
    P.Planche,
    P.Type,
    CASt(P.Longueur*P.Largeur AS REAL) Surface,
    P.Analyse,
    C.Culture,
    coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante,
    C.Etat,
    coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
    coalesce(CASE WHEN C.Espacement>0 THEN round(1.0/C.Espacement*100*C.Nb_rangs/P.Largeur/E.Densité*100)
             ELSE round(I.Dose_semis/E.Dose_semis*100) END,100) Densité_pc,
    E.N*C.Longueur*P.Largeur N_esp,
    E.P*C.Longueur*P.Largeur P_esp,
    E.K*C.Longueur*P.Largeur K_esp,
    sum(F.N) N_fert,
    sum(F.P) P_fert,
    sum(F.K) K_fert,
    group_concat(F.Fertilisant||' '||CAST(F.Quantité AS TEXT)||'kg -> '||F.Espèce,x'0a0a') Fertilisants,
    P.Notes
FROM Planches P
LEFT JOIN Cultures C USING(Planche)
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Fertilisations F USING(Culture)
WHERE coalesce(C.Terminée,'') NOT LIKE 'v%'
GROUP BY Culture
ORDER BY Planche,Date_MEP;

CREATE VIEW Planches_Ilots AS SELECT
    CAST(substr(P.Planche,1,(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Ilot_nb_car')) AS TEXT) Ilot,
    P.Type,
    CAST(count() AS INTEGER) Nb_planches,
    CAST(round(sum(P.Longueur),2) AS REAL) Longueur,
    CAST(round(sum(P.Longueur*P.Largeur),2)AS REAL) Surface,
    P.Rotation
FROM Planches P
GROUP BY Ilot,Type,Rotation
ORDER BY Ilot,Type,Rotation;

CREATE VIEW Planches_Unités_prod AS SELECT --- Les unités de production sont des regroupements de planches.\nElles ne sont pas saisies mais déduites des planches saisies.\nLe débuts du nom des planches indique leur ilot (paramètre 'Ilot_nb_car') puis leur unité de production (paramètre 'UP_nb_car').
    CAST(substr(P.Planche,1,(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Ilot_nb_car')+(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='UP_nb_car')) AS TEXT) Unité_prod,
    P.Type,
    CAST(count() AS INTEGER) Nb_planches,
    CAST(round(sum(P.Longueur),2) AS REAL) Longueur,
    CAST(round(sum(P.Longueur*P.Largeur),2)AS REAL) Surface,
    P.Rotation
FROM Planches P
GROUP BY Unité_prod,Type,Rotation
ORDER BY Unité_prod,Type,Rotation;

CREATE VIEW Planches__bilan_fert AS SELECT
    P.Saison,
    P.Planche,
    P.Type,
    P.Surface,
    CAST(count(P.Culture) AS INT) Nb_cu,
    CAST(group_concat(P.Culture||' - '||P.Variété_ou_It_plante||' - '||P.Etat||' - '||strftime('%d/%m/%Y',P.Date_MEP),x'0a0a') AS TEXT) Cultures,
    CAST(CAST(sum(P.N_esp*P.Densité_pc/100) AS INTEGER)||'-'||
         CAST(sum(P.P_esp*P.Densité_pc/100) AS INTEGER)||'-'||
         CAST(sum(P.K_esp*P.Densité_pc/100) AS INTEGER) AS TEXT) Besoins_NPK,
    CAST(A.N||'-'||A.P||'-'||A.K||' ('||A.Analyse||')'||x'0a0a'||
         coalesce(A.Interprétation,'Pas d''interprétation') AS TEXT) Analyse_sol,
    CAST(group_concat(P.Fertilisants,x'0a0a') AS TEXT) Fertilisants,
    CAST(CAST(sum(P.N_fert) AS INTEGER)||'-'||CAST(sum(P.P_fert) AS INTEGER)||'-'||CAST(sum(P.K_fert) AS INTEGER) AS TEXT) Apports_NPK,
    CAST(round(min(sum(coalesce(P.N_fert,0))/total(P.N_esp*P.Densité_pc/100),
    sum(coalesce(P.P_fert,0))/total(P.P_esp*P.Densité_pc/100),
    sum(coalesce(P.K_fert,0))/total(P.K_esp*P.Densité_pc/100))*100) AS INTEGER) Fert_pc,
    CAST(sum(P.N_esp*P.Densité_pc/100)-sum(P.N_fert) AS INT) N_manq,
    CAST(sum(P.P_esp*P.Densité_pc/100)-sum(P.P_fert) AS INT) P_manq,
    CAST(sum(P.K_esp*P.Densité_pc/100)-sum(P.K_fert) AS INT) K_manq,
    P.Notes
FROM Pl_bilan_fert P
LEFT JOIN Analyses_de_sol A USING(Analyse)
GROUP BY P.Saison,P.Planche;

CREATE VIEW Planches__deficit_fert AS SELECT
    P.Planche,
    P.Type,
    CAST((SELECT group_concat(CAST(C.Culture AS TEXT)||' - '||coalesce(C.Variété,C.IT_plante,C.Espèce)||' - '||C.Etat,x'0a0a') FROM Cu_non_ter C
          WHERE C.Planche=P.Planche) AS TEXT) Cultures,
    (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
     WHERE (PBF1.Planche=P.Planche)AND(PBF1.Saison=(SELECT Valeur-3 FROM Params WHERE Paramètre='Année_culture'))) Fert_Nm3_pc,
    (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
     WHERE (PBF1.Planche=P.Planche)AND(PBF1.Saison=(SELECT Valeur-2 FROM Params WHERE Paramètre='Année_culture'))) Fert_Nm2_pc,
    (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
     WHERE (PBF1.Planche=P.Planche)AND(PBF1.Saison=(SELECT Valeur-1 FROM Params WHERE Paramètre='Année_culture'))) Fert_Nm1_pc,
    (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
     WHERE (PBF1.Planche=P.Planche)AND(PBF1.Saison=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture'))) Fert_N_pc
FROM Planches P
WHERE (P.Planche IN (SELECT PBF.Planche FROM Planches__bilan_fert PBF
                     WHERE (PBF.Fert_pc<(SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Déficit_fert'))AND
                                        (PBF.Saison BETWEEN (SELECT Valeur-2 FROM Params WHERE Paramètre='Année_culture')AND
                                                            (SELECT Valeur-1 FROM Params WHERE Paramètre='Année_culture'))));

CREATE VIEW Planif_espèces AS SELECT
    C.Espèce,
    CAST(count() AS INTEGER) Nb_planches,
    CAST(sum(C.Longueur)AS REAL) Longueur,
    CAST(sum(C.Longueur*C.Nb_rangs)AS REAL) Long_rang,
    CAST(sum(C.Longueur*C.Nb_rangs/C.Espacement*100) AS INTEGER) Nb_plants,
    CAST(sum(C.Surface)AS REAL) Surface,
    E.Rendement,
    E.Obj_annuel,
    CAST(sum(C.Prod_possible)AS REAL) Prod_possible,
    CAST(sum(C.Prod_possible)/E.Obj_annuel*100 AS INT) Couv_pc,
    CAST('Simulation planif' AS TEXT) Info
FROM Planif_planches C
LEFT JOIN Espèces E USING(Espèce)
GROUP BY Espèce
ORDER BY Espèce;

CREATE VIEW Planif_ilots AS SELECT
    CAST(substr(C.Planche,1,(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='Ilot_nb_car')) AS TEXT) Ilot,
    C.Espèce,
    min(Date_semis) Date_semis,
    min(Date_plantation) Date_plantation,
    CAST(count() AS INTEGER) Nb_planches,
    CAST(sum(C.Longueur)AS REAL) Longueur,
    CAST(sum(C.Longueur*C.Nb_rangs)AS REAL) Long_rang,
    CAST(sum(C.Longueur*C.Nb_rangs/C.Espacement*100) AS INTEGER) Nb_plants,
    CAST(sum(C.Surface)AS REAL) Surface,
    CAST(sum(C.Prod_possible)AS REAL) Prod_possible,
    CAST('Simulation planif' AS TEXT) Info
FROM Planif_planches C
GROUP BY Ilot,Espèce
ORDER BY Ilot,Espèce;

CREATE VIEW Planif_planches AS SELECT
    PL.Planche,
    E.Espèce,
    RD.IT_plante,
    (SELECT V.Variété FROM Variétés V WHERE V.Espèce=I.Espèce ORDER BY V.Qté_stock DESC) Variété, -- Les IT des plans de rotation ont toujours une Espèce.
    (SELECT V.Fournisseur FROM Variétés V WHERE V.Espèce=I.Espèce ORDER BY V.Qté_stock DESC) Fournisseur,
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
    CAST('Simulation planif' AS TEXT) Info
FROM Rotations_détails RD
JOIN Rotations R USING(Rotation)
LEFT JOIN Pl_Décalées PL USING(Rotation,Année)
JOIN ITP_planif I USING(IT_plante)
LEFT JOIN Espèces E USING(Espèce)
WHERE ((PL.Planche ISNULL)OR
       (SELECT (Valeur ISNULL) FROM Params WHERE Paramètre='Planifier_planches')OR
       (PL.Planche LIKE (SELECT Valeur FROM Params WHERE Paramètre='Planifier_planches')||'%'))AND
      ((RD.Fi_planches ISNULL) OR (Fi_planches LIKE '%'||substr(PL.Planche,-1,1) ||'%'))
ORDER BY coalesce(DATE(I.Date_plantation,(coalesce(RD.Décalage*I.Coef_plantation,0)*7)||' days'),
                  DATE(I.Date_semis,(coalesce(RD.Décalage*I.Coef_semis,0)*7)||' days'));

CREATE VIEW Planif_planches_asso AS SELECT -- Cultures annulles et vivaces non terminées
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
FROM Planif_planches C
UNION
SELECT -- Vivaces non terminées sur leurs planches réelles
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

CREATE VIEW Planif_récoltes AS
WITH RECURSIVE Semaines AS (
SELECT Planche||Espèce||Début_récolte Id,
    Espèce,
    Début_récolte Date_jour,
    Fin_récolte,
    Prod_possible,
    0 Offset
FROM Planif_planches
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

CREATE VIEW RD_asso_esp_présentes AS SELECT -- Pour chaque associations possible, compter les cultures d'espèces différentes présentes dans la rotation.
    R.IdxRotAnFp,
    min(R.S_MEP) S_MEP, -- min car si plusieurs culture d'une même espèce, une seule est nécessaire.
    max(R.S_récolte) S_récolte, -- max, idem.
    R.Association,
    R.Espèce,
    R.Requise,
    count() Nb
FROM RD_asso_possibles R
GROUP BY R.IdxRotAnFp,R.Association,R.Espèce,R.Requise;

CREATE VIEW RD_asso_esp_requises_présentes AS SELECT
    R.IdxRotAnFp,
    max(R.S_MEP) S_MEP, -- max car les cultures doivent être présentes en même temps.
    min(R.S_récolte) S_récolte, -- min, idem.
    R.Association,
    count() Nb_distinct,
    count(R.Requise) Nb_requises,
    sum(R.Nb) Nb
FROM RD_asso_esp_présentes R
GROUP BY R.IdxRotAnFp,R.Association;

CREATE VIEW RD_asso_possibles AS SELECT -- Liste des associations possibles pour chaque espèce d'une rotation.
    R.IdxRotAnFp,
    R.Espèce,
    R.S_MEP,
    R.S_récolte,
    A.Association,
    A.Espèce Espèce_A,
    A.Requise
FROM RD_espèces R
JOIN Associations_détails__Espèces_a A USING(Espèce)
ORDER BY R.IdxRotAnFp,R.Espèce,A.Association;

CREATE VIEW RD_asso_possibles_cult AS SELECT -- Liste des associations possibles pour chaque culture d'une rotation.
    R.ID,
    A.Association,
    -- A.Effet,
    A.Requise
FROM Rotations_détails R
LEFT JOIN RD_ITP RI USING(ID)
JOIN Associations_détails__Espèces_a A ON (A.Espèce=RI.Espèce);

CREATE VIEW RD_asso_présentes AS SELECT -- Sélectionner les associations présentes dans la rotation.
    R.*,
    -- A.Effet,
    A.Nb Nb_asso,
    A.Espèces
FROM RD_asso_esp_requises_présentes R
JOIN Associations__Espèces A USING(Association)
WHERE ((A.Nb>1)AND(R.Nb_distinct>1)AND(R.Nb_requises=A.Nb_requises))AND -- Association de plusieurs espèces/familles et les requises sont présentes.
      ((R.S_récolte-R.S_MEP)>=(SELECT CAST(Valeur AS REAL) FROM Params WHERE Paramètre='Nb_sem_croisement')); -- Croisement mini pour les cultures.

CREATE VIEW RD_espèces AS SELECT -- Liste des espèces d'une rotation.
    DISTINCT R.Rotation||'-'||R.Année||'-'||coalesce(R.Fi_planches,'') IdxRotAnFp,
    I.Espèce,
    -- E.Famille,
    max(coalesce(I.S_plantation+coalesce(R.décalage,0),
    I.S_semis+coalesce(R.décalage,0),52)) S_MEP,
    min(coalesce(I.S_récolte+coalesce(R.décalage,0),52)) S_récolte
FROM Rotations_détails R
LEFT JOIN ITP_sem_corrigées I USING(IT_plante)
LEFT JOIN Espèces E USING(Espèce)
GROUP BY IdxRotAnFp,Espèce;

CREATE VIEW RD_famille AS SELECT
    RD.ID,
    RD.Rotation,
    RD.Année,
    R.Nb_années,
    F.Famille,
    F.Intervalle
FROM Rotations_détails RD
LEFT JOIN Rotations R USING(Rotation)
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Espèces E USING(Espèce)
JOIN Familles F USING(Famille);

CREATE VIEW RD_ITP AS SELECT
    R.Rotation||CAST(2001+R.Année AS TEXT)||'-'|| -- Rotation, année.
        substr('0'||CAST((coalesce(I.S_plantation,I.S_semis,52)+coalesce(R.Décalage,0)) AS TEXT),-2)||'-'|| -- Semaine de mise en place.
        coalesce(R.Fi_planches,'')||'-'||
        CAST(2001+R.Année+iif(coalesce(I.S_plantation,I.S_semis)>I.S_récolte,1,0)AS TEXT)||'-'|| -- Année de récolte.
        substr('0'||CAST(coalesce(I.S_récolte,52)AS TEXT),-2)||'-'|| -- Semaine de récolte.
        I.Espèce IdxRDITP,
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
    I.Espacement,
    I.Esp_rangs,
    E.Espèce,
    E.Catégories,
    E.A_planifier,
    F.Famille,
    F.Intervalle
FROM Rotations_détails R
LEFT JOIN Rotations RO USING(Rotation)
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Familles F USING(Famille)
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

CREATE VIEW Rotations_détails__Tempo AS SELECT ---
    R.ID,
    RI.IdxRDITP,
    R.Rotation, ---
    R.Année, ---
    R.IT_plante, ---
    -- RI.Nb_rangs,
    RI.Espacement, ---
    RI.Esp_rangs, ---
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
                                          ((RI.Fi_planches ISNULL) OR (R2.Fi_planches ISNULL) OR (RI.Fi_planches ISNULL=R2.Fi_planches ISNULL))AND
                                          -- total d'occupation des planches en conflit supérieur à 100.
                                          (RI.Pc_planches+R2.Pc_planches>100)),
                                   (SELECT R3.S_Ferm_abs-52*RI.Nb_années FROM RD_ITP_2 R3
                                    WHERE (R3.Rotation=R.Rotation)AND
                                          ((RI.Fi_planches ISNULL) OR (R3.Fi_planches ISNULL) OR (RI.Fi_planches ISNULL=R3.Fi_planches ISNULL))AND
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
     END) AS TEXT) S_MEP, ---

    CAST(RI.S_Ferm-iif(RI.S_Ferm>52,52,0) AS INT) S_Ferm, ---
    RI.Catégories, ---

    CAST(
    coalesce((RI.S_semis+coalesce(R.Décalage,0)-iif(RI.S_semis>RI.S_plantation,52,0)) -- Semis année N-1.
             *7-6,0)||':'|| -- Conversion semaines -> jours puis se caler au lundi.
    coalesce((RI.S_semis+iif(R.Décalage ISNULL,coalesce(RI.Décal_max,0)+1, -- Montrer la période de semis.
                                                        R.Décalage+1) -- La semaine de semis est fixée, afficher un point (1 semaine).
                        -iif(RI.S_semis>RI.S_plantation,52,0)) -- Semis année N-1.
             *7-6,0)||':'|| -- Conversion semaines -> jours puis se caler au lundi.
    coalesce((RI.S_plantation+coalesce(R.Décalage,0))*7-6,0)||':'||
    coalesce((RI.S_plantation+iif(R.Décalage ISNULL,coalesce(RI.Décal_max,0)+1, -- Montrer la période de plantation.
                                  R.Décalage+1))
             *7-6,0)||':'|| -- Conversion semaines -> jours puis se caler au lundi.
    coalesce((RI.S_récolte+coalesce(R.Décalage,0))*7-6,0)||':'||
    coalesce((RI.S_récolte+coalesce(RI.D_récolte -- Si pas de récolte (D_récolte NULL) ne pas ajouter le décalage (pas de coalesce).
                                    +iif(R.Décalage ISNULL,coalesce(RI.Décal_max,0), -- Montrer la période de récolte plus décalage.
                                         R.Décalage+0),0)) -- La semaine de début de récolte est fixée, afficher la période de récolte.
             *7-6,0)
    AS TEXT) TEMPO, ---
    RI.A_planifier, ---

    CAST(CASE WHEN (RI.Type_planche NOTNULL) AND (RI.Type_planche<>(SELECT Type_planche FROM Rotations WHERE Rotation=R.Rotation) )
              THEN RI.Type_planche END AS TEXT) Conflit_type_planche,

    -- Conflit familles.
    CAST(
    CASE WHEN R.IT_plante NOTNULL -- Recherche de familles trop proches dans une rotation.
         THEN (SELECT group_concat(result,x'0a0a')
               FROM (SELECT DISTINCT result
                     FROM (SELECT RF.Famille || ' année ' || format('%i',RF.Année) result FROM RD_famille RF
                           WHERE (RF.Rotation=R.Rotation)AND --ITP de la rotation
                                 (RF.Année<>R.Année)AND -- que les ITP des autres années de la rotation
                                 (RF.Famille=RI.Famille)AND -- que les ITP de la même famille
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
    AS TEXT) Conflit_famille, ---

    -- Associations
    CAST(
    CASE WHEN R.IT_plante NOTNULL -- AND R.Pc_planches<100 AND R.Occupation IN('R','E')
         -- Recheche des associations dans la rotations
         THEN (CASE WHEN (SELECT count() FROM RD_asso_présentes RDAP
                          WHERE (R.Rotation||'-'||R.Année||'-'||coalesce(R.Fi_planches,'')=RDAP.IdxRotAnFp)AND -- Associations trouvées sur ces planches.
                                (RDAP.Association IN(SELECT RDAPC.Association FROM RD_asso_possibles_cult RDAPC
                                                     WHERE RDAPC.ID=R.ID)))>0 -- et la culture fait partie de l'association.
                    THEN (SELECT group_concat(RDAP.Association,x'0a0a') --||' : '||RDAP.Effet
                          FROM RD_asso_présentes RDAP
                          WHERE (R.Rotation||'-'||R.Année||'-'||coalesce(R.Fi_planches,'')=RDAP.IdxRotAnFp)AND
                                (RDAP.Association IN(SELECT RDAPC.Association FROM RD_asso_possibles_cult RDAPC
                                                     WHERE RDAPC.ID=R.ID)))
                    WHEN ((SELECT Valeur FROM Params WHERE Paramètre='Rot_Rech_ass_poss')='Oui')OR((R.Pc_planches<100)AND(R.Occupation IN('R','E')))
                    THEN 'Ass. possibles : '|| -- Proposer des associations.
                         (SELECT group_concat(Espèce_ou_famille,', ')
                          FROM (SELECT DISTINCT ADE.Espèce_ou_famille
                                FROM RD_asso_possibles_cult RDAPC
                                LEFT JOIN Associations_détails_aap ADE USING(Association)
                                WHERE (RDAPC.Association LIKE '%'||(SELECT Valeur FROM Params WHERE Paramètre='Asso_bénéfique'))AND
                                      (RDAPC.ID=R.ID)AND
                                      ((RDAPC.Requise NOTNULL)OR -- Espèce de la RD requise dans l'association
                                       (RDAPC.Association LIKE '%'||ADE.Espèce_ou_famille||'%'))AND -- Espèce de la RD incluse dans le nom de l'association
                                      NOT(RI.Espèce LIKE ADE.Espèce_ou_famille||'%')
                                ORDER BY Espèce_ou_famille))
                    END)
         END
    AS TEXT) Associations, ---
    RI.Famille, ---
    RI.Intervalle, ---
    R.Notes ---
FROM Rotations_détails R
LEFT JOIN RD_ITP RI USING(ID)
ORDER BY RI.IdxRDITP;
UPDATE fda_schema SET tbl_type='View as table' WHERE (name='Rotations_détails__Tempo');
UPDATE fda_schema SET readonly='x' WHERE (name='Rotations_détails__Tempo')AND(field_name IN('Catégories'));

CREATE VIEW Récoltes__Saisies AS SELECT ---
    R.ID, ---
    R.Date, ---
    C.Espèce, ---
    C.Planche Planche·s, ---
    R.Culture, ---
    R.Quantité, ---
    R.Réc_ter, ---
    C.Variété, ---
    CAST(round((SELECT sum(Quantité) FROM Récoltes R2 WHERE (R2.Culture=R.Culture)AND(R2.Date<=R.Date)),3)AS REAL) Total_réc, ---
    R.Notes ---
FROM Récoltes R
JOIN Cultures C USING(Culture)
WHERE (R.Date>DATE('now','-'||(SELECT CAST(Valeur AS INT) FROM Params WHERE Paramètre='C_historique_récolte')||' days'))OR
      (DATE(R.Date) ISNULL)
ORDER BY R.Date,C.Espèce,C.Planche,R.Culture;
UPDATE fda_schema SET tbl_type='View as table' WHERE (name='Récoltes__Saisies');

CREATE VIEW Récoltes_saison AS SELECT --- Récoltes avec calcul de la saison, en fonction de la période normale de récolte.
    CASE WHEN coalesce(C.terminée,'') NOT LIKE 'v%' -- Annuelle
         THEN C.Saison
         ELSE CAST(substr(R.Date,1,4)+ -- Vivace, saison = année de récolte,
              iif(CAST(strftime('%U',R.Date) AS INT)<coalesce(V.S_Récolte,I.S_Récolte,CAST(strftime('%U',C.Début_récolte) AS INT),0)
                                                                   -(52-coalesce(V.D_Récolte,I.D_Récolte,(C.Fin_récolte-C.Début_récolte)/7,0))/2,
                  -1, -- récolte année N+1 alors que début de récolte de la culture année N, la saison de récolte est N.
                  0) AS TEXT)
         END Saison, --- Saison à laquelle est affectée la récolte.
    -- CAST(strftime('%U',R.Date) AS INT) Sem_rec,
    R.*
FROM Récoltes R
JOIN Cultures C USING(Culture)
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Variétés V USING(Variété);

CREATE VIEW Variétés__cde_plants AS SELECT
    E.Famille,
    V.Espèce,
    V.Variété,
    CAST(round((SELECT sum(CASE WHEN C.Espacement>0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100 ELSE NULL END) FROM Cu_non_commencées C
                WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL)),
    2)AS REAL) Qté_nécess,
    V.Fournisseur,
    CASt((SELECT count() FROM Cu_non_commencées C WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL))AS INTEGER) Cu_non_commencées,
    CAST((SELECT sum(C.Longueur) FROM Cu_non_commencées C WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL))AS REAL) Long_planches,
    CAST((SELECT sum(C.Longueur*PL.Largeur) FROM Cu_non_commencées C LEFT JOIN Planches PL USING(Planche)
          WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL))AS REAL) Surface,
    V.Notes,
    E.Notes N_espèce,
    F.Notes N_famille
FROM Variétés V
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Familles F USING(Famille)
ORDER BY E.Famille,V.Espèce,V.Variété;

CREATE VIEW Variétés__inv_et_cde AS SELECT
    E.Famille,
    V.Espèce,
    V.Variété,
    CAST(round((SELECT sum(CASE WHEN C.Espacement>0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant/V.Nb_graines_g
                                ELSE C.Longueur*PL.Largeur*I.Dose_semis END) FROM Cu_non_commencées C
                                                                             LEFT JOIN ITP I USING(IT_plante)
                                                                             LEFT JOIN Planches PL USING(Planche)
                WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL)),2)AS REAL) Qté_nécess,
    V.Qté_stock,
    V.Qté_cde,
    CAST(round(max((SELECT sum(CASE WHEN C.Espacement>0 THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_plant/V.Nb_graines_g
                                    ELSE C.Longueur*PL.Largeur*I.Dose_semis
                                    END) FROM Cu_non_commencées C
                                         LEFT JOIN ITP I USING(IT_plante)
                                         LEFT JOIN Planches PL USING(Planche)
                    WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))
                   -coalesce(V.Qté_stock,0)-coalesce(V.Qté_cde,0),0),2)AS REAL) Qté_manquante,
    V.Fournisseur,
    V.Nb_graines_g,
    E.FG,
    CASt((SELECT count() FROM Cu_non_commencées C WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS INTEGER) Cu_non_commencées,
    CAST((SELECT sum(C.Longueur) FROM Cu_non_commencées C WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS REAL) Long_planches,
    CAST((SELECT sum(C.Longueur*PL.Largeur) FROM Cu_non_commencées C LEFT JOIN Planches PL USING(Planche)
          WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS REAL) Surface,
    CAST((SELECT round(sum(C.Longueur*C.Nb_rangs/C.Espacement*100) ) FROM Cu_non_commencées C
          WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS INTEGER) Nb_plants,
    V.Notes,
    E.Notes N_espèce,
    F.Notes N_famille
FROM Variétés V
LEFT JOIN Espèces E USING(Espèce)
LEFT JOIN Familles F USING(Famille)
ORDER BY E.Famille,V.Espèce,V.Variété;
UPDATE fda_schema SET readonly='x' WHERE (name='Variétés__inv_et_cde')AND(field_name IN('N_famille','N_espèce'));


