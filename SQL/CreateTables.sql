DROP TABLE IF EXISTS fda_schema;
CREATE TABLE fda_schema ( --- Extended schema of the DB.
    name TEXT, --- Name of table or view.
    field_name TEXT, ---
    type TEXT, --- Field data type if field_name not empty.
    description TEXT, --- Comment in the SQL CREATE TABLE statement, use for tooltip in the app.
    tbl_last BOOL, --- Go to last record when opening a tab.
    tbl_no_data_text TEXT, --- Text displayed if there is no record in the table or view.
    tbl_pixmap BLOB, --- Pixmap for menu entries.
    tbl_type TEXT, --- 'Table' or 'View'.
    base_data BOOL, --- 'x' : useful data for standard use. Can be reset (right mouse click).\n\n'Example' : can be deleted, can't be reset.\n\nNot empty value for table itself means all records will be deleted before reset.
    col_width INTEGER, --- Default column width.
    color TEXT, --- Background color for the column, if empty the table color is used.
    combo TEXT, --- Values for the combobox, no combobox if empty.
    fk_count TEXT, --- Count of possible foreign key values.
    fk_filter TEXT, --- SQL WHERE statement to restrict possible foreign key values.
    fk_sort_field TEXT, --- Sort field for the foreign table.
    master_table TEXT, --- Table referenced by the field.
    master_field TEXT, --- Field in the master table.
    money BOOL, --- Display right aligned and with 2 decimal places if data is REAL (regardless the field data type).
    multiline BOOL, --- The multi-line text editor can be call (ctrl+N).
    natural_sort BOOL, --- Header column where a blue down arow is displayed if user did'nt set another sort.
    readonly BOOL, --- User can't modify the data via this table or view.
    summary_order INT, --- If not empty the data is used to construct a row summary.
    unit TEXT) --- Unit of the data.
    ;

CREATE TABLE Analyses_de_sol ( --- Analyses de sol effectuées.
    Analyse TEXT PRIMARY KEY,
    Planche TEXT REFERENCES Planches (Planche) ON UPDATE CASCADE,
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
    Référence TEXT,
    Organisme TEXT,
    Notes TEXT) ---
    WITHOUT ROWID;

CREATE TABLE Associations_détails ( ---
    IdxAsReEsGrFa TEXT PRIMARY KEY, --- Clé calculée: Association||iif(Requise NOTNULL,'0'||Requise,'1 ')||'-'||coalesce(Espèce,Groupe,Famille)
    Association TEXT NOT NULL, --- Nom de l'association.
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE, ---
    Groupe TEXT, ---
    Famille TEXT REFERENCES Familles (Famille) ON UPDATE CASCADE, ---
    Requise BOOL, ---
    Notes TEXT) ---
    ;
UPDATE fda_schema SET base_data='x' WHERE name='Associations_détails';

CREATE TABLE Consommations ( ---
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    Date DATE NOT NULL, -- DEFAULT (DATE('now'))
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL,
    Quantité REAL NOT NULL,
    Prix REAL, --- Prix total pour cette consommassion.
    Destination TEXT REFERENCES Destinations (Destination) ON UPDATE CASCADE, -- ON DELETE SET NULL
    Notes TEXT);

CREATE TABLE Cultures (
    Culture INTEGER PRIMARY KEY AUTOINCREMENT,
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL,
    IT_plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE,
    Variété TEXT REFERENCES Variétés (Variété) ON UPDATE CASCADE,
    Fournisseur TEXT REFERENCES Fournisseurs (Fournisseur)  ON UPDATE CASCADE,
    Planche TEXT REFERENCES Planches (Planche) ON UPDATE CASCADE,
    Type TEXT AS ---
        (CASE WHEN (Date_plantation < Date_semis) OR (Début_récolte < Date_semis) OR (Fin_récolte < Date_semis) OR (Début_récolte < Date_plantation) OR (Fin_récolte < Date_plantation) OR (Fin_récolte < Début_récolte) THEN 'Erreur dates !'
              WHEN Terminée LIKE 'v%' THEN 'Vivace'
              WHEN Date_semis NOTNULL AND Date_plantation NOTNULL AND Début_récolte NOTNULL AND Fin_récolte NOTNULL THEN 'Semis pépinière'
              WHEN Date_semis ISNULL  AND Date_plantation NOTNULL AND Début_récolte NOTNULL AND Fin_récolte NOTNULL THEN 'Plant'
              WHEN Date_semis NOTNULL AND Date_plantation ISNULL  AND Début_récolte NOTNULL AND Fin_récolte NOTNULL THEN 'Semis en place'
              WHEN Date_semis NOTNULL AND Date_plantation NOTNULL AND Début_récolte ISNULL  AND Fin_récolte NOTNULL THEN 'Compagne'
              WHEN Date_semis NOTNULL AND Date_plantation ISNULL  AND Début_récolte ISNULL THEN 'Engrais vert'
              ELSE '?' END),
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
    Espacement INTEGER,
    A_faire TEXT,
    Notes TEXT);

