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
    D.Adresse,
    D.Site_web,
    D.Date_RAZ,
    (SELECT sum(Quantité) FROM Consommations Co WHERE (Co.Destination=D.Destination)AND(Co.Date >= D.Date_RAZ)) Consommation,
    (SELECT sum(Quantité*Prix) FROM Consommations Co WHERE (Co.Destination=D.Destination)AND(Co.Date >= D.Date_RAZ)) Valeur,
    D.Active,
    D.Notes
FROM Destinations D;

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

CREATE VIEW Variétés__inv_et_cde AS SELECT E.Famille,
       V.Espèce,
       V.Variété,
       CAST(round((SELECT sum(CASE WHEN C.Espacement>0
                              THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_trou/V.Nb_graines_g
                              ELSE C.Longueur*PL.Largeur*I.Dose_semis END)
                     FROM C_non_commencées C
                     LEFT JOIN ITP I USING(IT_plante)
                     LEFT JOIN Planches PL USING(Planche)
                     WHERE C.Variété=V.Variété), 2)AS REAL) Qté_nécess,
       V.Qté_stock,
       V.Qté_cde,
       CAST(round(max((SELECT sum(CASE WHEN C.Espacement>0
                                  THEN C.Longueur*C.Nb_rangs/C.Espacement*100*I.Nb_graines_trou/V.Nb_graines_g
                                  ELSE C.Longueur*PL.Largeur*I.Dose_semis END)
                         FROM C_non_commencées C
                         LEFT JOIN ITP I USING(IT_plante)
                         LEFT JOIN Planches PL USING(Planche)
                         WHERE (C.Variété=V.Variété) )-coalesce(V.Qté_stock,0)-coalesce(V.Qté_cde,0),0), 2)AS REAL) Qté_manquante,
       V.Fournisseur,
       V.Nb_graines_g,
       E.FG,
       -- (SELECT count()
       --    FROM C_en_place C
       --   WHERE (C.Variété=V.Variété) ) C_en_place,
       CASt((SELECT count()
             FROM C_non_commencées C
             WHERE C.Variété=V.Variété)AS INTEGER) C_non_commencées,
       CAST((SELECT sum(C.Longueur)
             FROM C_non_commencées C
             WHERE C.Variété=V.Variété)AS REAL) Long_planches,
       CAST((SELECT round(sum(C.Longueur*C.Nb_rangs/C.Espacement*100) )
             FROM C_non_commencées C
             WHERE C.Variété=V.Variété)AS INTEGER) Nb_plants,
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
       I.Type_culture,
       I.Déb_semis,
       I.Fin_semis,
       I.Déb_plantation,
       I.Fin_plantation,
       I.Déb_récolte,
       I.Fin_récolte,
       ItpTempo(I.Type_culture,
                ItpTempoNJ(I.Déb_semis),
                ItpTempoNJ(I.Fin_semis),
                ItpTempoNJ(I.Déb_plantation),
                ItpTempoNJ(I.Fin_plantation),
                ItpTempoNJ(I.Déb_récolte),
                ItpTempoNJ(I.Fin_récolte)) TEMPO,
       CAST(ItpTempoNJPeriode(ItpTempoNJ(I.Déb_semis),ItpTempoNJ(I.Fin_semis),365) AS INTEGER) J_semis,
       CAST(ItpTempoNJPeriode(ItpTempoNJ(I.Déb_plantation),ItpTempoNJ(I.Fin_plantation),365) AS INTEGER) J_plantation,
       CAST(ItpTempoNJPeriode(ItpTempoNJ(I.Déb_récolte),ItpTempoNJ(I.Fin_récolte),365) AS INTEGER) J_récolte,
       I.Nb_rangs,
       I.Espacement,
       I.Nb_graines_trou,
       I.Dose_semis,
       I.Notes,
       E.Notes N_espèce,
       E.Famille
FROM ITP I
LEFT JOIN Espèces E USING(Espèce);
--ORDER BY coalesce(I.Déb_semis,I.Déb_plantation,I.Type_culture);

