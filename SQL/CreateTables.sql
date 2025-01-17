QString sDDLTables = QStringLiteral(R"#(

BEGIN TRANSACTION;

CREATE TABLE Apports (Apport TEXT PRIMARY KEY,
                      Description TEXT,
                      Poids_m² REAL,
                      Notes) WITHOUT ROWID;

CREATE TABLE Cultures (Culture INTEGER PRIMARY KEY AUTOINCREMENT,
                       IT_plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE,
                       Variété TEXT REFERENCES Variétés (Variété) ON UPDATE CASCADE,
                       Fournisseur TEXT REFERENCES Fournisseurs (Fournisseur)  ON UPDATE CASCADE,
                       Planche TEXT REFERENCES Planches (Planche) ON UPDATE CASCADE,
                       Type TEXT AS (CASE WHEN (Date_plantation < Date_semis) OR (Début_récolte < Date_semis) OR (Fin_récolte < Date_semis) OR (Début_récolte < Date_plantation) OR (Fin_récolte < Date_plantation) OR (Fin_récolte < Début_récolte) THEN 'Erreur dates ?'
                                          WHEN Date_semis NOTNULL AND Date_plantation NOTNULL AND Début_récolte NOTNULL THEN 'Semis sous abris'
                                          WHEN Date_plantation NOTNULL AND Début_récolte NOTNULL THEN 'Plant'
                                          WHEN Date_semis NOTNULL AND Début_récolte NOTNULL THEN 'Semis direct'
                                          WHEN Date_semis NOTNULL AND Date_plantation NOTNULL THEN 'Sans récolte'
                                          WHEN Date_semis NOTNULL THEN 'Engrais vert' ELSE '?' END),
                       Etat TEXT AS (CASE WHEN Terminée NOTNULL THEN 'Terminée' --gris
                                          WHEN Récolte_faite NOTNULL THEN 'A terminer' --bleu
                                          WHEN Plantation_faite NOTNULL THEN 'En place' --vert
                                          WHEN Semis_fait NOTNULL THEN iif(Date_plantation IS NULL,'En place', --vert
                                                                                                   'Sous abris') --rouge
                                          ELSE 'Prévue'
                                          END),
                       D_planif TEXT,-- Pour pouvir mettre une année simple quand on veut forcer un recalcul de planif.
                       Date_semis DATE,
                       Semis_fait TEXT,
                       Date_plantation DATE,
                       Plantation_faite TEXT,
                       Début_récolte DATE,
                       Fin_récolte DATE,
                       Récolte_faite TEXT,
                       Terminée TEXT,
                       Longueur REAL,
                       Nb_rangs REAL,
                       Espacement REAL,
                       Notes TEXT);

CREATE TABLE Espèces (Espèce TEXT PRIMARY KEY,
                      Famille TEXT REFERENCES Familles (Famille) ON UPDATE CASCADE,
                      Rendement REAL,
                      Niveau TEXT,
                      Apport TEXT REFERENCES Apports (Apport) ON UPDATE CASCADE,
                      Dose_semis REAL,
                      Nb_graines_g REAL,
                      FG REAL,
                      T_germ TEXT,
                      Levée REAL,
                      Inventaire REAL,
                      Date_inv DATE,
                      A_planifier TEXT DEFAULT ('x'),
                      Notes TEXT) WITHOUT ROWID;

CREATE TABLE Familles (Famille TEXT PRIMARY KEY,
                       Intervalle REAL DEFAULT (1),
                       Notes TEXT) WITHOUT ROWID;

CREATE TABLE Fournisseurs (Fournisseur TEXT PRIMARY KEY,
                           Priorité INTEGER,
                           Site_web TEXT,
                           Notes TEXT) WITHOUT ROWID;

CREATE TABLE ITP (IT_plante TEXT PRIMARY KEY,
                  Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL,
                  Type_planche TEXT REFERENCES Types_planche (Type) ON UPDATE CASCADE,
                  Type_culture TEXT AS (CASE WHEN Déb_semis NOTNULL AND Déb_plantation NOTNULL AND Déb_récolte NOTNULL THEN 'Semis sous abris'
                                             WHEN Déb_plantation NOTNULL AND Déb_récolte NOTNULL THEN 'Plant'
                                             WHEN Déb_semis NOTNULL AND Déb_récolte NOTNULL THEN 'Semis direct'
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

CREATE TABLE Params (Section TEXT, Paramètre TEXT PRIMARY KEY, Description TEXT, Valeur TEXT, Unité TEXT, Date_modif DATE) WITHOUT ROWID;
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité, Date_modif) VALUES ('4 Planification', 'Année_culture', 'Année en cours de culture', '2024', NULL, NULL);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité, Date_modif) VALUES ('4 Planification', 'Année_planif', 'Année à planifier', '2025', NULL, NULL);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité, Date_modif) VALUES ('5 Cultures', 'Horizon_terminer', 'Voir les cultures à terminer sur une période de', '90', 'jours', NULL);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité, Date_modif) VALUES ('5 Cultures', 'Horizon_plantation', 'Voir les cultures à planter sur une période de', '90', 'jours', NULL);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité, Date_modif) VALUES ('5 Cultures', 'Horizon_récolte', 'Voir les cultures à récolter sur une période de', '90', 'jours', NULL);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité, Date_modif) VALUES ('5 Cultures', 'Horizon_semis', 'Voir les cultures à semer sur une période de', '90', 'jours', NULL);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité, Date_modif) VALUES ('2 Données de base', 'Ilot_nb_car', 'Nb de caractères du début du nom des planches qui désignent l''ilot de production. Ex: la planche "No1A" fait parti de l''ilot "No" si le paramètre vaut 2.', '2', 'car', NULL);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité, Date_modif) VALUES ('4 Planification', 'Planifier_planches', 'Début du nom des planches à planifier', NULL, NULL, NULL);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité, Date_modif) VALUES ('1 Général', 'Utilisateur', 'Personne, entreprise ou organisation utilisant cette BDD Potaléger', 'Ferme Légère', NULL, NULL);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité, Date_modif) VALUES ('3 Assolement', 'non utilisé 3', NULL, NULL, NULL, NULL);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité, Date_modif) VALUES ('6 Analyses', 'non utilisé 6', NULL, NULL, NULL, NULL);

