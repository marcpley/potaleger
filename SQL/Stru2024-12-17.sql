--
-- File generated with SQLiteStudio v3.4.9 on sam. déc. 21 18:25:50 2024
--
-- Text encoding used: UTF-8
--
PRAGMA foreign_keys = off;
BEGIN TRANSACTION;

-- Table: Apports
CREATE TABLE Apports (Apport TEXT PRIMARY KEY, Description TEXT, Poids_m² REAL, Notes) WITHOUT ROWID;

-- Table: Cultures
CREATE TABLE Cultures (Culture INTEGER PRIMARY KEY AUTOINCREMENT, IT_plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE, Variété TEXT REFERENCES Variétés (Variété) ON UPDATE CASCADE, Fournisseur TEXT REFERENCES Fournisseurs (Fournisseur), Planche TEXT REFERENCES Planches (Planche) ON UPDATE CASCADE, "TYPE(a)" TEXT AS (CASE WHEN (Date_plantation < Date_semis) OR (Début_récolte < Date_semis) OR (Fin_récolte < Date_semis) OR (Début_récolte < Date_plantation) OR (Fin_récolte < Date_plantation) OR (Fin_récolte < Début_récolte) THEN 'Erreur dates ?' WHEN Date_semis NOTNULL AND Date_plantation NOTNULL AND Début_récolte NOTNULL THEN 'Semis sous abris' WHEN Date_plantation NOTNULL AND Début_récolte NOTNULL THEN 'Plant' WHEN Date_semis NOTNULL AND Début_récolte NOTNULL THEN 'Semis direct' WHEN Date_semis NOTNULL AND Date_plantation NOTNULL THEN 'Sans récolte' WHEN Date_semis NOTNULL THEN 'Engrais vert' ELSE '?' END), Longueur REAL, Nb_rangs REAL, Espacement REAL, "D_planif(a)" DATE, Date_semis DATE, Semis_fait TEXT, Date_plantation DATE, Plantation_faite TEXT, Début_récolte DATE, Fin_récolte DATE, Récolte_faite TEXT, Terminée TEXT, Notes TEXT);

-- Table: Debug
CREATE TABLE Debug (Table_vue TEXT, Mot_clé TEXT, Description TEXT, Notes TEXT);

-- Table: Espèces
CREATE TABLE Espèces (Espèce TEXT PRIMARY KEY, Famille TEXT REFERENCES Familles (Famille) ON UPDATE CASCADE, "Rendement(i)" REAL, "Niveau(i)" TEXT REFERENCES Niveaux (Niveau) ON UPDATE CASCADE, "Apport(i)" TEXT REFERENCES Apports (Apport) ON UPDATE CASCADE, "Dose_semis(d)" REAL, Nb_graines_g REAL, "FG(i)" REAL, "T_germ(i)" TEXT, "Levée(i)" REAL, Inventaire REAL, Date_inv DATE, A_planifier TEXT, Notes TEXT) WITHOUT ROWID;

-- Table: Familles
CREATE TABLE Familles (Famille TEXT NOT NULL, Intervalle REAL DEFAULT (1), Notes TEXT, PRIMARY KEY (Famille)) WITHOUT ROWID;

-- Table: Fournisseurs
CREATE TABLE Fournisseurs (Fournisseur TEXT NOT NULL, Priorité INTEGER, "Site web" TEXT, Notes TEXT, PRIMARY KEY (Fournisseur)) WITHOUT ROWID;

