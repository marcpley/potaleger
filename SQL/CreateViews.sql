QString sDDLViews = QStringLiteral(R"#(
-- BEGIN TRANSACTION;

-- CREATE VIEW test AS
--     VALUES (Test('09-15','04-01')),
--     (Test('10-01','02-01')),
--     (Test('08-15','04-15')),
--     (Test('12-15','04-15'));

CREATE VIEW Info_Potaléger AS
    SELECT 1 N,
           'Version de la BDD' Info,
           '#DbVer#' Valeur;

CREATE VIEW Consommations__Saisies AS SELECT
       Co.ID,
       Co.Date,
       Co.Espèce,
       Co.Quantité,
       Co.Prix,
       Co.Destination,
       -- CAST(round(E.Inventaire
       --      +coalesce((SELECT sum(Quantité) FROM Récoltes Re WHERE (Re.Espèce=Co.Espèce)AND(Re.Date BETWEEN E.Date_inv AND Co.Date)),0)
       --      -coalesce((SELECT sum(Quantité) FROM Consommations Co2 WHERE (Co2.Espèce=Co.Espèce)AND(Co2.Date BETWEEN E.Date_inv AND Co.Date)),0),3)AS REAL) Stock,
       -- CAST(round((SELECT sum(Quantité) FROM Consommations Co3 WHERE (Co3.Espèce=Co.Espèce)AND
       --                                                         (Co3.Destination=Co.Destination)AND
       --                                                         (Co3.Date BETWEEN D.Date_RAZ AND Co.Date)),3)AS REAL) Sorties, Triggers marchent mal avec ces appels.
       Co.Notes
FROM Consommations Co
JOIN Espèces E USING(Espèce)
LEFT JOIN Destinations D USING(Destination)
WHERE (Co.Date>DATE('now','-'||(SELECT Valeur FROM Params WHERE Paramètre='Conso_historique')||' days'))OR
      (DATE(Co.Date) ISNULL) -- Détection de date incorecte
ORDER BY Co.Date,E.Espèce,D.Destination;

CREATE VIEW Destinations__conso AS SELECT
    D.Destination,
    D.Type,
    D.Adresse,
    D.Site_web,
    D.Date_RAZ,
    (SELECT sum(Quantité) FROM Consommations Co WHERE (Co.Destination=D.Destination)AND(Co.Date >= D.Date_RAZ)) Consommation,
    (SELECT sum(Quantité*Prix) FROM Consommations Co WHERE (Co.Destination=D.Destination)AND(Co.Date >= D.Date_RAZ)) Valeur,
    D.Active,
    D.Notes
FROM Destinations D;

CREATE VIEW Espèces__a AS SELECT
       Espèce,
       Famille,
       Rendement,
       Niveau,
       Favorable,
       Défavorable,
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
       Notes
FROM Espèces
WHERE Vivace ISNULL;

CREATE VIEW Espèces__inventaire AS SELECT
    E.Espèce,
    E.Date_inv,
    E.Inventaire,
    CAST(round((SELECT sum(Quantité) FROM Récoltes Re WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),3)AS REAL) Entrées,
    CAST(round((SELECT sum(Quantité) FROM Consommations Re WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),3)AS REAL) Sorties,
    CAST(round(E.Inventaire
               +coalesce((SELECT sum(Quantité) FROM Récoltes Re WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0)
               -coalesce((SELECT sum(Quantité) FROM Consommations Re WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0),3)AS REAL) Stock,
    E.Prix_kg,
    CAST(round((E.Inventaire
                +coalesce((SELECT sum(Quantité) FROM Récoltes Re WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0)
                -coalesce((SELECT sum(Quantité) FROM Consommations Re WHERE (Re.Espèce=E.Espèce)AND(Re.Date >= E.Date_inv)),0))*E.Prix_kg,3)AS REAL) Valeur,
    Notes
FROM Espèces E
WHERE E.Conservation NOT NULL;

CREATE VIEW Espèces__v AS SELECT
       Espèce,
       Famille,
       Rendement,
       Favorable,
       Défavorable,
       Taille,
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
       Notes
FROM Espèces
WHERE Vivace NOTNULL;

CREATE VIEW Fertilisants__inventaire AS SELECT
    F.Fertilisant,
    F.Date_inv,
    F.Inventaire,
    CAST(round((SELECT sum(Quantité) FROM Fertilisations Fe WHERE (Fe.Fertilisant=F.Fertilisant)AND(Fe.Date >= F.Date_inv)),3)AS REAL) Sorties,
    CAST(round(F.Inventaire
               -coalesce((SELECT sum(Quantité) FROM Fertilisations Fe WHERE (Fe.Fertilisant=F.Fertilisant)AND(Fe.Date >= F.Date_inv)),0),3)AS REAL) Stock,
    F.Prix_kg,
    CAST(round((F.Inventaire
               -coalesce((SELECT sum(Quantité) FROM Fertilisations Fe WHERE (Fe.Fertilisant=F.Fertilisant)AND(Fe.Date >= F.Date_inv)),0))*F.Prix_kg,3)AS REAL) Valeur,
    Notes
FROM Fertilisants F;

CREATE VIEW Variétés__inv_et_cde AS SELECT E.Famille,
       V.Espèce,
       V.Variété,
       CAST(round((SELECT sum(CASE WHEN C.Espacement>0
                              THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_trou/V.Nb_graines_g
                              ELSE C.Longueur*PL.Largeur*I.Dose_semis END)
                     FROM C_non_commencées C
                     LEFT JOIN ITP I USING(IT_plante)
                     LEFT JOIN Planches PL USING(Planche)
                     WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL)), 2)AS REAL) Qté_nécess,
       V.Qté_stock,
       V.Qté_cde,
       CAST(round(max((SELECT sum(CASE WHEN C.Espacement>0
                                  THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_trou/V.Nb_graines_g
                                  ELSE C.Longueur*PL.Largeur*I.Dose_semis END)
                         FROM C_non_commencées C
                         LEFT JOIN ITP I USING(IT_plante)
                         LEFT JOIN Planches PL USING(Planche)
                         WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL) )-coalesce(V.Qté_stock,0)-coalesce(V.Qté_cde,0),0), 2)AS REAL) Qté_manquante,
       V.Fournisseur,
       V.Nb_graines_g,
       E.FG,
       -- (SELECT count()
       --    FROM C_en_place C
       --   WHERE (C.Variété=V.Variété) ) C_en_place,
       CASt((SELECT count()
             FROM C_non_commencées C
             WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS INTEGER) C_non_commencées,
       CAST((SELECT sum(C.Longueur)
             FROM C_non_commencées C
             WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS REAL) Long_planches,
       CAST((SELECT sum(C.Longueur*PL.Largeur)
             FROM C_non_commencées C
             LEFT JOIN Planches PL USING(Planche)
             WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS REAL) Surface,
       CAST((SELECT round(sum(C.Longueur*C.Nb_rangs/C.Espacement*100) )
             FROM C_non_commencées C
             WHERE (C.Variété=V.Variété)AND(C.Date_semis NOTNULL))AS INTEGER) Nb_plants,
       V.Notes,
       E.Notes N_espèce,
       F.Notes N_famille
  FROM Variétés V
       LEFT JOIN Espèces E USING(Espèce)
       LEFT JOIN Familles F USING(Famille)
 ORDER BY E.Famille,
          V.Espèce,
          V.Variété;

CREATE VIEW Variétés__cde_plants AS SELECT
       E.Famille,
       V.Espèce,
       V.Variété,
       CAST(round((SELECT sum(CASE WHEN C.Espacement>0
                              THEN C.Longueur*C.Nb_rangs/C.Espacement*100
                              ELSE NULL END)
                     FROM C_non_commencées C
                     WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL)), 2)AS REAL) Qté_nécess,
       V.Fournisseur,
       CASt((SELECT count()
             FROM C_non_commencées C
             WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL))AS INTEGER) C_non_commencées,
       CAST((SELECT sum(C.Longueur)
             FROM C_non_commencées C
             WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL))AS REAL) Long_planches,
       CAST((SELECT sum(C.Longueur*PL.Largeur)
             FROM C_non_commencées C
             LEFT JOIN Planches PL USING(Planche)
             WHERE (C.Variété=V.Variété)AND(C.Date_semis ISNULL))AS REAL) Surface,
       V.Notes,
       E.Notes N_espèce,
       F.Notes N_famille
  FROM Variétés V
       LEFT JOIN Espèces E USING(Espèce)
       LEFT JOIN Familles F USING(Famille)
 ORDER BY E.Famille,
          V.Espèce,
          V.Variété;

CREATE VIEW ITP__Tempo AS SELECT I.IT_plante,
       I.Espèce,
       I.Type_planche,
       CASE WHEN I.Espèce NOTNULL AND I.Type_culture='Vivace' AND
                 (SELECT E.Vivace ISNULL FROM Espèces E WHERE E.Espèce=I.Espèce) THEN '!Espèce vivace ?'
            ELSE I.Type_culture
            END Type_culture,
       I.S_semis,
       I.S_plantation,
       I.S_récolte,
       I.D_récolte,
       I.Décal_max,

       SemToNJ(I.S_semis)||':'||
       SemToNJ(I.S_semis+coalesce(I.Décal_max,0.5))||':'||
       SemToNJ(I.S_plantation)||':'||
       SemToNJ(I.S_plantation+coalesce(I.Décal_max,0.5))||':'||
       SemToNJ(I.S_récolte)||':'||
       SemToNJ(I.S_récolte+coalesce(I.D_récolte+coalesce(I.Décal_max,0),0)) TEMPO, -- Si pas de récolte (D_récolte NULL) ne pas ajouter le décalage.

       CASE WHEN I.S_semis NOTNULL AND I.S_plantation NOTNULL
            THEN iif(I.S_plantation-I.S_semis<0,
                     I.S_plantation-I.S_semis+52,
                     I.S_plantation-I.S_semis)*7
            END J_pép,
       CASE WHEN coalesce(I.S_plantation,I.S_semis) NOTNULL AND I.S_récolte NOTNULL
            THEN iif(I.S_récolte-coalesce(I.S_plantation,I.S_semis)<0,
                     I.S_récolte-coalesce(I.S_plantation,I.S_semis)+52,
                     I.S_récolte-coalesce(I.S_plantation,I.S_semis))*7
            END J_en_pl,
       I.Nb_rangs,
       I.Espacement,
       I.Nb_graines_trou,
       I.Dose_semis,
       CASE WHEN I.Espacement>0
            THEN round(1.0/I.Espacement*100*I.Nb_rangs/(SELECT Valeur FROM Params WHERE Paramètre='Largeur_planches')/E.Densité*100)
            ELSE round(I.Dose_semis/E.Dose_semis*100)
            END Densité_pc,
       I.Notes,
       E.Notes N_espèce,
       E.Famille
FROM ITP I
LEFT JOIN Espèces E USING(Espèce);

CREATE VIEW Rotations_détails__Tempo AS SELECT
       R.ID,
       R.Rotation,
       R.Année,
       R.IT_plante,
       R.Pc_planches,
       R.Fi_planches,
       RI.Mise_en_place || (CASE
              WHEN -- Vérif que la culture n'est pas mise en place trop tôt par rapport à la culture antérieure dans la rotation.
                   (RI.Date_MEP < -- Mise en place au plus tôt de la culture courante
                    coalesce((SELECT Date_Ferm FROM R_ITP_CAnt(R.Rotation,RI.Date_MEP)), -- comparaison avec la fermeture au plus tôt de la culture antérieure (si la courante n'est pas la 1ère de la rotation)
                             (SELECT Date_Ferm FROM R_ITP_CDer(RI.Nb_années,R.Rotation)))) -- comparaison avec la fermeture au plus tôt de la dernière culture de la rotation
                    AND
                   -- et total d'occupation des planches en conflit supérieur à 100.
                   (RI.Pc_planches +
                    coalesce((SELECT Pc_planches FROM R_ITP_CAnt(R.Rotation,RI.Date_MEP)),
                             (SELECT Pc_planches FROM R_ITP_CDer(RI.Nb_années,R.Rotation))
                    )>100)
                    AND
                    -- et filtres de planche identiques ou nuls.
                    (RI.Fi_planches ISNULL OR
                     (coalesce((SELECT Fi_planches FROM R_ITP_CAnt(R.Rotation,RI.Date_MEP)),
                               (SELECT Fi_planches FROM R_ITP_CDer(RI.Nb_années,R.Rotation))) ISNULL) OR
                     (RI.Fi_planches =
                      coalesce((SELECT Fi_planches FROM R_ITP_CAnt(R.Rotation,RI.Date_MEP)),
                               (SELECT Fi_planches FROM R_ITP_CDer(RI.Nb_années,R.Rotation)))))
              THEN '*'
              -- Vérif que la culture n'est pas mise en place trop tard par rapport à la culture antérieure dans la rotation.
              WHEN RotDecalDateMeP(RI.Date_MEP) > -- Mise en place au plus tôt de la culture courante
                   coalesce(
                   (SELECT Date_Ferm FROM R_ITP_CAnt(R.Rotation,RI.Date_MEP)), -- comparaison avec la fermeture au plus tôt de la culture antérieure (si la courante n'est pas la 1ère de la rotation)
                   (SELECT Date_Ferm FROM R_ITP_CDer(RI.Nb_années,R.Rotation))) -- comparaison avec la fermeture au plus tôt de la dernière culture de la rotation
              THEN '-'
              ELSE ''
              END) Mise_en_place,

       SemToNJ(RI.S_semis-iif(RI.S_semis>RI.S_plantation,52,0))||':'||
       SemToNJ(RI.S_semis+coalesce(RI.Décal_max,0.5)-iif(RI.S_semis>RI.S_plantation,52,0))||':'||
       SemToNJ(RI.S_plantation)||':'||
       SemToNJ(RI.S_plantation+coalesce(RI.Décal_max,0.5))||':'||
       SemToNJ(RI.S_récolte)||':'||
       SemToNJ(RI.S_récolte+coalesce(RI.D_récolte+coalesce(RI.Décal_max,0),0)) TEMPO, -- Si pas de récolte (D_récolte NULL) ne pas ajouter le décalage.

       RI.A_planifier,
       CASE WHEN (RI.Type_planche NOTNULL) AND
                 (RI.Type_planche<>(SELECT Type_planche
                                      FROM Rotations
                                     WHERE Rotation=R.Rotation) ) THEN RI.Type_planche END Conflit_type_planche,
       CASE WHEN R.IT_plante NOTNULL
            -- Recherche de familles trop proches dans une rotation.
            THEN (SELECT group_concat(result,x'0a') FROM RF_trop_proches(R.Rotation,R.Année,(SELECT Famille FROM R_famille F WHERE F.ID=R.ID))) || x'0a0a'
            END ||
       'Familles possibles : ' || coalesce((SELECT group_concat(Famille,', ') FROM (
                                     SELECT DISTINCT RF.Famille
                                     FROM R_famille RF
                                     WHERE (RF.Rotation=R.Rotation)AND -- ITP dans la même rotation
                                           (RF.Année<>R.Année)AND -- ITP des autres années de la rotation
                                           NOT (R.Année BETWEEN RF.Année-RF.Intervalle+1 AND RF.Année+RF.Intervalle-1)AND -- pas en conflit lors du 1er cycle
                                           NOT (R.Année BETWEEN RF.Année-RF.Intervalle+1+RF.Nb_années AND RF.Année+RF.Intervalle-1+RF.Nb_années)
                                     UNION
                                     SELECT F.Famille FROM F_actives F
                                     WHERE NOT(F.Famille IN(SELECT Famille FROM R_famille F2 WHERE F2.Rotation=R.Rotation))
                                     )),
                                     'aucune') Conflit_famille,
       RI.Famille,
       RI.Intervalle,
       R.Notes
  FROM Rotations_détails R
       LEFT JOIN R_ITP RI USING(ID)
 ORDER BY RI.Ind;

CREATE VIEW Planches_Décalées AS SELECT
   PL.Planche,
   PL.Rotation,
   PL.Année
   +(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1-R.Année_1
   -(R.Nb_années*floor(((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+0-R.Année_1+PL.Année)/R.Nb_années)) Année,
   PL.Longueur,
   PL.Largeur
FROM Planches PL
     JOIN Rotations R USING(Rotation);

CREATE VIEW Cult_planif AS SELECT
   PL.Planche,
   E.Espèce,
   RD.IT_plante,
   (SELECT V.Variété FROM Variétés V WHERE V.Espèce=I.Espèce ORDER BY V.Qté_stock DESC) Variété, -- Les IT des plans de rotation ont toujours une Espèce.
   (SELECT V.Fournisseur FROM Variétés V WHERE V.Espèce=I.Espèce ORDER BY V.Qté_stock DESC) Fournisseur,
   -- CAST((SELECT Valeur FROM Params WHERE Paramètre='Année_planif')AS INTEGER) Année_à_planifier,
   PlanifCultureCalcDate(CASE WHEN (I.S_plantation NOTNULL) AND (I.S_plantation<I.S_semis)
                              THEN (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
                              ELSE (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
                              END,
                         I.S_semis) Date_semis,
   PlanifCultureCalcDate(coalesce(PlanifCultureCalcDate(CASE WHEN (I.S_plantation NOTNULL) AND (I.S_plantation<I.S_semis)
                                                        THEN (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
                                                        ELSE (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
                                                        END,
                                                        I.S_semis),
                                  (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture') ||'-01-01'),
                         I.S_plantation) Date_plantation,
   CAST(round((RD.Pc_planches/100*PL.Longueur),2)AS REAL) Longueur,
   I.Nb_rangs,
   I.Espacement,
   CAST(round(RD.Pc_planches/100*PL.Longueur*PL.Largeur,2)AS REAL) Surface,
   CAST(round(RD.Pc_planches/100*PL.Longueur*PL.Largeur*E.Rendement,2)AS REAL) Prod_possible,
   -- CAST((SELECT Valeur FROM Params WHERE Paramètre='Année_planif')AS INTEGER) Année_à_planifier
   'Simulation planif' Info
FROM Rotations_détails RD
     JOIN Rotations R USING(Rotation)
     JOIN Planches_Décalées PL USING(Rotation,Année)
     JOIN ITP I USING(IT_plante)
     LEFT JOIN Espèces E USING(Espèce)
WHERE ((SELECT (Valeur ISNULL) FROM Params WHERE Paramètre='Planifier_planches') OR
        (PL.Planche LIKE (SELECT Valeur FROM Params WHERE Paramètre='Planifier_planches')||'%')) AND
       ((RD.Fi_planches ISNULL) OR
        (Fi_planches LIKE '%'||substr(PL.Planche, -1, 1) ||'%'))
ORDER BY coalesce(Date_plantation, Date_semis);

CREATE VIEW Cult_planif_2 AS SELECT
   C.Planche,
   C.Espèce,
   C.IT_plante,
   C.Variété,
   C.Fournisseur,
   C.Date_semis,
   C.Date_plantation,
   CASE WHEN I.D_récolte NOTNULL
        THEN  PlanifCultureCalcDate(coalesce(C.Date_plantation,C.Date_semis),I.S_récolte)
        ELSE NULL
        END Début_récolte,
   CASE WHEN I.D_récolte NOTNULL
        THEN DATE(PlanifCultureCalcDate(coalesce(C.Date_plantation,C.Date_semis),I.S_récolte),'+'||(I.D_récolte*7)||' days')
        ELSE PlanifCultureCalcDate(coalesce(C.Date_plantation,C.Date_semis),I.S_récolte)
        END Fin_récolte,
   C.Longueur,
   C.Nb_rangs,
   C.Espacement,
   C.Surface,
   C.Prod_possible,
   C.Info
FROM Cult_planif C
JOIN ITP I USING(IT_plante)
WHERE I.S_récolte NOTNULL;

CREATE VIEW Cult_planif_récoltes AS
WITH RECURSIVE Semaines AS (
    SELECT
        Planche||Espèce||Début_récolte Id,
        Espèce,
        Début_récolte Date_jour,
        Fin_récolte,
        Prod_possible,
        0 Offset
    FROM Cult_planif_2 WHERE (Début_récolte NOTNULL)AND(Fin_récolte NOTNULL)
    UNION ALL
    SELECT
        Id,
        Espèce,
        date(Date_jour, '+7 day') Date_jour,
        Fin_récolte,
        Prod_possible,
        Offset + 1
    FROM Semaines
    WHERE Date_jour < Fin_récolte
),
Durées AS (
    SELECT
        Id,
        count() Nb_semaines
    FROM Semaines
    GROUP BY Id
)
SELECT
    S.Id,
    S.Espèce,
    S.Date_jour Date,
    CAST(
    CASE WHEN D.Nb_semaines=1 THEN S.Prod_possible
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

CREATE VIEW Cult_planif_ilots AS SELECT
    substr(C.Planche,1,(SELECT Valeur FROM Params WHERE Paramètre='Ilot_nb_car')) Ilot,
    C.Espèce,
    min(Date_semis) Date_semis,
    min(Date_plantation) Date_plantation,
    CAST(count() AS INTEGER) Nb_planches,
    CAST(sum(C.Longueur)AS REAL) Longueur,
    CAST(sum(C.Longueur*C.Nb_rangs)AS REAL) Long_rang,
    CAST(sum(C.Longueur*C.Nb_rangs/C.Espacement/100) AS INTEGER) Nb_plants,
    CAST(sum(C.Surface)AS REAL) Surface,
    CAST(sum(C.Prod_possible)AS REAL) Prod_possible,
    -- CAST((SELECT Valeur FROM Params WHERE Paramètre='Année_planif')AS INTEGER) Année_à_planifier
    'Simulation planif' Info
FROM Cult_planif C
GROUP BY Ilot,Espèce
ORDER BY Ilot,Espèce;

CREATE VIEW Cult_planif_espèces AS SELECT
    C.Espèce,
    CAST(count() AS INTEGER) Nb_planches,
    CAST(sum(C.Longueur)AS REAL) Longueur,
    CAST(sum(C.Longueur*C.Nb_rangs)AS REAL) Long_rang,
    CAST(sum(C.Longueur*C.Nb_rangs/C.Espacement/100) AS INTEGER) Nb_plants,
    CAST(sum(C.Surface)AS REAL) Surface,
    E.Rendement,
    E.Obj_annuel,
    CAST(sum(C.Prod_possible)AS REAL) Prod_possible,
    round(sum(C.Prod_possible)/E.Obj_annuel*100) Couv_pc,
    -- CAST((SELECT Valeur FROM Params WHERE Paramètre='Année_planif')AS INTEGER) Année_à_planifier
    'Simulation planif' Info
FROM Cult_planif C
LEFT JOIN Espèces E USING(Espèce)
GROUP BY Espèce
ORDER BY Espèce;

CREATE VIEW Cultures__Tempo AS SELECT
    dense_rank() OVER (ORDER BY Planche) AS num_planche,
    Planche,
    Culture,
    Variété_ou_It_plante,
    (TEMPO_NP || CASE WHEN Planche!=Lag THEN coalesce(Irrig,'') ELSE iif(Irrig NOTNULL,'x','') END) TEMPO_NP,
    (TEMPO_N  || CASE WHEN Planche!=Lag THEN coalesce(Irrig,'') ELSE iif(Irrig NOTNULL,'x','') END) TEMPO_N,
    (TEMPO_NN || CASE WHEN Planche!=Lag THEN coalesce(Irrig,'') ELSE iif(Irrig NOTNULL,'x','') END) TEMPO_NN,
    Saison,
    Longueur,
    Nb_rangs,
    Surface,
    A_faire,
    Notes
FROM Cultures__Tempo2;

CREATE VIEW Cultures__Tempo2 AS SELECT
       -- dense_rank() OVER (ORDER BY Planche) AS num_planche,
       -- row_number() OVER (ORDER BY Planche) AS num_planche2,
       -- rank() OVER (ORDER BY Planche) AS num_planche3,
       C.Planche,
       lag(Planche) OVER (ORDER BY Planche,coalesce(C.Date_plantation,C.Date_semis)) AS Lag,
       C.Culture,
       coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante,
       -- C.Type,
       coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
       CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')-1)AS TEXT)
            THEN CulTempo(julianday(substr(coalesce(C.Date_plantation,C.Date_semis),1,4)||'-01-01'),
                          C.Date_semis,C.Date_plantation,C.Début_récolte,C.Fin_récolte)||':'||E.Espèce||':'||
                 coalesce(C.Semis_fait,'')||':'||coalesce(C.Plantation_faite,'')||':'||coalesce(C.Récolte_faite,'')||':'||coalesce(C.Terminée,'')||':'
            ELSE ':::::::::::' END TEMPO_NP,
       CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
            THEN CulTempo(julianday(substr(coalesce(C.Date_plantation,C.Date_semis),1,4)||'-01-01'),
                          C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte)||':'||E.Espèce||':'||
                 coalesce(C.Semis_fait,'')||':'||coalesce(C.Plantation_faite,'')||':'||coalesce(C.Récolte_faite,'')||':'||coalesce(C.Terminée,'')||':'
            ELSE ':::::::::::' END TEMPO_N,
       CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)AS TEXT)
            THEN CulTempo(julianday(substr(coalesce(C.Date_plantation,C.Date_semis),1,4)||'-01-01'),
                          C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte)||':'||E.Espèce||':'||
                 coalesce(C.Semis_fait,'')||':'||coalesce(C.Plantation_faite,'')||':'||coalesce(C.Récolte_faite,'')||':'||coalesce(C.Terminée,'')||':'
            ELSE ':::::::::::' END TEMPO_NN,
       C.Saison,
       C.Longueur,
       C.Nb_rangs,
       C.Longueur*PL.Largeur Surface,
       PL.Irrig,
       E.Irrig Irrig_E,
       A_faire,
       C.Notes
  FROM Cultures C
       LEFT JOIN Espèces E USING(Espèce)
       -- LEFT JOIN ITP I USING(IT_plante)
       LEFT JOIN Planches PL USING(Planche)
 WHERE (C.Planche NOTNULL)AND
       (coalesce(C.Terminée,'') NOT LIKE 'v%')AND -- Pas de succession pour les vivaces.
       ((coalesce(C.Date_plantation,C.Date_semis) BETWEEN ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')-1)||'-01-01' AND
                                                           ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)||'-12-31'))--OR
        -- (C.Fin_récolte BETWEEN ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')-1)||'-01-01' AND
        --                        ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)||'-12-31'))
ORDER BY C.Planche,
         coalesce(C.Date_plantation,C.Date_semis);

CREATE VIEW Cultures__non_terminées AS SELECT
       C.Planche,
       C.Culture,
       C.Espèce,
       C.IT_plante,
       C.Variété,
       C.Fournisseur,
       C.Type,
       C.Saison,
       CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                 ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!=
                  (SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété)) THEN 'Err. variété !'
            ELSE C.Etat
            END Etat,
       C.D_planif,
       C.Date_semis,
       C.Semis_fait,
       C.Date_plantation,
       C.Plantation_faite,
       C.Début_récolte,
       C.Fin_récolte,
       C.Récolte_faite,
       CAST(round((SELECT sum(Quantité) FROM Recoltes_cul(C.Culture,C.Terminée,C.Début_récolte,C.Fin_récolte)),3)AS REAL) Qté_réc,
       C.Terminée,
       C.Longueur,
       C.Nb_rangs,
       C.Espacement,
       I.Nb_graines_trou,
       I.Dose_semis,
       CAST((C.Longueur * C.Nb_rangs / C.Espacement * 100)AS INTEGER) Nb_plants,
       CASE WHEN C.Espacement>0
            THEN round(1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100)
            ELSE round(I.Dose_semis/E.Dose_semis*100)
            END Densité_pc,
       C.A_faire,
       C.Notes,
       I.Notes N_IT_plante,
       PL.Notes N_Planche
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE NOT CulTer(C.Terminée)
ORDER BY    C.Planche,
            coalesce(C.Date_semis,C.Date_plantation),
            C.Espèce,C.IT_plante;

CREATE VIEW Cultures__à_semer AS SELECT
        C.Planche,
        C.Culture,
        C.Espèce,
        C.IT_plante,
        C.Variété,
        C.Fournisseur,
        C.Type,
        CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                  ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!=
                   (SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété)) THEN 'Err. variété !'
             ELSE C.Etat
             END Etat,
        C.D_planif,
        C.Date_semis,
        C.Semis_fait,
        C.Date_plantation,
        C.Début_récolte,
        C.Longueur,
        PL.Largeur,
        C.Nb_rangs,
        C.Espacement,
        I.Nb_graines_trou,
        I.Dose_semis,
        CAST((C.Longueur * C.Nb_rangs / C.Espacement * 100)AS INTEGER) Nb_plants,
       CASE WHEN C.Espacement>0
            THEN round(1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100)
            ELSE round(I.Dose_semis/E.Dose_semis*100)
            END Densité_pc,
        CAST((C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou)AS INTEGER) Nb_graines,
        CAST(round((CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou / E.Nb_graines_g
                    ELSE C.Longueur * PL.Largeur * I.Dose_semis END),2)AS REAL) Poids_graines,
        C.A_faire,
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   (NOT CulTer(Terminée)) AND
        (Date_semis < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_semis')||' days')) AND
        (coalesce(Semis_fait,'') NOT LIKE 'x%')
ORDER BY    C.Date_semis,C.Date_plantation,
            C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Cultures__à_semer_pep AS SELECT
        CAST(min(C.Culture)AS INTEGER) Culture, -- Pour servir d'index, colonne à cacher.
        group_concat(C.Planche,x'0a0a') Planches,
        group_concat(' '||C.Culture||' ',x'0a0a') Cultures,
        C.Espèce,
        C.IT_plante,
        C.Variété,
        C.Type,
        C.Etat,
        C.Date_semis,
        CASE WHEN sum(C.Semis_fait)>0 THEN sum(C.Semis_fait) ELSE min(C.Semis_fait) END Semis_fait,
        CASE WHEN min(C.Date_plantation)=max(C.Date_plantation) THEN strftime('%d/%m/%Y',min(C.Date_plantation))
        ELSE group_concat(strftime('%d/%m/%Y',C.Date_plantation),x'0a0a')
        END Dates_plantation,
        I.Nb_graines_trou,
        I.Dose_semis,
        group_concat(CAST(C.Longueur AS INTEGER)||'m x '||PL.Largeur||'m - '||
                     CAST(C.Nb_rangs AS INTEGER)||'rg, esp'||CAST(C.Espacement AS INTEGER)||'cm',x'0a0a') Rangs_espacement,
        CAST(sum((C.Longueur * C.Nb_rangs / C.Espacement * 100))AS INTEGER) Nb_plants,
        CAST(sum((C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou))AS INTEGER) Nb_graines,
        CAST(round(sum((CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou / E.Nb_graines_g
                        ELSE C.Longueur * PL.Largeur * I.Dose_semis END)),2)AS REAL) Poids_graines,
        min(C.A_faire) A_faire,
        min(C.Notes) Notes,
        I.Notes N_IT_plante,
        -- PL.Notes N_planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   (NOT CulTer(Terminée)) AND
        (Date_semis < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_semis')||' days')) AND
        (coalesce(Semis_fait,'') NOT LIKE 'x%') AND
        (Date_plantation NOTNULL)
GROUP BY C.Espèce,C.IT_plante,C.Variété,C.Type,C.Etat,C.Date_semis --,C.Semis_fait
ORDER BY    C.Date_semis,C.Date_plantation,
            C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Cultures__à_semer_EP AS SELECT
        C.Planche,
        C.Culture,
        C.Espèce,
        C.IT_plante,
        C.Variété,
        C.Fournisseur,
        C.Type,
        CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                  ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!=
                   (SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété)) THEN 'Err. variété !'
             ELSE C.Etat
             END Etat,
        C.D_planif,
        C.Date_semis,
        C.Semis_fait,
        C.Début_récolte,
        C.Longueur,
        PL.Largeur,
        C.Nb_rangs,
        C.Espacement,
        I.Nb_graines_trou,
        I.Dose_semis,
        CAST((C.Longueur * C.Nb_rangs / C.Espacement * 100)AS INTEGER) Nb_plants,
       CASE WHEN C.Espacement>0
            THEN round(1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100)
            ELSE round(I.Dose_semis/E.Dose_semis*100)
            END Densité_pc,
        CAST((C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou)AS INTEGER) Nb_graines,
        CAST(round((CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou / E.Nb_graines_g
                    ELSE C.Longueur * PL.Largeur * I.Dose_semis END),2)AS REAL) Poids_graines,
        C.A_faire,
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   (NOT CulTer(Terminée)) AND
        (Date_semis < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_semis')||' days')) AND
        (coalesce(Semis_fait,'') NOT LIKE 'x%') AND
        (Date_plantation ISNULL)
ORDER BY C.Date_semis,C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Cultures__à_planter AS SELECT
        C.Planche,
        C.Culture,
        C.Espèce,
        C.IT_plante,
        C.Variété,
        C.Fournisseur,
        C.Type,
        CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                  ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=C.IT_plante)!=
                   (SELECT V.Espèce FROM Variétés V WHERE V.Variété=C.Variété)) THEN 'Err. variété !'
             ELSE C.Etat
             END Etat,
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
        CAST((C.Longueur * C.Nb_rangs / C.Espacement * 100)AS INTEGER) Nb_plants,
        round(1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100) Densité_pc,
        C.A_faire,
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   (NOT CulTer(Terminée)) AND
        (Date_plantation < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_plantation')||' days')) AND
        -- (Plantation_faite ISNULL) AND
        (coalesce(Plantation_faite,'') NOT LIKE 'x%') AND
        ((Semis_fait NOTNULL)OR(Date_semis ISNULL))
ORDER BY    C.Date_plantation,
            C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Cultures__à_récolter AS SELECT
        C.Planche,
        C.Culture,
        C.Espèce,
        coalesce(C.Variété,C.IT_plante) Variété_ou_It_plante,
        C.Type,
        C.Etat,
        C.Date_semis,
        C.Semis_fait,
        C.Date_plantation,
        C.Plantation_faite,
        C.Début_récolte,
        C.Fin_récolte,
        C.Récolte_faite,
        CAST(round((SELECT sum(Quantité) FROM Recoltes_cul(C.Culture,C.Terminée,C.Début_récolte,C.Fin_récolte)),3)AS REAL) Qté_réc,
        C.Terminée,
        C.Longueur,
        C.Nb_rangs,
        C.Espacement,
        CAST((C.Longueur * C.Nb_rangs / C.Espacement * 100)AS INTEGER) Nb_plants,
        C.A_faire,
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_Planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   (NOT CulTer(C.Terminée)) AND
        (Début_récolte < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_récolte')||' days')) AND
        (coalesce(Récolte_faite,'') NOT LIKE 'x%')  AND
        ((Semis_fait NOTNULL)OR(Date_semis ISNULL)) AND
        ((Plantation_faite NOTNULL)OR(Date_plantation ISNULL))
ORDER BY    C.Début_récolte,
            C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Récoltes__Saisies AS SELECT
       R.ID,
       R.Date,
       C.Espèce,
       R.Culture,
       R.Quantité,
       NULL Répartir,
       C.Planche,
       C.Variété,
       -- CAST(round((SELECT sum(Quantité) FROM Récoltes R2 WHERE (R2.Culture=R.Culture)AND(R2.Date<=R.Date)),3)AS REAL) Qté_réc, Triggers marchent mal avec cet appel.
       R.Notes
FROM Récoltes R
JOIN Cultures C USING(Culture)
WHERE (R.Date>DATE('now','-'||(SELECT Valeur FROM Params WHERE Paramètre='C_historique_récolte')||' days'))OR
      (DATE(R.Date) ISNULL) -- Détection de date incorecte
ORDER BY R.Date,C.Espèce,R.Culture;

CREATE VIEW Cultures__à_terminer AS SELECT
        C.Planche,
        C.Culture,
        coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante,
        C.Type,
        C.Etat,
        C.Date_semis,
        C.Date_plantation,
        C.Début_récolte,
        C.Fin_récolte,
        C.Récolte_faite,
        C.Terminée,
        C.Longueur,
        C.A_faire,
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_Planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   C.Terminée ISNULL AND -- (NOT CulTer(C.Terminée)) AND Les vivaces ne sont jamais à terminer.
        ((C.Fin_récolte ISNULL)OR(C.Fin_récolte < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_terminer')||' days'))) AND
        ((Semis_fait NOTNULL)OR(Date_semis ISNULL)) AND
        ((Plantation_faite NOTNULL)OR(Date_plantation ISNULL)) AND
        ((Récolte_faite NOTNULL)OR(Début_récolte ISNULL))
ORDER BY    C.Fin_récolte,
            C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Cultures__A_faire AS SELECT
        C.Planche,
        C.Culture,
        C.Espèce,
        coalesce(C.Variété,C.IT_plante) Variété_ou_It_plante,
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

CREATE VIEW Cultures__vivaces AS SELECT
        C.Planche,
        C.Culture,
        coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante,
        -- C.Fournisseur,
        -- C.Type,
        C.Etat,
        C.D_planif,
        coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
        C.Début_récolte,
        C.Fin_récolte,
        C.Récolte_faite,
        C.Terminée,
        -- C.Longueur,
        C.A_faire,
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_Planche,
        E.Irrig Irrig_espèce,
        E.Favorable,
        E.Défavorable,
        E.Taille,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE  (C.Terminée LIKE 'v%')OR(E.Vivace NOTNULL)
ORDER BY C.Planche,
         coalesce(C.Variété,C.IT_plante);

CREATE VIEW Cultures__à_fertiliser AS SELECT
       group_concat(C.Planche,x'0a0a') Planches,
       group_concat(C.Culture||' ',x'0a0a') Cultures,
       C.Espèce,
       group_concat(C.Variété_ou_It_plante,x'0a0a') Variétés_ou_It_plante,
       Type,
       CASE WHEN C.Etat='Prévue' OR C.Etat='Pépinière' THEN 'A venir' ELSE C.Etat END Etat,
       C.Date_MEP,
       min(C.Début_récolte) Début_récolte,
       max(C.Fin_récolte) Fin_récolte,
       max(C.Pl_libre_le) Pl_libre_le,
       sum(C.Surface) Surface,
       CAST(round(min(sum(coalesce(C.N_fert,0))/sum(C.N_esp*C.Surface*C.Densité_pc/100),
                      sum(coalesce(C.P_fert,0))/sum(C.P_esp*C.Surface*C.Densité_pc/100),
                      sum(coalesce(C.K_fert,0))/sum(C.K_esp*C.Surface*C.Densité_pc/100))*100) AS INTEGER) Fert_pc,
       CAST(round(sum(C.N_esp*C.Surface*C.Densité_pc/100)) AS INTEGER)||'-'||
       CAST(round(sum(C.P_esp*C.Surface*C.Densité_pc/100)) AS INTEGER)||'-'||
       CAST(round(sum(C.K_esp*C.Surface*C.Densité_pc/100)) AS INTEGER) Besoins_NPK,
       C.★N_esp,
       C.★P_esp,
       C.★K_esp,
       (CAST(A.N AS TEXT)||'-'||CAST(A.P AS TEXT)||'-'||CAST(A.K AS TEXT))||' ('||A.Analyse||')'||x'0a0a'||
        coalesce(A.Interprétation,'Pas d''interprétation') Analyse_sol,
       A.☆N ☆N_sol,
       A.☆P ☆P_sol,
       A.☆K ☆K_sol,
       CAST(round(sum(C.N_fert)) AS INTEGER)||'-'||
       CAST(round(sum(C.P_fert)) AS INTEGER)||'-'||
       CAST(round(sum(C.K_fert)) AS INTEGER) Apports_NPK,
       round(sum(C.N_esp*C.Surface*C.Densité_pc/100)-sum(coalesce(C.N_fert,0))) N_manq,
       round(sum(C.P_esp*C.Surface*C.Densité_pc/100)-sum(coalesce(C.P_fert,0))) P_manq,
       round(sum(C.K_esp*C.Surface*C.Densité_pc/100)-sum(coalesce(C.K_fert,0))) K_manq,
       -- C.Fertilisant,
       C.A_faire,
       C.Notes,
       C.N_Planche,
       C.N_espèce
FROM C_à_fertiliser C
LEFT JOIN Analyses_de_sol A USING(Analyse)
GROUP BY C.Ilot,C.Espèce,C.Type,C.Etat,C.Date_MEP,Analyse_sol,C.A_faire,C.Notes,C.N_Planche,C.N_espèce;
-- ORDER BY C.Date_MEP;

CREATE VIEW C_à_fertiliser AS SELECT
        substr(C.Planche,1,(SELECT Valeur FROM Params WHERE Paramètre='Ilot_nb_car')) Ilot,
        C.Planche,
        C.Culture,
        E.Espèce,
        coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante,
        C.Type,
        C.Etat,
        coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
        C.Début_récolte,
        C.Fin_récolte,
        (SELECT max(CEP2.Fin_récolte) FROM C_en_place CEP2
         WHERE (CEP2.Planche=C.Planche)AND --Cultures sur la même planche
               (CEP2.Début_récolte<coalesce(C.Date_plantation,C.Date_semis)) -- dont la récolte commence avant la MEP de la culture courante
               ) Pl_libre_le,
        C.Longueur,
        C.Longueur*PL.Largeur Surface,
       coalesce(CASE WHEN C.Espacement>0
                     THEN round(1.0/C.Espacement*100*C.Nb_rangs/PL.Largeur/E.Densité*100)
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
        (SELECT CF.Fertilisant FROM Cu_Fertilisants CF WHERE CF.Culture=C.Culture) Fertilisant,
        (SELECT CF.Quantité FROM Cu_Fertilisants CF WHERE CF.Culture=C.Culture) Quantité,
        C.A_faire,
        C.Notes,
        PL.Notes N_Planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE  (E.N+E.P+E.K>0)AND(
        -- Cultures prévues
       ((C.Culture IN(SELECT CAV.Culture FROM C_à_venir CAV)) AND
       (coalesce(C.Date_plantation,C.Date_semis) < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_fertiliser')||' days')))
       OR
       -- Cultures en place dont la récolte n'est pas commencée.
       ((C.Culture IN(SELECT CEP3.Culture FROM C_en_place CEP3)) AND (coalesce(C.Récolte_faite,'') NOT LIKE 'x%')))
ORDER BY coalesce(Date_plantation,Date_semis),C.Espèce,C.IT_plante;

CREATE VIEW Fertilisations__Saisies AS SELECT
       F.ID,
       F.Date,
       F.Espèce,
       F.Culture,
       F.Fertilisant,
       F.Quantité,
       NULL Répartir,
       F.N,
       F.P,
       F.K,
       C.Planche,
       C.Variété,
       F.Notes
FROM Fertilisations F
JOIN Cultures C USING(Culture)
WHERE (F.Date>DATE('now','-'||(SELECT Valeur FROM Params WHERE Paramètre='Ferti_historique')||' days'))OR
      (DATE(F.Date) ISNULL) -- Détection de date incorecte
ORDER BY F.Date,F.Espèce,F.Culture;

CREATE VIEW Cultures__à_irriguer AS SELECT
    dense_rank() OVER (ORDER BY CaI.Planche) AS num_planche,
    CaI.Planche,
    CaI.Culture,
    CaI.Variété_ou_It_plante,
    CaI.Irrig_planche,
    CaI.Irrig_espèce,
    CaI.TEMPO_NP,
    (CaI.TEMPO_N  || iif(CaI.Irrig_planche NOTNULL,'x','')) TEMPO_N,
    CaI.TEMPO_NN,
    CaI.Saison,
    CaI.Longueur,
    CaI.Nb_rangs,
    CaI.Surface,
    CaI.A_faire,
    CaI.Notes
FROM Cultures__à_irriguer2 CaI
WHERE  ((CaI.Planche IN (SELECT CaI1.Planche FROM C_à_irriguer CaI1))AND(CaI.Irrig_planche ISNULL))OR
       ((NOT (CaI.Planche IN (SELECT CaI2.Planche FROM C_à_irriguer CaI2)))AND(CaI.Irrig_planche NOTNULL));

CREATE VIEW Cultures__à_irriguer2 AS SELECT
       C.Planche,
       lag(Planche) OVER (ORDER BY Planche,coalesce(C.Date_plantation,C.Date_semis)) AS Lag,
       C.Culture,
       coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante,
       coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
       CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')-1)AS TEXT)
            THEN CulTempo(julianday(substr(coalesce(C.Date_plantation,C.Date_semis),1,4)||'-01-01'),
                          C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte)||':'||E.Espèce||':'||
                 coalesce(C.Semis_fait,'')||':'||coalesce(C.Plantation_faite,'')||':'||coalesce(C.Récolte_faite,'')||':'||coalesce(C.Terminée,'')||':'
            ELSE ':::::::::::' END TEMPO_NP,
       CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
            THEN CulTempo(julianday(substr(coalesce(C.Date_plantation,C.Date_semis),1,4)||'-01-01'),
                          C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte)||':'||E.Espèce||':'||
                 coalesce(C.Semis_fait,'')||':'||coalesce(C.Plantation_faite,'')||':'||coalesce(C.Récolte_faite,'')||':'||coalesce(C.Terminée,'')||':'
            ELSE ':::::::::::' END TEMPO_N,
       CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)AS TEXT)
            THEN CulTempo(julianday(substr(coalesce(C.Date_plantation,C.Date_semis),1,4)||'-01-01'),
                          C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte)||':'||E.Espèce||':'||
                 coalesce(C.Semis_fait,'')||':'||coalesce(C.Plantation_faite,'')||':'||coalesce(C.Récolte_faite,'')||':'||coalesce(C.Terminée,'')||':'
            ELSE ':::::::::::' END TEMPO_NN,
       C.Saison,
       C.Longueur,
       C.Nb_rangs,
       C.Longueur*PL.Largeur Surface,
       PL.Irrig Irrig_planche,
       E.Irrig Irrig_espèce,
       C.A_faire,
       C.Notes
  FROM Cultures C
       LEFT JOIN Espèces E USING(Espèce)
       -- LEFT JOIN ITP I USING(IT_plante)
       LEFT JOIN Planches PL USING(Planche)
 WHERE (C.Planche NOTNULL)AND
       (C.Terminée ISNULL)AND --(NOT CulTer(C.Terminée)) AND Que les annuelles non terminée.
       ((coalesce(C.Date_plantation,C.Date_semis) BETWEEN ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')-1)||'-01-01' AND
                                                          ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)||'-12-31'))--OR
ORDER BY C.Planche,
          coalesce(C.Date_plantation,C.Date_semis),C.Espèce,C.IT_plante;

CREATE VIEW C_à_irriguer AS SELECT
        C.Planche,
        C.Culture
FROM Cultures C
LEFT JOIN Espèces E USING (Espèce)
-- LEFT JOIN ITP I USING (IT_plante)
WHERE  (NOT CulTer(C.Terminée)) AND
       (E.Irrig NOTNULL) AND
       (coalesce(C.Date_plantation,C.Date_semis) < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_Irrig_avant_MEP')||' days'))AND
       (coalesce(C.Date_plantation,C.Date_semis) > DATE('now','-'||(SELECT Valeur FROM Params WHERE Paramètre='C_Irrig_après_MEP')||' days'));

CREATE VIEW ITP__analyse_a AS SELECT
       I.IT_plante,
       I.Type_culture,
       I.S_semis,
       I.S_plantation,
       I.S_récolte,
       CAST((SELECT count() FROM Cultures C WHERE (C.IT_plante=I.IT_plante) AND C.Terminée ISNULL)AS INTEGER) Nb_cu_NT,
       CAST((SELECT count() FROM Cultures C WHERE (C.IT_plante=I.IT_plante) AND C.Terminée NOTNULL)AS INTEGER) Nb_cu_T,
       CAST((SELECT count() FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_TS,
       (SELECT min(substr(C.Date_semis,6)) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) Min_semis,
       (SELECT max(substr(C.Date_semis,6)) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) Max_semis,
       (SELECT min(substr(C.Date_plantation,6)) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) Min_plantation,
       (SELECT max(substr(C.Date_plantation,6)) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) Max_plantation,
       (SELECT min(substr(C.Début_récolte,6)) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) Min_recolte,
       (SELECT max(substr(C.Fin_récolte,6)) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) Max_recolte,
       CAST(round((SELECT sum(R.Quantité)
                   FROM C_ITP_analyse_a C LEFT JOIN Récoltes R USING(Culture)
                   WHERE C.IT_plante=I.IT_plante)/
                  (SELECT count() FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante),3)AS REAL) Qté_réc_moy
FROM ITP I
WHERE (SELECT count() FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante)>0
ORDER BY IT_plante;

CREATE VIEW C_ITP_analyse_a AS SELECT
       Culture,
       IT_plante,
       Date_semis,
       Date_plantation,
       Début_récolte,
       Fin_récolte
FROM Cultures
WHERE (Terminée NOT LIKE '%NS')AND -- Cultures significative
      (Terminée NOTNULL)AND (Terminée NOT LIKE 'v%'); -- Annuelles terminées

CREATE VIEW ITP__analyse_v AS SELECT
       I.IT_plante,
       I.Type_culture,
       I.S_semis,
       I.S_plantation,
       I.S_récolte,
       CAST((SELECT count() FROM Cultures C WHERE (C.IT_plante=I.IT_plante) AND NOT CulTer(C.Terminée))AS INTEGER) Nb_cu_NT,
       CAST((SELECT count() FROM Cultures C WHERE (C.IT_plante=I.IT_plante) AND CulTer(C.Terminée))AS INTEGER) Nb_cu_T,
       CAST((SELECT count() FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_S,
       (SELECT min(substr(C.Date_semis,6)) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) Min_semis,
       (SELECT max(substr(C.Date_semis,6)) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) Max_semis,
       (SELECT min(substr(C.Date_plantation,6)) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) Min_plantation,
       (SELECT max(substr(C.Date_plantation,6)) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) Max_plantation,
       (SELECT min(substr(C.Début_récolte,6)) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) Min_recolte,
       (SELECT max(substr(C.Fin_récolte,6)) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) Max_recolte,
       (SELECT sum(CulNbRecoltesTheo(C.Terminée,C.Fin_récolte,
                                     coalesce(I.D_récolte*7,0),
                                     (SELECT min(R.Date) FROM Récoltes R WHERE R.Culture=C.Culture)))
        FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) Nb_récoltes,
       CAST(round((SELECT sum(R.Quantité)
                   FROM C_ITP_analyse_v C LEFT JOIN Récoltes R USING(Culture)
                   WHERE C.IT_plante=I.IT_plante)/
                  (SELECT sum(CulNbRecoltesTheo(C.Terminée,C.Fin_récolte,
                                                coalesce(I.D_récolte*7,0),
                                                (SELECT min(R.Date) FROM Récoltes R WHERE R.Culture=C.Culture)))
                   FROM C_ITP_analyse_v C
                   WHERE C.IT_plante=I.IT_plante),3)AS REAL) Qté_réc_moy
FROM ITP I
WHERE (SELECT count() FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante)>0
ORDER BY IT_plante;

CREATE VIEW C_ITP_analyse_v AS SELECT
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

CREATE VIEW Cultures__analyse AS SELECT
       C.Culture,
       C.IT_plante,
       coalesce(C.Variété,C.Espèce) Variétés_ou_Espèce,
       C.Planche,
       C.Type,
       C.Saison,
       iif(C.Semis_fait NOTNULL,C.Date_semis,NULL) Date_semis,
       iif(C.Plantation_faite NOTNULL,C.Date_plantation,NULL) Date_plantation,
       iif(C.Récolte_faite NOTNULL,C.Début_récolte,NULL) Début_récolte,
       iif(C.Récolte_faite NOTNULL,C.Fin_récolte,NULL) Fin_récolte,
       C.Longueur*PL.Largeur Surface,
       CAST(round((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc,
       CAST(round(((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture)/(C.Longueur*PL.Largeur)),3)AS REAL) Rendement_C,
       E.Rendement,
       CASE WHEN E.Rendement NOTNULL
       THEN round(((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture)/(C.Longueur*PL.Largeur))/E.Rendement*100)
       ELSE NULL END Couv_pc,
       CulTempo(julianday(substr(coalesce(C.Date_semis,C.Date_plantation),1,4)||'-01-01'),
                          C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte)||':'||E.Espèce||':'||
              coalesce(C.Semis_fait,'')||':'||coalesce(C.Plantation_faite,'')||':'||coalesce(C.Récolte_faite,'')||':'||coalesce(C.Terminée,'')||':' TEMPO,
       C.Notes
  FROM Cultures C
       LEFT JOIN Espèces E USING(Espèce)
       -- LEFT JOIN ITP I USING(IT_plante)
       LEFT JOIN Planches PL USING(Planche)
 WHERE (C.Type IN('Semis pépinière','Plant','Semis en place')) AND C.Terminée NOTNULL AND (C.Terminée NOT LIKE '%NS')
ORDER BY C.Espèce,C.IT_plante,
          coalesce(C.Date_semis, C.Date_plantation);

CREATE VIEW Cultures__inc_dates AS SELECT
       C.Culture,
       C.IT_plante,
       coalesce(C.Variété,C.Espèce) Variétés_ou_Espèce,
       C.Planche,
       C.Type,
       C.Etat,
       CulIncDates(substr(coalesce(C.Date_semis,C.Date_plantation),1,4),
                   coalesce(I.S_semis,I.S_plantation),
                   C.Date_semis,I.S_semis,coalesce(I.Décal_max,0),
                   C.Date_plantation,I.S_plantation,
                   C.Début_récolte,coalesce(V.S_récolte,I.S_récolte),
                   C.Fin_récolte,coalesce(V.D_récolte,I.D_récolte,0)) Incohérence,
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
WHERE (coalesce(C.Terminée,'') NOT LIKE '%NS')AND
      ((C.Type LIKE '%?')OR
       (CulIncDates(substr(coalesce(C.Date_semis,C.Date_plantation),1,4),
                    coalesce(I.S_semis,I.S_plantation),
                    C.Date_semis,I.S_semis,coalesce(I.Décal_max,0),
                    C.Date_plantation,I.S_plantation,
                    C.Début_récolte,coalesce(V.S_récolte,I.S_récolte),
                    C.Fin_récolte,coalesce(V.D_récolte,I.D_récolte,0)) NOTNULL))
ORDER BY Culture;

CREATE VIEW C_non_commencées AS --Cultures ni semées (SEP ou SPep) ni plantées.
    SELECT *
      FROM Cultures
     WHERE NOT CulTer(Terminée) AND Semis_fait ISNULL AND Plantation_faite ISNULL
     ORDER BY Planche;

CREATE VIEW C_à_venir AS --Cultures ni semées (SD) ni plantées.
    SELECT *
      FROM Cultures
     WHERE NOT CulTer(Terminée) AND
           NOT ( (Semis_fait NOTNULL AND Date_plantation ISNULL) OR-- SD semé
                 Plantation_faite NOTNULL)-- Plant ou SPep planté
     ORDER BY Planche;

CREATE VIEW C_en_place AS --Cultures semées (SD) ou plantées.
    SELECT *
      FROM Cultures
     WHERE NOT CulTer(Terminée) AND
           ( (Semis_fait NOTNULL AND Date_plantation ISNULL) OR-- SD semé
             Plantation_faite NOTNULL)-- Plant ou SPep planté
     ORDER BY Planche;

CREATE VIEW C_planif AS SELECT
       C.Culture,
       C.IT_plante,
       C.Variété,
       I.S_semis,
       I.S_plantation,
       coalesce(V.S_récolte,I.S_récolte) S_récolte,
       coalesce(V.D_récolte,I.D_récolte) D_récolte,
       I.Décal_max,
       CASE WHEN I.S_semis NOTNULL AND I.S_plantation NOTNULL
            THEN iif(I.S_plantation-I.S_semis<0,
                     I.S_plantation-I.S_semis+52,
                     I.S_plantation-I.S_semis)*7
            END J_pép,
       CASE WHEN coalesce(I.S_plantation,I.S_semis) NOTNULL AND  coalesce(V.S_récolte,I.S_récolte) NOTNULL
            THEN iif(coalesce(V.S_récolte,I.S_récolte)-coalesce(I.S_plantation,I.S_semis)<0,
                     coalesce(V.S_récolte,I.S_récolte)-coalesce(I.S_plantation,I.S_semis)+52,
                     coalesce(V.S_récolte,I.S_récolte)-coalesce(I.S_plantation,I.S_semis))*7
            END J_en_pl
FROM Cultures C
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Variétés V USING(Variété)
WHERE NOT CulTer(C.Terminée);

CREATE VIEW F_actives AS
    SELECT *
      FROM Familles F
     WHERE ( (SELECT count()
                FROM ITP
                     LEFT JOIN Espèces USING(Espèce)
               WHERE Espèces.Famille=F.Famille) >0);

CREATE VIEW Espèces__manquantes AS SELECT
    E.Espèce,
    E.Rendement,
    E.Niveau,
    -- CAST((SELECT sum(Qté_stock) FROM Variétés V WHERE V.Espèce=E.Espèce)AS INTEGER) Qté_stock,
    E.Obj_annuel,
    CAST((SELECT sum(Prod_possible) FROM Cult_planif_espèces C WHERE C.Espèce=E.Espèce)AS INTEGER) Prod_possible,
    round((SELECT sum(Prod_possible) FROM Cult_planif_espèces C WHERE C.Espèce=E.Espèce)/E.Obj_annuel*100) Couv_pc,
    CAST((SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture')AS INTEGER) Année_à_planifier
FROM Espèces E
WHERE (E.A_planifier NOTNULL) AND
      (NOT(Espèce IN(SELECT Espèce FROM Cult_planif_espèces))OR
       ((SELECT sum(Prod_possible) FROM Cult_planif_espèces C WHERE C.Espèce=E.Espèce)<E.Obj_annuel));


CREATE VIEW C_esp_prod AS SELECT
    C.Saison,
    C.Culture,
    C.Espèce,
    E.Rendement,
    E.Niveau,
    CASE WHEN C.Saison=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
         THEN E.Obj_annuel
         ELSE NULL END Obj_annuel,
    CAST(round((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc,
    CAST(round(C.Longueur*P.Largeur*E.Rendement,3)AS REAL) Qté_prév,
    -- CASE WHEN (CulTer(C.Terminée))OR(C.Récolte_faite LIKE 'x%') THEN 'x' ELSE NULL END Récolte_faite,
    E.Notes N_espèce
FROM Cultures C
JOIN Planches P USING(Planche)
JOIN Espèces E USING(Espèce)
WHERE (coalesce(C.Terminée,'') NOT LIKE 'v%')AND(E.A_planifier NOTNULL)AND(E.Vivace ISNULL);

CREATE VIEW Espèces__couverture AS SELECT
    C.Saison,
    C.Espèce,
    C.Rendement,
    C.Niveau,
    CASE WHEN C.Saison=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
         THEN CAST((SELECT sum(Qté_stock) FROM Variétés V WHERE V.Espèce=C.Espèce)AS INTEGER)
         ELSE NULL END Qté_stock,
    C.Obj_annuel,
    round(sum(C.Qté_prév)) Qté_prév,
    round(sum(C.Qté_prév)/C.Obj_annuel*100) Couv_prév_pc,
    round(sum(C.Qté_réc)) Qté_réc,
    round(sum(C.Qté_réc)/sum(C.Qté_prév)*100) Couv_réc_pc,
    C.N_espèce Notes
FROM C_esp_prod C
GROUP BY Saison,Espèce
ORDER BY Saison,Espèce;

CREATE VIEW Planches__deficit_fert AS SELECT
       P.Planche,
       P.Type,
       (SELECT group_concat(CAST(C.Culture AS TEXT)||' - '||coalesce(C.Variété,C.IT_plante,C.Espèce)||' - '||C.Etat,x'0a0a')
        FROM Cultures C WHERE (C.Planche=P.Planche)AND (NOT CulTer(C.Terminée))) Cultures,
       (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
        WHERE (PBF1.Planche=P.Planche)AND(PBF1.Saison=(SELECT Valeur-3 FROM Params WHERE Paramètre='Année_culture'))) Fert_Nm3_pc,
       (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
        WHERE (PBF1.Planche=P.Planche)AND(PBF1.Saison=(SELECT Valeur-2 FROM Params WHERE Paramètre='Année_culture'))) Fert_Nm2_pc,
       (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
        WHERE (PBF1.Planche=P.Planche)AND(PBF1.Saison=(SELECT Valeur-1 FROM Params WHERE Paramètre='Année_culture'))) Fert_Nm1_pc,
       (SELECT PBF1.Fert_pc FROM Planches__bilan_fert PBF1
        WHERE (PBF1.Planche=P.Planche)AND(PBF1.Saison=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture'))) Fert_N_pc
FROM Planches P
WHERE (P.Planche IN (SELECT PBF.Planche
                     FROM Planches__bilan_fert PBF
                     WHERE (PBF.Fert_pc<(SELECT Valeur FROM Params WHERE Paramètre='Déficit_fert'))AND
                           (PBF.Saison BETWEEN (SELECT Valeur-2 FROM Params WHERE Paramètre='Année_culture') AND
                                               (SELECT Valeur-1 FROM Params WHERE Paramètre='Année_culture'))));

CREATE VIEW Planches__bilan_fert AS SELECT
       P.Saison,
       P.Planche,
       P.Type,
       P.Surface,
       count(P.Culture) Nb_cu,
       group_concat(P.Culture||' - '||P.Variété_ou_It_plante||' - '||P.Etat||' - '||strftime('%d/%m/%Y',P.Date_MEP),x'0a0a') Cultures,
       CAST(round(sum(P.N_esp*P.Densité_pc/100)) AS INTEGER)||'-'||
       CAST(round(sum(P.P_esp*P.Densité_pc/100)) AS INTEGER)||'-'||
       CAST(round(sum(P.K_esp*P.Densité_pc/100)) AS INTEGER) Besoins_NPK,
       (CAST(A.N AS TEXT)||'-'||CAST(A.P AS TEXT)||'-'||CAST(A.K AS TEXT))||' ('||A.Analyse||')'||x'0a0a'||
        coalesce(A.Interprétation,'Pas d''interprétation') Analyse_sol,
       -- sum(N_esp) N_esp,
       group_concat(P.Fertilisants,x'0a0a') Fertilisants,
       CAST(round(sum(P.N_fert)) AS INTEGER)||'-'||
       CAST(round(sum(P.P_fert)) AS INTEGER)||'-'||
       CAST(round(sum(P.K_fert)) AS INTEGER) Apports_NPK,
       -- sum(N_fert) N_fert,
       CAST(round(min(sum(coalesce(P.N_fert,0))/sum(P.N_esp*P.Densité_pc/100),
                      sum(coalesce(P.P_fert,0))/sum(P.P_esp*P.Densité_pc/100),
                      sum(coalesce(P.K_fert,0))/sum(P.K_esp*P.Densité_pc/100))*100) AS INTEGER) Fert_pc,
       round(sum(P.N_esp*P.Densité_pc/100)-sum(P.N_fert)) N_manq,
       round(sum(P.P_esp*P.Densité_pc/100)-sum(P.P_fert)) P_manq,
       round(sum(P.K_esp*P.Densité_pc/100)-sum(P.K_fert)) K_manq,
       P.Notes
FROM Pl_bilan_fert P
LEFT JOIN Analyses_de_sol A USING(Analyse)
GROUP BY P.Saison,P.Planche;

CREATE VIEW Pl_bilan_fert AS SELECT
       C.Saison,
       P.Planche,
       P.Type,
       P.Longueur*P.Largeur Surface,
       P.Analyse,
       C.Culture,
       coalesce(C.Variété,C.IT_plante,C.Espèce) Variété_ou_It_plante,
       C.Etat,
       coalesce(C.Date_plantation,C.Date_semis) Date_MEP,
       coalesce(CASE WHEN C.Espacement>0
                     THEN round(1.0/C.Espacement*100*C.Nb_rangs/P.Largeur/E.Densité*100)
                     ELSE round(I.Dose_semis/E.Dose_semis*100)
                     END,100) Densité_pc,
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
-- WHERE C.Saison=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
WHERE coalesce(C.Terminée,'') NOT LIKE 'v%'
GROUP BY Culture
ORDER BY Planche,Date_MEP;

CREATE VIEW R_famille AS
    SELECT RD.ID,
           RD.Rotation,
           RD.Année,
           R.Nb_années,
           F.Famille,
           F.Intervalle
      FROM Rotations_détails RD
           LEFT JOIN Rotations R USING(Rotation)
           LEFT JOIN ITP I USING(IT_plante)
           LEFT JOIN Espèces E USING(Espèce)
           LEFT JOIN Familles F USING(Famille);

CREATE VIEW R_ITP AS
    SELECT R.Rotation||format('%i', 2001+R.Année) ||'-'||
                       coalesce(I.S_plantation, I.S_semis,52) ||
                       format('%i', 2001+R.Année+iif(coalesce(I.S_plantation, I.S_semis)>I.S_récolte, 1, 0) ) ||'-'||
                       coalesce(I.S_récolte,52)||'-'||
                       coalesce(R.Fi_planches,'')||'-'||I.Espèce Ind,
           R.ID,
           R.Rotation,
           RO.Nb_années,
           R.Année,
           R.IT_plante,
           R.Pc_planches,
           R.Fi_planches,
           I.Type_planche,
           I.Type_culture,
           coalesce(I.S_plantation, I.S_semis) Mise_en_place,
           date(format('%i', 2001+R.Année)||'-01-01',SemToNJ(coalesce(I.S_plantation, I.S_semis))-1||' days') Date_MEP,
           date(format('%i', 2001+R.Année+iif(coalesce(I.S_plantation,I.S_semis)>I.S_récolte,1,0))||'-01-01',SemToNJ(coalesce(I.S_récolte,52))-1||' days') Date_Ferm,
           I.S_semis,
           I.S_plantation,
           I.S_récolte,
           I.D_récolte,
           I.Décal_max,
           E.A_planifier,
           F.Famille,
           F.Intervalle
      FROM Rotations_détails R
           LEFT JOIN Rotations RO USING(Rotation)
           LEFT JOIN ITP I USING(IT_plante)
           LEFT JOIN Espèces E USING(Espèce)
           LEFT JOIN Familles F USING(Famille)
     ORDER BY Ind;

CREATE VIEW Planches_Ilots AS
    SELECT substr(P.Planche, 1, (SELECT Valeur
                                   FROM Params
                                  WHERE Paramètre='Ilot_nb_car') ) Ilot,
           P.Type,
           CAST(count() AS INTEGER) Nb_planches,
           CAST(round(sum(P.Longueur),2) AS REAL) Longueur,
           CAST(round(sum(P.Longueur*P.Largeur),2)AS REAL) Surface,
           P.Rotation-- group_concat(P.Rotation, '/') ROTATION
      FROM Planches P
     GROUP BY Ilot,
              Type,
              Rotation
     ORDER BY Ilot,
              Type,
              Rotation;

-- COMMIT TRANSACTION;
)#");