CREATE TABLE Planches (Planche TEXT PRIMARY KEY,
                       Type TEXT REFERENCES Types_planche (Type) ON UPDATE CASCADE,
                       Longueur REAL,
                       Largeur REAL,
                       Rotation TEXT REFERENCES Rotations (Rotation) ON UPDATE CASCADE,
                       Année INTEGER, Notes TEXT) WITHOUT ROWID;

CREATE TABLE Rotations (Rotation TEXT PRIMARY KEY,
                        Type_planche TEXT REFERENCES Types_planche (Type) ON UPDATE CASCADE,
                        Année_1 INTEGER,
                        Nb_années INTEGER,
                        Notes TEXT) WITHOUT ROWID;

CREATE TABLE Rotations_détails (ID INTEGER PRIMARY KEY AUTOINCREMENT,
                                Rotation TEXT REFERENCES Rotations (Rotation) ON UPDATE CASCADE,
                                Année INTEGER DEFAULT (1),
                                IT_plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE,
                                Pc_planches REAL DEFAULT (100) NOT NULL,
                                -- Nb_planches INTEGER,
                                Fi_planches TEXT,
                                Notes TEXT);

CREATE TABLE Récoltes (Date DATE DEFAULT (DATE('now')) NOT NULL,
                       Culture INTEGER REFERENCES Cultures (Culture) ON DELETE CASCADE ON UPDATE CASCADE,
                       Quantité REAL);

CREATE TABLE Types_planche (Type TEXT PRIMARY KEY,
                            Notes TEXT) WITHOUT ROWID;

CREATE TABLE Variétés (Variété TEXT PRIMARY KEY,
                       Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE,
                       Nb_graines_g REAL,
                       Qté_stock REAL,
                       Qté_cde REAL,
                       Fournisseur TEXT REFERENCES Fournisseurs (Fournisseur) ON UPDATE CASCADE,
                       Notes TEXT) WITHOUT ROWID;

COMMIT TRANSACTION;

)#");
