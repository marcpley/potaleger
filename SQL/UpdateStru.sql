QString sDDL20250120 = QStringLiteral(R"#(
BEGIN TRANSACTION;

ALTER TABLE Rotations_détails DROP COLUMN Nb_planches;

COMMIT TRANSACTION;
)#");

QString sDDL20250227 = QStringLiteral(R"#(
BEGIN TRANSACTION;

CREATE TABLE Consommations (ID INTEGER PRIMARY KEY AUTOINCREMENT,
                       Date DATE,
                       Espèce TEXT,
                       Quantité REAL,
                       Prix REAL,
                       Destination TEXT,
                       Notes TEXT);

CREATE TABLE Destinations (Destination TEXT PRIMARY KEY,
                           Adresse TEXT,
                           Site_web TEXT,
                           Date_RAZ DATE,
                           Active TEXT,
                           Notes TEXT) WITHOUT ROWID;

COMMIT TRANSACTION;
)#");

QString sDDL20250305 = QStringLiteral(R"#(
BEGIN TRANSACTION;

ALTER TABLE Cultures ADD COLUMN Récolte_com BOOL;

UPDATE Cultures SET Récolte_com='x'
WHERE (Récolte_faite NOTNULL AND Début_récolte NOTNULL) OR
      ((SELECT count(*) FROM Récoltes R WHERE R.Culture=Cultures.Culture)>0);

COMMIT TRANSACTION;
)#");

QString sDDL20250325 = QStringLiteral(R"#(
BEGIN TRANSACTION;

UPDATE Cultures SET Récolte_faite='x '||Récolte_faite
WHERE Récolte_faite NOTNULL AND (coalesce(Récolte_faite,'') NOT LIKE 'x%');

UPDATE Cultures SET Récolte_faite=iif(Récolte_com LIKE 'x%','-'||substr(Récolte_com,2),Récolte_com)
WHERE Récolte_com NOTNULL AND Récolte_faite ISNULL;

-- ALTER TABLE Cultures DROP Récolte_com;

COMMIT TRANSACTION;
)#");

QString sDDL20250514 = QStringLiteral(R"#(
BEGIN TRANSACTION;

ALTER TABLE Espèces ADD COLUMN N REAL;
ALTER TABLE Espèces ADD COLUMN P REAL;
ALTER TABLE Espèces ADD COLUMN K REAL;

CREATE TABLE Fertilisants (Fertilisant TEXT PRIMARY KEY COLLATE POTACOLLATION,
                           Type TEXT COLLATE POTACOLLATION,
                           Fonction TEXT COLLATE POTACOLLATION,
                           pH REAL,
                           N REAL,
                           N_coef REAL,
                           P REAL,
                           P_coef REAL,
                           K REAL,
                           K_coef REAL,
                           Ca REAL,
                           Ca_coef REAL,
                           Fe REAL,
                           Fe_coef REAL,
                           Mg REAL,
                           Mg_coef REAL,
                           Na REAL,
                           Na_coef REAL,
                           S REAL,
                           S_coef REAL,
                           Si REAL,
                           Si_coef REAL) WITHOUT ROWID;

INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Algues marines','Amendement','Oligo-éléments, stimule croissance et résistance');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Broyat de bois','Couvert','Structurant, aère et nourrit les micro-organismes lignivores');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Broyat de chanvre','Couvert','Structurant azoté, bonne source de K et MO');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Cendre de bois','Amendement','Alcalinisant potassique, hausse du pH, apport de K, Ca et oligos');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Compost de champignons','Amendement','Résidu organique enrichi, apport de MO et Ca, structure');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Compost de déchets verts demi-mûr','Amendement','Stimule la vie du sol, MO, structure, nutriments pas encore disponibles');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Compost de déchets verts mûr','Amendement','Apport de MO stable, structuration');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Copeaux de corne','Engrais','Azote lent');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Crottin d’ânes','Amendement','Amendement équilibré à décomposition modérée');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Farine d’os','Engrais','Apport durable de P et Ca');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Farine de poisson','Engrais','Organique complet, riche en NPK, action moyennement rapide');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Farine de sang','Engrais','Apport d’azote soluble, action rapide');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fiente de poules séchées','Engrais','Organique concentré, apport direct de NPK, action rapide');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Foin','Couvert','Biomasse carbonée, alimentation du sol et protection contre l’érosion');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fumier de cheval','Amendement','Fertilisant organique fibreux, structuration du sol, fertilisation lente');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fumier de lapin','Amendement','Fertilisant organique très riche, utilisable frais');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fumier de mouton','Amendement','Fertilisant organique riche en NPK, chauffe bien');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fumier de poules','Amendement','Fertilisant organique NPK + MO, forte activité microbienne');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fumier de vache','Amendement','Fertilisant organique, apport de MO, effet structurant');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Guano','Engrais','Fertilisation rapide, riche en P et N');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Lombricompost','Amendement','Compost enrichi en microflore, stimulation biologique, fertilisation douce');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Paille','Couvert','Matière organique carbonée, structuration, alimentation des microbes');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Purin de consoude','Engrais','Biostimulant, apport de K et oligo-éléments');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Purin de pissenlit','Biostimulant','Stimulant racinaire et croissance');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Purin de prêle','Biostimulant','Fongistatique siliceux, renforcement des défenses naturelles');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Purin d’ortie','Engrais','Biostimulant azoté, stimulation végétative, faible NPK');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Tourteau de ricin','Engrais','Organique lent, riche en N, répulsif naturel (nématicide)');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Urine stockée','Engrais','Source d’azote minéral rapidement assimilable');