CREATE TABLE Destinations ( ---
    Destination TEXT PRIMARY KEY,
    Type TEXT,
    Adresse TEXT,
    Site_web TEXT,
    Date_RAZ DATE,
    Active BOOL DEFAULT ('x'),
    Interne BOOL, --- Auto-consommation.
    Notes TEXT) ---
    WITHOUT ROWID;

CREATE TABLE Espèces ( ---
    Espèce TEXT PRIMARY KEY, ---
    Famille TEXT REFERENCES Familles (Famille) ON UPDATE CASCADE, ---
    Catégories TEXT, --- Ce qui nous intéresse chez cette espèce.\nLes lettres suivantes seront remplacées par des symboles:\nLégume racine 🥕 (ra)\nLégume bulbe 🧅 (bu)\nLégume feuille et branche 🌿 (fb)\nLégume fleur 🌼 (fl)\nLégume fruit 🍆 (lf)\nGrain 🌽 (gr)\nPAM 🌺 (am)\nPetit fruit 🍓 (pf)\nFruitier 🍎 (fr)\nAgrume 🍊 (ag)\nEngrais vert 🟩 (ev)\nMellifère 🐝 (me)\nBois 🪓 (bo)\nArbre 🌳 (ar)
    Rendement REAL, ---
    Niveau TEXT, ---
    Densité REAL, ---
    Dose_semis REAL, ---
    Nb_graines_g REAL, ---
    FG REAL, ---
    T_germ TEXT, ---
    Levée REAL, ---
    Irrig TEXT, ---
    Conservation BOOL, ---
    A_planifier BOOL DEFAULT ('x'), ---
    Vivace BOOL, ---
    Besoins TEXT, ---
    S_taille INTEGER CONSTRAINT 'S_taille, semaine 1 à 52' CHECK (S_taille ISNULL OR S_taille BETWEEN 1 AND 52), --- N° de semaine (1 à 52) du début de la période de taille.
    Effet TEXT, --- Effet sur la croissance et la productionsur plantes proches (association).
    Usages TEXT, --- Propriété et usages de l'espèce (plantes aromatiques et médicinales).
    Obj_annuel REAL, ---
    N REAL, ---
    ★N TEXT AS (CASE WHEN N > 10 THEN 'Elevé' WHEN N < 5 THEN 'Faible' WHEN N NOTNULL THEN 'Moyen' END), ---
    P REAL, ---
    ★P TEXT AS (CASE WHEN P > 5 THEN 'Elevé' WHEN P < 2.55 THEN 'Faible' WHEN P NOTNULL THEN 'Moyen' END), ---
    K REAL, ---
    ★K TEXT AS (CASE WHEN K > 12 THEN 'Elevé' WHEN K < 7 THEN 'Faible' WHEN K NOTNULL THEN 'Moyen' END), ---
    Date_inv DATE, ---
    Inventaire REAL, ---
    Prix_kg REAL, ---
    Notes TEXT) ---
    WITHOUT ROWID;
UPDATE fda_schema SET base_data='x' WHERE (name='Espèces')AND(field_name IN('Espèce','Famille','Catégories','Rendement','Niveau','Densité','Dose_semis','Nb_graines_g','FG','T_germ','Levée','Vivace','Besoins','S_taille','Effet','Usages','N','P','K'));

CREATE TABLE Familles ( ---
    Famille TEXT PRIMARY KEY, ---
    Intervalle REAL DEFAULT (4), ---
    Effet TEXT, ---
    Notes TEXT) ---
    WITHOUT ROWID;
UPDATE fda_schema SET base_data='x' WHERE (name='Familles')AND(field_name NOTNULL);

