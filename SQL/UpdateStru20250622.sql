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

-- ALTER TABLE Variétés ADD COLUMN Déb_récolte TEXT;
-- ALTER TABLE Variétés ADD COLUMN Fin_récolte TEXT;

COMMIT TRANSACTION;
