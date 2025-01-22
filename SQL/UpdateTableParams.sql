QString sDDLTableParams = QStringLiteral(R"#(

BEGIN TRANSACTION;

CREATE TABLE Temp_Params AS SELECT * FROM Params;

DROP TABLE Params;

CREATE TABLE Params (Section TEXT, Paramètre TEXT, Description TEXT, Valeur TEXT, Unité TEXT);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité)
    VALUES  ('Général', 'Utilisateur', 'Personne, entreprise ou organisation utilisant cette BDD Potaléger', 'Ferme Légère', NULL),
            ('Données de base', 'Ilot_nb_car', 'Nb de caractères du début du nom des planches qui désignent l''ilot de production. Ex: la planche "No1A" fait parti de l''ilot "No" si le paramètre vaut 2.', '2', 'car'),
            ('Planification', 'Année_culture', 'Année en cours de culture', '2024', NULL),
            ('Planification', 'Année_planif', 'Année à planifier', '2025', NULL),
            ('Planification', 'Planifier_planches', 'Début du nom des planches à planifier', NULL, NULL),
            ('Cultures', 'Horizon_semis', 'Voir les cultures à semer sur une période de', '90', 'jours'),
            ('Cultures', 'Horizon_plantation', 'Voir les cultures à planter sur une période de', '90', 'jours'),
            ('Cultures', 'Horizon_récolte', 'Voir les cultures à récolter sur une période de', '90', 'jours'),
            ('Cultures', 'Horizon_terminer', 'Voir les cultures à terminer sur une période de', '90', 'jours');

UPDATE Params SET Valeur=coalesce((SELECT T.Valeur FROM Temp_Params T WHERE T.Paramètre=Params.Paramètre),Params.Valeur);

DROP TABLE Temp_Params;

COMMIT TRANSACTION;

)#");