CREATE TABLE Fertilisants ( ---
    Fertilisant TEXT PRIMARY KEY, ---
    Type TEXT, ---
    Fonction TEXT, ---
    Utilisation TEXT, ---
    pH REAL, ---
    N REAL, ---
    N_coef REAL, ---
    N_disp_pc REAL AS (round(N*N_coef/100,2)), ---
    P REAL, ---
    P_coef REAL, ---
    P_disp_pc REAL AS (round(P*P_coef/100,2)), ---
    K REAL, ---
    K_coef REAL, ---
    K_disp_pc REAL AS (round(K*K_coef/100,2)), ---
    Ca REAL, ---
    Ca_coef REAL, ---
    Ca_disp_pc REAL AS (round(Ca*Ca_coef/100,2)), ---
    Fe REAL, ---
    Fe_coef REAL, ---
    Fe_disp_pc REAL AS (round(Fe*Fe_coef/100,2)), ---
    Mg REAL, ---
    Mg_coef REAL, ---
    Mg_disp_pc REAL AS (round(Mg*Mg_coef/100,2)), ---
    Na REAL, ---
    Na_coef REAL, ---
    Na_disp_pc REAL AS (round(Na*Na_coef/100,2)), ---
    S REAL, ---
    S_coef REAL, ---
    S_disp_pc REAL AS (round(S*S_coef/100,2)), ---
    Si REAL, ---
    Si_coef REAL, ---
    Si_disp_pc REAL AS (round(Si*Si_coef/100,2)), ---
    Date_inv DATE, ---
    Inventaire REAL, ---
    Prix_kg REAL, ---
    Notes TEXT) ---
    WITHOUT ROWID;
UPDATE fda_schema SET base_data='x' WHERE (name='Fertilisants')AND(field_name IN('Fertilisant','Type','Fonction','pH','N','N_coef','P','P_coef','K','K_coef','Ca','Ca_coef','Fe','Fe_coef','Mg','Mg_coef','Na','Na_coef','S','S_coef','Si','Si_coef'));

CREATE TABLE Fertilisations (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    Date DATE NOT NULL, -- DEFAULT (DATE('now'))
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL,
    Culture INTEGER REFERENCES Cultures (Culture) ON UPDATE CASCADE NOT NULL,
    Fertilisant TEXT REFERENCES Fertilisants (Fertilisant) ON UPDATE CASCADE NOT NULL,
    Quantité REAL NOT NULL,
    N REAL,
    P REAL,
    K REAL,
    Notes TEXT);

CREATE TABLE Fournisseurs ( ---
    Fournisseur TEXT PRIMARY KEY, ---
    Type TEXT, ---
    Priorité INTEGER, ---
    Adresse TEXT, ---
    Site_web TEXT, ---
    Notes TEXT) ---
    WITHOUT ROWID;
UPDATE fda_schema SET base_data='x' WHERE (name='Fournisseurs')AND(field_name IN('Fournisseur','Type','Site_web','Notes'));

CREATE TABLE ITP ( ---
    IT_plante TEXT PRIMARY KEY, ---
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE, ---
    Type_planche TEXT, ---
    Type_culture TEXT AS (CASE WHEN S_semis NOTNULL AND S_plantation NOTNULL AND S_récolte NOTNULL AND D_récolte NOTNULL THEN 'Semis pépinière' ---
            WHEN S_semis ISNULL  AND S_plantation NOTNULL AND S_récolte NOTNULL AND D_récolte NOTNULL THEN 'Plant'
            WHEN S_semis NOTNULL AND S_plantation ISNULL  AND S_récolte NOTNULL AND D_récolte NOTNULL THEN 'Semis en place'
            WHEN S_semis NOTNULL AND S_plantation NOTNULL AND S_récolte NOTNULL AND D_récolte ISNULL THEN 'Compagne'
            WHEN S_semis NOTNULL AND S_plantation ISNULL     AND D_récolte ISNULL THEN 'Engrais vert'
            -- WHEN S_récolte NOTNULL AND D_récolte NOTNULL THEN 'Vivace'
            ELSE '?' END),
    S_semis INTEGER CONSTRAINT 'S_semis, semaine 1 à 52' CHECK (S_semis ISNULL OR S_semis BETWEEN 1 AND 52), ---
    S_plantation INTEGER CONSTRAINT 'S_plantation, semaine 1 à 52' CHECK (S_plantation ISNULL OR S_plantation BETWEEN 1 AND 52), ---
    S_récolte INTEGER CONSTRAINT 'S_récolte, semaine 1 à 52' CHECK (S_récolte ISNULL OR S_récolte BETWEEN 1 AND 52), ---
    D_récolte INTEGER CONSTRAINT 'D_récolte, 1 à 52 semaines' CHECK (D_récolte ISNULL OR D_récolte BETWEEN 1 AND 52), ---
    Décal_max INTEGER CONSTRAINT 'Décal_max, 0 à 52 semaines' CHECK (Décal_max ISNULL OR Décal_max BETWEEN 0 AND 52), ---
    -- Nb_rangs REAL,
    Espacement INTEGER, ---
    Esp_rangs INTEGER, ---
    Nb_graines_plant REAL, ---
    Dose_semis REAL, ---
    Notes TEXT) ---
    WITHOUT ROWID;