-- Table: ITP
CREATE TABLE ITP (IT_plante TEXT NOT NULL, Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE CONSTRAINT "Esp ?" NOT NULL ON CONFLICT REPLACE, Type_planche TEXT REFERENCES Types_planche (Type) ON UPDATE CASCADE, "TYPE(a)" TEXT CONSTRAINT "Calcul automatique en fonction des débuts de période" AS (CASE WHEN Déb_semis NOTNULL AND Déb_plantation NOTNULL AND Déb_récolte NOTNULL THEN 'Semis sous abris' WHEN Déb_plantation NOTNULL AND Déb_récolte NOTNULL THEN 'Plant' WHEN Déb_semis NOTNULL AND Déb_récolte NOTNULL THEN 'Semis direct' WHEN Déb_semis NOTNULL AND Déb_plantation NOTNULL THEN 'Sans récolte' WHEN Déb_semis NOTNULL THEN 'Engrais vert' ELSE '?' END), Déb_semis TEXT CONSTRAINT "Déb_semis, ex: 04-01 ou 04-15" CHECK (Déb_semis ISNULL OR Fmt_planif(Déb_semis)), Fin_semis TEXT CONSTRAINT "Fin_semis, ex: 05-01 ou 05-15" CHECK (Fin_semis ISNULL OR Fmt_planif(Fin_semis)), Déb_plantation TEXT CONSTRAINT "Déb_plantation, ex: 05-01 ou 05-15" CHECK (Déb_plantation ISNULL OR Fmt_Planif(Déb_plantation)), Fin_plantation TEXT CONSTRAINT "Fin_plantation, ex: 07-01 ou 07-15" CHECK (Fin_plantation ISNULL OR Fmt_planif(Fin_plantation)), Déb_récolte TEXT CONSTRAINT "Déb_récolte, ex: 08-01 ou 08-15" CHECK (Déb_récolte ISNULL OR Fmt_planif(Déb_récolte)), Fin_récolte TEXT CONSTRAINT "Fin_récolte, ex: 10-01 ou 10-15" CHECK (Fin_récolte ISNULL OR Fmt_planif(Fin_récolte)), Nb_rangs REAL, Espacement REAL, Nb_graines_trou REAL, Dose_semis REAL, Notes TEXT, PRIMARY KEY (IT_plante)) WITHOUT ROWID;

-- Table: Niveaux
CREATE TABLE Niveaux (Niveau INTEGER PRIMARY KEY, Description TEXT, Notes TEXT) WITHOUT ROWID;

-- Table: Params
CREATE TABLE Params (Paramètre TEXT PRIMARY KEY, Date_modif DATE, Description TEXT, Valeur TEXT, Espèce TEXT REFERENCES Espèces (Espèce) ON DELETE SET NULL ON UPDATE CASCADE) WITHOUT ROWID;

-- Table: Planches
CREATE TABLE Planches (Planche TEXT NOT NULL, Type TEXT REFERENCES Types_planche (Type) ON UPDATE CASCADE, Longueur REAL DEFAULT (8), Largeur REAL DEFAULT (0.8), Rotation TEXT REFERENCES Rotations (Rotation) ON UPDATE CASCADE, Année INTEGER, Notes TEXT, PRIMARY KEY (Planche)) WITHOUT ROWID;

-- Table: Rotations
CREATE TABLE Rotations (Rotation TEXT PRIMARY KEY, Type_planche TEXT REFERENCES Types_planche (Type) ON UPDATE CASCADE, Année_1 INTEGER, Nb_années INTEGER, Notes TEXT) WITHOUT ROWID;

-- Table: Rotations_détails
CREATE TABLE Rotations_détails (ID INTEGER PRIMARY KEY AUTOINCREMENT, Rotation TEXT REFERENCES Rotations (Rotation) ON UPDATE CASCADE, Année INTEGER DEFAULT (1), IT_Plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE, Pc_planches REAL DEFAULT (100) NOT NULL, Nb_planches INTEGER, Fi_planches TEXT, Notes TEXT);

-- Table: Récoltes
CREATE TABLE Récoltes (Date DATE DEFAULT (DATE('now')) NOT NULL, Culture INTEGER REFERENCES Cultures (Culture) ON DELETE CASCADE ON UPDATE CASCADE, Quantité REAL);

-- Table: Types_planche
CREATE TABLE Types_planche (Type TEXT PRIMARY KEY, Notes TEXT) WITHOUT ROWID;

-- Table: Variétés
CREATE TABLE Variétés (Variété TEXT PRIMARY KEY, Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE, Nb_graines_g REAL, Qté_stock REAL, Qté_cde REAL, Fournisseur TEXT REFERENCES Fournisseurs (Fournisseur) ON UPDATE CASCADE, Notes TEXT) WITHOUT ROWID;

-- View: 1             DONNEES
CREATE VIEW "1             DONNEES" AS SELECT 1 N,'Vues particulières des données de départs' Description
UNION
SELECT 2 N,'La création de ces données se fait directement dans les tables' Description;

-- View: 1-1 Inv et cde variétés
CREATE VIEW "1-1 Inv et cde variétés" AS SELECT trim(E.Famille) FAMILLE,
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

-- View: 1-2 Tempo ITP
CREATE VIEW "1-2 Tempo ITP" AS SELECT I.IT_plante,
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

