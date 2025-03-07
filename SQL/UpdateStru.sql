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
