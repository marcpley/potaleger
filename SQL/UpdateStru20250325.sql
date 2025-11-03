BEGIN TRANSACTION;

UPDATE Cultures SET Récolte_faite='x '||Récolte_faite
WHERE Récolte_faite NOTNULL AND (coalesce(Récolte_faite,'') NOT LIKE 'x%');

UPDATE Cultures SET Récolte_faite=iif(Récolte_com LIKE 'x%','-'||substr(Récolte_com,2),Récolte_com)
WHERE Récolte_com NOTNULL AND Récolte_faite ISNULL;

-- ALTER TABLE Cultures DROP Récolte_com;

COMMIT TRANSACTION;
