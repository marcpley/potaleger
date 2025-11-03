BEGIN TRANSACTION;

ALTER TABLE Cultures ADD COLUMN Récolte_com BOOL;

UPDATE Cultures SET Récolte_com='x'
WHERE (Récolte_faite NOTNULL AND Début_récolte NOTNULL) OR
      ((SELECT count(*) FROM Récoltes R WHERE R.Culture=Cultures.Culture)>0);

COMMIT TRANSACTION;
