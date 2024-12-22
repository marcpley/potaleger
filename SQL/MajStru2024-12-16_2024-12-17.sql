QString sDDL20241217 = QStringLiteral(R"(
CREATE VIEW Info_Potaléger AS SELECT 1 N,'Version de la BDD' Info, '2024-12-17' Valeur
UNION
SELECT 2, 'Utilisateur',(SELECT Valeur FROM Params WHERE Paramètre = 'Utilisateur')
UNION
SELECT 3, 'Variétés',(SELECT count() || ' dont '  FROM Variétés) ||
                     (SELECT count() || ' en culture actuellement et '  FROM (SELECT DISTINCT Variété FROM C_en_place ORDER BY Variété))  ||
                     (SELECT count() || ' en culture planifiée.'  FROM (SELECT DISTINCT Variété FROM C_a_venir ORDER BY Variété))
UNION
SELECT 4, 'Itinéraires techniques',(SELECT count() || ' dont '  FROM ITP) ||
                                   (SELECT count() || ' en culture actuellement et '  FROM (SELECT DISTINCT IT_Plante FROM C_en_place ORDER BY IT_Plante))  ||
                                   (SELECT count() || ' en culture planifiée.'  FROM (SELECT DISTINCT IT_Plante FROM C_a_venir ORDER BY IT_Plante))
UNION
SELECT 5, 'Planches',(SELECT count() || ' dont '  FROM Planches) ||
                     (SELECT count() || ' en culture actuellement et '  FROM (SELECT DISTINCT Planche FROM C_en_place ORDER BY Planche))  ||
                     (SELECT count() || ' en culture planifiée.'  FROM (SELECT DISTINCT Planche FROM C_a_venir ORDER BY Planche))
UNION
SELECT 6, 'Cultures',(SELECT count() || ' dont '  FROM Cultures) || (SELECT count() || ' non terminées.'  FROM Cultures WHERE Terminée ISNULL)
)");
