QString sDDLTableParams = QStringLiteral(R"#(

BEGIN TRANSACTION;

CREATE TABLE Temp_Params AS SELECT * FROM Params;

DROP TABLE Params;

CREATE TABLE Params (Section TEXT, Paramètre TEXT PRIMARY KEY, Description TEXT, Valeur TEXT, Unité TEXT);
INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité)
    VALUES  ('Général', 'Utilisateur', 'Personne, entreprise ou organisation utilisant cette BDD Potaléger', NULL, NULL),
            ('Général', 'Année_culture', 'Saison en cours de culture (la planification créé les cultures l''année suivante)', '2025', NULL),
            ('Général', 'Montrer_modifs', 'Montrer les données modifiées depuis le passage en mode édition (plus lent)', 'Oui', 'Oui/Non'),
            ('Général', 'Combo_Notes_Type', 'Types de notes, séparés par des ''|'' (vide pour texte libre)', 'A faire|Fait|Important', NULL),
            ('Général', 'Notes_Modif_dates', 'Permettre de modifier l''ID et les dates de création/modification des notes', 'Non', 'Oui/Non'),
            ('Général', 'Combo_Apports_Type', 'Types d''apports, séparés par des ''|'' (vide pour texte libre)', 'Minéral|Organique|Autre', NULL),
            ('Général', 'Combo_Fournisseurs_Type', 'Types de fournisseur, séparés par des ''|'' (vide pour texte libre)', 'Semences|Irrigation|Outils|Autre', NULL),
            ('Général', 'Combo_Destinations_Type', 'Types de destination, séparés par des ''|'' (vide pour texte libre)', 'Magasin|Particulier|Autre', NULL),
            ('Assolement', 'Ilot_nb_car', 'Nb de caractères du début du nom des planches qui désignent l''ilot de production. nEx: la planche "No1A" fait parti de l''ilot "No" si le paramètre vaut 2.', '2', 'car'),
            ('Assolement', 'Combo_Planches_Type', 'Types de planche, séparés par des ''|'' (vide pour texte libre)', 'Extérieur|Serre', NULL),
            ('Assolement', 'Combo_Planches_Irrig', 'Types d''irrigation des planches, séparés par des ''|'' (vide pour texte libre)', 'GàG|Asp', NULL),
            -- ('Planification', 'Année_planif', 'Saison à planifier', '2025', NULL),
            ('Planification', 'Planifier_planches', 'Début du nom des planches à planifier (vide pour toutes)', NULL, NULL),
            ('Planification', 'Planifier_retard', 'Si l''opération précédente (semis, plantation) est déjà faite et est en retard, avant de passer à l''année suivante, accepter de reporter l''opération suivante de maximum', '15', 'jours'),
            ('Cultures', 'C_horizon_semis', 'Voir les cultures à semer sur une période de', '90', 'jours'),
            ('Cultures', 'C_horizon_plantation', 'Voir les cultures à planter sur une période de', '90', 'jours'),
            ('Cultures', 'C_horizon_récolte', 'Voir les cultures à récolter sur une période de', '90', 'jours'),
            ('Cultures', 'C_historique_récolte', 'Voir les saisies de récolte anciennes de', '90', 'jours'),
            ('Cultures', 'C_avance_saisie_récolte', 'Saisie de récoltes avant la date ''Début_récolte'', avance possible de', '15', 'jours'),
            ('Cultures', 'C_retard_saisie_récolte', 'Saisie de récoltes après la date ''Fin_récolte'', retard possible de', '30', 'jours'),
            ('Cultures', 'C_horizon_terminer', 'Voir les cultures à terminer sur une période de', '90', 'jours'),
            ('Cultures', 'C_modif_N_culture', 'Permettre de modifier les n° de cultures', 'Non', 'Oui/Non'),
            ('Consommations', 'Conso_historique', 'Voir les consommations anciennes de', '90', 'jours'),
            ('Analyses', 'Tolérance_A_semis', 'Date de semis incohérente si elle est avant la période prévue, avance de plus de', '30', 'jours'),
            ('Analyses', 'Tolérance_R_semis', 'Date de semis incohérente si elle est après la période prévue, retard de plus de', '30', 'jours'),
            ('Analyses', 'Tolérance_A_plantation', 'Date de plantation incohérente si elle est avant la période prévue, avance de plus de', '30', 'jours'),
            ('Analyses', 'Tolérance_R_plantation', 'Date de plantation incohérente si elle est après la période prévue, retard de plus de', '30', 'jours'),
            ('Analyses', 'Tolérance_A_récolte', 'Dates de récolte incohérentes si elles commencent avant la période prévue, avance de plus de', '30', 'jours'),
            ('Analyses', 'Tolérance_R_récolte', 'Dates de récolte incohérentes si elles finissent après la période prévue, retard de plus de', '30', 'jours');

UPDATE Params SET Valeur=coalesce((SELECT T.Valeur FROM Temp_Params T WHERE T.Paramètre=Params.Paramètre),Params.Valeur);

DROP TABLE Temp_Params;

COMMIT TRANSACTION;

)#");