UPDATE fda_schema SET base_data='x' WHERE (name='ITP')AND(field_name IN('IT_plante','Espèce','Type_planche','S_semis','S_plantation','S_récolte','D_récolte','Décal_max','Espacement','Esp_rangs','Nb_graines_plant','Dose_semis','Notes'));

CREATE TABLE Notes (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    Date_création DATE,
    Date_modif DATE,
    Type TEXT,
    Description TEXT,
    Texte TEXT);

-- CREATE TABLE Params (
--     Section TEXT,
--     Paramètre TEXT,
--     Description TEXT,
--     Valeur TEXT,
--     Unité TEXT,
--     Date_modif DATE);

CREATE TABLE Planches ( ---
    Planche TEXT PRIMARY KEY, ---
    Type TEXT, ---
    Longueur REAL, ---
    Largeur REAL, ---
    Irrig TEXT, ---
    Rotation TEXT REFERENCES Rotations (Rotation) ON UPDATE CASCADE, ---
    Année INTEGER, ---
    Analyse TEXT REFERENCES Analyses_de_sol (Analyse) ON DELETE SET NULL ON UPDATE CASCADE, ---
    Planches_influencées TEXT, ---
    Notes TEXT) ---
    WITHOUT ROWID;
UPDATE fda_schema SET base_data='Example' WHERE (name='Planches')AND(field_name IN('Planche','Type','Longueur','Largeur','Rotation','Année'));

CREATE TABLE Rotations ( ---
    Rotation TEXT PRIMARY KEY, ---
    Type_planche TEXT, ---
    Année_1 INTEGER, ---
    Nb_années INTEGER, ---
    Notes TEXT) ---
    WITHOUT ROWID;
UPDATE fda_schema SET base_data='Example' WHERE (name='Rotations')AND(field_name IN('Rotation','Type_planche','Année_1','Nb_années'));

CREATE TABLE Rotations_détails ( ---
    ID INTEGER PRIMARY KEY AUTOINCREMENT, ---
    Rotation TEXT REFERENCES Rotations (Rotation) ON UPDATE CASCADE NOT NULL, ---
    Année INTEGER DEFAULT (1) NOT NULL ON CONFLICT REPLACE, ---
    IT_plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE, ---
    Pc_planches REAL DEFAULT (100) NOT NULL ON CONFLICT REPLACE, ---
    Occupation TEXT, ---
    Fi_planches TEXT, ---
    Décalage INTEGER, ---
    Notes TEXT) ---
    ;
UPDATE fda_schema SET base_data='Example' WHERE (name='Rotations_détails')AND(field_name IN('Rotation','Année','IT_plante','Pc_planches','Occupation','Fi_planches','Décalage'));

CREATE TABLE Récoltes ( --- Récoltes pour chaque culture.
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    Date DATE NOT NULL, -- DEFAULT (DATE('now'))
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL,
    Culture INTEGER REFERENCES Cultures (Culture) ON UPDATE CASCADE NOT NULL,
    Quantité REAL NOT NULL,
    Réc_ter BOOL, --- La récolte est terminée pour cette culture.\n\nIl suffit d'une seule ligne de récolte avec une valeur non vide pour que le champ 'Récolte faite' de la culture passe à 'x'.
    Notes TEXT);

CREATE TABLE Variétés ( ---
    Variété TEXT PRIMARY KEY, ---
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL, ---
    Nb_graines_g REAL, ---
    Qté_stock REAL, ---
    Qté_cde REAL, ---
    Fournisseur TEXT REFERENCES Fournisseurs (Fournisseur) ON UPDATE CASCADE, ---
    S_récolte INTEGER CONSTRAINT 'S_récolte, semaine 1 à 52' CHECK (S_récolte ISNULL OR S_récolte BETWEEN 1 AND 52), ---
    D_récolte INTEGER CONSTRAINT 'D_récolte, 1 à 52 semaines' CHECK (D_récolte ISNULL OR D_récolte BETWEEN 1 AND 52), ---
    PJ INTEGER, ---
    IT_plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE, ---
    Notes TEXT) ---
    WITHOUT ROWID;
UPDATE fda_schema SET base_data='x' WHERE (name='Variétés')AND(field_name IN('Variété','Espèce','Nb_graines_g'));

