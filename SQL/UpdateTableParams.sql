DROP TABLE IF EXISTS Temp_Params;
CREATE TABLE Temp_Params AS SELECT * FROM Params;

DROP TABLE IF EXISTS Params;

CREATE TABLE Params ( ---
    ---row_summary Section|:: . ,Paramètre|:: : ,Valeur|:: ,Unité
    Section TEXT, ---
    Paramètre TEXT PRIMARY KEY, --- Clé de recherche unique du paramètre.
    Description TEXT, ---
    Valeur TEXT, ---
    Unité TEXT,
    color TEXT) ---HIDDEN
    ;
UPDATE fda_f_schema SET readonly='x' WHERE (name='Params')AND(field_name!='Valeur');

INSERT INTO Params (Section, Paramètre, Description, Valeur, Unité)
    VALUES  ('Général', 'Utilisateur', 'Personne, entreprise ou organisation utilisant cette BDD Potaléger', NULL, NULL),
            ('Général', 'Devise', 'Symbole de la devise pour les valeurs monétaires','€', NULL),
            ('Général', 'Année_culture', 'Saison en cours de culture (la planification créé les cultures l''année suivante)', '2025', NULL),
            ('Général', 'Montrer_modifs', 'Montrer les modifications depuis le dernier rechargement des données (plus lent)', 'Oui', 'Oui/Non'),
            ('Général', 'Combo_Notes_Type', 'Types de notes, séparés par des ''|'' (vide pour texte libre)', 'A faire|Fait|Important', NULL),
            ('Général', 'Notes_Modif_dates', 'Permettre de modifier l''ID et les dates de création/modification des notes', 'Non', 'Oui/Non'),
            ('Général', 'Export_sep_col', 'Séparateur de colonnes pour les exports de données (défaut point-virgule)', NULL, NULL), -- todo: devrait être en anglais.
            ('Général', 'Export_sep_decim', 'Séparateur décimal pour les exports de données (défaut système)', NULL, NULL),
            ('Associations', 'Asso_bénéfique', 'Les associations bénéfiques se terminent par', ' +', NULL),
            ('Associations', 'Nb_sem_croisement', 'Temps de croisement mini pour que l''association soit effective.\nTemps de croisement : de la date de mise en place de la culture la plus tardive jusqu''au milieu de récolte de la culture la plus avancée.', 4, 'semaines'),
            ('Fournisseurs', 'Combo_Fournisseurs_Type', 'Types de fournisseur, séparés par des ''|'' (vide pour texte libre)', 'Semences|Irrigation|Outils|Autre', NULL),
            ('Assolement', 'Ilot_nb_car', 'En début du nom des planches, nb de caractères qui désignent l''ilot de production.'||x'0a0a'||'Ex: la planche "No1A" fait parti de l''ilot "No" si le paramètre vaut 2.', '2', 'car'),
            ('Assolement', 'UP_nb_car', 'Dans le nom des planches, après l''ilot, nb de caractères qui désignent l''unité de production.'||x'0a0a'||'Ex: la planche "No1A" fait parti de l''unité "No1" si le paramètre vaut 1 et que ''Ilot_nb_car'' vaut 2.', '1', 'car'),
            ('Assolement', 'Combo_Planches_Type', 'Types de planche, séparés par des ''|'' (vide pour texte libre)', 'Extérieur|Serre', NULL),
            ('Assolement', 'Largeur_planches', 'Largeur de planche par défaut', 0.8,'m'),
            ('Assolement', 'Rot_Rech_ass_poss', 'Dans les plans de rotation, toujours rechercher les associations possibles'||x'0a0a'||'Si non: rechercher seulement si ''Pc_planches'' < 100 et ''Occupation'' = R ou E', 'Oui', 'Oui/Non'),
            ('Irrigation', 'Combo_Planches_Irrig', 'Types d''irrigation des planches, séparés par des ''|'' (vide pour texte libre)', 'GàG|Asp', NULL),
            ('Irrigation', 'Combo_Espèces_Irrig', 'Types d''irrigation, séparés par des ''|'' (vide pour texte libre)', 'GàG|Asp', NULL),
            ('Irrigation', 'C_Irrig_avant_MEP', 'Voir les cultures à irriguer dont la date de mise en place est dans moins de', '15', 'jours'),
            ('Irrigation', 'C_Irrig_après_MEP', 'Période d''irrigation après date de mise en place (semis en place ou plantation)', '30', 'jours'),
            ('Planification', 'Planifier_planches', 'Début du nom des planches à planifier (vide pour toutes)', NULL, NULL),
            ('Planification', 'Planifier_retard', 'Si l''opération précédente (semis, plantation) est déjà faite et est en retard, avant de passer à l''année suivante, accepter de reporter l''opération suivante de maximum', '15', 'jours'),
            ('Cultures', 'C_horizon_semis', 'Voir les cultures à semer dont la date de semis est dans moins de', '90', 'jours'),
            ('Cultures', 'C_horizon_plantation', 'Voir les cultures à planter dont la date de plantation est dans moins de', '90', 'jours'),
            ('Cultures', 'C_horizon_récolte', 'Voir les cultures à récolter dont la date de début de récolte est dans moins de', '90', 'jours'),
            ('Cultures', 'C_voir_récolte', 'Voir les saisies de récolte depuis la saison (vide pour toutes)', NULL, NULL),
            ('Cultures', 'C_récolte_après_MEP', 'Récoltes: nb jours mini entre date de mise en place (semis en place ou plantation) et dates de récolte', '15', 'jours'),
            ('Cultures', 'C_récolte_avance', 'Récoltes: avance maxi entre date de récolte réelle (Saisie des récoltes) et prévue (Cultures, ''Début_récolte'')', '15', 'jours'),
            ('Cultures', 'C_récolte_prolongation', 'Récoltes: prolongation maxi entre date de récolte réelle (Saisie des récoltes) et prévue (Cultures, ''Fin_récolte'')', '30', 'jours'),
            ('Cultures', 'C_horizon_terminer', 'Voir les cultures à terminer dont la date de fin de récolte est dans moins de', '15', 'jours'),
            ('Cultures', 'C_modif_N_culture', 'Permettre de modifier les n° de cultures (jusqu''à la prochaine mise à jour du schéma de BDD)', 'Non', 'Oui/Non'),
            ('Destinations', 'Combo_Destinations_Type', 'Types de destination, séparés par des ''|'' (vide pour texte libre)', 'Magasin|Particulier|Autre', NULL),
            ('Consommations', 'Conso_historique', 'Voir les consommations anciennes de', '90', 'jours'),
            ('Fertilisation', 'Combo_Fertilisants_Type', 'Types de fertilisants, séparés par des ''|'' (vide pour texte libre)', 'Amendement|Biostimulant|Couvert|Engrais', NULL),
            ('Fertilisation', 'C_horizon_fertiliser', 'Voir les cultures à fertiliser dont la date de mise en place est dans moins de', '90', 'jours'),
            ('Fertilisation', 'Ferti_historique', 'Voir les fertilisation anciennes de', '90', 'jours'),
            ('Fertilisation', 'Ferti_avance', 'Fertilisations avant la date mise en place, avance possible de', '60', 'jours'),
            ('Fertilisation', 'Ferti_retard', 'Fertilisations après la date ''Début_récolte'', retard possible de', '30', 'jours'),
            ('Fertilisation', 'Ferti_coef_N',  'Coefficient de disponibilité de l''azote (dépend fortement de la minéralisation de la matière organique donc très lié à l''activité biologique)', '70', '%'),
            ('Fertilisation', 'Ferti_coef_P',  'Coefficient de disponibilité du phosphore (très sensible au pH, bloqué à pH<6 (fixé sur Fe/Al) ou pH>7.5 (fixé sur Ca))', '50', '%'),
            ('Fertilisation', 'Ferti_coef_K',  'Coefficient de disponibilité du potassium (peu influencé par le pH, mais peut être lessivé dans les sols sableux)', '85', '%'),
            ('Fertilisation', 'Ferti_coef_Ca', 'Coefficient de disponibilité du calcium (plus disponible en sols neutres à basiques)', '80', '%'),
            ('Fertilisation', 'Ferti_coef_Fe', 'Coefficient de disponibilité du fer (bloqué à pH basique, disponibilité maximale en sol acide)', '60', '%'),
            ('Fertilisation', 'Ferti_coef_Mg', 'Coefficient de disponibilité du magnésium (plus disponible en sols neutres à basiques)', '70', '%'),
            ('Fertilisation', 'Ferti_coef_Na', 'Coefficient de disponibilité du sodium', '75', '%'),
            ('Fertilisation', 'Ferti_coef_S',  'Coefficient de disponibilité du soufre (suit souvent le cycle de l’azote et est très lié à la MO)', '70', '%'),
            ('Fertilisation', 'Ferti_coef_Si', 'Coefficient de disponibilité du silicium', '60', '%'),
            ('Fertilisation', 'Déficit_fert', 'Seuil en dessous duquel la planche est concidérée en déficit de fertilisation', '50', '%'),
            ('Analyses', 'Tolérance_A_semis', 'Date de semis incohérente si elle est avant la période prévue, avance de plus de', '30', 'jours'),
            ('Analyses', 'Tolérance_R_semis', 'Date de semis incohérente si elle est après la période prévue, retard de plus de', '30', 'jours'),
            ('Analyses', 'Tolérance_A_plantation', 'Date de plantation incohérente si elle est avant la période prévue, avance de plus de', '30', 'jours'),
            ('Analyses', 'Tolérance_R_plantation', 'Date de plantation incohérente si elle est après la période prévue, retard de plus de', '30', 'jours'),
            ('Analyses', 'Tolérance_A_récolte', 'Dates de récolte incohérentes si elles commencent avant la période prévue, avance de plus de', '30', 'jours'),
            ('Analyses', 'Tolérance_R_récolte', 'Dates de récolte incohérentes si elles finissent après la période prévue, retard de plus de', '30', 'jours'),
            ('Danger', 'SQL_données', 'Permettre de modifier les données avec une requète utilisateur',null, 'Oui!/Non'),
            ('Danger', 'SQL_schéma', 'Permettre de modifier le schéma de la base de données avec une requète utilisateur',null, 'Oui!/Non');

UPDATE Params SET
    Valeur=coalesce((SELECT T.Valeur FROM Temp_Params T WHERE T.Paramètre=Params.Paramètre),Params.Valeur),
    color=(SELECT F.color FROM fda_t_schema F WHERE F.name LIKE Section||'%');

DROP TABLE Temp_Params;

UPDATE Params SET
    Valeur='Non'
WHERE Paramètre='C_modif_N_culture';
