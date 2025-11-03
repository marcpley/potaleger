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