CREATE VIEW Rotations_détails__Tempo AS SELECT
       R.ID,
       R.Rotation,
       R.Année,
       R.IT_plante,
       R.Pc_planches,
       R.Fi_planches,
       -- RI.Mise_en_place || (CASE
       --        WHEN -- Vérif que la culture n'est pas mise en place trop tôt par rapport à la culture antérieure dans la rotation.
       --             (RI.Date_MEP < -- Mise en place au plus tôt de la culture courante
       --              coalesce(
       --              (SELECT Date_Ferm FROM R_ITP -- comparaison avec la fermeture au plus tôt de la culture antérieure (si la courante n'est pas la 1ère de la rotation)
       --               WHERE (Rotation=R.Rotation)AND(Rotation||Date_MEP < R.Rotation || RI.Date_MEP)
       --               ORDER BY Ind DESC
       --               LIMIT 1),
       --              (SELECT DATE(Date_Ferm,'-'||RI.Nb_années||' years') FROM R_ITP -- comparaison avec la fermeture au plus tôt de la dernière culture de la rotation
       --               WHERE (Rotation=R.Rotation)
       --               ORDER BY Ind DESC
       --               LIMIT 1)
       --              ))AND
       --             -- et total d'occupation des planches en conflit supérieur à 100.
       --             (RI.Pc_planches +
       --              coalesce(
       --              (SELECT Pc_planches FROM R_ITP -- culture antérieure (si la courante n'est pas la 1ère de la rotation)
       --               WHERE (Rotation=R.Rotation)AND(Rotation||Date_MEP < R.Rotation || RI.Date_MEP)
       --               ORDER BY Ind DESC
       --               LIMIT 1),
       --              (SELECT Pc_planches FROM R_ITP -- dernière culture de la rotation
       --               WHERE (Rotation=R.Rotation)
       --               ORDER BY Ind DESC
       --               LIMIT 1)
       --              )>100)
       --        THEN '*'
       --        -- Vérif que la culture n'est pas mise en place trop tard par rapport à la culture antérieure dans la rotation.
       --        WHEN RotDecalDateMeP(RI.Date_MEP) > -- Mise en place au plus tôt de la culture courante
       --             coalesce(
       --             (SELECT Date_Ferm FROM R_ITP -- comparaison avec la fermeture au plus tôt de la culture antérieure (si la courante n'est pas la 1ère de la rotation)
       --              WHERE (Rotation=R.Rotation)AND(Rotation||Date_MEP < R.Rotation || RI.Date_MEP)
       --              ORDER BY Ind DESC
       --              LIMIT 1),
       --             (SELECT DATE(Date_Ferm,'-'||RI.Nb_années||' years') FROM R_ITP -- comparaison avec la fermeture au plus tôt de la dernière culture de la rotation
       --              WHERE (Rotation=R.Rotation)
       --              ORDER BY Ind DESC
       --              LIMIT 1)
       --             )
       --        THEN '-'
       --        ELSE ''
       --        END) Mise_en_place,
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
              THEN '*' --||RI.Fi_planches||(SELECT Fi_planches FROM R_ITP_CAnt(R.Rotation,RI.Date_MEP))
              -- Vérif que la culture n'est pas mise en place trop tard par rapport à la culture antérieure dans la rotation.
              WHEN RotDecalDateMeP(RI.Date_MEP) > -- Mise en place au plus tôt de la culture courante
                   coalesce(
                   (SELECT Date_Ferm FROM R_ITP_CAnt(R.Rotation,RI.Date_MEP)), -- comparaison avec la fermeture au plus tôt de la culture antérieure (si la courante n'est pas la 1ère de la rotation)
                   (SELECT Date_Ferm FROM R_ITP_CDer(RI.Nb_années,R.Rotation))) -- comparaison avec la fermeture au plus tôt de la dernière culture de la rotation
              THEN '-'
              ELSE ''
              END) Mise_en_place,
       RotTempo(RI.Type_culture, ItpTempoNJ(RI.Déb_semis), ItpTempoNJ(RI.Fin_semis), ItpTempoNJ(RI.Déb_plantation), ItpTempoNJ(RI.Fin_plantation), ItpTempoNJ(RI.Déb_récolte), ItpTempoNJ(RI.Fin_récolte) ) TEMPO,
       RI.A_planifier,
       CASE WHEN (RI.Type_planche NOTNULL) AND
                 (RI.Type_planche<>(SELECT Type_planche
                                      FROM Rotations
                                     WHERE Rotation=R.Rotation) ) THEN RI.Type_planche END Conflit_type_planche,
       CASE WHEN R.IT_plante NOTNULL
            -- Recherche de familles trop proches dans une rotation.
            THEN (SELECT group_concat(result,x'0a') FROM RF_trop_proches(R.Rotation,R.Année,(SELECT Famille FROM R_famille F WHERE F.ID=R.ID))) || x'0a0a'
            END ||
       -- 'Familles possibles : ' || (SELECT group_concat(Famille,', ') FROM (
       --                              SELECT DISTINCT F.Famille
       --                              FROM Rotations_détails RD
       --                                   LEFT JOIN Rotations R USING(Rotation)
       --                                   LEFT JOIN ITP I USING(IT_Plante)
       --                                   LEFT JOIN Espèces E USING(Espèce)
       --                                   LEFT JOIN Familles F USING(Famille)
       --                              WHERE (RD.Rotation=(SELECT Rotation FROM R_famille WHERE ID=R.ID))AND -- ITP dans la même rotation
       --                                    (RD.Année<>(SELECT Année FROM R_famille WHERE ID=R.ID))AND -- ITP des autres années de la rotation
       --                                    NOT ((SELECT Année FROM R_famille WHERE ID=R.ID) BETWEEN RD.Année-F.Intervalle+1 AND RD.Année+F.Intervalle-1)AND -- pas en conflit lors du 1er cycle
       --                                    NOT ((SELECT Année FROM R_famille WHERE ID=R.ID) BETWEEN RD.Année-F.Intervalle+1+R.Nb_années AND RD.Année+F.Intervalle-1+R.Nb_années)
       --                              UNION
       --                              SELECT F.Famille FROM F_actives F
       --                              WHERE NOT(F.Famille IN(SELECT Famille FROM R_famille WHERE Rotation=(SELECT Rotation FROM R_famille WHERE ID=R.ID)))
       --                              )) Conflit_famille,
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

-- CREATE VIEW IT_Rotations_vérif AS SELECT I.Espèce,
--        coalesce(IR.IT_plante,IC.IT_plante) IT_plante,
--        IR.Nb_planches Nb_planches_P,
--        IR.Longueur Long_P,
--        IR.Long_rang Long_rang_P,
--        IR.Nb_plants Nb_plants_P,
--        (SELECT Valeur FROM Params WHERE Paramètre='Année_planif') Année_à_planifier,
--        IC.Nb_planches Nb_planches_C,
--        IC.Longueur Long_C,
--        IC.Long_rang Long_rang_C,
--        IC.Nb_plants Nb_plants_C,
--        (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') Année_en_cours,
--        CASE WHEN IC.LONGUEUR NOTNULL THEN CAST((coalesce(IR.LONGUEUR,0)-coalesce(IC.LONGUEUR,0))/IC.LONGUEUR*100 AS INTEGER)
--             ELSE 1000 END Diff_long,
--        'pc' Unité
-- FROM IT_rotations IR
-- FULL JOIN IT_cultures IC USING(IT_plante)
-- LEFT JOIN ITP I USING(IT_plante)
-- UNION
-- SELECT I.Espèce,
--        'z________________________TOTAL '||I.Espèce IT_plante,
--        sum(IR.NB_PLANCHES) Nb_planches_P,
--        sum(IR.LongueuR) Long_P,
--        sum(IR.Long_rang) Long_rang_P,
--        sum(IR.Nb_plants) Nb_plants_P,
--        null Année_à_planifier,
--        sum(IC.Nb_planches) Nb_planches_C,
--        sum(IC.Longueur) Long_C,
--        sum(IC.Long_rang) Long_rang_C,
--        sum(IC.Nb_plants) Nb_plants_C,
--        null Année_en_cours,
--        CAST(sum(coalesce(IR.LONGUEUR,0)-coalesce(IC.LONGUEUR,0)) AS INTEGER) Diff_long,
--        'm' Unité
-- FROM IT_rotations IR
-- FULL JOIN IT_cultures IC USING(IT_plante)
-- LEFT JOIN ITP I USING(IT_plante)
-- GROUP BY Espèce
-- ORDER BY Espèce,IT_plante;

CREATE VIEW Cult_planif AS SELECT
   PL.Planche,
   E.Espèce,
   RD.IT_plante,
   (SELECT V.Variété FROM Variétés V WHERE V.Espèce=I.Espèce ORDER BY V.Qté_stock DESC) Variété,
   (SELECT V.Fournisseur FROM Variétés V WHERE V.Espèce=I.Espèce ORDER BY V.Qté_stock DESC) Fournisseur,
   -- CAST((SELECT Valeur FROM Params WHERE Paramètre='Année_planif')AS INTEGER) Année_à_planifier,
   PlanifCultureCalcDate(CASE WHEN (I.Déb_plantation NOTNULL) AND (I.Déb_plantation<I.Déb_semis)
                              THEN (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
                              ELSE (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
                              END,
                         I.Déb_semis) Date_semis,
   PlanifCultureCalcDate(coalesce(PlanifCultureCalcDate(CASE WHEN (I.Déb_plantation NOTNULL) AND (I.Déb_plantation<I.Déb_semis)
                                                        THEN (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
                                                        ELSE (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
                                                        END,
                                                        I.Déb_semis),
                                  (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture') ||'-01-01'),
                         I.Déb_plantation) Date_plantation,
   CAST(round((RD.Pc_planches/100*PL.Longueur),2)AS REAL) Longueur,
   I.Nb_rangs,
   I.Espacement,
   CAST(round(RD.Pc_planches/100*PL.Longueur*PL.Largeur,2)AS REAL) Surface,
   CAST(round(RD.Pc_planches/100*PL.Longueur*PL.Largeur*E.Rendement,2)AS REAL) Prod_possible,
   -- CAST((SELECT Valeur FROM Params WHERE Paramètre='Année_planif')AS INTEGER) Année_à_planifier
   'Simulation planif' Info
FROM Rotations_détails RD
     JOIN Planches PL USING(Rotation,Année)
     JOIN ITP I USING(IT_plante)
     LEFT JOIN Espèces E USING(Espèce)
WHERE ((SELECT (Valeur ISNULL) FROM Params WHERE Paramètre='Planifier_planches') OR
        (PL.Planche LIKE (SELECT Valeur FROM Params WHERE Paramètre='Planifier_planches')||'%')) AND
       ((RD.Fi_planches ISNULL) OR
        (Fi_planches LIKE '%'||substr(PL.Planche, -1, 1) ||'%'))
ORDER BY coalesce(Date_plantation, Date_semis);

CREATE VIEW Cult_planif_ilots AS SELECT
    substr(C.Planche,1,(SELECT Valeur FROM Params WHERE Paramètre='Ilot_nb_car')) Ilot,
    C.Espèce,
    min(Date_semis) Date_semis,
    min(Date_plantation) Date_plantation,
    CAST(count() AS INTEGER) Nb_planches,
    CAST(sum(C.Longueur)AS REAL) Longueur,
    CAST(sum(C.Longueur*coalesce(C.Nb_rangs,1))AS REAL) Long_rang,
    CAST(sum(C.Longueur*coalesce(C.Nb_rangs,1)/coalesce(C.Espacement/100,1)) AS INTEGER) Nb_plants,
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
    CAST(sum(C.Longueur*coalesce(C.Nb_rangs,1))AS REAL) Long_rang,
    CAST(sum(C.Longueur*coalesce(C.Nb_rangs,1)/coalesce(C.Espacement/100,1)) AS INTEGER) Nb_plants,
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

-- CREATE VIEW Successions_par_planche AS SELECT
--        PL.Planche,
--        CAST((SELECT count()
--              FROM C_en_place C
--              WHERE C.Planche=PL.Planche) AS INTEGER) Nb_cu_EP,
--        (SELECT group_concat(IT_plante, '/')
--         FROM (SELECT DISTINCT IT_plante
--               FROM C_en_place C
--               WHERE C.Planche=PL.Planche)) ITP_en_place,
--        (SELECT max(Fin_récolte)
--         FROM C_en_place C
--            WHERE C.Planche=PL.Planche) Libre_le,

--        CAST((SELECT count()
--             FROM C_à_venir C
--             WHERE C.Planche=PL.Planche) AS INTEGER) Nb_cu_AV,
--        (SELECT group_concat(IT_plante, '/')
--         FROM (SELECT DISTINCT IT_plante
--               FROM C_à_venir C
--               WHERE C.Planche=PL.Planche)) ITP_à_venir,
--        (SELECT min(coalesce(Date_plantation,Date_semis))
--         FROM C_à_venir C
--         WHERE C.Planche=PL.Planche) En_place_le,
--        CAST(((SELECT min(julianday(coalesce(Date_plantation,Date_semis)))
--                        FROM C_à_venir C
--                        WHERE C.Planche=PL.Planche) - (SELECT max(julianday(Fin_récolte))
--                                                       FROM C_en_place C
--                                                       WHERE C.Planche=PL.Planche)) AS INTEGER) Nb_J_inoccupée
--   FROM Planches PL;

CREATE VIEW Cultures__Tempo AS SELECT
       -- row_number() OVER (ORDER BY Planche) AS num_planche,
       -- rank() OVER (ORDER BY Planche) AS num_planche,
       dense_rank() OVER (ORDER BY Planche) AS num_planche,
       C.Planche,
       C.Culture,
       coalesce(C.Variété,C.IT_plante) Variété_ou_It_plante,
       -- C.Type,
       -- C.Date_semis,
       -- C.Date_plantation,
       -- C.Début_récolte,
       -- C.Fin_récolte,
       CASE WHEN substr(coalesce(C.Date_semis,C.Date_plantation),1,4)=CAST(((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')-1)AS TEXT)
            THEN CulTempo(C.Type, C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte)||':'||E.Espèce||':'||
                 coalesce(C.Semis_fait,'')||':'||coalesce(C.Plantation_faite,'')||':'||coalesce(C.Récolte_faite,'')
            ELSE NULL END TEMPO_NP,
       CASE WHEN substr(coalesce(C.Date_semis,C.Date_plantation),1,4)=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
            THEN CulTempo(C.Type, C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte)||':'||E.Espèce||':'||
            coalesce(C.Semis_fait,'')||':'||coalesce(C.Plantation_faite,'')||':'||coalesce(C.Récolte_faite,'')
            ELSE NULL END TEMPO_N,
       CASE WHEN substr(coalesce(C.Date_semis,C.Date_plantation),1,4)=CAST(((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)AS TEXT)
            THEN CulTempo(C.Type, C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte)||':'||E.Espèce||':'||
            coalesce(C.Semis_fait,'')||':'||coalesce(C.Plantation_faite,'')||':'||coalesce(C.Récolte_faite,'')
            ELSE NULL END TEMPO_NN,
       C.Saison,
       C.Longueur,
       C.Nb_rangs,
       C.Longueur*PL.Largeur Surface,
       C.Notes
  FROM Cultures C
       LEFT JOIN ITP I USING(IT_plante)
       LEFT JOIN Planches PL USING(Planche)
       LEFT JOIN Espèces E USING(Espèce)
 WHERE (C.Planche NOTNULL)AND
       ((coalesce(C.Date_semis, C.Date_plantation) BETWEEN ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')-1)||'-01-01' AND
                                                           ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)||'-12-31'))--OR
        -- (C.Fin_récolte BETWEEN ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')-1)||'-01-01' AND
        --                        ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)||'-12-31'))
ORDER BY C.Planche,
          coalesce(C.Date_semis, C.Date_plantation);

CREATE VIEW Cultures__non_terminées AS SELECT
        C.Planche,
        C.Culture,
        C.IT_plante,
        C.Variété,
        C.Fournisseur,
        C.Type,
        C.Saison,
        CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                  ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=IT_plante)!=
                   (SELECT V.Espèce FROM Variétés V WHERE V.Variété=Variété)) THEN 'Err. variété'
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
        CAST(round((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc,
        C.Terminée,
        C.Longueur,
        C.Nb_rangs,
        C.Espacement,
        I.Nb_graines_trou,
        I.Dose_semis,
        CAST((C.Longueur * C.Nb_rangs / C.Espacement * 100)AS INTEGER) Nb_plants,
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_Planche
FROM Cultures C
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE C.Terminée ISNULL
ORDER BY    C.Planche,
            coalesce(C.Date_semis,C.Date_plantation),
            C.IT_plante;

CREATE VIEW Cultures__à_semer AS SELECT
        C.Planche,
        C.Culture,
        C.IT_plante,
        C.Variété,
        C.Fournisseur,
        C.Type,
        CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                  ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=IT_plante)!=
                   (SELECT V.Espèce FROM Variétés V WHERE V.Variété=Variété)) THEN 'Err. variété'
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
        CAST((C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou)AS INTEGER) Nb_graines,
        CAST(round((CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou / E.Nb_graines_g
                    ELSE C.Longueur * PL.Largeur * I.Dose_semis END),2)AS REAL) Poids_graines,
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
LEFT JOIN Espèces E USING (Espèce)
WHERE   (Terminée ISNULL) AND
        (Date_semis < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_semis')||' days')) AND
        (coalesce(Semis_fait,'') NOT LIKE 'x%')
ORDER BY    Date_semis,Date_plantation,
            Planche,
            IT_plante;

CREATE VIEW Cultures__à_semer_SA AS SELECT
        CAST(min(C.Culture)AS INTEGER) Culture, -- Pour servir d'index, colonne à cacher.
        group_concat(C.Planche,x'0a0a') Planches,
        group_concat(C.Culture||' ',x'0a0a') Cultures,
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
        min(C.Notes) Notes,
        I.Notes N_IT_plante,
        -- PL.Notes N_planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
LEFT JOIN Espèces E USING (Espèce)
WHERE   (Terminée ISNULL) AND
        (Date_semis < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_semis')||' days')) AND
        (coalesce(Semis_fait,'') NOT LIKE 'x%') AND
        (Date_plantation NOTNULL)
GROUP BY C.IT_plante,C.Variété,C.Type,C.Etat,C.Date_semis --,C.Semis_fait
ORDER BY    Date_semis,Date_plantation,
            Planche,
            IT_plante;

CREATE VIEW Cultures__à_semer_D AS SELECT
        C.Planche,
        C.Culture,
        C.IT_plante,
        C.Variété,
        C.Fournisseur,
        C.Type,
        CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                  ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=IT_plante)!=
                   (SELECT V.Espèce FROM Variétés V WHERE V.Variété=Variété)) THEN 'Err. variété'
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
        CAST((C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou)AS INTEGER) Nb_graines,
        CAST(round((CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou / E.Nb_graines_g
                    ELSE C.Longueur * PL.Largeur * I.Dose_semis END),2)AS REAL) Poids_graines,
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
LEFT JOIN Espèces E USING (Espèce)
WHERE   (Terminée ISNULL) AND
        (Date_semis < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_semis')||' days')) AND
        (coalesce(Semis_fait,'') NOT LIKE 'x%') AND
        (Date_plantation ISNULL)
ORDER BY    Date_semis,
            Planche,
            IT_plante;

CREATE VIEW Cultures__à_planter AS SELECT
        C.Planche,
        C.Culture,
        C.IT_plante,
        C.Variété,
        C.Fournisseur,
        C.Type,
        CASE WHEN Variété NOTNULL AND IT_plante NOTNULL AND
                  ((SELECT I.Espèce FROM ITP I WHERE I.IT_plante=IT_plante)!=
                   (SELECT V.Espèce FROM Variétés V WHERE V.Variété=Variété)) THEN 'Err. variété'
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
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
LEFT JOIN Espèces E USING (Espèce)
WHERE   (Terminée ISNULL) AND
        (Date_plantation < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_plantation')||' days')) AND
        -- (Plantation_faite ISNULL) AND
        (coalesce(Plantation_faite,'') NOT LIKE 'x%') AND
        ((Semis_fait LIKE 'x%')OR(Date_semis ISNULL))
ORDER BY    Date_plantation,
            Planche,
            IT_plante;

CREATE VIEW Cultures__à_récolter AS SELECT
        C.Planche,
        C.Culture,
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
        CAST(round((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc,
        C.Terminée,
        C.Longueur,
        C.Nb_rangs,
        C.Espacement,
        CAST((C.Longueur * C.Nb_rangs / C.Espacement * 100)AS INTEGER) Nb_plants,
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_Planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
LEFT JOIN Espèces E USING (Espèce)
WHERE   (C.Terminée ISNULL) AND
        (Début_récolte < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_récolte')||' days')) AND
        (coalesce(Récolte_faite,'') NOT LIKE 'x%')  AND
        ((Semis_fait LIKE 'x%')OR(Date_semis ISNULL)) AND
        ((Plantation_faite LIKE 'x%')OR(Date_plantation ISNULL))
ORDER BY    C.Début_récolte,
            C.Planche,
            C.IT_plante;

CREATE VIEW Récoltes__Saisies AS SELECT
       R.ID,
       R.Date,
       I.Espèce,
       R.Culture,
       R.Quantité,
       NULL Répartir,
       C.Planche,
       C.Variété,
       -- CAST(round((SELECT sum(Quantité) FROM Récoltes R2 WHERE (R2.Culture=R.Culture)AND(R2.Date<=R.Date)),3)AS REAL) Qté_réc, Triggers marchent mal avec cet appel.
       R.Notes
FROM Récoltes R
JOIN Cultures C USING(Culture)
LEFT JOIN ITP I USING(IT_plante)
WHERE (R.Date>DATE('now','-'||(SELECT Valeur FROM Params WHERE Paramètre='C_historique_récolte')||' days'))OR
      (DATE(R.Date) ISNULL) -- Détection de date incorecte
ORDER BY R.Date,I.Espèce,R.Culture;

CREATE VIEW Cultures__à_terminer AS SELECT
        C.Planche,
        C.Culture,
        coalesce(C.Variété,C.IT_plante) Variété_ou_It_plante,
        C.Type,
        C.Etat,
        C.Date_semis,
        C.Date_plantation,
        C.Début_récolte,
        C.Fin_récolte,
        C.Récolte_faite,
        C.Terminée,
        C.Longueur,
        C.Notes,
        I.Notes N_IT_plante,
        PL.Notes N_Planche,
        E.Notes N_espèce
FROM Cultures C
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
LEFT JOIN Espèces E USING (Espèce)
WHERE   (C.Terminée ISNULL) AND
        ((C.Fin_récolte ISNULL)OR(C.Fin_récolte < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_terminer')||' days'))) AND
        ((Semis_fait LIKE 'x%')OR(Date_semis ISNULL)) AND
        ((Plantation_faite LIKE 'x%')OR(Date_plantation ISNULL)) AND
        ((Récolte_faite LIKE 'x%')OR(Début_récolte ISNULL))
ORDER BY    C.Fin_récolte,
            C.Planche,
            C.IT_plante;

CREATE VIEW ITP__analyse AS SELECT I.IT_plante,
       I.Type_culture,
       I.Déb_semis,
       I.Déb_plantation,
       I.Déb_récolte,
       CAST((SELECT count()
             FROM Cultures C
             WHERE (C.IT_plante=I.IT_plante) AND C.Terminée ISNULL)AS INTEGER) Nb_cu_NT,
       CAST((SELECT count()
             FROM Cultures C
             WHERE (C.IT_plante=I.IT_plante) AND
                   C.Terminée NOTNULL)AS INTEGER) Nb_cu_T,
       CAST((SELECT count()
             FROM Cultures C
             WHERE (C.IT_plante=I.IT_plante) AND
                   C.Terminée NOTNULL AND (C.Terminée != 'NS'))AS INTEGER) Nb_cu_TS,
       (SELECT min(substr(C.Date_semis, 6))
        FROM Cultures C
        WHERE (C.IT_plante=I.IT_plante) AND C.Terminée NOTNULL AND (C.Terminée != 'NS')) Min_semis,
       (SELECT max(substr(C.Date_semis, 6) )
        FROM Cultures C
        WHERE (C.IT_plante=I.IT_plante) AND C.Terminée NOTNULL AND (C.Terminée != 'NS')) Max_semis,
       (SELECT min(substr(C.Date_plantation, 6) )
        FROM Cultures C
        WHERE (C.IT_plante=I.IT_plante) AND C.Terminée NOTNULL AND (C.Terminée != 'NS')) Min_plantation,
       (SELECT max(substr(C.Date_plantation, 6) )
        FROM Cultures C
        WHERE (C.IT_plante=I.IT_plante) AND C.Terminée NOTNULL AND (C.Terminée != 'NS')) Max_plantation,
       (SELECT min(substr(C.Début_récolte, 6) )
        FROM Cultures C
        WHERE (C.IT_plante=I.IT_plante) AND C.Terminée NOTNULL AND (C.Terminée != 'NS')) Min_recolte,
       (SELECT max(substr(C.Fin_récolte, 6) )
        FROM Cultures C
        WHERE (C.IT_plante=I.IT_plante) AND C.Terminée NOTNULL AND (C.Terminée != 'NS')) Max_recolte,
       CAST(round((SELECT sum(Quantité)
                   FROM Cultures C LEFT JOIN Récoltes R USING(Culture)
                   WHERE (C.IT_plante=I.IT_plante) AND C.Terminée NOTNULL AND (C.Terminée != 'NS'))/
                  (SELECT count()
                   FROM Cultures C JOIN Récoltes R USING(Culture)
                   WHERE (C.IT_plante=I.IT_plante) AND C.Terminée NOTNULL AND (C.Terminée != 'NS')),3)AS REAL) Qté_réc_moy
  FROM ITP I
 ORDER BY IT_plante;

CREATE VIEW Cultures__analyse AS SELECT
       C.Culture,
       C.IT_plante,
       C.Variété,
       C.Planche,
       C.Type,
       C.Saison,
       C.Date_semis,
       C.Date_plantation,
       C.Début_récolte,
       C.Fin_récolte,
       C.Longueur*PL.Largeur Surface,
       CAST(round((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc,
       CAST(round(((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture)/(C.Longueur*PL.Largeur)),3)AS REAL) Rendement_C,
       E.Rendement,
       CASE WHEN E.Rendement NOTNULL
       THEN round(((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture)/(C.Longueur*PL.Largeur))/E.Rendement*100)
       ELSE NULL END Perf_pc,
       CulTempo(C.Type, C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte)||':'||E.Espèce TEMPO,
       C.Notes
  FROM Cultures C
       LEFT JOIN ITP I USING(IT_plante)
       LEFT JOIN Planches PL USING(Planche)
       LEFT JOIN Espèces E USING(Espèce)
 WHERE (C.Type!='Engrais vert') AND (C.Type!='Sans récolte') AND C.Terminée NOTNULL AND (C.Terminée!='NS')
ORDER BY C.IT_plante,
          coalesce(C.Date_semis, C.Date_plantation);

CREATE VIEW Cultures__inc_dates AS SELECT
       C.Culture,
       C.IT_plante,
       C.Variété,
       C.Planche,
       C.Type,
       C.Etat,
       CulIncDates(substr(coalesce(C.Date_semis,C.Date_plantation),1,4),
                   coalesce(I.Déb_semis,I.Déb_plantation),
                   C.Date_semis,I.Déb_semis,I.Fin_semis,
                   C.Date_plantation,I.Déb_plantation,I.Fin_plantation,
                   C.Début_récolte,I.Déb_récolte,
                   C.Fin_récolte,I.Fin_récolte) Incohérence,
       I.Déb_semis,
       I.Fin_semis,
       C.Date_semis,
       I.Déb_plantation,
       I.Fin_plantation,
       C.Date_plantation,
       I.Déb_récolte,
       I.Fin_récolte Fin_récolte_ITP,
       C.Début_récolte,
       C.Fin_récolte,
       C.Notes
FROM Cultures C
     LEFT JOIN ITP I USING(IT_plante)
WHERE (C.Terminée ISNULL OR C.Terminée != 'NS')AND
      ((C.Type LIKE '%?')OR
       (CulIncDates(substr(coalesce(C.Date_semis,C.Date_plantation),1,4),
                    coalesce(I.Déb_semis,I.Déb_plantation),
                    C.Date_semis,I.Déb_semis,I.Fin_semis,
                    C.Date_plantation,I.Déb_plantation,I.Fin_plantation,
                    C.Début_récolte,I.Déb_récolte,
                    C.Fin_récolte,I.Fin_récolte) NOTNULL))
ORDER BY Culture;

CREATE VIEW C_non_commencées AS --Cultures ni semées (SD ou SSA) ni plantées.
    SELECT *
      FROM Cultures
     WHERE Terminée ISNULL AND Semis_fait ISNULL AND Plantation_faite ISNULL
     ORDER BY Planche;

CREATE VIEW C_à_venir AS --Cultures ni semées (SD) ni plantées.
    SELECT *
      FROM Cultures
     WHERE Terminée ISNULL AND
           NOT ( (Semis_fait NOTNULL AND Date_plantation ISNULL) OR-- SD semé
                 Plantation_faite NOTNULL)-- Plant ou SSA planté
     ORDER BY Planche;

CREATE VIEW C_en_place AS --Cultures semées (SD) ou plantées.
    SELECT *
      FROM Cultures
     WHERE Terminée ISNULL AND
           ( (Semis_fait NOTNULL AND Date_plantation ISNULL) OR-- SD semé
             Plantation_faite NOTNULL)-- Plant ou SSA planté
     ORDER BY Planche;

CREATE VIEW C_esp_prod AS SELECT
    C.Culture,
    E.Espèce,
    C.Saison,
    CASE WHEN (C.Terminée NOTNULL)OR(C.Récolte_faite LIKE 'x%')
         THEN CAST(round((SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture),3)AS REAL)
         ELSE CAST(round(C.Longueur*P.Largeur*E.Rendement,3)AS REAL)
         END Production,
    CASE WHEN (C.Terminée NOTNULL)OR(C.Récolte_faite LIKE 'x%')
         THEN 'Réel'
         ELSE 'Possible'
         END RP
FROM Cultures C
JOIN Planches P USING(Planche)
JOIN ITP I USING(IT_plante)
JOIN Espèces E USING(Espèce);

CREATE VIEW F_actives AS
    SELECT *
      FROM Familles F
     WHERE ( (SELECT count()
                FROM ITP
                     LEFT JOIN Espèces USING(Espèce)
               WHERE Espèces.Famille=F.Famille) >0);

-- CREATE VIEW IT_cultures AS
--     SELECT IT_plante,
--            sum(NB_PLANCHES) NB_PLANCHES,
--            sum(LONGUEUR) LONGUEUR,
--            sum(LONG_RANG) LONG_RANG,
--            sum(NB_PLANTS) NB_PLANTS,
--            (SELECT Valeur
--               FROM Params
--              WHERE Paramètre='Année_culture') ANNEE
--       FROM IT_cultures_ilots
--      GROUP BY IT_plante;

-- CREATE VIEW IT_cultures_ilots AS
--     SELECT IT_plante,
--            substr(PLANCHE, 1, (SELECT Valeur
--                                  FROM Params
--                                 WHERE Paramètre='Ilot_nb_car') ) ILOT,
--            sum(NB_PLANCHES) NB_PLANCHES,
--            sum(LONGUEUR) LONGUEUR,
--            sum(LONG_RANG) LONG_RANG,
--            sum(NB_PLANTS) NB_PLANTS,
--            (SELECT Valeur
--               FROM Params
--              WHERE Paramètre='Année_culture') ANNEE
--       FROM IT_cultures_planches
--      GROUP BY IT_plante,
--               ILOT;

-- CREATE VIEW IT_cultures_planches AS
--     SELECT trim(C.IT_plante) IT_plante,
--            C.Planche PLANCHE,
--            1 NB_PLANCHES,-- 2 ITP sur la même planche la même année se partagent la planche, donc on en compte qu'une
--            sum(C.Longueur) LONGUEUR,
--            sum(C.Longueur*coalesce(C.Nb_rangs, 1) ) LONG_RANG,
--            CAST(sum(C.Longueur*coalesce(C.Nb_rangs, 1) /coalesce(C.Espacement/100, 1) ) AS INTEGER) NB_PLANTS,-- C.Longueur,
--            (SELECT Valeur
--               FROM Params
--              WHERE Paramètre='Année_culture') ANNEE
--       FROM Cultures C-- LEFT JOIN Rotations R USING(Rotation)
--      WHERE (substr(coalesce(C.Date_plantation, C.Date_semis), 1, 4) = (SELECT Valeur-- LEFT JOIN Planches P USING(Rotation)
--                                                                          FROM Params
--                                                                         WHERE Paramètre='Année_culture') )
--      GROUP BY IT_plante,
--               PLANCHE
--      ORDER BY IT_plante,
--               PLANCHE;

-- CREATE VIEW IT_rotations AS
--     SELECT Espèce,
--            IT_plante,
--            sum(Nb_planches) Nb_planches,
--            sum(Longueur) Longueur,
--            sum(Long_rang) Long_rang,
--            sum(Nb_plants) Nb_plants,
--            CAST((SELECT Valeur
--                  FROM Params
--                  WHERE Paramètre='Année_planif')AS INTEGER) Année_à_planifier
--       FROM Cult_planif_ilots
--      GROUP BY IT_plante;

-- CREATE VIEW Cult_planif_ilots AS
--     SELECT E.Espèce,
--            RD.IT_plante,
--            substr(P.Planche, 1, (SELECT Valeur
--                                    FROM Params
--                                   WHERE Paramètre='Ilot_nb_car') ) Ilot,
--            CAST(count() AS INTEGER) Nb_planches,
--            CAST(round(sum(P.Longueur*RD.Pc_planches/100),2)AS REAL) Longueur,
--            CAST(round(sum(P.Longueur*coalesce(I.Nb_rangs, 1) *RD.Pc_planches/100),2)AS REAL) Long_rang,
--            CAST(sum(P.Longueur*coalesce(I.Nb_rangs, 1) /coalesce(I.Espacement/100, 1) *RD.Pc_planches/100) AS INTEGER) Nb_plants,
--            CAST(round(sum(P.Longueur*RD.Pc_planches/100*P.Largeur),2)AS REAL) Surface,
--            CAST(round(sum(P.Longueur*RD.Pc_planches/100*P.Largeur)*E.Rendement,2)AS REAL) Prod_possible,
--            CAST((SELECT Valeur
--                  FROM Params
--                  WHERE Paramètre='Année_planif')AS INTEGER) Année_à_planifier
--       FROM Rotations_détails RD
--            LEFT JOIN ITP I USING(IT_plante)
--            LEFT JOIN Espèces E USING(Espèce)
--            LEFT JOIN Rotations R USING(Rotation)
--            LEFT JOIN Planches P ON (P.Rotation=RD.Rotation) AND
--                                    (P.Année=RD.Année) AND
--                                    (RD.Fi_planches ISNULL OR
--                                     ((RD.Fi_planches LIKE '%'||substr(P.Planche, -1, 1) ||'%') ) )
--      WHERE IT_plante NOTNULL
--      GROUP BY Espèce,
--               IT_plante,
--               Ilot
--      ORDER BY Espèce,
--               IT_plante,
--               Ilot;

-- CREATE VIEW IT_rotations_manquants AS
--     SELECT Espèce,
--            IT_plante,
--            E.Rendement,
--            E.Niveau,
--            E.Apport,
--            CAST((SELECT sum(Qté_stock) FROM Variétés V WHERE V.Espèce=E.Espèce)AS INTEGER) Qté_stock,
--            ITP.Type_planche,
--            ITP.Type_culture,
--            ITP.Déb_semis,
--            ITP.Déb_plantation,
--            ITP.Déb_récolte,
--            CAST((SELECT Valeur FROM Params WHERE Paramètre='Année_planif')AS INTEGER) Année_à_planifier
--       FROM ITP
--       LEFT JOIN Espèces E USING(Espèce)
--      WHERE NOT (Espèce IN(SELECT Espèce FROM Cult_planif_ilots) ) AND
--            E.A_planifier NOTNULL;

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

CREATE VIEW Espèces__couverture AS SELECT
    E.Espèce,
    E.Rendement,
    E.Niveau,
    CAST((SELECT sum(Qté_stock) FROM Variétés V WHERE V.Espèce=E.Espèce)AS INTEGER) Qté_stock,
    E.Obj_annuel,
    CAST((SELECT sum(Production) FROM C_esp_prod C WHERE (C.Saison=(SELECT Valeur-1 FROM Params WHERE Paramètre='Année_culture'))AND
                                                         (C.Espèce=E.Espèce))AS INTEGER) Prod_Nm1,
    CAST(((SELECT sum(Production) FROM C_esp_prod C WHERE (C.Saison=(SELECT Valeur-1 FROM Params WHERE Paramètre='Année_culture'))AND
                                                          (C.Espèce=E.Espèce))/E.Obj_annuel*100)AS INTEGER) Couv_Nm1_pc,
    CAST((SELECT sum(Production) FROM C_esp_prod C WHERE (C.Saison=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture'))AND
                                                         (C.Espèce=E.Espèce))AS INTEGER) Prod_N,
    CAST(((SELECT sum(Production) FROM C_esp_prod C WHERE (C.Saison=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture'))AND
                                                          (C.Espèce=E.Espèce))/E.Obj_annuel*100)AS INTEGER) Couv_N_pc,
    CAST((SELECT sum(Production) FROM C_esp_prod C WHERE (C.Saison=(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'))AND
                                                         (C.Espèce=E.Espèce))AS INTEGER) Prod_Np1,
    CAST(((SELECT sum(Production) FROM C_esp_prod C WHERE (C.Saison=(SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture'))AND
                                                          (C.Espèce=E.Espèce))/E.Obj_annuel*100)AS INTEGER) Couv_Np1_pc,
    E.Notes
FROM Espèces E
WHERE E.A_planifier NOTNULL;

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
    SELECT R.Rotation||format('%i', 2000+R.Année) ||'-'||coalesce(I.Déb_plantation, I.Déb_semis,'12-31') ||format('%i', 2000+R.Année+iif(coalesce(I.Déb_plantation, I.Déb_semis) >coalesce(I.Déb_récolte, I.Fin_récolte), 1, 0) ) ||'-'||coalesce(I.Déb_récolte, I.Fin_récolte, '12-31') Ind,-- coalesce(R.Fi_planches,'')||
           R.ID,
           R.Rotation,
           RO.Nb_années,
           R.Année,
           R.IT_plante,
           R.Pc_planches,
           R.Fi_planches,
           I.Type_planche,
           I.Type_culture,
           coalesce(I.Déb_plantation, I.Déb_semis) Mise_en_place,
           format('%i', 2000+R.Année) ||'-'||coalesce(I.Déb_plantation, I.Déb_semis) Date_MEP,
           format('%i', 2000+R.Année+iif(coalesce(I.Déb_plantation, I.Déb_semis) >coalesce(I.Déb_récolte, I.Fin_récolte), 1, 0) ) ||'-'||coalesce(I.Déb_récolte, I.Fin_récolte, '12-31') Date_Ferm,
           I.Déb_semis,
           I.Fin_semis,
           I.Déb_plantation,
           I.Fin_plantation,
           I.Déb_récolte,
           I.Fin_récolte,
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