COMMIT TRANSACTION;
)#");

QString sDDL20250615 = QStringLiteral(R"#(
BEGIN TRANSACTION;

-- ALTER TABLE Espèces ADD COLUMN Vivace BOOL;
-- ALTER TABLE Espèces ADD COLUMN Favorable TEXT;
-- ALTER TABLE Espèces ADD COLUMN Défavorable TEXT;
-- ALTER TABLE Espèces ADD COLUMN Taille TEXT;

INSERT INTO Espèces (Espèce,Notes)
SELECT 'Inconnue','A supprimer après mise à jour des variétés.' WHERE (SELECT count() FROM Espèces WHERE Espèce='Inconnue')=0;

UPDATE Variétés SET Espèce='Inconnue' WHERE Espèce ISNULL;

DELETE FROM Espèces WHERE (Espèce='Inconnue')AND((SELECT count(*) FROM Variétés C WHERE C.Espèce='Inconnue')=0);

COMMIT TRANSACTION;
)#");

QString sDDL20250622 = QStringLiteral(R"#(
BEGIN TRANSACTION;

INSERT INTO Espèces (Espèce,Notes)
SELECT 'Inconnue','A supprimer après mise à jour des variétés.' WHERE (SELECT count() FROM Espèces WHERE Espèce='Inconnue')=0;

UPDATE Variétés SET Espèce='Inconnue' WHERE Espèce ISNULL;

ALTER TABLE Cultures ADD COLUMN Espèce TEXT;

UPDATE Cultures SET Espèce=(SELECT V.Espèce FROM Variétés V WHERE V.Variété=Cultures.Variété) WHERE Espèce ISNULL;
UPDATE Cultures SET Espèce=(SELECT I.Espèce FROM ITP I WHERE I.IT_plante=Cultures.IT_plante) WHERE Espèce ISNULL;
UPDATE Cultures SET Espèce='Inconnue' WHERE Espèce ISNULL;

DELETE FROM Espèces WHERE (Espèce='Inconnue')AND
                          ((SELECT count(*) FROM Cultures C WHERE C.Espèce='Inconnue')=0)AND
                          ((SELECT count(*) FROM Variétés V WHERE V.Espèce='Inconnue')=0);

COMMIT TRANSACTION;
)#");