-- View: 1-4 Stock récoltes
CREATE VIEW "1-4 Stock récoltes" AS SELECT E.Espèce,
       E.Inventaire,
       E.Date_inv,
       E.Inventaire+(SELECT sum(R.Quantité)
                     FROM Récoltes R LEFT JOIN Cultures C USING(Culture) 
                                     LEFT JOIN ITP I USING(IT_Plante)
                     WHERE I.Espèce=E.Espèce) Stock_récolte
FROM Espèces E WHERE E.Inventaire NOTNULL;

-- View: 2             PLANIFICATION
CREATE VIEW "2             PLANIFICATION" AS SELECT 'Différentes vues permettant de créer et renseigner les cultures de la prochaine saison' Description;

-- View: 2-1 Rotations Tempo
CREATE VIEW "2-1 Rotations Tempo" AS SELECT R.Rotation,
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

-- View: 2-2 Rotations ITP Ilot
CREATE VIEW "2-2 Rotations ITP Ilot" AS SELECT IT_PLANTE,
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
       E.A_planifier NOTNULL
--UNION
--SELECT ' Longueur totale de planches dans les rotations' IT_PLANTE,
--       NULL ILOT,
--       (SELECT sum(LONGUEUR) FROM ITP_rotations) LONGUEUR,
--       'sur '||(SELECT sum(LONGUEUR) FROM Planches) ANNEE;

-- View: 2-3 Rotations vérif
CREATE VIEW "2-3 Rotations vérif" AS SELECT trim(I.Espèce) ESPECE,
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

-- View: 2-4 Cultures planif
CREATE VIEW "2-4 Cultures planif" AS SELECT trim(RD.IT_Plante) IT_PLANTE,
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

-- View: 2-4 Successions par planche
CREATE VIEW "2-4 Successions par planche" AS SELECT PL.Planche,
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

-- View: 3             SUIVI DES CULTURES
CREATE VIEW "3             SUIVI DES CULTURES" AS SELECT 'Listes de cultures regroupées par planche et par état d''avancement' Description;

-- View: 3-0 Cultures non terminées
CREATE VIEW "3-0 Cultures non terminées" AS SELECT C.Planche,
       C.Culture * 10/10 CULTURE,
       C.IT_plante,
       C.Variété,
       C."TYPE(a)",
       ColoCult(C.Culture) Color,
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

-- View: 3-1 Semences nécessaires
CREATE VIEW "3-1 Semences nécessaires" AS SELECT C.Variété VARIETE,
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

-- View: 3-2 Semis à faire 
CREATE VIEW "3-2 Semis à faire " AS SELECT C.Planche,
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

-- View: 3-3 Plantations à faire 
CREATE VIEW "3-3 Plantations à faire " AS SELECT C.Planche,
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

-- View: 3-4 Récoltes à faire
CREATE VIEW "3-4 Récoltes à faire" AS SELECT trim(C.Planche) PLANCHE,
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

-- View: 3-41 Saisie récoltes
CREATE VIEW "3-41 Saisie récoltes" AS SELECT R.*,
       trim(C.Planche) PLANCHE,
       trim(I.Espèce) ESPECE
FROM Récoltes R
LEFT JOIN Cultures C USING(Culture)
LEFT JOIN ITP I USING(IT_Plante)
WHERE R.Date>DATE('now','-30 days')
ORDER BY R.Date,R.Culture;

-- View: 3-5 A terminer
CREATE VIEW "3-5 A terminer" AS SELECT trim(C.Planche) PLANCHE,
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

-- View: 4             STATISTIQUES
CREATE VIEW "4             STATISTIQUES" AS SELECT 'Statistiques sur les cultures' Description;

-- View: 4-1 Statitiques ITP
CREATE VIEW "4-1 Statitiques ITP" AS SELECT I.IT_Plante,
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

-- View: 4-2 Tempo cultures (Espèce)
CREATE VIEW "4-2 Tempo cultures (Espèce)" AS SELECT C.Culture CULTURE,
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

-- View: C_a_venir
CREATE VIEW C_a_venir AS SELECT * FROM Cultures
WHERE Terminée ISNULL AND NOT((Semis_fait NOTNULL AND Date_plantation ISNULL)--SD semé
                              OR Plantation_faite NOTNULL)--Plant ou SSA planté
ORDER BY Planche;

-- View: C_en_place
CREATE VIEW C_en_place AS SELECT * FROM Cultures
WHERE Terminée ISNULL AND ((Semis_fait NOTNULL AND Date_plantation ISNULL)--SD semé
                           OR Plantation_faite NOTNULL)--Plant ou SSA planté
