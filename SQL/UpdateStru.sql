QString sDDL20250120=QStringLiteral(R"#(
BEGIN TRANSACTION;

ALTER TABLE Rotations_détails DROP COLUMN Nb_planches;

COMMIT TRANSACTION;
)#");

QString sDDL20250227=QStringLiteral(R"#(
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

QString sDDL20250305=QStringLiteral(R"#(
BEGIN TRANSACTION;

ALTER TABLE Cultures ADD COLUMN Récolte_com BOOL;

UPDATE Cultures SET Récolte_com='x'
WHERE (Récolte_faite NOTNULL AND Début_récolte NOTNULL) OR
      ((SELECT count(*) FROM Récoltes R WHERE R.Culture=Cultures.Culture)>0);

COMMIT TRANSACTION;
)#");

QString sDDL20250325=QStringLiteral(R"#(
BEGIN TRANSACTION;

UPDATE Cultures SET Récolte_faite='x '||Récolte_faite
WHERE Récolte_faite NOTNULL AND (coalesce(Récolte_faite,'') NOT LIKE 'x%');

UPDATE Cultures SET Récolte_faite=iif(Récolte_com LIKE 'x%','-'||substr(Récolte_com,2),Récolte_com)
WHERE Récolte_com NOTNULL AND Récolte_faite ISNULL;

-- ALTER TABLE Cultures DROP Récolte_com;

COMMIT TRANSACTION;
)#");

QString sDDL20250514=QStringLiteral(R"#(
BEGIN TRANSACTION;

ALTER TABLE Espèces ADD COLUMN N REAL;
ALTER TABLE Espèces ADD COLUMN P REAL;
ALTER TABLE Espèces ADD COLUMN K REAL;

CREATE TABLE Fertilisants (Fertilisant TEXT PRIMARY KEY,
                           Type TEXT,
                           Fonction TEXT,
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

QString sDDL20250615=QStringLiteral(R"#(
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

QString sDDL20250622=QStringLiteral(R"#(
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

QString sDDL20250728=QStringLiteral(R"#(
BEGIN TRANSACTION;

DROP TABLE IF EXISTS sqlean_define;

DROP TRIGGER IF EXISTS ITP_UPDATE_FinsPériodes;
DROP TRIGGER IF EXISTS Variétés_UPDATE_FinsPériodes;

ALTER TABLE ITP ADD COLUMN S_semis INTEGER;
ALTER TABLE ITP ADD COLUMN S_plantation INTEGER;
ALTER TABLE ITP ADD COLUMN S_récolte INTEGER;
ALTER TABLE ITP ADD COLUMN D_récolte INTEGER;
ALTER TABLE ITP ADD COLUMN Décal_max INTEGER;

ALTER TABLE Variétés ADD COLUMN S_récolte INTEGER;
ALTER TABLE Variétés ADD COLUMN D_récolte INTEGER;

UPDATE ITP SET S_semis=1 WHERE Déb_semis='01-01';
UPDATE ITP SET S_semis=3 WHERE Déb_semis='01-15';
UPDATE ITP SET S_semis=5 WHERE Déb_semis='02-01';
UPDATE ITP SET S_semis=7 WHERE Déb_semis='02-15';
UPDATE ITP SET S_semis=10 WHERE Déb_semis='03-01';
UPDATE ITP SET S_semis=12 WHERE Déb_semis='03-15';
UPDATE ITP SET S_semis=14 WHERE Déb_semis='04-01';
UPDATE ITP SET S_semis=16 WHERE Déb_semis='04-15';
UPDATE ITP SET S_semis=18 WHERE Déb_semis='05-01';
UPDATE ITP SET S_semis=20 WHERE Déb_semis='05-15';
UPDATE ITP SET S_semis=23 WHERE Déb_semis='06-01';
UPDATE ITP SET S_semis=25 WHERE Déb_semis='06-15';
UPDATE ITP SET S_semis=27 WHERE Déb_semis='07-01';
UPDATE ITP SET S_semis=29 WHERE Déb_semis='07-15';
UPDATE ITP SET S_semis=31 WHERE Déb_semis='08-01';
UPDATE ITP SET S_semis=33 WHERE Déb_semis='08-15';
UPDATE ITP SET S_semis=36 WHERE Déb_semis='09-01';
UPDATE ITP SET S_semis=38 WHERE Déb_semis='09-15';
UPDATE ITP SET S_semis=40 WHERE Déb_semis='10-01';
UPDATE ITP SET S_semis=42 WHERE Déb_semis='10-15';
UPDATE ITP SET S_semis=45 WHERE Déb_semis='11-01';
UPDATE ITP SET S_semis=47 WHERE Déb_semis='11-15';
UPDATE ITP SET S_semis=49 WHERE Déb_semis='12-01';
UPDATE ITP SET S_semis=51 WHERE Déb_semis='12-15';
UPDATE ITP SET S_plantation=1 WHERE Déb_plantation='01-01';
UPDATE ITP SET S_plantation=3 WHERE Déb_plantation='01-15';
UPDATE ITP SET S_plantation=5 WHERE Déb_plantation='02-01';
UPDATE ITP SET S_plantation=7 WHERE Déb_plantation='02-15';
UPDATE ITP SET S_plantation=10 WHERE Déb_plantation='03-01';
UPDATE ITP SET S_plantation=12 WHERE Déb_plantation='03-15';
UPDATE ITP SET S_plantation=14 WHERE Déb_plantation='04-01';
UPDATE ITP SET S_plantation=16 WHERE Déb_plantation='04-15';
UPDATE ITP SET S_plantation=18 WHERE Déb_plantation='05-01';
UPDATE ITP SET S_plantation=20 WHERE Déb_plantation='05-15';
UPDATE ITP SET S_plantation=23 WHERE Déb_plantation='06-01';
UPDATE ITP SET S_plantation=25 WHERE Déb_plantation='06-15';
UPDATE ITP SET S_plantation=27 WHERE Déb_plantation='07-01';
UPDATE ITP SET S_plantation=29 WHERE Déb_plantation='07-15';
UPDATE ITP SET S_plantation=31 WHERE Déb_plantation='08-01';
UPDATE ITP SET S_plantation=33 WHERE Déb_plantation='08-15';
UPDATE ITP SET S_plantation=36 WHERE Déb_plantation='09-01';
UPDATE ITP SET S_plantation=38 WHERE Déb_plantation='09-15';
UPDATE ITP SET S_plantation=40 WHERE Déb_plantation='10-01';
UPDATE ITP SET S_plantation=42 WHERE Déb_plantation='10-15';
UPDATE ITP SET S_plantation=45 WHERE Déb_plantation='11-01';
UPDATE ITP SET S_plantation=47 WHERE Déb_plantation='11-15';
UPDATE ITP SET S_plantation=49 WHERE Déb_plantation='12-01';
UPDATE ITP SET S_plantation=51 WHERE Déb_plantation='12-15';
UPDATE ITP SET S_récolte=1 WHERE Déb_récolte='01-01';
UPDATE ITP SET S_récolte=3 WHERE Déb_récolte='01-15';
UPDATE ITP SET S_récolte=5 WHERE Déb_récolte='02-01';
UPDATE ITP SET S_récolte=7 WHERE Déb_récolte='02-15';
UPDATE ITP SET S_récolte=10 WHERE Déb_récolte='03-01';
UPDATE ITP SET S_récolte=12 WHERE Déb_récolte='03-15';
UPDATE ITP SET S_récolte=14 WHERE Déb_récolte='04-01';
UPDATE ITP SET S_récolte=16 WHERE Déb_récolte='04-15';
UPDATE ITP SET S_récolte=18 WHERE Déb_récolte='05-01';
UPDATE ITP SET S_récolte=20 WHERE Déb_récolte='05-15';
UPDATE ITP SET S_récolte=23 WHERE Déb_récolte='06-01';
UPDATE ITP SET S_récolte=25 WHERE Déb_récolte='06-15';
UPDATE ITP SET S_récolte=27 WHERE Déb_récolte='07-01';
UPDATE ITP SET S_récolte=29 WHERE Déb_récolte='07-15';
UPDATE ITP SET S_récolte=31 WHERE Déb_récolte='08-01';
UPDATE ITP SET S_récolte=33 WHERE Déb_récolte='08-15';
UPDATE ITP SET S_récolte=36 WHERE Déb_récolte='09-01';
UPDATE ITP SET S_récolte=38 WHERE Déb_récolte='09-15';
UPDATE ITP SET S_récolte=40 WHERE Déb_récolte='10-01';
UPDATE ITP SET S_récolte=42 WHERE Déb_récolte='10-15';
UPDATE ITP SET S_récolte=45 WHERE Déb_récolte='11-01';
UPDATE ITP SET S_récolte=47 WHERE Déb_récolte='11-15';
UPDATE ITP SET S_récolte=49 WHERE Déb_récolte='12-01';
UPDATE ITP SET S_récolte=51 WHERE Déb_récolte='12-15';

UPDATE ITP SET S_récolte=1 WHERE Déb_récolte ISNULL AND Fin_récolte='01-01';
UPDATE ITP SET S_récolte=3 WHERE Déb_récolte ISNULL AND Fin_récolte='01-15';
UPDATE ITP SET S_récolte=5 WHERE Déb_récolte ISNULL AND Fin_récolte='02-01';
UPDATE ITP SET S_récolte=7 WHERE Déb_récolte ISNULL AND Fin_récolte='02-15';
UPDATE ITP SET S_récolte=10 WHERE Déb_récolte ISNULL AND Fin_récolte='03-01';
UPDATE ITP SET S_récolte=12 WHERE Déb_récolte ISNULL AND Fin_récolte='03-15';
UPDATE ITP SET S_récolte=14 WHERE Déb_récolte ISNULL AND Fin_récolte='04-01';
UPDATE ITP SET S_récolte=16 WHERE Déb_récolte ISNULL AND Fin_récolte='04-15';
UPDATE ITP SET S_récolte=18 WHERE Déb_récolte ISNULL AND Fin_récolte='05-01';
UPDATE ITP SET S_récolte=20 WHERE Déb_récolte ISNULL AND Fin_récolte='05-15';
UPDATE ITP SET S_récolte=23 WHERE Déb_récolte ISNULL AND Fin_récolte='06-01';
UPDATE ITP SET S_récolte=25 WHERE Déb_récolte ISNULL AND Fin_récolte='06-15';
UPDATE ITP SET S_récolte=27 WHERE Déb_récolte ISNULL AND Fin_récolte='07-01';
UPDATE ITP SET S_récolte=29 WHERE Déb_récolte ISNULL AND Fin_récolte='07-15';
UPDATE ITP SET S_récolte=31 WHERE Déb_récolte ISNULL AND Fin_récolte='08-01';
UPDATE ITP SET S_récolte=33 WHERE Déb_récolte ISNULL AND Fin_récolte='08-15';
UPDATE ITP SET S_récolte=36 WHERE Déb_récolte ISNULL AND Fin_récolte='09-01';
UPDATE ITP SET S_récolte=38 WHERE Déb_récolte ISNULL AND Fin_récolte='09-15';
UPDATE ITP SET S_récolte=40 WHERE Déb_récolte ISNULL AND Fin_récolte='10-01';
UPDATE ITP SET S_récolte=42 WHERE Déb_récolte ISNULL AND Fin_récolte='10-15';
UPDATE ITP SET S_récolte=45 WHERE Déb_récolte ISNULL AND Fin_récolte='11-01';
UPDATE ITP SET S_récolte=47 WHERE Déb_récolte ISNULL AND Fin_récolte='11-15';
UPDATE ITP SET S_récolte=49 WHERE Déb_récolte ISNULL AND Fin_récolte='12-01';
UPDATE ITP SET S_récolte=51 WHERE Déb_récolte ISNULL AND Fin_récolte='12-15';

UPDATE ITP SET S_récolte=52 WHERE Déb_récolte ISNULL AND Fin_récolte ISNULL AND Type_culture IN('Engrais vert','Sans récolte');

UPDATE ITP SET Décal_max=1-S_plantation WHERE Fin_plantation='01-01';
UPDATE ITP SET Décal_max=3-S_plantation WHERE Fin_plantation='01-15';
UPDATE ITP SET Décal_max=5-S_plantation WHERE Fin_plantation='02-01';
UPDATE ITP SET Décal_max=7-S_plantation WHERE Fin_plantation='02-15';
UPDATE ITP SET Décal_max=10-S_plantation WHERE Fin_plantation='03-01';
UPDATE ITP SET Décal_max=12-S_plantation WHERE Fin_plantation='03-15';
UPDATE ITP SET Décal_max=14-S_plantation WHERE Fin_plantation='04-01';
UPDATE ITP SET Décal_max=16-S_plantation WHERE Fin_plantation='04-15';
UPDATE ITP SET Décal_max=18-S_plantation WHERE Fin_plantation='05-01';
UPDATE ITP SET Décal_max=20-S_plantation WHERE Fin_plantation='05-15';
UPDATE ITP SET Décal_max=23-S_plantation WHERE Fin_plantation='06-01';
UPDATE ITP SET Décal_max=25-S_plantation WHERE Fin_plantation='06-15';
UPDATE ITP SET Décal_max=27-S_plantation WHERE Fin_plantation='07-01';
UPDATE ITP SET Décal_max=29-S_plantation WHERE Fin_plantation='07-15';
UPDATE ITP SET Décal_max=31-S_plantation WHERE Fin_plantation='08-01';
UPDATE ITP SET Décal_max=33-S_plantation WHERE Fin_plantation='08-15';
UPDATE ITP SET Décal_max=36-S_plantation WHERE Fin_plantation='09-01';
UPDATE ITP SET Décal_max=38-S_plantation WHERE Fin_plantation='09-15';
UPDATE ITP SET Décal_max=40-S_plantation WHERE Fin_plantation='10-01';
UPDATE ITP SET Décal_max=42-S_plantation WHERE Fin_plantation='10-15';
UPDATE ITP SET Décal_max=45-S_plantation WHERE Fin_plantation='11-01';
UPDATE ITP SET Décal_max=47-S_plantation WHERE Fin_plantation='11-15';
UPDATE ITP SET Décal_max=49-S_plantation WHERE Fin_plantation='12-01';
UPDATE ITP SET Décal_max=51-S_plantation WHERE Fin_plantation='12-15';

UPDATE ITP SET Décal_max=1-S_semis WHERE Décal_max ISNULL AND Fin_semis='01-01';
UPDATE ITP SET Décal_max=3-S_semis WHERE Décal_max ISNULL AND Fin_semis='01-15';
UPDATE ITP SET Décal_max=5-S_semis WHERE Décal_max ISNULL AND Fin_semis='02-01';
UPDATE ITP SET Décal_max=7-S_semis WHERE Décal_max ISNULL AND Fin_semis='02-15';
UPDATE ITP SET Décal_max=10-S_semis WHERE Décal_max ISNULL AND Fin_semis='03-01';
UPDATE ITP SET Décal_max=12-S_semis WHERE Décal_max ISNULL AND Fin_semis='03-15';
UPDATE ITP SET Décal_max=14-S_semis WHERE Décal_max ISNULL AND Fin_semis='04-01';
UPDATE ITP SET Décal_max=16-S_semis WHERE Décal_max ISNULL AND Fin_semis='04-15';
UPDATE ITP SET Décal_max=18-S_semis WHERE Décal_max ISNULL AND Fin_semis='05-01';
UPDATE ITP SET Décal_max=20-S_semis WHERE Décal_max ISNULL AND Fin_semis='05-15';
UPDATE ITP SET Décal_max=23-S_semis WHERE Décal_max ISNULL AND Fin_semis='06-01';
UPDATE ITP SET Décal_max=25-S_semis WHERE Décal_max ISNULL AND Fin_semis='06-15';
UPDATE ITP SET Décal_max=27-S_semis WHERE Décal_max ISNULL AND Fin_semis='07-01';
UPDATE ITP SET Décal_max=29-S_semis WHERE Décal_max ISNULL AND Fin_semis='07-15';
UPDATE ITP SET Décal_max=31-S_semis WHERE Décal_max ISNULL AND Fin_semis='08-01';
UPDATE ITP SET Décal_max=33-S_semis WHERE Décal_max ISNULL AND Fin_semis='08-15';
UPDATE ITP SET Décal_max=36-S_semis WHERE Décal_max ISNULL AND Fin_semis='09-01';
UPDATE ITP SET Décal_max=38-S_semis WHERE Décal_max ISNULL AND Fin_semis='09-15';
UPDATE ITP SET Décal_max=40-S_semis WHERE Décal_max ISNULL AND Fin_semis='10-01';
UPDATE ITP SET Décal_max=42-S_semis WHERE Décal_max ISNULL AND Fin_semis='10-15';
UPDATE ITP SET Décal_max=45-S_semis WHERE Décal_max ISNULL AND Fin_semis='11-01';
UPDATE ITP SET Décal_max=47-S_semis WHERE Décal_max ISNULL AND Fin_semis='11-15';
UPDATE ITP SET Décal_max=49-S_semis WHERE Décal_max ISNULL AND Fin_semis='12-01';
UPDATE ITP SET Décal_max=51-S_semis WHERE Décal_max ISNULL AND Fin_semis='12-15';

UPDATE ITP SET Décal_max=Décal_max+52 WHERE Décal_max<0;

UPDATE ITP SET D_récolte=1-S_récolte WHERE Fin_récolte='01-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=3-S_récolte WHERE Fin_récolte='01-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=5-S_récolte WHERE Fin_récolte='02-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=7-S_récolte WHERE Fin_récolte='02-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=10-S_récolte WHERE Fin_récolte='03-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=12-S_récolte WHERE Fin_récolte='03-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=14-S_récolte WHERE Fin_récolte='04-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=16-S_récolte WHERE Fin_récolte='04-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=18-S_récolte WHERE Fin_récolte='05-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=20-S_récolte WHERE Fin_récolte='05-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=23-S_récolte WHERE Fin_récolte='06-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=25-S_récolte WHERE Fin_récolte='06-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=27-S_récolte WHERE Fin_récolte='07-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=29-S_récolte WHERE Fin_récolte='07-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=31-S_récolte WHERE Fin_récolte='08-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=33-S_récolte WHERE Fin_récolte='08-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=36-S_récolte WHERE Fin_récolte='09-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=38-S_récolte WHERE Fin_récolte='09-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=40-S_récolte WHERE Fin_récolte='10-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=42-S_récolte WHERE Fin_récolte='10-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=45-S_récolte WHERE Fin_récolte='11-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=47-S_récolte WHERE Fin_récolte='11-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=49-S_récolte WHERE Fin_récolte='12-01' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=51-S_récolte WHERE Fin_récolte='12-15' AND NOT(Type_culture IN('Engrais vert','Sans récolte'));

UPDATE ITP SET D_récolte=max(D_récolte-Décal_max,1) WHERE D_récolte NOTNULL AND NOT(Type_culture IN('Engrais vert','Sans récolte'));
UPDATE ITP SET D_récolte=D_récolte+52 WHERE D_récolte NOTNULL AND D_récolte<=0;

UPDATE Variétés SET S_récolte=1 WHERE Déb_récolte='01-01';
UPDATE Variétés SET S_récolte=3 WHERE Déb_récolte='01-15';
UPDATE Variétés SET S_récolte=5 WHERE Déb_récolte='02-01';
UPDATE Variétés SET S_récolte=7 WHERE Déb_récolte='02-15';
UPDATE Variétés SET S_récolte=10 WHERE Déb_récolte='03-01';
UPDATE Variétés SET S_récolte=12 WHERE Déb_récolte='03-15';
UPDATE Variétés SET S_récolte=14 WHERE Déb_récolte='04-01';
UPDATE Variétés SET S_récolte=16 WHERE Déb_récolte='04-15';
UPDATE Variétés SET S_récolte=18 WHERE Déb_récolte='05-01';
UPDATE Variétés SET S_récolte=20 WHERE Déb_récolte='05-15';
UPDATE Variétés SET S_récolte=23 WHERE Déb_récolte='06-01';
UPDATE Variétés SET S_récolte=25 WHERE Déb_récolte='06-15';
UPDATE Variétés SET S_récolte=27 WHERE Déb_récolte='07-01';
UPDATE Variétés SET S_récolte=29 WHERE Déb_récolte='07-15';
UPDATE Variétés SET S_récolte=31 WHERE Déb_récolte='08-01';
UPDATE Variétés SET S_récolte=33 WHERE Déb_récolte='08-15';
UPDATE Variétés SET S_récolte=36 WHERE Déb_récolte='09-01';
UPDATE Variétés SET S_récolte=38 WHERE Déb_récolte='09-15';
UPDATE Variétés SET S_récolte=40 WHERE Déb_récolte='10-01';
UPDATE Variétés SET S_récolte=42 WHERE Déb_récolte='10-15';
UPDATE Variétés SET S_récolte=45 WHERE Déb_récolte='11-01';
UPDATE Variétés SET S_récolte=47 WHERE Déb_récolte='11-15';
UPDATE Variétés SET S_récolte=49 WHERE Déb_récolte='12-01';
UPDATE Variétés SET S_récolte=51 WHERE Déb_récolte='12-15';

UPDATE Variétés SET D_récolte=1-S_récolte WHERE Fin_récolte='01-01';
UPDATE Variétés SET D_récolte=3-S_récolte WHERE Fin_récolte='01-15';
UPDATE Variétés SET D_récolte=5-S_récolte WHERE Fin_récolte='02-01';
UPDATE Variétés SET D_récolte=7-S_récolte WHERE Fin_récolte='02-15';
UPDATE Variétés SET D_récolte=10-S_récolte WHERE Fin_récolte='03-01';
UPDATE Variétés SET D_récolte=12-S_récolte WHERE Fin_récolte='03-15';
UPDATE Variétés SET D_récolte=14-S_récolte WHERE Fin_récolte='04-01';
UPDATE Variétés SET D_récolte=16-S_récolte WHERE Fin_récolte='04-15';
UPDATE Variétés SET D_récolte=18-S_récolte WHERE Fin_récolte='05-01';
UPDATE Variétés SET D_récolte=20-S_récolte WHERE Fin_récolte='05-15';
UPDATE Variétés SET D_récolte=23-S_récolte WHERE Fin_récolte='06-01';
UPDATE Variétés SET D_récolte=25-S_récolte WHERE Fin_récolte='06-15';
UPDATE Variétés SET D_récolte=27-S_récolte WHERE Fin_récolte='07-01';
UPDATE Variétés SET D_récolte=29-S_récolte WHERE Fin_récolte='07-15';
UPDATE Variétés SET D_récolte=31-S_récolte WHERE Fin_récolte='08-01';
UPDATE Variétés SET D_récolte=33-S_récolte WHERE Fin_récolte='08-15';
UPDATE Variétés SET D_récolte=36-S_récolte WHERE Fin_récolte='09-01';
UPDATE Variétés SET D_récolte=38-S_récolte WHERE Fin_récolte='09-15';
UPDATE Variétés SET D_récolte=40-S_récolte WHERE Fin_récolte='10-01';
UPDATE Variétés SET D_récolte=42-S_récolte WHERE Fin_récolte='10-15';
UPDATE Variétés SET D_récolte=45-S_récolte WHERE Fin_récolte='11-01';
UPDATE Variétés SET D_récolte=47-S_récolte WHERE Fin_récolte='11-15';
UPDATE Variétés SET D_récolte=49-S_récolte WHERE Fin_récolte='12-01';
UPDATE Variétés SET D_récolte=51-S_récolte WHERE Fin_récolte='12-15';

UPDATE Variétés SET D_récolte=D_récolte+52 WHERE D_récolte NOTNULL AND D_récolte<=0;

COMMIT TRANSACTION;
)#");
