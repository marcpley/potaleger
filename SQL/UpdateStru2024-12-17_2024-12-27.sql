QString sDDL20241227 = QStringLiteral(R"#(
BEGIN TRANSACTION;
DROP VIEW IF EXISTS "1             DONNEES";
DROP VIEW IF EXISTS "1-1 Inv et cde variétés";
CREATE VIEW "1-1_Inv_et_cde_variétés" AS SELECT trim(E.Famille) FAMILLE,
       V.Espèce,
       V.Variété,
       V.Qté_stock,
       V.Qté_cde,
       V.Fournisseur,
       E.Nb_graines_g,
       E.[FG(i)],
       (SELECT count()
          FROM C_en_place C
         WHERE (C.Variété=V.Variété) ) C_EN_PLACE,
       (SELECT count()
          FROM C_a_venir C
         WHERE (C.Variété=V.Variété) ) C_A_VENIR,
       V.Notes,
       E.Notes N_espèce,
       F.Notes N_famille
  FROM Variétés V
       LEFT JOIN Espèces E USING(Espèce)
       LEFT JOIN Familles F USING(Famille)
 ORDER BY E.Famille,
          V.Espèce,
          V.Variété;
DROP VIEW IF EXISTS "1-2 Tempo ITP";
CREATE VIEW "1-2_Tempo_ITP" AS SELECT I.IT_plante,
       I.Type_planche,
       I."TYPE(a)",
       I.Déb_semis,
       I.Fin_semis,
       I.Déb_plantation,
       I.Fin_plantation,
       I.Déb_récolte,
       I.Fin_récolte,
       ItpTempo(I."TYPE(a)",
                ItpTempoNJ(I.Déb_semis),
                ItpTempoNJ(I.Fin_semis),
                ItpTempoNJ(I.Déb_plantation),
                ItpTempoNJ(I.Fin_plantation),
                ItpTempoNJ(I.Déb_récolte),
                ItpTempoNJ(I.Fin_récolte)) __Jan____Fév___Mars___Avril___Mai____Juin___Juil___Août____Sept___Oct____Nov____Déc__,
       ItpTempoNJPeriode(ItpTempoNJ(I.Déb_semis),ItpTempoNJ(I.Fin_semis),365) J_SEMIS,
       ItpTempoNJPeriode(ItpTempoNJ(I.Déb_plantation),ItpTempoNJ(I.Fin_plantation),365) J_PLANTATION,
       ItpTempoNJPeriode(ItpTempoNJ(I.Déb_récolte),ItpTempoNJ(I.Fin_récolte),365) J_RECOLTE,
       I.Notes,
       trim(I.Espèce) ESPECE,
       trim(E.Famille) FAMILLE
FROM ITP I
LEFT JOIN Espèces E USING(Espèce)
ORDER BY coalesce(I.Déb_semis,I.Déb_plantation,I."TYPE(a)");
DROP VIEW IF EXISTS "1-4 Stock récoltes";
CREATE VIEW "1-4_Stock_récoltes" AS SELECT E.Espèce,
       E.Inventaire,
       E.Date_inv,
       E.Inventaire+(SELECT sum(R.Quantité)
                     FROM Récoltes R LEFT JOIN Cultures C USING(Culture)
                                     LEFT JOIN ITP I USING(IT_Plante)
                     WHERE I.Espèce=E.Espèce) Stock_récolte
FROM Espèces E WHERE E.Inventaire NOTNULL;
DROP VIEW IF EXISTS "2             PLANIFICATION";
DROP VIEW IF EXISTS "2-1 Rotations Tempo";
CREATE VIEW "2-1_Rotations_Tempo" AS SELECT R.Rotation,
       R.Année,
       R.IT_Plante,
       R.Pc_planches,
       R.Fi_planches,
       RI.MISE_EN_PLACE || RotConflitMeP(R.Rotation,RI.DATE_MEP,RI.NB_ANNEES) MISE_EN_PLACE,
       trim(RI.DEB_REC) DEB_REC,
       trim(RI.FIN_REC) FIN_REC,
       RotTempo(RI.[TYPE(a)], ItpTempoNJ(RI.DEB_SEMIS), ItpTempoNJ(RI.FIN_SEMIS), ItpTempoNJ(RI.DEB_PLANT), ItpTempoNJ(RI.FIN_PLANT), ItpTempoNJ(RI.DEB_REC), ItpTempoNJ(RI.FIN_REC) ) __Jan____Fév___Mars___Avril___Mai____Juin___Juil___Août____Sept___Oct____Nov____Déc__,
       RI.A_planifier Espèce_à_planifier,
       RotConflitPlanche(RI.TYPE_PLANCHE,R.Rotation) CONFLIT_TYPE_PLANCHE,
       CASE WHEN R.IT_Plante NOTNULL THEN RotConflitFamille(R.ID) ELSE 'Familles possible : ' || RotFamPossibles(R.ID) END CONFLIT_FAMILLE,
       trim(RI.FAMILLE) FAMILLE,
       trim(RI.INTERV) INTERV,
       R.Notes
  FROM Rotations_détails R
       LEFT JOIN R_ITP RI USING(ID)
 ORDER BY RI.Ind;
DROP VIEW IF EXISTS "2-2 Rotations ITP Ilot";
CREATE VIEW "2-2_Rotations_ITP_Ilot" AS SELECT IT_PLANTE,
       ILOT,
       LONGUEUR,
       ANNEE
  FROM ITP_rotations_ilots
UNION
SELECT IT_Plante,
       NULL ILOT,
       0 LONGUEUR,
       (SELECT Valeur
          FROM Params
         WHERE Paramètre='Année planif') ANNEE
  FROM ITP LEFT JOIN Espèces E USING(Espèce)
 WHERE NOT (IT_Plante IN(SELECT IT_PLANTE FROM ITP_rotations_ilots))AND
       E.A_planifier NOTNULL;
DROP VIEW IF EXISTS "2-3 Rotations vérif";
CREATE VIEW "2-3_Rotations_vérif" AS SELECT trim(I.Espèce) ESPECE,
       coalesce(IR.IT_PLANTE,IC.IT_PLANTE) IT_PLANTE,
       IR.NB_PLANCHES NB_PLANCHES_P,
       IR.LONGUEUR LONG_P,
       IR.LONG_RANG LONG_RANG_P,
       IR.NB_PLANTS NB_PLANTS_P,
       (SELECT Valeur FROM Params WHERE Paramètre='Année_planif') PLANIF,
       IC.NB_PLANCHES NB_PLANCHES_C,
       IC.LONGUEUR LONG_C,
       IC.LONG_RANG LONG_RANG_C,
       IC.NB_PLANTS NB_PLANTS_C,
       (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') CULTURES,
       CASE WHEN IC.LONGUEUR NOTNULL THEN CAST((coalesce(IR.LONGUEUR,0)-coalesce(IC.LONGUEUR,0))/IC.LONGUEUR*100 AS INTEGER)
            ELSE 1000 END DIFF_LONG,
       'pc' UNITE
FROM ITP_rotations IR
FULL JOIN ITP_cultures IC USING(IT_PLANTE)
LEFT JOIN ITP I USING(IT_PLANTE)
UNION
SELECT trim(I.Espèce) ESPECE,
       'z------------------------TOTAL '||I.Espèce IT_PLANTE,
       sum(IR.NB_PLANCHES) NB_PLANCHES_P,
       sum(IR.LONGUEUR) LONG_P,
       sum(IR.LONG_RANG) LONG_RANG_P,
       sum(IR.NB_PLANTS) NB_PLANTS_P,
       null PLANIF,
       sum(IC.NB_PLANCHES) NB_PLANCHES_C,
       sum(IC.LONGUEUR) LONG_C,
       sum(IC.LONG_RANG) LONG_RANG_C,
       sum(IC.NB_PLANTS) NB_PLANTS_C,
       null CULTURES,
       CAST(sum(coalesce(IR.LONGUEUR,0)-coalesce(IC.LONGUEUR,0)) AS INTEGER) DIFF_LONG,
       'm' UNITE
FROM ITP_rotations IR
FULL JOIN ITP_cultures IC USING(IT_PLANTE)
LEFT JOIN ITP I USING(IT_PLANTE)
GROUP BY ESPECE
ORDER BY ESPECE,IT_PLANTE;
DROP VIEW IF EXISTS "2-4 Cultures planif";
CREATE VIEW "2-4_Cultures_planif" AS SELECT trim(RD.IT_Plante) IT_PLANTE,
       (SELECT V.Variété
          FROM Variétés V
         WHERE V.Espèce=I.Espèce
         ORDER BY V.Qté_stock DESC) VARIETE,
       (SELECT V.Fournisseur
          FROM Variétés V
         WHERE V.Espèce=I.Espèce
         ORDER BY V.Qté_stock DESC) FOURNISSEUR,
       trim(PL.Planche) PLANCHE,
       (SELECT Valeur
          FROM Params
         WHERE Paramètre='Année_planif') D_PLANIF,
       PlanifCultureCalcDate(I.Déb_semis, CASE WHEN (I.Déb_plantation NOTNULL) AND
                                                    (I.Déb_plantation<I.Déb_semis) THEN (SELECT Valeur-1
                                                                                           FROM Params
                                                                                          WHERE Paramètre='Année_planif') ||'-01-01' ELSE (SELECT Valeur
                                                                                                                                             FROM Params
                                                                                                                                            WHERE Paramètre='Année_planif') ||'-01-01' END) DATE_SEMIS,
       PlanifCultureCalcDate(I.Déb_plantation, coalesce(PlanifCultureCalcDate(I.Déb_semis, CASE WHEN (I.Déb_plantation NOTNULL) AND
                                                                                                     (I.Déb_plantation<I.Déb_semis) THEN (SELECT Valeur-1
                                                                                                                                            FROM Params
                                                                                                                                           WHERE Paramètre='Année_planif') ||'-01-01' ELSE (SELECT Valeur
                                                                                                                                                                                              FROM Params
                                                                                                                                                                                             WHERE Paramètre='Année_planif') ||'-01-01' END), (SELECT Valeur
                                                                                                                                                                                                                                                 FROM Params
                                                                                                                                                                                                                                                WHERE Paramètre='Année_planif') ||'-01-01') ) DATE_PLANTATION,
       RD.Pc_planches/100*PL.Longueur LONGUEUR,
       I.Nb_rangs*100/100 NB_RANGS,
       I.Espacement*100/100 ESPACEMENT,
       'Simulation planif' INFO
  FROM Rotations_détails RD
       JOIN Planches PL USING(Rotation,
       Année)
       JOIN ITP I USING(IT_Plante)
 WHERE ( (SELECT (Valeur ISNULL)
            FROM Params
           WHERE Paramètre='Planifier_planches') OR
         (PL.Planche LIKE (SELECT Valeur
                             FROM Params
                            WHERE Paramètre='Planifier_planches') ||'%') ) AND
       ( (RD.Fi_planches ISNULL) OR
         (Fi_planches LIKE '%'||substr(PL.Planche, -1, 1) ||'%') )
 ORDER BY coalesce(DATE_PLANTATION, DATE_SEMIS);
DROP VIEW IF EXISTS "2-4 Successions par planche";
CREATE VIEW "2-4_Successions_par_planche" AS SELECT PL.Planche,
       (SELECT count()
          FROM C_en_place C
         WHERE C.Planche=PL.Planche ) NB_CU_EP,
       (SELECT group_concat(IT_Plante, '/')
          FROM (SELECT DISTINCT IT_Plante
                  FROM C_en_place C
                 WHERE C.Planche=PL.Planche) ) ITP_EN_PLACE,
       (SELECT max(Fin_récolte)
          FROM C_en_place C
         WHERE C.Planche=PL.Planche ) LIBRE_LE,

       (SELECT count()
          FROM C_a_venir C
         WHERE C.Planche=PL.Planche) NB_CU_AV,
       (SELECT group_concat(IT_Plante, '/')
          FROM (SELECT DISTINCT IT_Plante
                  FROM C_a_venir C
                 WHERE C.Planche=PL.Planche)) ITP_A_VENIR,

       (SELECT min(coalesce(Date_semis,Date_plantation))
          FROM C_a_venir C
         WHERE C.Planche=PL.Planche ) A_FAIRE_LE
  FROM Planches PL;
DROP VIEW IF EXISTS "3             SUIVI DES CULTURES";
DROP VIEW IF EXISTS "3-0 Cultures non terminées";
CREATE VIEW "3-0_Cultures_non_terminées" AS SELECT C.Planche,
       C.Culture * 10/10 CULTURE,
       C.IT_plante,
       C.Variété,
       C."TYPE(a)",
       ColoCult(C.Culture) Couleur,
       C."D_planif(a)",
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
       round(C.Longueur * C.Nb_rangs / C.Espacement * 100) NB_PLANTS_PLANCHE,
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
DROP VIEW IF EXISTS "3-1 Semences nécessaires";
CREATE VIEW "3-1_Semences_nécessaires" AS SELECT C.Variété VARIETE,
       count() NB_CULT,
       min(coalesce(C.Date_semis,C.Date_plantation)) DATE,
       sum(C.Longueur) LONG_PLANCHES,
       round(sum(C.Longueur*PL.Largeur),2) SURF_PLANCHES,
       round(sum(C.Longueur * C.Nb_rangs / C.Espacement * 100)) NB_PLANTS,
       round(sum(C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou)) NB_GRAINES,
       round(sum(CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou / VC.Nb_graines_g
                 ELSE C.Longueur * PL.Largeur * I.Dose_semis END),2) POIDS_GRAINES,
       round((SELECT sum(V.Qté_stock) FROM Variétés V WHERE V.Variété=VC.Variété),2) QTE_STOCK,
       round((SELECT sum(V.Qté_cde) FROM Variétés V WHERE V.Variété=VC.Variété),2) QTE_CDE,
       max(round((sum(CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou / VC.Nb_graines_g
                      ELSE C.Longueur * PL.Largeur * I.Dose_semis END))
                -(SELECT sum(coalesce(V.Qté_stock,0)+coalesce(V.Qté_cde,0)) FROM Variétés V WHERE V.Variété=VC.Variété),2),0) QTE_MANQUANTE,
       VC.Notes N_VARIETE
  FROM Cultures C
       LEFT JOIN
       ITP I USING (
           It_plante
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
DROP VIEW IF EXISTS "3-2 Semis à faire ";
CREATE VIEW "3-2_Semis_à_faire " AS SELECT C.Planche,
       C.Culture * 10/10 CULTURE,
       C.IT_plante,
       C.Variété,
       C."TYPE(a)",
       C."D_planif(a)",
       C.Date_semis,
       C.Semis_fait,
       C.Date_plantation,
       C.Plantation_faite,
       C.Début_récolte,
       C.Longueur,
       PL.Largeur * 1 LARGEUR,
       C.Nb_rangs,
       C.Espacement,
       I.Nb_graines_trou *1 NB_GRAINES_TROU,
       I.Dose_semis * 1 DOSE_SEMIS,
       round(C.Longueur * C.Nb_rangs / C.Espacement * 100) NB_PLANTS_PLANCHES,
       round(C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou) NB_GRAINES_PLANCHES,
       round(CASE WHEN C.Espacement > 0 THEN C.Longueur * C.Nb_rangs / C.Espacement * 100 * I.Nb_graines_trou / E.Nb_graines_g
       ELSE C.Longueur * PL.Largeur * I.Dose_semis END) POIDS_GRAINES_PLANCHES,
       C.Notes,
       I.Notes N_IT_plante,
       PL.Notes N_planche,
       E.Notes N_espèce
  FROM Cultures C
       LEFT JOIN
       ITP I USING (
           It_plante
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
          It_plante;
DROP VIEW IF EXISTS "3-3 Plantations à faire ";
CREATE VIEW "3-3_Plantations_à_faire " AS SELECT C.Planche,
       C.Culture * 10/10 CULTURE,
       trim(C.IT_plante) IT_PLANTE,
       C.Variété,
       C."TYPE(a)",
       C.Date_semis,
       C.Semis_fait,
       C.Date_plantation,
       C.Plantation_faite,
       C.Début_récolte,
       C.Longueur,
       PL.Largeur * 10 / 10 LARGEUR,
       C.Nb_rangs,
       C.Espacement,
       round(C.Longueur * C.Nb_rangs / C.Espacement * 100) NB_PLANTS_PLANCHE,
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
          It_plante;
DROP VIEW IF EXISTS "3-4 Récoltes à faire";
CREATE VIEW "3-4_Récoltes_à_faire" AS SELECT trim(C.Planche) PLANCHE,
       C.Culture * 10/10 CULTURE,
       trim(C.IT_plante) IT_PLANTE,
       C."TYPE(a)",
       C.Date_semis,
       C.Semis_fait,
       C.Date_plantation,
       C.Plantation_faite,
       C.Début_récolte,
       C.Fin_récolte,
       C.Récolte_faite,
       (SELECT sum(Quantité) FROM Récoltes R WHERE R.Culture=C.Culture) DEJA_REC,
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
DROP VIEW IF EXISTS "3-41 Saisie récoltes";
CREATE VIEW "3-41_Saisie_récoltes" AS SELECT R.*,
       trim(C.Planche) PLANCHE,
       trim(I.Espèce) ESPECE
FROM Récoltes R
LEFT JOIN Cultures C USING(Culture)
LEFT JOIN ITP I USING(IT_Plante)
WHERE R.Date>DATE('now','-30 days')
ORDER BY R.Date,R.Culture;
DROP VIEW IF EXISTS "3-5 A terminer";
CREATE VIEW "3-5_A_terminer" AS SELECT trim(C.Planche) PLANCHE,
       C.Culture * 10/10 CULTURE,
       trim(C.IT_plante) IT_PLANTE,
       C."TYPE(a)",
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
DROP VIEW IF EXISTS "4             STATISTIQUES";
DROP VIEW IF EXISTS "4-1 Statitiques ITP";
CREATE VIEW "4-1_Statitiques_ITP" AS SELECT I.IT_Plante,
       I."TYPE(a)",
       I.Déb_semis,
       I.Déb_plantation,
       I.Déb_récolte,
       (SELECT count()
          FROM Cultures C
         WHERE C.It_plante=I.It_plante) NB,
       (SELECT count()
          FROM Cultures C
         WHERE (C.It_plante=I.It_plante) AND
               (C.Terminée NOTNULL) ) NB_TER,
       (SELECT min(substr(C.Date_semis, 6) )
          FROM Cultures C
         WHERE C.It_plante=I.It_plante) MIN_SEMIS,
       (SELECT max(substr(C.Date_semis, 6) )
          FROM Cultures C
         WHERE C.It_plante=I.It_plante) MAX_SEMIS,
       (SELECT min(substr(C.Date_plantation, 6) )
          FROM Cultures C
         WHERE C.It_plante=I.It_plante) MIN_PLANTATION,
       (SELECT max(substr(C.Date_plantation, 6) )
          FROM Cultures C
         WHERE C.It_plante=I.It_plante) MAX_PLANTATION,
       (SELECT min(substr(C.Début_récolte, 6) )
          FROM Cultures C
         WHERE C.It_plante=I.It_plante) MIN_RECOLTE,
       (SELECT max(substr(C.Fin_récolte, 6) )
          FROM Cultures C
         WHERE C.It_plante=I.It_plante) MAX_RECOLTE,
       (SELECT sum(Quantité)
          FROM Cultures C LEFT JOIN Récoltes R USING(Culture)
         WHERE (C.It_plante=I.It_plante) AND
               (C.Terminée NOTNULL) )/
       (SELECT count()
          FROM Cultures C JOIN Récoltes R USING(Culture)
         WHERE (C.It_plante=I.It_plante) AND
               (C.Terminée NOTNULL) ) QTE_REC_MOY
  FROM ITP I
 ORDER BY It_plante;
DROP VIEW IF EXISTS "4-2 Tempo cultures (Espèce)";
CREATE VIEW "4-2_Tempo_cultures_(Espèce)" AS SELECT C.Culture CULTURE,
       trim(C.IT_plante) IT_PLANTE,
       trim(C.Variété) VARIETE,
       trim(C.Planche) PLANCHE,
       C."TYPE(a)",
       trim(C.Date_semis) DATE_SEMIS,
       trim(C.Date_plantation) DATE_PLANTATION,
       trim(C.Début_récolte) DEBUT_RECOLTE,
       trim(C.Fin_récolte) FIN_RECOLTE,
       trim(C.Récolte_faite) RECOLTE_FAITE,
       CulTempo(C."TYPE(a)", C.Date_semis, C.Date_plantation, C.Début_récolte, C.Fin_récolte)
 __Jan____Fév___Mars___Avril___Mai____Juin___Juil___Août____Sept___Oct____Nov____Déc__,
       C.Notes
  FROM Cultures C
       LEFT JOIN ITP I USING(IT_Plante)
 WHERE (C.Terminée NOTNULL) AND
       ( (I.Espèce= (SELECT Espèce
                       FROM Params
                      WHERE Paramètre='Focus') ) OR
         (SELECT Espèce ISNULL
            FROM Params
           WHERE Paramètre='Focus') )
 ORDER BY C.IT_plante,
          coalesce(C.Date_semis, C.Date_plantation);

DROP VIEW IF EXISTS "Info_Potaléger";
CREATE VIEW Info_Potaléger AS
    SELECT 1 N,
           'Version de la BDD' Info,
           '2024-12-27' Valeur
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
                              FROM (SELECT DISTINCT IT_Plante
                                      FROM C_en_place
                                     ORDER BY IT_Plante) ) || (SELECT count() ||' en culture planifiée.'
                                                                 FROM (SELECT DISTINCT IT_Plante
                                                                         FROM C_a_venir
                                                                        ORDER BY IT_Plante) )
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

PRAGMA foreign_keys = 0;
CREATE TABLE sqlitestudio_temp_table AS SELECT * FROM Fournisseurs;
DROP TABLE Fournisseurs;
CREATE TABLE Fournisseurs (Fournisseur TEXT NOT NULL, Priorité INTEGER, Site_web TEXT, Notes TEXT, PRIMARY KEY (Fournisseur)) WITHOUT ROWID;
INSERT INTO Fournisseurs (Fournisseur, Priorité, Site_web, Notes) SELECT Fournisseur, Priorité, "Site web", Notes FROM sqlitestudio_temp_table;
DROP TABLE sqlitestudio_temp_table;
PRAGMA foreign_keys = 1;

COMMIT TRANSACTION;
)#");