ORDER BY Planche;

-- View: F_actives
CREATE VIEW F_actives AS SELECT * FROM Familles F
WHERE ((SELECT count() FROM ITP LEFT JOIN Espèces USING(Espèce) WHERE Espèces.Famille=F.Famille)>1);

-- View: Info_Potaléger
CREATE VIEW Info_Potaléger AS SELECT 1 N,'Version de la BDD' Info, '2024-12-17' Valeur UNION SELECT 2, 'Utilisateur',(SELECT Valeur FROM Params WHERE Paramètre = 'Utilisateur') UNION SELECT 3, 'Variétés',(SELECT count() || ' dont '  FROM Variétés) ||                      (SELECT count() || ' en culture actuellement et '  FROM (SELECT DISTINCT Variété FROM C_en_place ORDER BY Variété))  ||                      (SELECT count() || ' en culture planifiée.'  FROM (SELECT DISTINCT Variété FROM C_a_venir ORDER BY Variété)) UNION SELECT 4, 'Itinéraires techniques',(SELECT count() || ' dont '  FROM ITP) ||                                    (SELECT count() || ' en culture actuellement et '  FROM (SELECT DISTINCT IT_Plante FROM C_en_place ORDER BY IT_Plante))  ||                                    (SELECT count() || ' en culture planifiée.'  FROM (SELECT DISTINCT IT_Plante FROM C_a_venir ORDER BY IT_Plante)) UNION SELECT 5, 'Planches',(SELECT count() || ' dont '  FROM Planches) ||                      (SELECT count() || ' en culture actuellement et '  FROM (SELECT DISTINCT Planche FROM C_en_place ORDER BY Planche))  ||                      (SELECT count() || ' en culture planifiée.'  FROM (SELECT DISTINCT Planche FROM C_a_venir ORDER BY Planche)) UNION SELECT 6, 'Cultures',(SELECT count() || ' dont '  FROM Cultures) || (SELECT count() || ' non terminées.'  FROM Cultures WHERE Terminée ISNULL);

