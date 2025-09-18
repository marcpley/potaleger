QString sDDLViews=QStringLiteral(R"#(
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

       coalesce((I.S_semis*7-6),0)||':'||
       coalesce(((I.S_semis+coalesce(I.Décal_max,0.5))*7-6),0)||':'||
       coalesce((I.S_plantation*7-6),0)||':'||
       coalesce(((I.S_plantation+coalesce(I.Décal_max,0.5))*7-6),0)||':'||
       coalesce((I.S_récolte*7-6),0)||':'||
       coalesce(((I.S_récolte+coalesce(I.D_récolte+coalesce(I.Décal_max,0),0))*7-6),0) TEMPO, -- Si pas de récolte (D_récolte NULL) ne pas ajouter le décalage.

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

CREATE VIEW ITP_sem_corrigées AS SELECT
       I.IT_plante,
       I.Espèce,
       (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture') A_planif,
       iif(I.S_semis>I.S_plantation,I.S_semis-52,I.S_semis) S_semis,
       I.S_plantation,
       iif(I.S_récolte<coalesce(I.S_plantation,I.S_semis),I.S_récolte+52,I.S_récolte) S_récolte,
       I.D_récolte,
       I.Nb_rangs,
       I.Espacement
FROM ITP I;

CREATE VIEW ITP_planif AS SELECT
       I.IT_plante,
       I.Espèce,
       I.S_semis,
       I.S_plantation,
       I.S_récolte,
       I.D_récolte,
       DATE(A_planif||'-01-01',(I.S_semis*7-6-1)||' days') Date_semis,
       DATE(A_planif||'-01-01',(I.S_plantation*7-6-1)||' days') Date_plantation,
       iif(I.D_récolte NOTNULL,DATE(A_planif||'-01-01',(I.S_récolte*7-6-1)||' days'),NULL) Début_récolte,
       iif(I.D_récolte NOTNULL,DATE(A_planif||'-01-01',((I.S_récolte+I.D_récolte)*7-6-1)||' days'),
                               DATE(A_planif||'-01-01',(I.S_récolte*7-6-1)||' days')) Fin_récolte,
       I.Nb_rangs,
       I.Espacement
FROM ITP_sem_corrigées I;

CREATE VIEW Cult_planif AS SELECT -- Dates théoriques au plus tôt, tenant compte des dates déjà existantes.
       CDT.Culture,
       CDT.Date_semis,
       DATE(CDT.Date_plantation,coalesce((julianday(C.Date_semis)-julianday(CDT.Date_semis)),0)||' days') Date_plantation,
       iif(CDT.Terminée ISNULL,
           DATE(CDT.Début_récolte,coalesce((julianday(C.Date_plantation)-julianday(CDT.Date_plantation)), -- Anuelle, décaler date de récolte avec retard de MEP.
                                           (julianday(C.Date_semis)-julianday(CDT.Date_semis)),0)||' days'),
           DATE(CDT.Début_récolte,max(coalesce(CDT.PJ,0)
                                      -(substr(CDT.Début_récolte,1,4)-substr(coalesce(C.Date_plantation,C.Date_semis),1,4)) -- Nb année en place
                                      ,0)||' years')) Début_récolte, -- Vivace, décaler date de récolte avec PJ.
       iif(CDT.Terminée ISNULL,
           DATE(CDT.Fin_récolte,coalesce((julianday(C.Date_plantation)-julianday(CDT.Date_plantation)), -- Anuelle, décaler date de récolte avec retard de MEP.
                                         (julianday(C.Date_semis)-julianday(CDT.Date_semis)),0)||' days'),
           DATE(CDT.Fin_récolte,max(coalesce(CDT.PJ,0)
                                      -(substr(CDT.Début_récolte,1,4)-substr(coalesce(C.Date_plantation,C.Date_semis),1,4)) -- Nb année en place
                                      ,0)||' years')) Fin_récolte -- Vivace, décaler date de récolte avec PJ.
FROM Cult_dates_théo CDT
LEFT JOIN Cultures C USING(Culture);

CREATE VIEW Cult_dates_théo AS SELECT -- Dates théoriques au plus tôt.
       C.Culture,
       DATE(C.A_planif||'-01-01',(C.S_semis*7-6-1)||' days') Date_semis,
       DATE(C.A_planif||'-01-01',(C.S_plantation*7-6-1)||' days') Date_plantation,
       iif(C.D_récolte NOTNULL,DATE(C.A_planif||'-01-01',(C.S_récolte*7-6-1)||' days'),NULL) Début_récolte,
       iif(C.D_récolte NOTNULL,DATE(C.A_planif||'-01-01',((C.S_récolte+C.D_récolte)*7-6-1)||' days'),
                               DATE(C.A_planif||'-01-01',(C.S_récolte*7-6-1)||' days')) Fin_récolte,
       C.PJ,
       C.Terminée
FROM Cult_dates_théo2 C;

CREATE VIEW Cult_dates_théo2 AS SELECT -- Semaines corrigées: pas de n° semaine inférieur au n° semaine opération précédente.
       C.Culture,
       C.A_planif,
       iif(C.S_semis>coalesce(C.S_plantation,C.S_semis),C.S_semis-52,C.S_semis) S_semis,
       C.S_plantation,
       iif(C.S_récolte<coalesce(C.S_plantation,C.S_semis),C.S_récolte+52,C.S_récolte) S_récolte,
       C.D_récolte,
       C.PJ,
       C.Terminée
FROM Cult_dates_théo3 C;

CREATE VIEW Cult_dates_théo3 AS SELECT -- Avec année de planification et semaines théoriques (ITP et Variétés)
       C.Culture,
       coalesce(substr(C.D_planif,1,4), -- Année demandée par utilisateur.
                substr(C.Date_plantation,1,4),substr(C.Date_semis,1,4), -- Année de mise en place de la culture.
                (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture')) A_planif, -- Année après la saison courante.
       I.S_semis,
       I.S_plantation,
       coalesce(V.S_récolte,I.S_récolte) S_récolte,
       coalesce(V.D_récolte,I.D_récolte,0) D_récolte,
       V.PJ,
       C.Terminée
FROM Cult_non_ter C
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Variétés V USING(Variété);

CREATE VIEW Cult_répartir_récolte AS SELECT
       C.Culture,C.Espèce,C.Planche,C.Longueur,
       max(DATE(C.Début_récolte,'-'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_avance')||' days'),
           DATE(coalesce(C.Date_plantation,C.Date_semis),'+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_après_MEP')||' days')) Début_récolte_possible,
       DATE(C.Fin_récolte,'+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_prolongation')||' days') Fin_récolte_possible
FROM Cultures C
WHERE ((Semis_fait NOTNULL AND Date_plantation ISNULL) OR -- SD semé
       Plantation_faite NOTNULL); -- Plant ou SPep planté

CREATE VIEW Cult_répartir_fertilisation AS SELECT
       C.Culture,C.Espèce,C.Planche,C.Longueur*P.Largeur Surface,
       DATE(coalesce(C.Date_plantation,C.Date_semis),'-'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='Ferti_avance')||' days') Début_fertilisation_possible,
       DATE(C.Début_récolte,'+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='Ferti_retard')||' days') Fin_fertilisation_possible
FROM Cultures C
JOIN Planches P USING(Planche);

CREATE VIEW Rotations_détails__Tempo AS SELECT
       R.ID,
       R.Rotation,
       R.Année,
       R.IT_plante,
       R.Pc_planches,
       R.Fi_planches,
       CAST(RI.S_MEP AS TEXT) || (CASE
              -- Vérif que la culture n'est pas mise en place trop tôt par rapport à la culture antérieure dans la rotation.
              WHEN (RI.S_MEP_abs < -- Mise en place au plus tôt de la culture courante
                    coalesce((SELECT R2.S_Ferm_abs FROM R_ITP_2 R2 WHERE (R2.Rotation=R.Rotation)AND(R2.S_MEP_abs<=RI.S_MEP_abs)AND(R2.ID!=RI.ID)AND -- comparaison avec la fermeture au plus tôt de la culture antérieure (si la courante n'est pas la 1ère de la rotation)
                                                                         ((RI.Fi_planches ISNULL) OR (R2.Fi_planches ISNULL) OR -- et filtres de planche identiques ou nuls.
                                                                          (RI.Fi_planches ISNULL=R2.Fi_planches ISNULL))AND
                                                                         (RI.Pc_planches+R2.Pc_planches>100)), -- total d'occupation des planches en conflit supérieur à 100.
                             (SELECT R3.S_Ferm_abs-52*RI.Nb_années FROM R_ITP_2 R3 WHERE (R3.Rotation=R.Rotation)AND -- comparaison avec la fermeture au plus tôt de la dernière culture de la rotation
                                                                         ((RI.Fi_planches ISNULL) OR (R3.Fi_planches ISNULL) OR -- et filtres de planche identiques ou nuls.
                                                                          (RI.Fi_planches ISNULL=R3.Fi_planches ISNULL))AND
                                                                         (RI.Pc_planches+R3.Pc_planches>100)), -- total d'occupation des planches en conflit supérieur à 100.
                             0))
              THEN '*'
              -- Vérif que la culture n'est pas mise en place trop tard par rapport à la culture antérieure dans la rotation.
              WHEN RI.S_MEP_abs-(SELECT Nb_sem FROM RotDecalSMeP WHERE Semaine=RI.S_MEP) > -- Mise en place au plus tôt de la culture courante
                   coalesce((SELECT R2.S_Ferm_abs FROM R_ITP_2 R2 WHERE (R2.Rotation=R.Rotation)AND(R2.S_MEP_abs<=RI.S_MEP_abs)AND(R2.ID!=RI.ID)), -- comparaison avec la fermeture au plus tôt de la culture antérieure (si la courante n'est pas la 1ère de la rotation)
                            (SELECT R3.S_Ferm_abs-52*RI.Nb_années FROM R_ITP_2 R3 WHERE R3.Rotation=R.Rotation)) -- comparaison avec la fermeture au plus tôt de la dernière culture de la rotation
              THEN '-'
              ELSE ''
              END) S_MEP,
       RI.S_Ferm,
       coalesce((RI.S_semis-iif(RI.S_semis>RI.S_plantation,52,0))*7-6,0)||':'||
       coalesce((RI.S_semis+coalesce(RI.Décal_max,0.5)-iif(RI.S_semis>RI.S_plantation,52,0))*7-6,0)||':'||
       coalesce((RI.S_plantation)*7-6,0)||':'||
       coalesce((RI.S_plantation+coalesce(RI.Décal_max,0.5))*7-6,0)||':'||
       coalesce((RI.S_récolte)*7-6,0)||':'||
       coalesce((RI.S_récolte+coalesce(RI.D_récolte+coalesce(RI.Décal_max,0),0))*7-6,0) TEMPO, -- Si pas de récolte (D_récolte NULL) ne pas ajouter le décalage.

       RI.A_planifier,
       CASE WHEN (RI.Type_planche NOTNULL) AND
                 (RI.Type_planche<>(SELECT Type_planche
                                      FROM Rotations
                                     WHERE Rotation=R.Rotation) ) THEN RI.Type_planche END Conflit_type_planche,
       CASE WHEN R.IT_plante NOTNULL
            -- Recherche de familles trop proches dans une rotation.
            THEN (SELECT group_concat(result,x'0a0a') FROM
                 (SELECT DISTINCT result FROM
                 (SELECT RF.Famille || ' année ' || format('%i',RF.Année) result
                  -- FROM (RF_trop_proches(R.Rotation,R.Année,(SELECT Famille FROM R_famille F WHERE F.ID=R.ID))) || x'0a0a'
                  FROM R_famille RF
                  WHERE (RF.Rotation=R.Rotation)AND --ITP de la rotation
                        (RF.Année<>R.Année)AND -- que les ITP des autres années de la rotation
                        (RF.Famille=RI.Famille)AND -- que les ITP de la même famille
                        ((R.Année BETWEEN RF.Année-RF.Intervalle+1 AND RF.Année+RF.Intervalle-1)OR -- en conflit lors du 1er cycle
                         (R.Année BETWEEN RF.Année-RF.Intervalle+1+RF.Nb_années AND RF.Année+RF.Intervalle-1+RF.Nb_années))))) || x'0a0a' -- en conflit lors du 2ème cycle
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

CREATE VIEW RotDecalSMeP AS SELECT -- nb max de semaines de planche libre avant en fonction de la semaine de MEP.
    1 Semaine,
    23 Nb_sem
UNION
VALUES (2,23),(3,24),(4,24),(5,25), -- janv
       (6,25),(7,26),(8,27),(9,28), -- fév
       (10,29),(11,30),(12,31),(13,32), -- mars
       (14,33),(15,34),(16,34),(17,35), -- avr
       (18,35),(19,35),(20,34),(21,34),(22,33), -- mai
       (23,32),(24,31),(25,30),(26,29), -- juin
       (27,28),(28,27),(29,26),(30,25),(31,24), -- juil
       (32,23),(33,22),(34,21),(35,20), -- aout
       (36,20),(37,19),(38,19),(39,18), -- sept
       (40,18),(41,17),(42,17),(43,17),(44,18), -- oct
       (45,18),(46,19),(47,19),(48,20), -- nov
       (49,20),(50,21),(51,21),(52,22),(53,22); -- déc

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

CREATE VIEW Planif_planches AS SELECT
   PL.Planche,
   E.Espèce,
   RD.IT_plante,
   (SELECT V.Variété FROM Variétés V WHERE V.Espèce=I.Espèce ORDER BY V.Qté_stock DESC) Variété, -- Les IT des plans de rotation ont toujours une Espèce.
   (SELECT V.Fournisseur FROM Variétés V WHERE V.Espèce=I.Espèce ORDER BY V.Qté_stock DESC) Fournisseur,
   I.Date_semis,
   -- PlanifCultureCalcDate(CASE WHEN (I.S_plantation NOTNULL) AND (I.S_plantation<I.S_semis)
   --                            THEN (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
   --                            ELSE (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
   --                            END,
   --                       I.S_semis) Date_semis,
   I.Date_plantation,
   -- PlanifCultureCalcDate(coalesce(PlanifCultureCalcDate(CASE WHEN (I.S_plantation NOTNULL) AND (I.S_plantation<I.S_semis)
   --                                                      THEN (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
   --                                                      ELSE (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture') ||'-01-01'
   --                                                      END,
   --                                                      I.S_semis),
   --                                (SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture') ||'-01-01'),
   --                       I.S_plantation) Date_plantation,
   I.Début_récolte,
   I.Fin_récolte,
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
     JOIN ITP_planif I USING(IT_plante)
     LEFT JOIN Espèces E USING(Espèce)
WHERE ((SELECT (Valeur ISNULL) FROM Params WHERE Paramètre='Planifier_planches') OR
        (PL.Planche LIKE (SELECT Valeur FROM Params WHERE Paramètre='Planifier_planches')||'%')) AND
       ((RD.Fi_planches ISNULL) OR
        (Fi_planches LIKE '%'||substr(PL.Planche, -1, 1) ||'%'))
ORDER BY coalesce(Date_plantation, Date_semis);

-- CREATE VIEW Planif_planches_2 AS SELECT
--    C.Planche,
--    C.Espèce,
--    C.IT_plante,
--    C.Variété,
--    C.Fournisseur,
--    C.Date_semis,
--    C.Date_plantation,
--    CASE WHEN I.D_récolte NOTNULL
--         THEN  PlanifCultureCalcDate(coalesce(C.Date_plantation,C.Date_semis),I.S_récolte)
--         ELSE NULL
--         END Début_récolte,
--    CASE WHEN I.D_récolte NOTNULL
--         THEN DATE(PlanifCultureCalcDate(coalesce(C.Date_plantation,C.Date_semis),I.S_récolte),'+'||(I.D_récolte*7)||' days')
--         ELSE PlanifCultureCalcDate(coalesce(C.Date_plantation,C.Date_semis),I.S_récolte)
--         END Fin_récolte,
--    C.Longueur,
--    C.Nb_rangs,
--    C.Espacement,
--    C.Surface,
--    C.Prod_possible,
--    C.Info
-- FROM Planif_planches C
-- JOIN ITP I USING(IT_plante)
-- WHERE I.S_récolte NOTNULL;

CREATE VIEW Planif_récoltes AS
WITH RECURSIVE Semaines AS (
    SELECT
        Planche||Espèce||Début_récolte Id,
        Espèce,
        Début_récolte Date_jour,
        Fin_récolte,
        Prod_possible,
        0 Offset
    FROM Planif_planches WHERE (Début_récolte NOTNULL)AND(Fin_récolte NOTNULL)
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

CREATE VIEW Planif_ilots AS SELECT
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
FROM Planif_planches C
GROUP BY Ilot,Espèce
ORDER BY Ilot,Espèce;

CREATE VIEW Planif_espèces AS SELECT
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
FROM Planif_planches C
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
            THEN TEMPO ELSE ':::::::::::' END TEMPO_NP,
       CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
            THEN TEMPO ELSE ':::::::::::' END TEMPO_N,
       CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)AS TEXT)
            THEN TEMPO ELSE ':::::::::::' END TEMPO_NN,
       C.Saison,
       C.Longueur,
       C.Nb_rangs,
       C.Longueur*PL.Largeur Surface,
       PL.Irrig,
       E.Irrig Irrig_E,
       A_faire,
       C.Notes
  FROM Cultures__Tempo3 C
       LEFT JOIN Espèces E USING(Espèce)
       -- LEFT JOIN ITP I USING(IT_plante)
       LEFT JOIN Planches PL USING(Planche)
WHERE (Planche NOTNULL)AND
      (coalesce(Terminée,'') NOT LIKE 'v%')AND -- Pas de succession pour les vivaces.
      ((coalesce(Date_plantation,Date_semis) BETWEEN ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')-1)||'-01-01' AND
                                                     ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)||'-12-31'))
ORDER BY C.Planche,
         coalesce(C.Date_plantation,C.Date_semis);

CREATE VIEW Cultures__Tempo3 AS SELECT
       *,
       CAST(iif(Date_semis ISNULL,0,CAST(jdS-jd0 AS INTEGER)) AS TEXT)||':'||
       CAST(iif(Date_semis ISNULL,0,CAST(jdS-jd0 AS INTEGER)+4) AS TEXT)||':'||
       CAST(iif(Date_plantation ISNULL,0,CAST(jdP-jd0 AS INTEGER)) AS TEXT)||':'||
       CAST(iif(Date_plantation ISNULL,0,CAST(jdP-jd0 AS INTEGER)+4) AS TEXT)||':'||
       CAST(iif(Début_récolte ISNULL,0,CAST(jdDR-jd0 AS INTEGER)) AS TEXT)||':'||
       CAST(iif(Fin_récolte ISNULL,0,CAST(jdFR-jd0 AS INTEGER)+iif(Début_récolte=Fin_récolte,4,0)) AS TEXT)||':'||
       Espèce||':'||
       coalesce(Semis_fait,'')||':'||coalesce(Plantation_faite,'')||':'||coalesce(Récolte_faite,'')||':'||coalesce(Terminée,'')||':' TEMPO
FROM Cultures__Tempo4;

CREATE VIEW Cultures__Tempo4 AS SELECT
       *,
       julianday(substr(coalesce(Date_plantation,Date_semis),1,4)||'-01-01') jd0,
       julianday(Date_semis) jdS,
       julianday(Date_plantation) jdP,
       julianday(Début_récolte) jdDR,
       julianday(Fin_récolte) jdFR
FROM Cultures;

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
       CAST(round((SELECT Quantité FROM Recoltes_cult R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc,
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
FROM Cult_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
ORDER BY    C.Planche,
            coalesce(C.Date_semis,C.Date_plantation),
            C.Espèce,C.IT_plante;

CREATE VIEW Cult_non_ter AS SELECT *
FROM Cultures WHERE NOT((Terminée NOTNULL)AND(Terminée!='v')AND(Terminée!='V'));

CREATE VIEW Cult_ter AS SELECT *
FROM Cultures WHERE (Terminée NOTNULL)AND(Terminée!='v')AND(Terminée!='V');

CREATE VIEW Recoltes_cult AS SELECT
       C.Culture,
       count(*) Nb_réc,
       min(R.Date) Date_min,
       max(R.Date) Date_max,
       sum(R.Quantité) Quantité
FROM Récoltes R
JOIN Cultures C USING(Culture)
WHERE (coalesce(C.Terminée,'') NOT LIKE 'v%')OR(C.Début_récolte ISNULL)OR(C.Fin_récolte ISNULL)OR -- Toutes les récoltes pour les annuelles ou les vivaces sans période de récolte.
        -- Récolte de l'année pour les vivaces.
      ((R.Date >= DATE(C.Début_récolte,'-'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_avance')||' days'))AND
       (R.Date <= DATE(C.Fin_récolte,'+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_prolongation')||' days')))
GROUP BY Culture
ORDER BY Culture;

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
FROM Cult_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   (Date_semis < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_semis')||' days')) AND
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
FROM Cult_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   (Date_semis < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_semis')||' days')) AND
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
FROM Cult_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   (Date_semis < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_semis')||' days')) AND
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
FROM Cult_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   (Date_plantation < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_plantation')||' days')) AND
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
        CAST(round((SELECT Quantité FROM Recoltes_cult R WHERE R.Culture=C.Culture),3)AS REAL) Qté_réc,
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
FROM Cult_non_ter C
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   (Début_récolte < DATE('now','+'||(SELECT Valeur FROM Params WHERE Paramètre='C_horizon_récolte')||' days')) AND
        (coalesce(Récolte_faite,'') NOT LIKE 'x%')  AND
        ((Semis_fait NOTNULL)OR(Date_semis ISNULL)) AND
        ((Plantation_faite NOTNULL)OR(Date_plantation ISNULL))
ORDER BY    C.Début_récolte,
            C.Planche,C.Espèce,C.IT_plante;

CREATE VIEW Récoltes__Saisies AS SELECT
       R.ID,
       R.Date,
       C.Espèce,
       C.Planche Planche·s,
       R.Culture,
       R.Quantité,
       -- C.Planche,
       C.Variété,
       -- CAST(round((SELECT sum(Quantité) FROM Récoltes R2 WHERE (R2.Culture=R.Culture)AND(R2.Date<=R.Date)),3)AS REAL) Qté_réc, Triggers marchent mal avec cet appel.
       R.Notes
FROM Récoltes R
JOIN Cultures C USING(Culture)
WHERE (R.Date>DATE('now','-'||(SELECT Valeur FROM Params WHERE Paramètre='C_historique_récolte')||' days'))OR
      (DATE(R.Date) ISNULL) -- Détection de date incorecte
ORDER BY R.Date,C.Espèce,C.Planche,R.Culture;

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
FROM Cultures C -- Cult_non_ter C Les vivaces ne sont jamais à terminer.
LEFT JOIN Espèces E USING (Espèce)
LEFT JOIN ITP I USING (IT_plante)
LEFT JOIN Planches PL USING (Planche)
WHERE   C.Terminée ISNULL AND
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
       max(C.Pl_libre_le) Pl_libre_le,
       C.Date_MEP,
       min(C.Début_récolte) Début_récolte,
       max(C.Fin_récolte) Fin_récolte,
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
       C.Planche Planche·s,
       F.Culture,
       F.Fertilisant,
       F.Quantité,
       -- NULL Répartir,
       F.N,
       F.P,
       F.K,
       -- C.Planche,
       C.Variété,
       F.Notes
FROM Fertilisations F
JOIN Cultures C USING(Culture)
WHERE (F.Date>DATE('now','-'||(SELECT Valeur FROM Params WHERE Paramètre='Ferti_historique')||' days'))OR
      (DATE(F.Date) ISNULL) -- Détection de date incorecte
ORDER BY F.Date,F.Espèce,C.Planche,F.Culture;

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
            THEN TEMPO ELSE ':::::::::::' END TEMPO_NP,
       CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture')
            THEN TEMPO ELSE ':::::::::::' END TEMPO_N,
       CASE WHEN substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=CAST(((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)AS TEXT)
            THEN TEMPO ELSE ':::::::::::' END TEMPO_NN,
       C.Saison,
       C.Longueur,
       C.Nb_rangs,
       C.Longueur*PL.Largeur Surface,
       PL.Irrig Irrig_planche,
       E.Irrig Irrig_espèce,
       C.A_faire,
       C.Notes
  FROM Cultures__Tempo3 C
       LEFT JOIN Espèces E USING(Espèce)
       -- LEFT JOIN ITP I USING(IT_plante)
       LEFT JOIN Planches PL USING(Planche)
WHERE (Planche NOTNULL)AND
      (C.Terminée ISNULL) AND -- Que les annuelles non terminée.
      ((coalesce(Date_plantation,Date_semis) BETWEEN ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')-1)||'-01-01' AND
                                                     ((SELECT Valeur FROM Params WHERE Paramètre='Année_culture')+1)||'-12-31'))
ORDER BY C.Planche,
          coalesce(C.Date_plantation,C.Date_semis),C.Espèce,C.IT_plante;

CREATE VIEW C_à_irriguer AS SELECT
        C.Planche,
        C.Culture
FROM Cult_non_ter C
LEFT JOIN Espèces E USING (Espèce)
-- LEFT JOIN ITP I USING (IT_plante)
WHERE  (E.Irrig NOTNULL) AND
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
       (SELECT min(strftime('%U',C.Date_semis)+1) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) S_semis_min,
       (SELECT max(strftime('%U',C.Date_semis)+1) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) S_semis_max,
       (SELECT min(strftime('%U',C.Date_plantation)+1) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) S_plant_min,
       (SELECT max(strftime('%U',C.Date_plantation)+1) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) S_plant_max,
       (SELECT min(strftime('%U',C.Début_récolte)+1) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) S_récolte_min,
       (SELECT max(strftime('%U',C.Fin_récolte)+1) FROM C_ITP_analyse_a C WHERE C.IT_plante=I.IT_plante) S_récolte_max,
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
       CAST((SELECT count() FROM Cult_non_ter C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_NT,
       CAST((SELECT count() FROM Cult_ter C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_T,
       CAST((SELECT count() FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante)AS INTEGER) Nb_cu_S,
       (SELECT min(strftime('%U',C.Date_semis)+1) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) S_semis_min,
       (SELECT max(strftime('%U',C.Date_semis)+1) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) S_semis_max,
       (SELECT min(strftime('%U',C.Date_plantation)+1) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) S_plant_min,
       (SELECT max(strftime('%U',C.Date_plantation)+1) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) S_plant_max,
       (SELECT min(strftime('%U',C.Début_récolte)+1) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) S_récolte_min,
       (SELECT max(strftime('%U',C.Fin_récolte)+1) FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) S_récolte_max,
       (SELECT sum(ceil((julianday(min(C.Fin_récolte,DATE('now',coalesce(I.D_récolte*7,0)||' days')))-
                         julianday((SELECT min(R.Date) FROM Récoltes R WHERE R.Culture=C.Culture)))/365))
        FROM C_ITP_analyse_v C WHERE C.IT_plante=I.IT_plante) Nb_récoltes,
       CAST(round((SELECT sum(R.Quantité)
                   FROM C_ITP_analyse_v C LEFT JOIN Récoltes R USING(Culture)
                   WHERE C.IT_plante=I.IT_plante)/
                  (SELECT sum(ceil((julianday(min(C.Fin_récolte,DATE('now',coalesce(I.D_récolte*7,0)||' days')))-
                                    julianday((SELECT min(R.Date) FROM Récoltes R WHERE R.Culture=C.Culture)))/365))
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
       TEMPO,
       C.Notes
FROM Cultures__Tempo3 C
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
     LEFT JOIN Cult_inc_dates CID USING(Culture)
WHERE (coalesce(C.Terminée,'') NOT LIKE '%NS')AND
      ((C.Type LIKE '%?')OR
       (CID.Incohérence NOTNULL))
ORDER BY Culture;

CREATE VIEW Cult_inc_dates AS SELECT
       Culture,
       iif(Inc_semisD>0,'Semis trop tôt: '||Inc_semisD||' j',
       iif(Inc_semisF>0,'Semis trop tard: '||Inc_semisF||' j',
       iif(Inc_plantD>0,'Plant. trop tôt: '||Inc_plantD||' j',
       iif(Inc_plantF>0,'Plant. trop tard: '||Inc_plantF||' j',
       iif(Inc_RecD>0,'Récolte trop tôt: '||Inc_RecD||' j',
       iif(Inc_RecF>0,'Récolte trop tard: '||Inc_RecF||' j',
       NULL)))))) Incohérence
FROM Cult_inc_dates2;

CREATE VIEW Cult_inc_dates2 AS SELECT
       Culture,
       CAST((julianday(CP.Date_semis)-julianday(C.Date_semis)-(SELECT Valeur FROM Params WHERE Paramètre='Tolérance_A_semis'))AS INTEGER) Inc_semisD,
       CAST((julianday(C.Date_semis)-julianday(CP.Date_semis)-(coalesce(I.Décal_max,0)+1)*7-(SELECT Valeur FROM Params WHERE Paramètre='Tolérance_R_semis'))AS INTEGER) Inc_semisF,
       CAST((julianday(CP.Date_plantation)-julianday(C.Date_plantation)-(SELECT Valeur FROM Params WHERE Paramètre='Tolérance_A_plantation'))AS INTEGER) Inc_plantD,
       CAST((julianday(C.Date_plantation)-julianday(CP.Date_plantation)-(coalesce(I.Décal_max,0)+1)*7-(SELECT Valeur FROM Params WHERE Paramètre='Tolérance_R_plantation'))AS INTEGER) Inc_plantF,
       CAST((julianday(CP.Début_récolte)-julianday(C.Début_récolte)-(SELECT Valeur FROM Params WHERE Paramètre='Tolérance_A_récolte'))AS INTEGER) Inc_RecD,
       CAST((julianday(C.Fin_récolte)-julianday(CP.Fin_récolte)-(coalesce(I.Décal_max,0)+1)*7-(SELECT Valeur FROM Params WHERE Paramètre='Tolérance_R_récolte'))AS INTEGER) Inc_RecF
FROM Cultures C
LEFT JOIN Cult_dates_théo CP USING(Culture)
LEFT JOIN ITP I USING(IT_plante)
WHERE (coalesce(C.Terminée,'') NOT LIKE '%NS');

CREATE VIEW C_non_commencées AS --Cultures ni semées (SEP ou SPep) ni plantées.
    SELECT *
      FROM Cult_non_ter
     WHERE Semis_fait ISNULL AND Plantation_faite ISNULL
     ORDER BY Planche;

CREATE VIEW C_à_venir AS --Cultures ni semées (SD) ni plantées.
    SELECT *
      FROM Cult_non_ter
     WHERE NOT ((Semis_fait NOTNULL AND Date_plantation ISNULL) OR-- SD semé
                 Plantation_faite NOTNULL)-- Plant ou SPep planté
     ORDER BY Planche;

CREATE VIEW C_en_place AS --Cultures semées (SD) ou plantées.
    SELECT *
      FROM Cult_non_ter
     WHERE ( (Semis_fait NOTNULL AND Date_plantation ISNULL) OR-- SD semé
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
FROM Cult_non_ter C
LEFT JOIN ITP I USING(IT_plante)
LEFT JOIN Variétés V USING(Variété);

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
    CAST((SELECT sum(Prod_possible) FROM Planif_espèces C WHERE C.Espèce=E.Espèce)AS INTEGER) Prod_possible,
    round((SELECT sum(Prod_possible) FROM Planif_espèces C WHERE C.Espèce=E.Espèce)/E.Obj_annuel*100) Couv_pc,
    CAST((SELECT Valeur+1 FROM Params WHERE Paramètre='Année_culture')AS INTEGER) Année_à_planifier
FROM Espèces E
WHERE (E.A_planifier NOTNULL) AND
      (NOT(Espèce IN(SELECT Espèce FROM Planif_espèces))OR
       ((SELECT sum(Prod_possible) FROM Planif_espèces C WHERE C.Espèce=E.Espèce)<E.Obj_annuel));


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
        FROM Cult_non_ter C WHERE C.Planche=P.Planche) Cultures,
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
           JOIN Familles F USING(Famille);

CREATE VIEW R_ITP AS SELECT
    R.Rotation||format('%i', 2001+R.Année) ||'-'||
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
    coalesce(I.S_plantation, I.S_semis) S_MEP,
    coalesce(I.S_récolte+iif(coalesce(I.S_plantation,I.S_semis)>I.S_récolte,52,0),52)+coalesce(I.D_récolte+coalesce(I.Décal_max,0),0) S_Ferm, -- Si pas de durée de récolte (EV), ne pas prendre en compte le décalage, l'EV peut être viré quand on veut.
    coalesce(I.S_plantation, I.S_semis)+(R.Année-1)*52 S_MEP_abs,
    coalesce(I.S_récolte+iif(coalesce(I.S_plantation,I.S_semis)>I.S_récolte,52,0),52)+coalesce(I.D_récolte+coalesce(I.Décal_max,0),0)+(R.Année-1)*52 S_Ferm_abs,
    -- date(format('%i', 2001+R.Année)||'-01-01',(coalesce(I.S_plantation, I.S_semis))*7-6-1||' days') Date_MEP,
    -- date(format('%i', 2001+R.Année+iif(coalesce(I.S_plantation,I.S_semis)>I.S_récolte,1,0))||'-01-01',(coalesce(I.S_récolte,52))*7-6-1||' days') Date_Ferm,
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

CREATE VIEW R_ITP_2 AS SELECT -- Pour trouver la culture antérieure (si la courante n'est pas la 1ère) ou la dernière culture de la rotation.
    ID,
    Rotation,
    S_MEP_abs,
    S_Ferm_abs,
    Pc_planches,
    Fi_planches
FROM R_ITP
ORDER BY Ind DESC;

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
