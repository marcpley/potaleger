
---------------
-- Main menu --
---------------

INSERT INTO fada_launchers (launcher_name,type,parent,item_index)
VALUES ('Données de base','Submenu','Main menu',100),
       ('Assolement','Submenu','Main menu',110),
       ('Planification','Submenu','Main menu',120),
       ('Cultures','Submenu','Main menu',130),
       ('Fertilisation','Submenu','Main menu',140),
       ('Stock','Submenu','Main menu',150),
       ('Analyses','Submenu','Main menu',160);

-- Données de base

INSERT INTO fada_launchers (launcher_name,type,parent,item_index,name,title,filters)
VALUES ('Familles botaniques','Menu item','Données de base',10,'Familles','Familles',NULL),
       ('Espèces','Submenu','Données de base',20,NULL,NULL,NULL),
       ('Associations','Menu item','Données de base',30,'Associations_détails__Saisies',NULL,
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Favorables|Association LIKE ''% +'''||x'0a0a'||
        'Défavorables|Association LIKE ''% !'''||x'0a0a'||
        'Autres|NOT(Association LIKE ''% +'') AND NOT(Association LIKE ''% !'')'),
       ('Fournisseurs','Menu item','Données de base',40,'Fournisseurs',NULL,NULL),
       ('Variétés','Menu item','Données de base',50,'Variétés',NULL,
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Annuelles|(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Vivaces|(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'),
       ('Itinéraires techniques','Menu item','Données de base',60,'ITP__Tempo','ITP',
        'Pour'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Annuelles|(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Vivaces|(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Génériques|(Espèce ISNULL)'||x'0a0a'||
        'A planifier|(SELECT (E.A_planifier NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)');

INSERT INTO fada_launchers (launcher_name,type,parent,item_index,name,title,filters)
VALUES ('Espèces annuelles','Menu item','Espèces',10,'Espèces__a','Espèces an.',NULL),
       ('Espèces vivaces','Menu item','Espèces',20,'Espèces__v','Espèces vi.',NULL),
       ('Toutes','Menu item','Espèces',30,'Espèces','Espèces',NULL);

-- Assolement

INSERT INTO fada_launchers (launcher_name,type,parent,item_index,name,title,filters)
VALUES ('Rotations','Submenu','Assolement',10,NULL,NULL,NULL),
       ('Planches','Menu item','Assolement',20,'Planches',NULL,NULL),
       ('Succession de cultures parent planches','Menu item','Assolement',30,'Cultures__Succ_planches','Succ. planches',NULL),
       ('Ilots','Menu item','Assolement',40,'Assolement_Ilots',NULL,NULL),
       ('Unités de production','Menu item','Assolement',50,'Assolement_Unités_prod','Unités prod.',NULL);

INSERT INTO fada_launchers (launcher_name,type,parent,item_index,name,title,filters)
VALUES ('Entetes','Menu item','Rotations',10,'Rotations',NULL,NULL),
       ('Détails','Menu item','Rotations',20,'Rotations_détails__Tempo','Rot. (détails)',NULL),
       ('Occupation des planches','Menu item','Rotations',30,'Rotations_détails__Tempo_occup','Rot. (occupation)',NULL),
       ('Espèces manquantes dans les rotations','Menu item','Rotations',40,'Espèces__manquantes','Espèces manquantes',NULL);

-- Planification

INSERT INTO fada_launchers (launcher_name,type,parent,item_index,name,title,filters,graph)
VALUES ('Cultures prévues par espèce','Menu item','Planification',10,'Planif_espèces','Cult.prévues espèces',NULL,NULL),
       ('Cultures prévues par ilots','Menu item','Planification',20,'Planif_ilots','Cult.prévues ilots',NULL,NULL),
       ('Cultures prévues par planches','Menu item','Planification',30,'Planif_planches','Cult.prévues',
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Pas créées|(Déjà_créée ISNULL)'||x'0a0a'||
        'Pas validées|(Validée ISNULL)'||x'0a0a'||
        'Pas créées pas validées|(Déjà_créée ISNULL)AND(Validée ISNULL)'||x'0a0a'||
        'Créées et validées|(Déjà_créée NOTNULL)AND(Validée NOTNULL)'||x'0a0a'||
        'Conflit et validées|(Déjà_en_place LIKE ''%!'')AND(Validée NOTNULL)'||x'0a0a'||
        'Créées ou conflit et validées|(Déjà_en_place NOTNULL)AND(Validée NOTNULL)'||x'0a0a'||
        'Validées|Validée NOTNULL',NULL),
       ('Récoltes prévues par mois','Menu item','Planification',40,'Planif_récoltes_m','Réc. prévues mois',NULL,
        'Date|GroupMonth||Qté_réc|Sum|Line|||Valeur|Sum|Line'),
       ('Récoltes prévues par semaines','Menu item','Planification',50,'Planif_récoltes_s','Réc. prévues sem.',NULL,
        'Date|GroupWeek||Qté_réc|Sum|Line|||Valeur|Sum|Line'),
       ('Associations dans les cultures prévues','Menu item','Planification',60,'Planif_associations','Asso.prévues',NULL,NULL),
       ('Créer les cultures','Menu item','Planification',70,'Planif_creer_cultures',NULL,NULL,NULL),
       --('Créer les cultures (script)','Menu item','Planification',71,'CreerCulturesScript',NULL,NULL,NULL),
       ('Semences nécessaires','Menu item','Planification',80,'Variétés__inv_et_cde','Inv. et cde semences',
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Annuelles|(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Vivaces|(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Espèces concernées|TN.Espèce IN(SELECT V.Espèce FROM Variétés__inv_et_cde V WHERE (V.Qté_nécess>0)OR(V.Qté_cde>0)OR(Notes LIKE ''%!''))',NULL),
       ('Plants nécessaires','Menu item','Planification',90,'Variétés__cde_plants','Commande plants',
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Annuelles|(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Vivaces|(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Espèces concernées|TN.Espèce IN(SELECT V.Espèce FROM Variétés__cde_plants V WHERE (V.Qté_nécess>0)OR(Notes LIKE ''%!''))',NULL);

-- Cultures

INSERT INTO fada_launchers (launcher_name,type,parent,item_index,name,title,filters,graph)
VALUES ('Toutes les cultures non terminées','Menu item','Cultures',10,'Cultures__non_terminées','Non terminées',
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Annuelles|(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Vivaces|(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)',NULL),
       (NULL,'Spacer','Cultures',20,NULL,NULL,NULL,NULL),
       ('A semer','Submenu','Cultures',30,NULL,NULL,NULL,NULL),
       ('A planter','Menu item','Cultures',40,'Cultures__à_planter',NULL,
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Annuelles|(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Vivaces|(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)',NULL),
       ('A irriguer','Menu item','Cultures',50,'Cultures__à_irriguer',NULL,NULL,NULL),
       ('A récolter','Menu item','Cultures',60,'Cultures__à_récolter','A récolter',
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Annuelles|(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Vivaces|(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)',NULL),
       ('Récoltes','Menu item','Cultures',70,'Récoltes__Saisies','Récoltes',
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Annuelles|(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Vivaces|(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)',NULL),
       ('A terminer','Menu item','Cultures',80,'Cultures__à_terminer',NULL,NULL,NULL),
       ('Trucs à faire','Menu item','Cultures',90,'Cultures__A_faire','A faire',
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Annuelles|(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Vivaces|(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)',NULL),
       (NULL,'Spacer','Cultures',100,NULL,NULL,NULL,NULL),
       ('Vivaces','Menu item','Cultures',110,'Cultures__vivaces',NULL,
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Non ter.|(Terminée=''v'')OR(Terminée=''V'')',NULL),
       ('Associations dans les cultures non terminées','Menu item','Cultures',120,'Associations__présentes','Ass. en cours',
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Bénéfiques|Association LIKE ''%''||(SELECT Valeur FROM Params WHERE Paramètre=''Asso_bénéfique'')'||x'0a0a'||
        'Non bénéfiques|NOT(Association LIKE ''%''||(SELECT Valeur FROM Params WHERE Paramètre=''Asso_bénéfique''))'||x'0a0a'||
        'Avec annuelles|Nb_cultures>Nb_vivaces',NULL),
       ('Toutes les cultures','Menu item','Cultures',130,'Cultures','Cultures',
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Annuelles 2021|(Saison=''2021'')AND(coalesce(Terminée,'') NOT LIKE (''v%''))'||x'0a0a'|| --todo
        'Annuelles 2022|(Saison=''2022'')AND(coalesce(Terminée,'') NOT LIKE (''v%''))'||x'0a0a'||
        'Annuelles 2023|(Saison=''2023'')AND(coalesce(Terminée,'') NOT LIKE (''v%''))'||x'0a0a'||
        'Annuelles 2024|(Saison=''2024'')AND(coalesce(Terminée,'') NOT LIKE (''v%''))'||x'0a0a'||
        'Annuelles 2025|(Saison=''2025'')AND(coalesce(Terminée,'') NOT LIKE (''v%''))'||x'0a0a'||
        'Annuelles 2026|(Saison=''2026'')AND(coalesce(Terminée,'') NOT LIKE (''v%''))'||x'0a0a'||
        'Annuelles|(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Vivaces|(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)',NULL);

INSERT INTO fada_launchers (launcher_name,type,parent,item_index,name,title,filters,graph)
VALUES ('Toutes','Menu item','A semer',10,'Cultures__à_semer','A semer',
        'Voir'||x'0a0a'||
        'Toutes|TRUE'||x'0a0a'||
        'Annuelles|(SELECT (E.Vivace ISNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)'||x'0a0a'||
        'Vivaces|(SELECT (E.Vivace NOTNULL) FROM Espèces E WHERE E.Espèce=TN.Espèce)',NULL),
       ('Semis pépinières','Menu item','A semer',20,'Cultures__à_semer_pep','A semer (pépinière)',NULL,NULL),
       ('Semis en place','Menu item','A semer',30,'Cultures__à_semer_EP','A semer (en place)',NULL,NULL);

-- Fertilisation

INSERT INTO fada_launchers (launcher_name,type,parent,item_index,name,title,filters,graph)
VALUES ('Analyses de sol','Menu item','Fertilisation',10,'Analyses_de_sol','Analyses sol',NULL,NULL),
       ('Fertilisants','Menu item','Fertilisation',20,'Fertilisants',NULL,NULL,NULL),
       ('Inventaire','Menu item','Fertilisation',30,'Fertilisants__inventaire','Inventaire F.',NULL,NULL),
       ('Fertilisations','Menu item','Fertilisation',40,'Fertilisations__Saisies',NULL,NULL,NULL),
       ('Bilan par planches','Menu item','Fertilisation',50,'Planches__bilan_fert','Bilan fert.',
        'Saison'||x'0a0a'||
        '2021|Saison=''2021'''||x'0a0a'|| --todo
        '2022|Saison=''2022'''||x'0a0a'||
        '2023|Saison=''2023'''||x'0a0a'||
        '2024|Saison=''2024'''||x'0a0a'||
        '2025|Saison=''2025'''||x'0a0a'||
        '2026|Saison=''2026''',NULL),
       ('Planches en déficit','Menu item','Fertilisation',60,'Planches__deficit_fert','Déficit',NULL,NULL);

-- Stock

INSERT INTO fada_launchers (launcher_name,type,parent,item_index,name,title,filters,graph)
VALUES ('Destinations','Menu item','Stock',10,'Destinations__conso',NULL,NULL,NULL),
       ('Saisie des consommations','Menu item','Stock',20,'Consommations__Saisies','Consommations',NULL,NULL),
       ('Inventaire','Menu item','Stock',30,'Espèces__inventaire','Inventaire E.',NULL,NULL);

-- Analyses

INSERT INTO fada_launchers (launcher_name,type,parent,item_index,name,title,filters,graph)
VALUES ('Bilans annuels','Menu item','Analyses',10,'Bilans_annuels','Bilans',NULL,NULL),
       ('Bilans annuels par espèces','Menu item','Analyses',20,'Espèces__Bilans_annuels','Bilan espèces',
        'Saison'||x'0a0a'||
        '2021|Saison=''2021'''||x'0a0a'||
        '2022|Saison=''2022'''||x'0a0a'||
        '2023|Saison=''2023'''||x'0a0a'||
        '2024|Saison=''2024'''||x'0a0a'||
        '2025|Saison=''2025'''||x'0a0a'||
        '2026|Saison=''2026'''||x'0a0a'||
        'Toutes|TRUE',NULL),
       (NULL,'Spacer','Analyses',30,NULL,NULL,NULL,NULL),
       ('Itinéraires techniques annuelles','Menu item','Analyses',40,'ITP__analyse_a','Analyse IT',NULL,NULL),
       ('Itinéraires techniques vivaces','Menu item','Analyses',50,'ITP__analyse_v','Analyse IT',NULL,NULL),
       ('Cultures annuelles','Submenu','Analyses',60,NULL,NULL,NULL,NULL),
       ('Incohérences dates cultures','Menu item','Analyses',70,'Cultures__inc_dates','Inc. dates cultures',NULL,NULL);

INSERT INTO fada_launchers (launcher_name,type,parent,item_index,name,title,filters,graph)
VALUES ('par culture','Menu item','Cultures annuelles',10,'Cultures__analyse','Analyse cultures',NULL,NULL),
       ('par variété','Menu item','Cultures annuelles',20,'Variétés__analyse','Analyse variétés',NULL,NULL),
       ('par itinéraire','Menu item','Cultures annuelles',30,'ITP__analyse','Analyse itinéraires',NULL,NULL),
       ('par espèce et saison','Menu item','Cultures annuelles',40,'Espèces__analyse_saison','Analyse  esp. saison',NULL,NULL),
       ('par espèce et type de planche','Menu item','Cultures annuelles',50,'Espèces__analyse_type_planche','Analyse esp. type pl.',NULL,NULL),
       ('par espèce','Menu item','Cultures annuelles',60,'Espèces__analyse','Analyse espèce',NULL,NULL);
