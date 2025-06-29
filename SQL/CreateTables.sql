QString sDDLTables = QStringLiteral(R"#(

-- BEGIN TRANSACTION;

-- CREATE TABLE Apports (Apport TEXT PRIMARY KEY COLLATE POTACOLLATION,
--                       Type TEXT COLLATE POTACOLLATION,
--                       Description TEXT COLLATE POTACOLLATION,
--                       Poids_m² REAL,
--                       Notes TEXT) WITHOUT ROWID;

CREATE TABLE Analyses_de_sol (Analyse TEXT PRIMARY KEY COLLATE POTACOLLATION,
                           Planche TEXT REFERENCES Planches (Planche) ON UPDATE CASCADE COLLATE POTACOLLATION,
                           Date DATE NOT NULL,
                           Sable_grossier REAL,
                           Sable_fin REAL,
                           Limon_grossier REAL,
                           Limon_fin REAL,
                           Argile REAL,
                           pH REAL,
                           MO REAL,
                           IB REAL,
                           CEC REAL,
                           Cations REAL,
                           N REAL,
                           ☆N TEXT AS (CASE WHEN N >= 1.1 THEN 'Elevé' WHEN N <= 0.9 THEN 'Faible' WHEN N NOTNULL THEN 'Moyen' END),
                           P REAL,
                           ☆P TEXT AS (CASE WHEN P >= 0.12 THEN 'Elevé' WHEN P <= 0.08 THEN 'Faible' WHEN P NOTNULL THEN 'Moyen' END),
                           K REAL,
                           ☆K TEXT AS (CASE WHEN K >= 0.15 THEN 'Elevé' WHEN K <= 0.12 THEN 'Faible' WHEN K NOTNULL THEN 'Moyen' END),
                           C REAL,
                           CN REAL,
                           Ca REAL,
                           Mg REAL,
                           Na REAL,
                           Interprétation TEXT,
                           Référence TEXT COLLATE POTACOLLATION,
                           Organisme TEXT COLLATE POTACOLLATION,
                           Notes TEXT) WITHOUT ROWID;

CREATE TABLE Consommations (ID INTEGER PRIMARY KEY AUTOINCREMENT,
                       Date DATE NOT NULL, -- DEFAULT (DATE('now'))
                       Espèce TEXT REFERENCES Espèces (Espèce) ON DELETE CASCADE ON UPDATE CASCADE NOT NULL COLLATE POTACOLLATION,
                       Quantité REAL NOT NULL,
                       Prix REAL,
                       Destination TEXT REFERENCES Destinations (Destination) ON DELETE SET NULL ON UPDATE CASCADE COLLATE POTACOLLATION,
                       Notes TEXT);

CREATE TABLE Cultures (Culture INTEGER PRIMARY KEY AUTOINCREMENT,
                       Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL COLLATE POTACOLLATION,
                       IT_plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE COLLATE POTACOLLATION,
                       Variété TEXT REFERENCES Variétés (Variété) ON UPDATE CASCADE COLLATE POTACOLLATION,
                       Fournisseur TEXT REFERENCES Fournisseurs (Fournisseur)  ON UPDATE CASCADE COLLATE POTACOLLATION,
                       Planche TEXT REFERENCES Planches (Planche) ON UPDATE CASCADE COLLATE POTACOLLATION,
                       Type TEXT AS (CASE WHEN (Date_plantation < Date_semis) OR (Début_récolte < Date_semis) OR (Fin_récolte < Date_semis) OR (Début_récolte < Date_plantation) OR (Fin_récolte < Date_plantation) OR (Fin_récolte < Début_récolte) THEN 'Erreur dates ?'
                                          WHEN Terminée LIKE 'v%' THEN 'Vivace'
                                          WHEN Date_semis NOTNULL AND Date_plantation NOTNULL AND Début_récolte NOTNULL THEN 'Semis pépinière'
                                          WHEN Date_plantation NOTNULL AND Début_récolte NOTNULL THEN 'Plant'
                                          WHEN Date_semis NOTNULL AND Début_récolte NOTNULL THEN 'Semis en place'
                                          WHEN Date_semis NOTNULL AND Date_plantation NOTNULL THEN 'Sans récolte'
                                          WHEN Date_semis NOTNULL THEN 'Engrais vert' ELSE '?' END),
                       Saison TEXT AS (CASE WHEN coalesce(Terminée,'') NOT LIKE 'v%' -- Anuelle
                                            THEN substr(coalesce(Date_plantation,Date_semis,Début_récolte,Fin_récolte),1,4)
                                            ELSE substr(coalesce(Début_récolte,Date_plantation,Date_semis,Fin_récolte),1,4) -- Vivace
                                            END),
                       Etat TEXT AS (CASE WHEN coalesce(Terminée,'') NOT LIKE 'v%' -- Anuelle
                                          THEN (CASE WHEN Terminée NOTNULL THEN 'Terminée' --gris
                                                     WHEN Récolte_faite LIKE 'x%' THEN 'A terminer' --bleu
                                                     WHEN Récolte_faite NOTNULL THEN 'Récolte' --violet
                                                     WHEN Plantation_faite NOTNULL THEN 'En place' --vert
                                                     WHEN Semis_fait NOTNULL THEN iif(Date_plantation IS NULL,'En place', --vert
                                                                                                              'Pépinière') --rouge
                                                     ELSE 'Prévue'
                                                     END)
                                          -- Vivace
                                          ELSE (CASE WHEN (Terminée != 'v')AND(Terminée != 'V') THEN 'Terminée' --gris
                                                     WHEN Récolte_faite LIKE 'x%' THEN 'En place' --vert
                                                     WHEN Récolte_faite NOTNULL THEN 'Récolte' --violet
                                                     WHEN Plantation_faite NOTNULL THEN 'En place' --vert
                                                     WHEN Semis_fait NOTNULL THEN iif(Date_plantation IS NULL,'En place', --vert
                                                                                                              'Pépinière') --rouge
                                                     ELSE 'Prévue'
                                                     END)
                                          END),
                       D_planif TEXT,-- Format TEXT pour pouvoir mettre une année simple quand on veut forcer un recalcul de planif.
                       Date_semis DATE,
                       Semis_fait BOOL,
                       Date_plantation DATE,
                       Plantation_faite BOOL,
                       Début_récolte DATE,
                       -- Récolte_com BOOL,
                       Fin_récolte DATE,
                       Récolte_faite BOOL,
                       Terminée BOOL,
                       Longueur REAL,
                       Nb_rangs REAL,
                       Espacement REAL,
                       A_faire TEXT,
                       Notes TEXT);

CREATE TABLE Cu_Fertilisants (Culture INTEGER REFERENCES Cultures (Culture) ON DELETE CASCADE ON UPDATE CASCADE NOT NULL,
                              Fertilisant TEXT REFERENCES Fertilisants (Fertilisant) ON DELETE CASCADE ON UPDATE CASCADE NOT NULL COLLATE POTACOLLATION,
                              Quantité REAL);

CREATE TABLE Destinations (Destination TEXT PRIMARY KEY COLLATE POTACOLLATION,
                           Type TEXT COLLATE POTACOLLATION,
                           Adresse TEXT COLLATE POTACOLLATION,
                           Site_web TEXT,
                           Date_RAZ DATE,
                           Active BOOL DEFAULT ('x'),
                           Notes TEXT) WITHOUT ROWID;

CREATE TABLE Espèces (Espèce TEXT PRIMARY KEY COLLATE POTACOLLATION,
                      Famille TEXT REFERENCES Familles (Famille) ON UPDATE CASCADE COLLATE POTACOLLATION,
                      Rendement REAL,
                      Niveau TEXT COLLATE POTACOLLATION,
                      Densité INTEGER,
                      Dose_semis REAL,
                      Nb_graines_g REAL,
                      FG REAL,
                      T_germ TEXT,
                      Levée REAL,
                      Irrig TEXT COLLATE POTACOLLATION,
                      Conservation BOOL,
                      A_planifier BOOL DEFAULT ('x'),
                      Vivace BOOL,
                      Favorable TEXT,
                      Défavorable TEXT,
                      Taille TEXT,
                      Obj_annuel REAL,
                      N REAL,
                      ★N TEXT AS (CASE WHEN N > 10 THEN 'Elevé' WHEN N < 5 THEN 'Faible' WHEN N NOTNULL THEN 'Moyen' END),
                      P REAL,
                      ★P TEXT AS (CASE WHEN P > 5 THEN 'Elevé' WHEN P < 2.55 THEN 'Faible' WHEN P NOTNULL THEN 'Moyen' END),
                      K REAL,
                      ★K TEXT AS (CASE WHEN K > 12 THEN 'Elevé' WHEN K < 7 THEN 'Faible' WHEN K NOTNULL THEN 'Moyen' END),
                      Date_inv DATE,
                      Inventaire REAL,
                      Prix_kg REAL,
                      Notes TEXT) WITHOUT ROWID;

CREATE TABLE Familles (Famille TEXT PRIMARY KEY COLLATE POTACOLLATION,
                       Intervalle REAL DEFAULT (4),
                       Notes TEXT) WITHOUT ROWID;

CREATE TABLE Fertilisants (Fertilisant TEXT PRIMARY KEY COLLATE POTACOLLATION,
                           Type TEXT COLLATE POTACOLLATION,
                           Fonction TEXT,
                           Utilisation TEXT,
                           pH REAL,
                           N REAL,
                           N_coef REAL,
                           N_disp_pc REAL AS (round(N*N_coef/100,2)),
                           P REAL,
                           P_coef REAL,
                           P_disp_pc REAL AS (round(P*P_coef/100,2)),
                           K REAL,
                           K_coef REAL,
                           K_disp_pc REAL AS (round(K*K_coef/100,2)),
                           Ca REAL,
                           Ca_coef REAL,
                           Ca_disp_pc REAL AS (round(Ca*Ca_coef/100,2)),
                           Fe REAL,
                           Fe_coef REAL,
                           Fe_disp_pc REAL AS (round(Fe*Fe_coef/100,2)),
                           Mg REAL,
                           Mg_coef REAL,
                           Mg_disp_pc REAL AS (round(Mg*Mg_coef/100,2)),
                           Na REAL,
                           Na_coef REAL,
                           Na_disp_pc REAL AS (round(Na*Na_coef/100,2)),
                           S REAL,
                           S_coef REAL,
                           S_disp_pc REAL AS (round(S*S_coef/100,2)),
                           Si REAL,
                           Si_coef REAL,
                           Si_disp_pc REAL AS (round(Si*Si_coef/100,2)),
                           Date_inv DATE,
                           Inventaire REAL,
                           Prix_kg REAL,
                           Notes TEXT) WITHOUT ROWID;

CREATE TABLE Fertilisations (ID INTEGER PRIMARY KEY AUTOINCREMENT,
                       Date DATE NOT NULL, -- DEFAULT (DATE('now'))
                       Espèce TEXT REFERENCES Espèces (Espèce) ON DELETE CASCADE ON UPDATE CASCADE NOT NULL COLLATE POTACOLLATION,
                       Culture INTEGER REFERENCES Cultures (Culture) ON DELETE CASCADE ON UPDATE CASCADE NOT NULL,
                       Fertilisant TEXT REFERENCES Fertilisants (Fertilisant) ON DELETE CASCADE ON UPDATE CASCADE NOT NULL COLLATE POTACOLLATION,
                       Quantité REAL NOT NULL,
                       N REAL,
                       P REAL,
                       K REAL,
                       Notes TEXT);

CREATE TABLE Fournisseurs (Fournisseur TEXT PRIMARY KEY COLLATE POTACOLLATION,
                           Type TEXT COLLATE POTACOLLATION,
                           Priorité INTEGER,
                           Adresse TEXT COLLATE POTACOLLATION,
                           Site_web TEXT,
                           Notes TEXT) WITHOUT ROWID;

CREATE TABLE ITP (IT_plante TEXT PRIMARY KEY COLLATE POTACOLLATION,
                  Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE COLLATE POTACOLLATION,
                  Type_planche TEXT COLLATE POTACOLLATION, -- REFERENCES Types_planche (Type) ON UPDATE CASCADE
                  Type_culture TEXT AS (CASE WHEN Déb_semis NOTNULL AND Déb_plantation NOTNULL AND Déb_récolte NOTNULL THEN 'Semis pépinière'
                                             WHEN Déb_plantation NOTNULL AND Déb_récolte NOTNULL THEN 'Plant'
                                             WHEN Déb_semis NOTNULL AND Déb_récolte NOTNULL THEN 'Semis en place'
                                             WHEN Déb_semis NOTNULL AND Déb_plantation NOTNULL THEN 'Sans récolte'
                                             WHEN Déb_semis NOTNULL THEN 'Engrais vert' ELSE '?' END),
                  Déb_semis TEXT CONSTRAINT 'Déb_semis, ex: 04-01 ou 04-15' CHECK (Déb_semis #FmtPlanif#),
                  Fin_semis TEXT CONSTRAINT 'Fin_semis, ex: 05-01 ou 05-15' CHECK (Fin_semis  #FmtPlanif#),
                  Déb_plantation TEXT CONSTRAINT 'Déb_plantation, ex: 05-01 ou 05-15' CHECK (Déb_plantation  #FmtPlanif#),
                  Fin_plantation TEXT CONSTRAINT 'Fin_plantation, ex: 07-01 ou 07-15' CHECK (Fin_plantation #FmtPlanif#),
                  Déb_récolte TEXT CONSTRAINT 'Déb_récolte, ex: 08-01 ou 08-15' CHECK (Déb_récolte #FmtPlanif#),
                  Fin_récolte TEXT CONSTRAINT 'Fin_récolte, ex: 10-01 ou 10-15' CHECK (Fin_récolte #FmtPlanif#),
                  Nb_rangs REAL,
                  Espacement REAL,
                  Nb_graines_trou REAL,
                  Dose_semis REAL,
                  Notes TEXT) WITHOUT ROWID;

CREATE TABLE Notes (ID INTEGER PRIMARY KEY AUTOINCREMENT,
                    Date_création DATE,
                    Date_modif DATE,
                    Type TEXT COLLATE POTACOLLATION,
                    Description TEXT COLLATE POTACOLLATION,
                    Texte TEXT);

CREATE TABLE Params (Section TEXT, Paramètre TEXT, Description TEXT, Valeur TEXT, Unité TEXT, Date_modif DATE);

CREATE TABLE Planches (Planche TEXT PRIMARY KEY COLLATE POTACOLLATION,
                       Type TEXT COLLATE POTACOLLATION, -- REFERENCES Types_planche (Type) ON UPDATE CASCADE
                       Longueur REAL,
                       Largeur REAL,
                       Irrig TEXT COLLATE POTACOLLATION,
                       Rotation TEXT REFERENCES Rotations (Rotation) ON UPDATE CASCADE COLLATE POTACOLLATION,
                       Année INTEGER,
                       Analyse TEXT REFERENCES Analyses_de_sol (Analyse) ON DELETE SET NULL ON UPDATE CASCADE,
                       Notes TEXT) WITHOUT ROWID;

CREATE TABLE Rotations (Rotation TEXT PRIMARY KEY COLLATE POTACOLLATION,
                        Type_planche TEXT COLLATE POTACOLLATION, -- REFERENCES Types_planche (Type) ON UPDATE CASCADE
                        Année_1 INTEGER,
                        Nb_années INTEGER,
                        Notes TEXT) WITHOUT ROWID;

CREATE TABLE Rotations_détails (ID INTEGER PRIMARY KEY AUTOINCREMENT,
                                Rotation TEXT REFERENCES Rotations (Rotation) ON UPDATE CASCADE COLLATE POTACOLLATION,
                                Année INTEGER DEFAULT (1) NOT NULL ON CONFLICT REPLACE,
                                IT_plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE COLLATE POTACOLLATION,
                                Pc_planches REAL DEFAULT (100) NOT NULL ON CONFLICT REPLACE,
                                -- Nb_planches INTEGER,
                                Fi_planches TEXT,
                                Notes TEXT);

CREATE TABLE Récoltes (ID INTEGER PRIMARY KEY AUTOINCREMENT,
                       Date DATE NOT NULL, -- DEFAULT (DATE('now'))
                       Espèce TEXT REFERENCES Espèces (Espèce) ON DELETE CASCADE ON UPDATE CASCADE NOT NULL COLLATE POTACOLLATION,
                       Culture INTEGER REFERENCES Cultures (Culture) ON DELETE CASCADE ON UPDATE CASCADE NOT NULL,
                       Quantité REAL NOT NULL,
                       Notes TEXT);

-- CREATE TABLE Types_planche (Type TEXT PRIMARY KEY COLLATE POTACOLLATION,
--                             Notes TEXT) WITHOUT ROWID;

CREATE TABLE Variétés (Variété TEXT PRIMARY KEY COLLATE POTACOLLATION,
                       Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL COLLATE POTACOLLATION,
                       Nb_graines_g REAL,
                       Qté_stock REAL,
                       Qté_cde REAL,
                       Fournisseur TEXT REFERENCES Fournisseurs (Fournisseur) ON UPDATE CASCADE COLLATE POTACOLLATION,
                       Déb_récolte TEXT CONSTRAINT 'Déb_récolte, ex: 08-01 ou 08-15' CHECK (Déb_récolte #FmtPlanif#),
                       Fin_récolte TEXT CONSTRAINT 'Fin_récolte, ex: 10-01 ou 10-15' CHECK (Fin_récolte #FmtPlanif#),
                       PJ INTEGER,
                       IT_plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE COLLATE POTACOLLATION,
                       Notes TEXT) WITHOUT ROWID;

-- COMMIT TRANSACTION;

)#");
