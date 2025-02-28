QString sDDLTables = QStringLiteral(R"#(

-- BEGIN TRANSACTION;

CREATE TABLE Apports (Apport TEXT PRIMARY KEY,
                      Description TEXT,
                      Poids_m² REAL,
                      Notes TEXT) WITHOUT ROWID;

CREATE TABLE Consommations (ID INTEGER PRIMARY KEY AUTOINCREMENT,
                       Date DATE DEFAULT (DATE('now')) NOT NULL,
                       Espèce TEXT REFERENCES Espèces (Espèce) ON DELETE CASCADE ON UPDATE CASCADE NOT NULL,
                       Quantité REAL NOT NULL,
                       Prix REAL,
                       Destination TEXT REFERENCES Destinations (Destination) ON DELETE SET NULL ON UPDATE CASCADE,
                       Notes TEXT);

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
                       Saison TEXT AS (substr(coalesce(Date_plantation,Date_semis,Début_récolte,Fin_récolte),1,4)),
                       Etat TEXT AS (CASE WHEN Terminée NOTNULL THEN 'Terminée' --gris
                                          WHEN Récolte_faite NOTNULL THEN 'A terminer' --bleu
                                          WHEN Plantation_faite NOTNULL THEN 'En place' --vert
                                          WHEN Semis_fait NOTNULL THEN iif(Date_plantation IS NULL,'En place', --vert
                                                                                                   'Sous abris') --rouge
                                          ELSE 'Prévue'
                                          END),
                       D_planif TEXT,-- Format TEXT pour pouvior mettre une année simple quand on veut forcer un recalcul de planif.
                       Date_semis DATE,
                       Semis_fait BOOL,
                       Date_plantation DATE,
                       Plantation_faite BOOL,
                       Début_récolte DATE,
                       Fin_récolte DATE,
                       Récolte_faite BOOL,
                       Terminée BOOL,
                       Longueur REAL,
                       Nb_rangs REAL,
                       Espacement REAL,
                       Notes TEXT);

CREATE TABLE Destinations (Destination TEXT PRIMARY KEY,
                           Adresse TEXT,
                           Site_web TEXT,
                           Date_RAZ DATE,
                           Active BOOL DEFAULT ('x'),
                           Notes TEXT) WITHOUT ROWID;

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
                      Date_inv DATE,
                      Inventaire REAL,
                      Prix_kg REAL,
                      Conservation BOOL,
                      A_planifier BOOL DEFAULT ('x'),
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

CREATE TABLE Params (Section TEXT, Paramètre TEXT, Description TEXT, Valeur TEXT, Unité TEXT, Date_modif DATE);

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

CREATE TABLE Récoltes (ID INTEGER PRIMARY KEY AUTOINCREMENT,
                       Date DATE DEFAULT (DATE('now')) NOT NULL,
                       Espèce TEXT REFERENCES Espèces (Espèce) ON DELETE CASCADE ON UPDATE CASCADE NOT NULL,
                       Culture INTEGER REFERENCES Cultures (Culture) ON DELETE CASCADE ON UPDATE CASCADE NOT NULL,
                       Quantité REAL NOT NULL,
                       Notes TEXT);

CREATE TABLE Types_planche (Type TEXT PRIMARY KEY,
                            Notes TEXT) WITHOUT ROWID;

CREATE TABLE Variétés (Variété TEXT PRIMARY KEY,
                       Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE,
                       Nb_graines_g REAL,
                       Qté_stock REAL,
                       Qté_cde REAL,
                       Fournisseur TEXT REFERENCES Fournisseurs (Fournisseur) ON UPDATE CASCADE,
                       Notes TEXT) WITHOUT ROWID;

-- COMMIT TRANSACTION;

)#");
