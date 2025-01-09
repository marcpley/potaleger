QString sDDLViews = QStringLiteral(R"#(
BEGIN TRANSACTION;

CREATE VIEW test AS
    VALUES (Test('09-15','04-01')),
    (Test('10-01','02-01')),
    (Test('08-15','04-15')),
    (Test('12-15','04-15'));

CREATE VIEW Info_Potaléger AS
    SELECT 1 N,
           'Version de la BDD' Info,
           '2024-12-30' Valeur
    UNION
    SELECT 2,
           'Utilisateur',
           (SELECT Valeur
              FROM Params
             WHERE Paramètre='Utilisateur')
    UNION
    SELECT 3,
           'Variétés',
           (SELECT count() ||' dont '
              FROM Variétés) || (SELECT count() ||' en culture actuellement et '
                                   FROM (SELECT DISTINCT Variété
                                           FROM C_en_place
                                          ORDER BY Variété) ) || (SELECT count() ||' en culture planifiée.'
                                                                    FROM (SELECT DISTINCT Variété
                                                                            FROM C_a_venir
                                                                           ORDER BY Variété) )
    UNION
    SELECT 4,
           'Itinéraires techniques',
           (SELECT count() ||' dont '
              FROM ITP) || (SELECT count() ||' en culture actuellement et '
                              FROM (SELECT DISTINCT IT_plante
                                      FROM C_en_place
                                     ORDER BY IT_plante) ) || (SELECT count() ||' en culture planifiée.'
                                                                 FROM (SELECT DISTINCT IT_plante
                                                                         FROM C_a_venir
                                                                        ORDER BY IT_plante) )
    UNION
    SELECT 5,
           'Planches',
           (SELECT count() ||' dont '
              FROM Planches) || (SELECT count() ||' en culture actuellement et '
                                   FROM (SELECT DISTINCT Planche
                                           FROM C_en_place
                                          ORDER BY Planche) ) || (SELECT count() ||' en culture planifiée.'
                                                                    FROM (SELECT DISTINCT Planche
                                                                            FROM C_a_venir
                                                                           ORDER BY Planche) )
    UNION
    SELECT 6,
           'Cultures',
           (SELECT count() ||' dont '
              FROM Cultures) || (SELECT count() ||' non terminées.'
                                   FROM Cultures
                                  WHERE Terminée ISNULL);

CREATE VIEW Variétés__inv_et_cde AS SELECT E.Famille,
       V.Espèce,
       V.Variété,
       V.Qté_stock,
       V.Qté_cde,
       V.Fournisseur,
       E.Nb_graines_g,
       E.FG,
       (SELECT count()
          FROM C_en_place C
         WHERE (C.Variété=V.Variété) ) C_en_place,
       (SELECT count()
          FROM C_a_venir C
         WHERE (C.Variété=V.Variété) ) C_a_venir,
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
       ItpTempoNJPeriode(ItpTempoNJ(I.Déb_semis),ItpTempoNJ(I.Fin_semis),365) J_semis,
       ItpTempoNJPeriode(ItpTempoNJ(I.Déb_plantation),ItpTempoNJ(I.Fin_plantation),365) J_plantation,
       ItpTempoNJPeriode(ItpTempoNJ(I.Déb_récolte),ItpTempoNJ(I.Fin_récolte),365) J_récolte,
       I.Notes,
       I.Espèce,
       E.Famille
FROM ITP I
LEFT JOIN Espèces E USING(Espèce)
ORDER BY coalesce(I.Déb_semis,I.Déb_plantation,I.Type_culture);

CREATE VIEW Espèces__Stock_récoltes AS SELECT E.Espèce,
       E.Inventaire,
       E.Date_inv,
       E.Inventaire+(SELECT sum(R.Quantité)
                     FROM Récoltes R LEFT JOIN Cultures C USING(Culture)
                                     LEFT JOIN ITP I USING(IT_plante)
                     WHERE I.Espèce=E.Espèce) Stock_récolte
FROM Espèces E WHERE E.Inventaire NOTNULL;

CREATE VIEW Rotations__Tempo AS SELECT R.Rotation,
       R.Année,
       R.IT_plante,
       R.Pc_planches,
       R.Fi_planches,
       '<RotConflitMeP>'  Mise_en_place, --RI.MISE_EN_PLACE || RotConflitMeP(R.Rotation,RI.DATE_MEP,RI.NB_ANNEES) MISE_EN_PLACE,
       RI.Déb_récolte,
       RI.Fin_récolte,
       '<RotTempo>'  TEMPO, --RotTempo(RI.Type_culture, ItpTempoNJ(RI.DEB_SEMIS), ItpTempoNJ(RI.FIN_SEMIS), ItpTempoNJ(RI.DEB_PLANT), ItpTempoNJ(RI.FIN_PLANT), ItpTempoNJ(RI.DEB_REC), ItpTempoNJ(RI.FIN_REC) ) __Jan____Fév___Mars___Avril___Mai____Juin___Juil___Août____Sept___Oct____Nov____Déc__,
       RI.A_planifier,
       '<RotConflitPlanche>'  CONFLIT_TYPE_PLANCHE, --RotConflitPlanche(RI.TYPE_PLANCHE,R.Rotation) CONFLIT_TYPE_PLANCHE,
       '<RotConflitFamille>'  CONFLIT_FAMILLE, --CASE WHEN R.IT_plante NOTNULL THEN RotConflitFamille(R.ID) ELSE 'Familles possible : ' || RotFamPossibles(R.ID) END CONFLIT_FAMILLE,
       RI.Famille,
       RI.Intervalle,
       R.Notes
  FROM Rotations_détails R
       LEFT JOIN R_ITP RI USING(ID)
 ORDER BY RI.Ind;

CREATE VIEW IT_Rotations_vérif AS SELECT I.Espèce,
       coalesce(IR.IT_plante,IC.IT_plante) IT_plante,
       IR.Nb_planches Nb_planches_P,
       IR.Longueur Long_P,
       IR.Long_rang Long_rang_P,
       IR.Nb_plants Nb_plants_P,
       (SELECT Valeur FROM Params WHERE Paramètre='Année_planif') Année_à_planifier,
       IC.Nb_planches Nb_planches_C,
       IC.Longueur Long_C,
       IC.Long_rang Long_rang_C,
       IC.Nb_plants Nb_plants_C,
       (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') Année_en_cours,
       CASE WHEN IC.LONGUEUR NOTNULL THEN CAST((coalesce(IR.LONGUEUR,0)-coalesce(IC.LONGUEUR,0))/IC.LONGUEUR*100 AS INTEGER)
            ELSE 1000 END Diff_long,
       'pc' Unité
FROM IT_rotations IR
FULL JOIN IT_cultures IC USING(IT_plante)
LEFT JOIN ITP I USING(IT_plante)
UNION
SELECT I.Espèce,
       'z________________________TOTAL '||I.Espèce IT_plante,
       sum(IR.NB_PLANCHES) Nb_planches_P,
       sum(IR.LongueuR) Long_P,
       sum(IR.Long_rang) Long_rang_P,
       sum(IR.Nb_plants) Nb_plants_P,
       null Année_à_planifier,
       sum(IC.Nb_planches) Nb_planches_C,
       sum(IC.Longueur) Long_C,
       sum(IC.Long_rang) Long_rang_C,
       sum(IC.Nb_plants) Nb_plants_C,
       null Année_en_cours,
       CAST(sum(coalesce(IR.LONGUEUR,0)-coalesce(IC.LONGUEUR,0)) AS INTEGER) Diff_long,
       'm' Unité
FROM IT_rotations IR
FULL JOIN IT_cultures IC USING(IT_plante)
LEFT JOIN ITP I USING(IT_plante)
GROUP BY Espèce
ORDER BY Espèce,IT_plante;

CREATE VIEW Cult_planif AS SELECT trim(RD.IT_plante) IT_plante,
       (SELECT V.Variété
          FROM Variétés V
         WHERE V.Espèce=I.Espèce
         ORDER BY V.Qté_stock DESC) Variété,
       (SELECT V.Fournisseur
          FROM Variétés V
         WHERE V.Espèce=I.Espèce
         ORDER BY V.Qté_stock DESC) Fournisseur,
       PL.Planche,
       (SELECT Valeur
          FROM Params
         WHERE Paramètre='Année_planif') Année_à_planifier,
       PlanifCultureCalcDate(CASE WHEN (I.Déb_plantation NOTNULL) AND (I.Déb_plantation<I.Déb_semis)
                                  THEN (SELECT Valeur-1 FROM Params WHERE Paramètre='Année_planif') ||'-01-01'
                                  ELSE (SELECT Valeur FROM Params WHERE Paramètre='Année_planif') ||'-01-01'
                                  END,
                             I.Déb_semis) DATE_SEMIS,
       PlanifCultureCalcDate(coalesce(PlanifCultureCalcDate(CASE WHEN (I.Déb_plantation NOTNULL) AND (I.Déb_plantation<I.Déb_semis)
                                                            THEN (SELECT Valeur-1 FROM Params WHERE Paramètre='Année_planif') ||'-01-01'
                                                            ELSE (SELECT Valeur FROM Params WHERE Paramètre='Année_planif') ||'-01-01'
                                                            END,
                                                            I.Déb_semis),
                                      (SELECT Valeur FROM Params WHERE Paramètre='Année_planif') ||'-01-01'),
                             I.Déb_plantation) DATE_PLANTATION,
       RD.Pc_planches/100*PL.Longueur Longueur,
       I.Nb_rangs,
       I.Espacement,
       'Simulation planif' Info
  FROM Rotations_détails RD
       JOIN Planches PL USING(Rotation,
       Année)
       JOIN ITP I USING(IT_plante)
 WHERE ( (SELECT (Valeur ISNULL)
            FROM Params
           WHERE Paramètre='Planifier_planches') OR
         (PL.Planche LIKE (SELECT Valeur
                             FROM Params
                            WHERE Paramètre='Planifier_planches') ||'%') ) AND
       ( (RD.Fi_planches ISNULL) OR
         (Fi_planches LIKE '%'||substr(PL.Planche, -1, 1) ||'%') )
 ORDER BY coalesce(Date_plantation, Date_semis);

CREATE VIEW Successions_par_planche AS SELECT PL.Planche,
       (SELECT count()
          FROM C_en_place C
         WHERE C.Planche=PL.Planche ) Nb_cu_EP,
       (SELECT group_concat(IT_plante, '/')
          FROM (SELECT DISTINCT IT_plante
                  FROM C_en_place C
                 WHERE C.Planche=PL.Planche) ) ITP_en_place,
       (SELECT max(Fin_récolte)
          FROM C_en_place C
         WHERE C.Planche=PL.Planche ) Libre_le,

       (SELECT count()
          FROM C_a_venir C
         WHERE C.Planche=PL.Planche) Nb_cu_AV,
       (SELECT group_concat(IT_plante, '/')
          FROM (SELECT DISTINCT IT_plante
                  FROM C_a_venir C
                 WHERE C.Planche=PL.Planche)) ITP_à_venir,

       (SELECT min(coalesce(Date_semis,Date_plantation))
          FROM C_a_venir C
         WHERE C.Planche=PL.Planche ) A_faire_le
  FROM Planches PL;

CREATE VIEW Cultures__non_terminées AS SELECT C.Planche,
       C.Culture,
       C.IT_plante,
       C.Variété,
       C.Type,
       C.Etat,
       C.D_planif,
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
       I.Nb_graines_trou,
       I.Dose_semis,
       round(C.Longueur * C.Nb_rangs / C.Espacement * 100) Nb_plants_planche,
       C.Notes,
       I.Notes N_IT_plante,
       PL.Notes N_Planche
  FROM Cultures C
       LEFT JOIN
       ITP I USING (
           IT_plante
       )
       LEFT JOIN
       Planches PL USING (
           Planche
       )
 WHERE C.Terminée ISNULL
 ORDER BY C.Planche,
          C.Date_plantation,
          C.IT_plante;

CREATE VIEW Cultures__semences_nécessaires AS SELECT C.Variété, --todo fusionner avec inventaire
       count() Nb_cult,
       min(coalesce(C.Date_semis,C.Date_plantation)) Date,
       sum(C.Longueur) Long_planches,
       round(sum(C.Longueur*PL.Largeur),2) Surf_planches,
       round(sum(C.Longueur * C.Nb_rangs / C.Espacement * 100)) Nb_plants,
       round(sum(C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou)) Nb_graines,
       round(sum(CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou / VC.Nb_graines_g
                 ELSE C.Longueur * PL.Largeur * I.Dose_semis END),2) Poids_graines,
       round((SELECT sum(V.Qté_stock) FROM Variétés V WHERE V.Variété=VC.Variété),2) Qté_stock,
       round((SELECT sum(V.Qté_cde) FROM Variétés V WHERE V.Variété=VC.Variété),2) Qté_cde,
       max(round((sum(CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou / VC.Nb_graines_g
                      ELSE C.Longueur * PL.Largeur * I.Dose_semis END))
                -(SELECT sum(coalesce(V.Qté_stock,0)+coalesce(V.Qté_cde,0)) FROM Variétés V WHERE V.Variété=VC.Variété),2),0) Qté_manquante,
       VC.Notes N_variété
  FROM Cultures C
       LEFT JOIN
       ITP I USING (
           IT_plante
       )
       LEFT JOIN
       Planches PL USING (
           Planche
       )
       LEFT JOIN Variétés VC USING(Variété)
WHERE C.Terminée ISNULL AND
      ((C.Date_semis NOTNULL AND C.Semis_fait ISNULL)OR--SD ou SSA à faire
       (C.Date_semis ISNULL AND C.Date_plantation NOTNULL AND C.Plantation_faite ISNULL))--Plant à planter
 GROUP BY C.Variété
 ORDER BY C.Variété;

CREATE VIEW Cultures__Semis_à_faire AS SELECT C.Planche,
       C.Culture,
       C.IT_plante,
       C.Variété,
       C.Type,
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
       I.Nb_graines_trou *1 Nb_graines_trou,
       I.Dose_semis,
       round(C.Longueur * C.Nb_rangs / C.Espacement * 100) Nb_plants_planches,
       round(C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou) Nb_graines_planches,
       round(CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou / E.Nb_graines_g
       ELSE C.Longueur * PL.Largeur * I.Dose_semis END) Poids_graines_planches,
       C.Notes,
       I.Notes N_IT_plante,
       PL.Notes N_planche,
       E.Notes N_espèce
  FROM Cultures C
       LEFT JOIN
       ITP I USING (
           IT_plante
       )
       LEFT JOIN
       Planches PL USING (
           Planche
       )
       LEFT JOIN
       Espèces E USING (
           Espèce
       )
 WHERE (Terminée ISNULL) AND
       (Date_semis < DATE('now','+30 days')) AND
       (Semis_fait ISNULL)
 ORDER BY Date_semis,
          Planche,
          IT_plante;

CREATE VIEW Cultures__Plantations_à_faire AS SELECT C.Planche,
       C.Culture,
       C.IT_plante,
       C.Variété,
       C.Type,
       C.Date_semis,
       C.Semis_fait,
       C.Date_plantation,
       C.Plantation_faite,
       C.Début_récolte,
       C.Longueur,
       PL.Largeur,
       C.Nb_rangs,
       C.Espacement,
       round(C.Longueur * C.Nb_rangs / C.Espacement * 100) Nb_plants_planche,
       C.Notes,
       P.Notes N_plante,
       PL.Notes N_planche
  FROM Cultures C
       LEFT JOIN
       ITP P USING (
           IT_plante
       )
       LEFT JOIN
       Planches PL USING (
           Planche
       )
 WHERE (Terminée ISNULL) AND
       (Date_plantation < DATE('now','+60 days')) AND
       (Plantation_faite ISNULL) AND
       ((Semis_fait NOTNULL)OR(Date_semis ISNULL))
 ORDER BY Date_plantation,
          Planche,
          IT_plante;

CREATE VIEW Cultures__Récoltes_à_faire AS SELECT C.Planche,
       C.Culture,
       C.IT_plante,
       C.Type,
       C.Date_semis,
       C.Semis_fait,
       C.Date_plantation,
       C.Plantation_faite,
       C.Début_récolte,
       C.Fin_récolte,
       C.Récolte_faite,
       (SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture) Déja_réc,
       C.Terminée,
       C.Longueur,
       C.Notes,
       I.Notes N_IT_plante,
       PL.Notes N_Planche
  FROM Cultures C
       LEFT JOIN
       ITP I USING (
           IT_plante
       )
       LEFT JOIN
       Planches PL USING (
           Planche
       )
 WHERE (C.Terminée ISNULL) AND
       (Début_récolte < DATE('now','+30 days')) AND
       (Récolte_faite ISNULL)  AND
       ((Semis_fait NOTNULL)OR(Date_semis ISNULL)) AND
       ((Plantation_faite NOTNULL)OR(Date_plantation ISNULL))
 ORDER BY C.Planche,
          C.Début_récolte,
          C.IT_plante;

CREATE VIEW Saisie_récoltes AS SELECT R.*,
       C.Planche,
       I.Espèce
FROM Récoltes R
LEFT JOIN Cultures C USING(Culture)
LEFT JOIN ITP I USING(IT_plante)
WHERE R.Date>DATE('now','-30 days')
ORDER BY R.Date,R.Culture;

CREATE VIEW Cultures__à_terminer AS SELECT C.Planche,
       C.Culture,
       C.IT_plante,
       C.Type,
       C.Date_semis,
       C.Semis_fait,
       C.Date_plantation,
       C.Plantation_faite,
       C.Début_récolte,
       C.Fin_récolte,
       C.Récolte_faite,
       C.Terminée,
       C.Longueur,
       C.Notes,
       I.Notes N_IT_plante,
       PL.Notes N_Planche
  FROM Cultures C
       LEFT JOIN
       ITP I USING (
           IT_plante
       )
       LEFT JOIN
       Planches PL USING (
           Planche
       )
 WHERE (C.Terminée ISNULL) AND
       ((C.Fin_récolte ISNULL)OR(C.Fin_récolte < DATE('now','+30 days'))) AND
       ((Semis_fait NOTNULL)OR(Date_semis ISNULL)) AND
       ((Plantation_faite NOTNULL)OR(Date_plantation ISNULL)) AND
       ((Récolte_faite NOTNULL)OR(Début_récolte ISNULL))
 ORDER BY C.Planche,
          C.Fin_récolte,
          C.IT_plante;

CREATE VIEW ITP__analyse AS SELECT I.IT_plante,
       I.Type_culture,
       I.Déb_semis,
       I.Déb_plantation,
       I.Déb_récolte,
       (SELECT count()
          FROM Cultures C
         WHERE C.IT_plante=I.IT_plante) Nb_cult,
       (SELECT count()
          FROM Cultures C
         WHERE (C.IT_plante=I.IT_plante) AND
               (C.Terminée NOTNULL) ) Nb_ter,
       (SELECT min(substr(C.Date_semis, 6) )
          FROM Cultures C
         WHERE C.IT_plante=I.IT_plante) Min_semis,
       (SELECT max(substr(C.Date_semis, 6) )
          FROM Cultures C
         WHERE C.IT_plante=I.IT_plante) Max_semis,
       (SELECT min(substr(C.Date_plantation, 6) )
          FROM Cultures C
         WHERE C.IT_plante=I.IT_plante) Min_plantation,
       (SELECT max(substr(C.Date_plantation, 6) )
          FROM Cultures C
         WHERE C.IT_plante=I.IT_plante) Max_plantation,
       (SELECT min(substr(C.Début_récolte, 6) )
          FROM Cultures C
         WHERE C.IT_plante=I.IT_plante) Min_recolte,
       (SELECT max(substr(C.Fin_récolte, 6) )
          FROM Cultures C
         WHERE C.IT_plante=I.IT_plante) Max_recolte,
       (SELECT sum(Quantité)
          FROM Cultures C LEFT JOIN Récoltes R USING(Culture)
         WHERE (C.IT_plante=I.IT_plante) AND
               (C.Terminée NOTNULL) )/
       (SELECT count()
          FROM Cultures C JOIN Récoltes R USING(Culture)
         WHERE (C.IT_plante=I.IT_plante) AND
               (C.Terminée NOTNULL) ) Qté_réc_moy
  FROM ITP I
 ORDER BY IT_plante;

CREATE VIEW Cultures__Tempo_Espèce AS SELECT C.Culture,
       C.IT_plante,
       C.Variété,
       C.Planche,
       C.Type,
       C.Date_semis,
       C.Date_plantation,
       C.Début_récolte,
       C.Fin_récolte,
       C.Récolte_faite,
       CulTempo(C.Type, C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte) TEMPO,
       C.Notes
  FROM Cultures C
       LEFT JOIN ITP I USING(IT_plante)
 WHERE C.Terminée NOTNULL
ORDER BY C.IT_plante,
          coalesce(C.Date_semis, C.Date_plantation);

CREATE VIEW C_non_commencées AS --Cultures ni semées (SD ou SSA) ni plantées.
    SELECT *
      FROM Cultures
     WHERE Terminée ISNULL AND Semis_fait ISNULL AND Plantation_faite ISNULL
     ORDER BY Planche;

CREATE VIEW C_a_venir AS --Cultures ni semées (SD) ni plantées.
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

CREATE VIEW F_actives AS
    SELECT *
      FROM Familles F
     WHERE ( (SELECT count()
                FROM ITP
                     LEFT JOIN Espèces USING(Espèce)
               WHERE Espèces.Famille=F.Famille) >0);

CREATE VIEW IT_cultures AS
    SELECT IT_plante,
           sum(NB_PLANCHES) NB_PLANCHES,
           sum(LONGUEUR) LONGUEUR,
           sum(LONG_RANG) LONG_RANG,
           sum(NB_PLANTS) NB_PLANTS,
           (SELECT Valeur
              FROM Params
             WHERE Paramètre='Année_culture') ANNEE
      FROM IT_cultures_ilots
     GROUP BY IT_plante;

CREATE VIEW IT_cultures_ilots AS
    SELECT IT_plante,
           substr(PLANCHE, 1, (SELECT Valeur
                                 FROM Params
                                WHERE Paramètre='Ilot_nb_car') ) ILOT,
           sum(NB_PLANCHES) NB_PLANCHES,
           sum(LONGUEUR) LONGUEUR,
           sum(LONG_RANG) LONG_RANG,
           sum(NB_PLANTS) NB_PLANTS,
           (SELECT Valeur
              FROM Params
             WHERE Paramètre='Année_culture') ANNEE
      FROM IT_cultures_planches
     GROUP BY IT_plante,
              ILOT;

CREATE VIEW IT_cultures_planches AS
    SELECT trim(C.IT_plante) IT_plante,
           C.Planche PLANCHE,
           1 NB_PLANCHES,-- 2 ITP sur la même planche la même année se partagent la planche, donc on en compte qu'une
           sum(C.Longueur) LONGUEUR,
           sum(C.Longueur*coalesce(C.Nb_rangs, 1) ) LONG_RANG,
           CAST(sum(C.Longueur*coalesce(C.Nb_rangs, 1) /coalesce(C.Espacement/100, 1) ) AS INTEGER) NB_PLANTS,-- C.Longueur,
           (SELECT Valeur
              FROM Params
             WHERE Paramètre='Année_culture') ANNEE
      FROM Cultures C-- LEFT JOIN Rotations R USING(Rotation)
     WHERE (substr(coalesce(C.Date_plantation, C.Date_semis), 1, 4) = (SELECT Valeur-- LEFT JOIN Planches P USING(Rotation)
                                                                         FROM Params
                                                                        WHERE Paramètre='Année_culture') )
     GROUP BY IT_plante,
              PLANCHE
     ORDER BY IT_plante,
              PLANCHE;

CREATE VIEW IT_rotations AS
    SELECT Espèce,
           IT_plante,
           sum(Nb_planches) Nb_planches,
           sum(Longueur) Longueur,
           sum(Long_rang) Long_rang,
           sum(Nb_plants) Nb_plants,
           (SELECT Valeur
              FROM Params
             WHERE Paramètre='Année_planif') Année_à_planifier
      FROM IT_rotations_ilots
     GROUP BY IT_plante;

CREATE VIEW IT_rotations_ilots AS
    SELECT E.Espèce,
           RD.IT_plante,
           substr(P.Planche, 1, (SELECT Valeur
                                   FROM Params
                                  WHERE Paramètre='Ilot_nb_car') ) Ilot,
           count() Nb_planches,
           sum(P.Longueur*RD.Pc_planches/100) Longueur,
           sum(P.Longueur*coalesce(I.Nb_rangs, 1) *RD.Pc_planches/100) Long_rang,
           CAST(sum(P.Longueur*coalesce(I.Nb_rangs, 1) /coalesce(I.Espacement/100, 1) *RD.Pc_planches/100) AS INTEGER) Nb_plants,
           (SELECT Valeur
              FROM Params
             WHERE Paramètre='Année_planif') Année_à_planifier
      FROM Rotations_détails RD
           LEFT JOIN ITP I USING(IT_plante)
           LEFT JOIN Espèces E USING(Espèce)
           LEFT JOIN Rotations R USING(Rotation)
           LEFT JOIN Planches P ON (P.Rotation=RD.Rotation) AND
                                   (P.Année=RD.Année) AND
                                   (RD.Fi_planches ISNULL OR
                                    ((RD.Fi_planches LIKE '%'||substr(P.Planche, -1, 1) ||'%') ) )
     WHERE IT_plante NOTNULL
     GROUP BY Espèce,
              IT_plante,
              Ilot
     ORDER BY Espèce,
              IT_plante,
              Ilot;

CREATE VIEW IT_rotations_manquants AS
    SELECT Espèce,
           IT_plante,
           E.Rendement,
           E.Niveau,
           E.Apport,
           (SELECT sum(Qté_stock) FROM Variétés V WHERE V.Espèce=E.Espèce) Qté_stock,
           ITP.Type_planche,
           ITP.Type_culture,
           ITP.Déb_semis,
           ITP.Déb_plantation,
           ITP.Déb_récolte,
           (SELECT Valeur FROM Params WHERE Paramètre='Année_planif') Année_à_planifier
      FROM ITP
      LEFT JOIN Espèces E USING(Espèce)
     WHERE NOT (Espèce IN(SELECT Espèce FROM IT_rotations_ilots) ) AND
           E.A_planifier NOTNULL;

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
    SELECT R.Rotation||format('%i', 2000+R.Année) ||'-'||coalesce(I.Déb_plantation, I.Déb_semis) ||format('%i', 2000+R.Année+iif(coalesce(I.Déb_plantation, I.Déb_semis) >coalesce(I.Déb_récolte, I.Fin_récolte), 1, 0) ) ||'-'||coalesce(I.Déb_récolte, I.Fin_récolte, '12-31') Ind,-- coalesce(R.Fi_planches,'')||
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
           count() Nb_planches,
           sum(P.Longueur) Longueur,
           sum(P.Longueur*P.Largeur) Surface,
           P.Rotation-- group_concat(P.Rotation, '/') ROTATION
      FROM Planches P
     GROUP BY Ilot,
              Type,
              Rotation
     ORDER BY Ilot,
              Type,
              Rotation;

COMMIT TRANSACTION;
)#");