-- View: ITP_cultures
CREATE VIEW ITP_cultures AS SELECT IT_PLANTE,
       sum(NB_PLANCHES) NB_PLANCHES,
       sum(LONGUEUR) LONGUEUR,
       sum(LONG_RANG) LONG_RANG,
       sum(NB_PLANTS) NB_PLANTS,
       (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') ANNEE
FROM ITP_cultures_ilots
GROUP BY IT_PLANTE;

-- View: ITP_cultures_ilots
CREATE VIEW ITP_cultures_ilots AS SELECT IT_PLANTE,
       substr(PLANCHE,1,(SELECT Valeur FROM Params WHERE Paramètre='Ilot_nb_car')) ILOT,
       sum(NB_PLANCHES) NB_PLANCHES,
       sum(LONGUEUR) LONGUEUR,
       sum(LONG_RANG) LONG_RANG,
       sum(NB_PLANTS) NB_PLANTS,
       (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') ANNEE
FROM ITP_cultures_planches
GROUP BY IT_PLANTE,ILOT;

-- View: ITP_cultures_planches
CREATE VIEW ITP_cultures_planches AS SELECT trim(C.IT_PLANTE) IT_PLANTE,
       C.Planche PLANCHE,
       1 NB_PLANCHES, -- 2 ITP sur la même planche la même année se partagent la planche, donc on en compte qu'une
       sum(C.Longueur) LONGUEUR,
       sum(C.Longueur*coalesce(C.Nb_rangs,1)) LONG_RANG,
       CAST(sum(C.Longueur*coalesce(C.Nb_rangs,1)/coalesce(C.Espacement/100,1)) AS INTEGER) NB_PLANTS,
       --C.Longueur,
       (SELECT Valeur FROM Params WHERE Paramètre='Année_culture') ANNEE
  FROM Cultures C
--       LEFT JOIN Rotations R USING(Rotation) 
--       LEFT JOIN Planches P USING(Rotation) 
 WHERE (substr(coalesce(C.Date_plantation,C.Date_semis),1,4)=(SELECT Valeur FROM Params WHERE Paramètre='Année_culture'))
 GROUP BY IT_PLANTE,PLANCHE
 ORDER BY IT_PLANTE,PLANCHE;

-- View: ITP_rotations
CREATE VIEW ITP_rotations AS SELECT IT_PLANTE,
       sum(NB_PLANCHES) NB_PLANCHES,
       sum(LONGUEUR) LONGUEUR,
       sum(LONG_RANG) LONG_RANG,
       sum(NB_PLANTS) NB_PLANTS,
       (SELECT Valeur FROM Params WHERE Paramètre='Année planif') ANNEE
FROM ITP_rotations_ilots
GROUP BY IT_PLANTE;

-- View: ITP_rotations_ilots
CREATE VIEW ITP_rotations_ilots AS SELECT trim(RD.IT_PLANTE) IT_PLANTE,
       substr(P.Planche, 1, (SELECT Valeur
                               FROM Params
                              WHERE Paramètre='Ilot_nb_car') ) ILOT,
       count() NB_PLANCHES,
       sum(P.Longueur*RD.Pc_planches/100) LONGUEUR,
       sum(P.Longueur*coalesce(I.Nb_rangs, 1) *RD.Pc_planches/100) LONG_RANG,
       CAST(sum(P.Longueur*coalesce(I.Nb_rangs, 1) /coalesce(I.Espacement/100, 1) *RD.Pc_planches/100) AS INTEGER) NB_PLANTS,
       (SELECT Valeur
          FROM Params
         WHERE Paramètre='Année_planif') ANNEE
  FROM Rotations_détails RD
       LEFT JOIN ITP I USING(IT_Plante) 
       LEFT JOIN Rotations R USING(Rotation) 
       LEFT JOIN Planches P ON (P.Rotation=RD.Rotation) AND
                               (P.Année=RD.Année)AND
                               (RD.Fi_planches ISNULL OR((RD.Fi_planches LIKE '%'||substr(P.Planche,-1,1)||'%')))
 WHERE IT_Plante NOTNULL
 GROUP BY IT_PLANTE,
          ILOT
 ORDER BY IT_PLANTE,
          ILOT;

-- View: R_famille
CREATE VIEW R_famille AS SELECT RD.ID,RD.Rotation,RD.Année,
       R.Nb_années,
       F.Famille,
       F.Intervalle
  FROM Rotations_détails RD
       LEFT JOIN Rotations R USING(Rotation) 
       LEFT JOIN ITP I USING(IT_Plante) 
       LEFT JOIN Espèces E USING(Espèce) 
       LEFT JOIN Familles F USING(Famille);

-- View: R_ITP
CREATE VIEW R_ITP AS SELECT R.Rotation||--coalesce(R.Fi_planches,'')||
         format('%i',2000+R.Année) ||'-'||coalesce(I.Déb_plantation, I.Déb_semis) ||
         format('%i',2000+R.Année+iif(coalesce(I.Déb_plantation, I.Déb_semis) >coalesce(I.Déb_récolte, I.Fin_récolte), 1, 0) ) ||
                     '-'||coalesce(I.Déb_récolte, I.Fin_récolte, '12-31') Ind,
       R.ID,
       R.Rotation,
       RO.Nb_années*10/10 NB_ANNEES,
       R.Année,
       R.IT_Plante,
       R.Pc_planches,
       R.Fi_planches,
       trim(I.Type_planche) TYPE_PLANCHE,
       I.[TYPE(a)],
       coalesce(I.Déb_plantation, I.Déb_semis) MISE_EN_PLACE,
       format('%i',2000+R.Année) ||'-'||coalesce(I.Déb_plantation, I.Déb_semis) DATE_MEP,
       format('%i',2000+R.Année+iif(coalesce(I.Déb_plantation, I.Déb_semis) >coalesce(I.Déb_récolte, I.Fin_récolte), 1, 0) ) ||'-'||coalesce(I.Déb_récolte, I.Fin_récolte, '12-31') DATE_FERM,
       trim(I.Déb_semis) DEB_SEMIS,
       trim(I.Fin_semis) FIN_SEMIS,
       trim(I.Déb_plantation) DEB_PLANT,
       trim(I.Fin_plantation) FIN_PLANT,
       trim(I.Déb_récolte) DEB_REC,
       trim(I.Fin_récolte) FIN_REC,
       E.A_planifier,
       trim(F.Famille) FAMILLE,
       F.Intervalle*10/10 INTERV
  FROM Rotations_détails R
       LEFT JOIN Rotations RO USING(Rotation)
       LEFT JOIN ITP I USING(IT_Plante) 
       LEFT JOIN Espèces E USING(Espèce) 
       LEFT JOIN Familles F USING(Famille) 
 ORDER BY Ind;

COMMIT TRANSACTION;
PRAGMA foreign_keys = on;
