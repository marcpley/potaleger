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
