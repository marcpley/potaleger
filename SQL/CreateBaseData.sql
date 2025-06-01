QString sSQLBaseData = QStringLiteral(R"#(

-- BEGIN TRANSACTION;

INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Alliacées', 4.0, 'Ail, ciboulette, oignon, poireau

Sensibles à la pourriture blanche.

Plantes herbacées vivaces, le plus souvent à bulbe, rarement rhizomateuses.
Feuilles simples, fleurs régulières. Le fruit est une capsule. Les graines sont
albuminées.

Régions tempérées chaudes et subtropicales.

Wikipedia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Apiacées', 4.0, 'Carotte, Céleri, Coriandre, Cumin, Fenouil, Panais, Persil

Risques de maladies fongiques du sol.

Tige creuse et dotée de canaux sécréteurs de résines et d''essences odorantes.
Feuilles alternes, composées, pennées, au pétiole engainant au niveau des
nœuds. Leur inflorescence est généralement une ombelle. Fruits secs doubles.

Régions tempérées.

Wikipedia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Astéracées', 3.0, 'Artichaut, chicorée, laitue, tournesol

Risques de sclérotiniose.

Tige herbacée généralement dressée. Feuilles sans stipules, souvent simples.
Fleurs minuscules, réunies en inflorescences appelées capitules.

Principalement dans les régions tempérées.

Wikipedia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Brassicacées', 3.0, 'Choux, colza, cresson, moutarde, navet, raifort

Risques d’hernie du chou, altises.

Plantes herbacées. Iinflorescence généralement en grappe simple.

Surtout présentes dans l''hémisphère nord.

Wikipedia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Chénopodiacées', 3.0, 'Betterave, chénopodes, épinard, poirée (ou bette)

Maladies racinaires comme le rhizoctone.

Plantes herbacées.');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Convolvulacées', 4.0, 'Liserons, patate douce

Sensible aux maladies fongiques (fusariose, pourriture noire) et aux nématodes à galles (Meloidogyne spp.).

Racine pivotante, ramifiée, charnue. Tiges dressées ou prostrées, cylindriques,
ramifiées. Feuilles alternes, pétiolées, sans stipules. Exsudent souvent un
latex blanc. Fleurs généralement grandes et très colorées.

Régions tropicales et subtropicales

Wikipedia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Cucurbitacées', 3.0, 'Concombre, courges, courgettes, melon

Problèmes de fusariose et d’oïdium.

Généralement des plantes herbacées, à port rampant ou grimpant. Feuilles
alternes sans stipule. Grandes fleurs. Le fruit est en général une baie.

Wikipedia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Euphorbiacées', 5.0, 'Manioc, ricin

Sensible à la bactériose vasculaire, risque de  nématodes et de
cochenilles farineuses.

Plantes à fleurs dicotylédones.

Wikipedia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Fabacées', 3.0, 'Arachide, fèves, haricots, lentilles, luzerne, pois chiche, soja, trèfle

Sensibles aux nématodes.

Activité symbiotique de fixation de l''azote atmosphérique grâce à des bactéries.

Wikipédia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Hydrophyllacées', 3.0, 'Phacélie');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Lamiacées', 3.0, 'Basilic, mélisse, menthe, romarin, sauge, serpolet, thym

Sensibles à  la fusariose, l’oïdium et la verticilliose.

Producteurs d''huiles essentielles, souvent mellifères.

Wikipedia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Poacées', 4.0, 'Blé, maïs, riz

Plantes herbacées à tiges cylindriques aux entrenœuds creux. Feuilles alternes dont la gaine enveloppe la tige. Inflorescence élémentaire en épillets, fleurs réduites aux organes sexuels.

Wikipedia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Polygonacée', 4.0, 'Oseille, rhubarbe, sarrasin

Sensible à certaines rouilles et fusarioses.

Plantes herbacées, le plus souvent vivaces. Racines fibreuses qui  forment des
racines secondaires latérales qui colonisent l''espace disponible. Feuilles
simples généralement alternes.  Riches en acide oxalique (toxiques en grande
quantité).

Régions de l''hémisphère nord.

Wikipedia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Solanacées', 4.0, 'Aubergine, piment, pomme de terre, tomate

Feuilles généralement alternes. 5 pétales soudés entre eux à des hauteurs
variables. Fruit charnu ou sec. Graines habituellement oléagineuses.

Wikipedia');
INSERT INTO Familles (Famille, Intervalle, Notes) VALUES ('Valérianacées', 3.0, 'Mâche, valériane

Sensible au mildiou (Peronospora valerianellae), à la sclérotiniose. Rique de de fonte des semis (Pythium, Rhizoctonia).');

-- INSERT INTO Apports (Apport, Type, Description, Poids_m², Notes) VALUES ('Aucun',NULL, NULL, 0.0, NULL);
-- INSERT INTO Apports (Apport, Type, Description, Poids_m², Notes) VALUES ('CDM20','Organique', 'Compost demi-mûr', 20.0, NULL);
-- INSERT INTO Apports (Apport, Type, Description, Poids_m², Notes) VALUES ('CDM40','Organique', 'Compost demi-mûr', 40.0, NULL);
-- INSERT INTO Apports (Apport, Type, Description, Poids_m², Notes) VALUES ('CM20','Organique', 'Compost mûr', 20.0, NULL);
-- INSERT INTO Apports (Apport, Type, Description, Poids_m², Notes) VALUES ('CM40','Organique', 'Compost mûr', 40.0, NULL);

INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Ail', 'Alliacées', 0.7, 'Facile', NULL, NULL, NULL, NULL, NULL, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Aubergine', 'Solanacées', 2.0, 'Difficile', 0.04, 250.0, 6.0, '24-30', 6.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Betterave', 'Chénopodiacées', 2.5, 'Facile', 1.0, 50.0, 6.0, '10-30', 10.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Blette', 'Chénopodiacées', 8.0, 'Facile', 1.0, 60.0, 5.0, '10-30', 15.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Carotte', 'Apiacées', 3.0, 'Difficile', 0.35, 925.0, 4.5, '7-30', 15.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Chou brocoli', 'Brassicacées', 1.5, 'Moyen', 0.03, 250.0, 4.5, '7-30', 7.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Chou de Bruxelles', 'Brassicacées', 1.0, 'Moyen', 0.04, 350.0, 5.5, '7-20', 7.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Chou de Chine', 'Brassicacées', 2.0, 'Difficile', 0.03, 350.0, 2.5, '7-30', 5.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Chou frisé', 'Brassicacées', 2, 'Facile', 0.03, 300.0, 5.5, '7-20', 10.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Chou pommé', 'Brassicacées', 2, 'Moyen', 0.03, 300.0, 5.0, '7-30', 6.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Chou-fleur', 'Brassicacées', 1.5, 'Difficile', 0.03, 400.0, 5.5, '7-30', 9.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Chou-rave', 'Brassicacées', 2.0, 'Facile', 0.03, 300.0, 5.0, '7-30', 9.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Concombre', 'Cucurbitacées', 1.8, 'Facile', 0.125, 35.0, 10.0, '16-35', 9.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Courge', 'Cucurbitacées', 5, 'Difficile', 0.4, 4.58, 5.0, '21-35', 8.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Courgette', 'Cucurbitacées', 4, 'Facile', 0.5, 7.7, 5.0, '21-35', 8.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Céleri', 'Apiacées', 5.0, 'Moyen', 0.02, 2500.0, 7.0, '16-21', 15.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Epinard', 'Chénopodiacées', 2.0, 'Moyen', 0.75, 110.0, 4.5, '7-24', 10.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Fenouil', 'Apiacées', 2.0, 'Moyen', 0.5, 200.0, 4.0, '12', 9.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Fève', 'Fabacées', 4.0, 'Moyen', 45.0, 0.75, 5.0, '8', 15.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Haricot mangetout', 'Fabacées', 1.5, 'Moyen', 10.0, 7, 3.0, '16-30', 6.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Haricot à écosser', 'Fabacées', 0.2, 'Moyen', 10.0, 3, 3.0, '16-30', 6.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Laitue', 'Astéracées', 2.2, 'Moyen', 0.15, 800.0, 4.5, '5-27', 7.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Maïs', 'Poacées', 1.0, 'Moyen', 0.8, 4.5, 2.0, '16-30', 12.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Melon', 'Cucurbitacées', 1.6, 'Difficile', 0.15, 35.0, 7.0, '24-35', 6.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Moutarde', 'Brassicacées', NULL, NULL, 1.3, 130.0, NULL, '3-5', 6.0, NULL, 'Pour mélange engrais vert');
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Mâche', 'Valérianacées', 1.0, 'Moyen', 0.6, 660.0, 5.0, '15-20', 13.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Mélange', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Mélilot', 'Fabacées', NULL, NULL, 1.75, 120.0, NULL, '5-10', 8.0, NULL, 'Pour mélange engrais vert');
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Navet', 'Brassicacées', 2.2, 'Moyen', 0.3, 600.0, 4.5, '7-30', 5.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Oignon', 'Alliacées', 3.0, 'Moyen', 0.4, 264.0, 2.0, '10-30', 22.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Panais', 'Apiacées', 1.5, 'Moyen', 0.5, 220.0, 1.0, '20', 17.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Patate douce', 'Convovulvacées', 2, 'Difficile', NULL, NULL, NULL, '25', NULL, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Persil', 'Apiacées', 1.0, 'Moyen', 1.0, 600.0, 3.0, '16-30', 20.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Phacélie', 'Hydrophyllacées', NULL, NULL, 1.5, 550.0, NULL, '4-5', 8.0, NULL, 'Pour mélange engrais vert');
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Physalis', 'Solanacées', 1, NULL, NULL,  825.0, 8.0, '20-22', 25.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Poireau', 'Alliacées', 1.8, 'Moyen', 0.17, 300.3, 2.0, '13-24', 15.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Poivron piment', 'Solanacées', 2.5, 'Moyen', 0.075, 150.0, 4.0, '18-35', 12.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Pomme de terre', 'Solanacées', 2.8, 'Moyen', NULL, NULL, NULL, NULL, NULL, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Radis', 'Brassicacées', 0.7, 'Facile', 4.5, 120.0, 4.5, '7-32', 4.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Radis noir', 'Brassicacées', 2.5, 'Facile', 4.5, 120.0, 4.5, '7-32', 3.5, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Ricin', 'Euphorbiacées', NULL, NULL, 0.5, 1.0, NULL, NULL, NULL, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Roquette', 'Brassicacées', 2.5, 'Facile', NULL, 660.0, NULL, NULL, NULL, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Sarrasin', 'Polygonacée', NULL, NULL, 5.5, 5.5, NULL, '10-15', 8.0, NULL, 'Pour mélange engrais vert');
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Sorgho', 'Poacées', NULL, NULL, 4.5, 27.0, NULL, '20-30', 8.0, NULL, 'Pour mélange engrais vert');
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Tomate', 'Solanacées', 8, 'Moyen', 0.03, 350.0, 4.0, '16-30', 7.0, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Topinambour', 'Astéracées', 1.0, 'Facile', NULL, NULL, NULL, NULL, NULL, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Tournesol', 'Apiacées', 1, NULL, NULL, 50.0, 7.0, NULL, NULL, 'Oui', NULL);
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Trèfle blanc', 'Fabacées', NULL, NULL, 1.0, 50.0, NULL, '5-10', 8.0, NULL, 'Pour mélange engrais vert');
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Trèfle violet', 'Fabacées', NULL, NULL, 2.0, 40.0, NULL, '5-10', 8.0, NULL, 'Pour mélange engrais vert');
INSERT INTO Espèces (Espèce, Famille, Rendement, Niveau, Dose_semis, Nb_graines_g, FG, T_germ, Levée, A_planifier, Notes) VALUES ('Vesce', 'Fabacées', NULL, NULL, 12.0, 45.0, NULL, '5-10', 8.0, NULL, 'Pour mélange engrais vert');

INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Algues marines','Amendement','Oligo-éléments, stimule croissance et résistance');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Broyat de bois','Couvert','Structurant, aère et nourrit les micro-organismes lignivores');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Broyat de chanvre','Couvert','Structurant azoté, bonne source de K et MO');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Cendre de bois','Amendement','Alcalinisant potassique, hausse du pH, apport de K, Ca et oligos');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Compost de champignons','Amendement','Résidu organique enrichi, apport de MO et Ca, structure');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Compost de déchets verts demi-mûr','Amendement','Stimule la vie du sol, MO, structure, nutriments pas encore disponibles');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Compost de déchets verts mûr','Amendement','Apport de MO stable, structuration');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Copeaux de corne','Engrais','Azote lent');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Crottin d’ânes','Amendement','Amendement équilibré à décomposition modérée');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Farine d’os','Engrais','Apport durable de P et Ca');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Farine de poisson','Engrais','Organique complet, riche en NPK, action moyennement rapide');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Farine de sang','Engrais','Apport d’azote soluble, action rapide');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fiente de poules séchées','Engrais','Organique concentré, apport direct de NPK, action rapide');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Foin','Couvert','Biomasse carbonée, alimentation du sol et protection contre l’érosion');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fumier de cheval','Amendement','Fertilisant organique fibreux, structuration du sol, fertilisation lente');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fumier de lapin','Amendement','Fertilisant organique très riche, utilisable frais');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fumier de mouton','Amendement','Fertilisant organique riche en NPK, chauffe bien');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fumier de poules','Amendement','Fertilisant organique NPK + MO, forte activité microbienne');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Fumier de vache','Amendement','Fertilisant organique, apport de MO, effet structurant');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Guano','Engrais','Fertilisation rapide, riche en P et N');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Lombricompost','Amendement','Compost enrichi en microflore, stimulation biologique, fertilisation douce');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Paille','Couvert','Matière organique carbonée, structuration, alimentation des microbes');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Purin de consoude','Engrais','Biostimulant, apport de K et oligo-éléments');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Purin de pissenlit','Biostimulant','Stimulant racinaire et croissance');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Purin de prêle','Biostimulant','Fongistatique siliceux, renforcement des défenses naturelles');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Purin d’ortie','Engrais','Biostimulant azoté, stimulation végétative, faible NPK');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Tourteau de ricin','Engrais','Organique lent, riche en N, répulsif naturel (nématicide)');
INSERT INTO Fertilisants (Fertilisant, Type,Fonction) VALUES ('Urine stockée','Engrais','Source d’azote minéral rapidement assimilable');



INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Agrosemens','Semences', NULL, 'https://www.agrosemens.com', 'Bio');
INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Baumaux', 'Semences', NULL, 'https://www.graines-baumaux.fr', NULL);
INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Biau germe', 'Semences', NULL, 'https://www.biaugerme.com', 'Bio');
INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Del Païs', 'Semences', NULL, 'https://grainesdelpais.com', 'Bio');
INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Essembio', 'Semences', NULL, 'https://essembio.com', 'Bio');
INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Gautier', 'Semences', NULL, 'https://www.gautiersemences.com', NULL);
INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Germinance', 'Semences', NULL, 'https://www.germinance.com', 'Bio');
INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Kokopelli', 'Semences', NULL, 'https://kokopelli-semences.fr', 'Bio');
INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Pensez Sauvage', 'Semences', NULL, 'https://pensezsauvage.org', 'Bio');
INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Prosem', 'Semences', NULL, 'https://www.prosem.fr', NULL);
INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Sativa', 'Semences', NULL, 'https://www.sativa-semencesbio.fr', 'Bio');
INSERT INTO Fournisseurs (Fournisseur, Type, Priorité, Site_web, Notes) VALUES ('Voltz', 'Semences', NULL, 'https://www.graines-voltz.com', NULL);

-- INSERT INTO Types_planche (Type, Notes) VALUES ('Extérieur', NULL);
-- INSERT INTO Types_planche (Type, Notes) VALUES ('Serre', NULL);

INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC1A', 'Extérieur', 50.0, 1.0, 'Grande culture', 3, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC1B', 'Extérieur', 50.0, 1.0, 'Grande culture', 3, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC1C', 'Extérieur', 50.0, 1.0, 'Grande culture', 3, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC1D', 'Extérieur', 50.0, 1.0, 'Grande culture', 3, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC2A', 'Extérieur', 50.0, 1.0, 'Grande culture', 2, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC2B', 'Extérieur', 50.0, 1.0, 'Grande culture', 2, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC2C', 'Extérieur', 50.0, 1.0, 'Grande culture', 2, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC2D', 'Extérieur', 50.0, 1.0, 'Grande culture', 2, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC3A', 'Extérieur', 50.0, 1.0, 'Grande culture', 1, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC3B', 'Extérieur', 50.0, 1.0, 'Grande culture', 1, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC3C', 'Extérieur', 50.0, 1.0, 'Grande culture', 1, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('GC3D', 'Extérieur', 50.0, 1.0, 'Grande culture', 1, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po1A', 'Extérieur', 8.0, 0.8, 'Potager', 1, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po1B', 'Extérieur', 8.0, 0.8, 'Potager', 1, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po1C', 'Extérieur', 8.0, 0.8, 'Potager', 1, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po1D', 'Extérieur', 8.0, 0.8, 'Potager', 1, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po2A', 'Extérieur', 8.0, 0.8, 'Potager', 5, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po2B', 'Extérieur', 8.0, 0.8, 'Potager', 5, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po2C', 'Extérieur', 8.0, 0.8, 'Potager', 5, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po2D', 'Extérieur', 8.0, 0.8, 'Potager', 5, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po3A', 'Extérieur', 8.0, 0.8, 'Potager', 4, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po3B', 'Extérieur', 8.0, 0.8, 'Potager', 4, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po3C', 'Extérieur', 8.0, 0.8, 'Potager', 4, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po3D', 'Extérieur', 8.0, 0.8, 'Potager', 4, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po4A', 'Extérieur', 8.0, 0.8, 'Potager', 3, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po4B', 'Extérieur', 8.0, 0.8, 'Potager', 3, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po4C', 'Extérieur', 8.0, 0.8, 'Potager', 3, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po4D', 'Extérieur', 8.0, 0.8, 'Potager', 3, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po5A', 'Extérieur', 8.0, 0.8, 'Potager', 2, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po5B', 'Extérieur', 8.0, 0.8, 'Potager', 2, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po5C', 'Extérieur', 8.0, 0.8, 'Potager', 2, NULL);
INSERT INTO Planches (Planche, Type, Longueur, Largeur, Rotation, Année, Notes) VALUES ('Po5D', 'Extérieur', 8.0, 0.8, 'Potager', 2, NULL);

INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Ail plant. automne', 'Ail', 'Serre', NULL, NULL, '10-15', '12-15', '06-15', '08-01', 3.0, 15.0, NULL, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Ail plant. printemps', 'Ail', 'Extérieur', NULL, NULL, '02-15', '04-01', '07-01', '08-15', 3.0, 15.0, NULL, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Aubergine', 'Aubergine', NULL, '02-15', '03-15', '05-01', '06-15', '07-15', '10-01', 1.0, 60.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Betterave plant. automne', 'Betterave', 'Serre', '08-15', '11-01', '09-15', '12-01', '01-15', '04-15', 4.0, 10.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Betterave plant. printemps', 'Betterave', 'Extérieur', '03-01', '06-01', '04-01', '08-01', '05-15', '12-01', 4.0, 10.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Blette', 'Blette', 'Extérieur', '06-01', '07-01', '07-01', '10-01', '08-15', '04-15', 1.0, 70.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Carotte pleine terre', 'Carotte', 'Extérieur', '04-01', '07-01', NULL, NULL, '07-01', '11-01', 4.0, 5.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Carotte pleine terre conservation', 'Carotte', 'Extérieur', '05-15', '07-01', NULL, NULL, '10-01', '03-15', 4.0, 5.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Carotte serre', 'Carotte', 'Serre', '02-01', '04-01', NULL, NULL, '05-01', '07-01', 4.0, 5.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Carotte serre conservation', 'Carotte', 'Serre', '10-01', '12-01', NULL, NULL, '04-01', '06-01', 4.0, 5.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Chou brocoli', 'Chou brocoli', 'Extérieur', '03-01', '08-01', '04-15', '08-15', '06-01', '12-01', 2.0, 60.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Chou de Bruxelles', 'Chou de Bruxelles', 'Extérieur', '03-15', '05-01', '05-15', '06-15', '09-01', '04-01', 1.0, 60.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Chou de Chine', 'Chou de Chine', 'Extérieur', '06-01', '08-15', NULL, NULL, '09-15', '12-01', 1.0, 60.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Chou frisé', 'Chou frisé', 'Extérieur', '04-01', '05-01', '06-01', '07-01', '10-01', '02-01', 2.0, 50.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Chou pommé plant. automne', 'Chou pommé', NULL, '08-15', '09-15', '10-15', '11-15', '04-01', '07-01', 2.0, 50.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Chou pommé plant. printemps', 'Chou pommé', NULL, '02-15', '04-01', '04-15', '05-01', '07-01', '09-01', 1.0, 50.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Chou pommé plant. été', 'Chou pommé', NULL, '05-15', '06-15', '07-15', '08-15', '12-01', '04-01', 2.0, 67.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Chou-fleur plant. printemps', 'Chou-fleur', 'Extérieur', '02-01', '05-01', '04-01', '07-01', '07-01', '10-01', 1.0, 70.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Chou-fleur plant. été', 'Chou-fleur', 'Extérieur', '04-15', '07-01', '07-15', '09-01', '03-01', '07-01', 1.0, 70.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Chou-rave plant. printemps', 'Chou-rave', 'Extérieur', '03-01', '07-15', '04-15', '08-15', '07-01', '11-01', 3.0, 20.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Concombre', 'Concombre', 'Serre', '03-01', '05-01', '04-01', '06-01', '06-01', '10-15', 1.0, 50.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Courge', 'Courge', 'Extérieur', '03-01', '04-15', '03-15', '05-01', '09-01', '11-15', 1.0, 100.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Courgette', 'Courgette', 'Extérieur', '05-15', '07-15', '06-15', '08-01', '07-01', '11-01', 1.0, 100.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Courgette sous serre', 'Courgette', 'Serre', '03-01', '04-15', '03-15', '05-01', '05-15', '09-01', 1.0, 100.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Céleri', 'Céleri', 'Extérieur', '03-01', '05-01', '05-15', '07-01', '08-15', '01-01', 2.0, 50.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('EV LD automne commerce', 'Mélange', 'Extérieur', '09-01', '11-01', NULL, NULL, NULL, NULL, NULL, NULL, NULL, 3.33, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('EV LD printemps Sorgho Trèfle Mélilot Sarrasin', 'Mélange', 'Extérieur', '04-15', '06-01', NULL, NULL, NULL, NULL, NULL, NULL, NULL, 2.9, 'Sorgo 1 g/m²

Trèfle 0.375 g/m²

Mélilot 0.5 g/m²

Sarrasin 1.25 g/m²');
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('EV Ser automne Seigle Vesce Moutarde Phacélie', 'Mélange', 'Serre', '09-15', '11-15', NULL, NULL, NULL, '02-15', NULL, NULL, NULL, 13.0, 'Seigle 8 g/m²

Vesce 4 g/m²

Moutarde 0,5 g/m²

Phacélie 0,5 g/m²');
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('EV Ser printemps Sorgho Trèfle Mélilot Sarrasin', 'Mélange', 'Serre', '03-15', '05-01', NULL, NULL, NULL, '06-01', NULL, NULL, NULL, 2.9, 'Sorgo 1 g/m²

Trèfle 0.375 g/m²

Mélilot 0.5 g/m²

Sarrasin 1.25 g/m²');
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('EV automne Seigle Vesce Moutarde Phacélie', 'Mélange', 'Extérieur', '08-15', '10-15', NULL, NULL, NULL, '03-15', NULL, NULL, NULL, 13.0, 'Seigle 8 g/m²

Vesce 4 g/m²

Moutarde 0,5 g/m²

Phacélie 0,5 g/m²');
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('EV printemps Sorgho Trèfle Mélilot Sarrasin', 'Mélange', 'Extérieur', '04-15', '06-01', NULL, NULL, NULL, '07-01', NULL, NULL, NULL, 2.9, 'Sorgo 1 g/m²

Trèfle 0.375 g/m²

Mélilot 0.5 g/m²

Sarrasin 1.25 g/m²');
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Epinard d''automne', 'Epinard', 'Extérieur', '08-15', '10-15', NULL, NULL, '10-15', '12-15', 3.0, 2.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Epinard d''été', 'Epinard', 'Extérieur', '04-01', '07-01', NULL, NULL, '06-01', '10-01', 3.0, 2.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Epinard de printemps', 'Epinard', 'Extérieur', '02-15', '04-15', NULL, NULL, '04-15', '06-15', 3.0, 2.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Fenouil', 'Fenouil', 'Extérieur', '07-15', '09-01', NULL, NULL, '11-01', '01-01', 2.0, 2.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Fève', 'Fève', 'Extérieur', '09-15', '11-15', NULL, NULL, '04-01', '07-01', 2.0, 15.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Haricot Ext', 'Haricot mangetout', 'Extérieur', '04-15', '08-15', NULL, NULL, '06-15', '11-01', 2.0, 30.0, 4.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Haricot Ser', 'Haricot mangetout', 'Serre', '03-01', '05-01', '04-01', '06-01', '06-01', '11-01', 2.0, 40.0, 4.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Haricot Tarbais (maïs)', 'Haricot à écosser', 'Extérieur', '05-01', '06-01', NULL, NULL, '10-15', '11-01', 2.0, 25.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Laitue plant. automne ext', 'Laitue', 'Extérieur', '08-15', '09-15', '09-01', '10-01', '04-01', '06-15', 3.0, 30.0, 2.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Laitue plant. hiver sous serre', 'Laitue', 'Serre', '10-01', '01-01', '11-01', '02-01', '02-01', '04-15', 3.0, 30.0, 2.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Laitue plant. printemps', 'Laitue', 'Extérieur', '04-01', '06-01', '04-15', '06-15', '06-15', '09-01', 3.0, 30.0, 2.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Laitue plant. printemps précoce', 'Laitue', 'Extérieur', '02-01', '04-15', '03-01', '05-01', '05-15', '06-15', 3.0, 30.0, 2.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Laitue plant. été', 'Laitue', 'Extérieur', '07-01', '08-01', '08-01', '08-15', '09-01', '11-01', 3.0, 30.0, 2.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Maïs', 'Maïs', 'Extérieur', '04-15', '05-15', NULL, NULL, '09-01', '10-15', 2.0, 25.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Melon', 'Melon', 'Extérieur', '04-01', '06-01', '05-15', '07-01', '08-01', '10-15', 1.0, 70.0, 2.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Mâche d''automne', 'Mâche', 'Extérieur', '09-01', '12-01', NULL, NULL, '12-01', '04-01', 5.0, 5.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Mâche d''hiver', 'Mâche', 'Extérieur', '12-01', '03-01', '02-01', '04-01', '04-01', '06-01', 5.0, 5.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Mâche d''été', 'Mâche', 'Extérieur', '06-01', '09-01', NULL, NULL, '09-01', '01-01', 5.0, 5.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Mâche de printemps', 'Mâche', 'Extérieur', '03-01', '06-01', NULL, NULL, '06-01', '09-01', 5.0, 5.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Navet précoce semis printemps', 'Navet', 'Extérieur', '03-15', '05-01', NULL, NULL, '05-15', '08-01', 4.0, 3.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Navet semis automne', 'Navet', 'Extérieur', '07-15', '09-01', NULL, NULL, '09-15', '12-01', 4.0, 3.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Navet semis printemps', 'Navet', 'Extérieur', '05-01', '07-15', NULL, NULL, '08-01', '10-01', 4.0, 3.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Oignon Bulbilles printemps', 'Oignon', 'Extérieur', NULL, NULL, '02-01', '04-15', '07-01', '09-01', 4.0, 10.0, NULL, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Oignon automne sous serre', 'Oignon', 'Serre', '09-01', '09-15', '11-01', '11-15', '04-01', '05-15', 2.0, 10.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Oignon hiver sous serre', 'Oignon', 'Serre', '12-01', '12-15', '01-01', '02-01', '03-01', '04-15', '07-01', 10.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Oignon plant. printemps', 'Oignon', 'Extérieur', '02-15', '03-15', '05-15', '06-15', '08-15', '09-15', 4.0, 15.0, 3.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Panais', 'Panais', 'Extérieur', '04-01', '06-15', NULL, NULL, '08-01', '12-15', 4.0, 15.0, 2.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Patate douce par bouturage', 'Patate douce', 'Extérieur', '11-01', '12-01', '05-15', '06-15', '10-15', '11-15', 1.0, 40.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Persil', 'Persil', 'Extérieur', '03-01', '07-01', NULL, NULL, '05-01', '11-01', 3.0, 2.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Physalis', 'Physalis', 'Extérieur', '03-01', '04-01', '05-01', '06-15', '10-01', '12-01', 1.0, 90.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Poireau plant. d''automne', 'Poireau', 'Extérieur', '08-01', '09-01', '09-15', '10-01', '04-15', '06-01', 4.0, 10.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Poireau plant. printemps', 'Poireau', 'Extérieur', '03-15', '05-01', '06-01', '07-15', '10-01', '01-01', 4.0, 10.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Poireau précoce', 'Poireau', 'Extérieur', '02-15', '03-15', '05-01', '06-01', '07-15', '11-01', 4.0, 10.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Poivron piment ext', 'Poivron piment', 'Extérieur', '03-01', '04-15', '05-15', '06-15', '08-15', '11-01', 2.0, 55.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Poivron piment serre', 'Poivron piment', 'Serre', '02-15', '04-01', '05-01', '06-01', '07-15', '11-01', 2.0, 55.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Pomme de terre conservation', 'Pomme de terre', 'Extérieur', NULL, NULL, '02-01', '04-15', '05-01', '07-01', 1.0, 40.0, NULL, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Radis d''hiver', 'Radis noir', 'Extérieur', '08-01', '10-15', NULL, NULL, '11-01', '03-15', 2.0, 10.0, 2.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Radis d''été', 'Radis', 'Extérieur', '05-01', '09-01', NULL, NULL, '06-15', '11-15', 4.0, 4.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Ricin', 'Ricin', NULL, '03-01', '05-15', '04-01', '05-01', NULL, '11-15', 1.0, 300.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Roquette', 'Roquette', 'Extérieur', '03-01', '10-01', NULL, NULL, '05-01', '12-01', 5.0, 5.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Tomate hative ext', 'Tomate', 'Extérieur', '03-01', '04-01', '05-01', '06-01', '07-15', '10-15', 2.0, 70.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Tomate hative serre', 'Tomate', 'Serre', '02-15', '03-15', '04-15', '05-15', '06-15', '10-15', 2.0, 70.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Tomate tardive', 'Tomate', NULL, '04-01', '05-01', '06-01', '07-15', '09-01', '11-15', 2.0, 75.0, 1.0, NULL, NULL);
INSERT INTO ITP (IT_plante, Espèce, Type_planche, Déb_semis, Fin_semis, Déb_plantation, Fin_plantation, Déb_récolte, Fin_récolte, Nb_rangs, Espacement, Nb_graines_trou, Dose_semis, Notes) VALUES ('Topinambour', 'Topinambour', 'Extérieur', NULL, NULL, '01-01', '04-01', '11-01', '04-01', 2.0, 60.0, NULL, NULL, NULL);

INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Ail Cledor', 'Ail', NULL, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Ail Rose de Lautrec', 'Ail', NULL, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Aubergine Barbentane', 'Aubergine', 250.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Aubergine Uriase', 'Aubergine', 250.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Aubergine japonaise de la levée', 'Aubergine', 250.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Batavia reine des glaces', 'Laitue', 800.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Betterave 3 root grex', 'Betterave', 50.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Betterave Chioggia', 'Betterave', 50.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Betterave Detroit', 'Betterave', 50.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Betterave Robushka', 'Betterave', 50.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Carotte Colmar à Coeur rouge 2', 'Carotte', 925.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Carotte Demi longue rouge sang', 'Carotte', 925.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Carotte Ronde Marché de Paris', 'Carotte', 925.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Carotte Touchon', 'Carotte', 925.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chicorée scarole cornet d''anjou', 'Laitue', 800.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chicorée scarole géante maraîchère', 'Laitue', 800.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou brocoli Rasmus', 'Chou brocoli', 250.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou de Bruxelles Groeninger', 'Chou de Bruxelles', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou de Bruxelles de Rosny', 'Chou de Bruxelles', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou de Chine Bilko', 'Chou de Chine', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou de Chine Petsaï', 'Chou de Chine', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou frisé Kale', 'Chou frisé', 300.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou frisé winster', 'Chou frisé', 300.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou pommé Amager Lav', 'Chou pommé', 300.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou pommé Pointu de Chateaurenard', 'Chou pommé', 300.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou pommé Thurnen', 'Chou pommé', 300.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou pommé Tête Noire', 'Chou pommé', 300.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou-fleur Neckarperle', 'Chou-fleur', 336.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Chou-rave Azur Star', 'Chou-rave', 300.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Concombre Marketmore', 'Concombre', 35.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Concombre Palestinien', 'Concombre', 35.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Courge Butternut', 'Courge', 4.58, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Courge Potimarron Red Kuri', 'Courge', 4.58, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Courgette Black Beauty', 'Courgette', 7.7, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Courgette Gold Rush', 'Courgette', 7.7, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Céleri Elne', 'Céleri', 2500.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Céleri Plein blanc Pascal', 'Céleri', 2500.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Céleri branche', 'Céleri', 2500.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Epinard Géant d hiver Verdil', 'Epinard', 110.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Epinard Matador', 'Epinard', 110.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Epinard Viking', 'Epinard', 110.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Fève Aguadulce', 'Fève', 0.75, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Fève de Séville', 'Fève', 0.75, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Hannibal été automne', 'Poireau', 300.3, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Haricot Black Turtle', 'Haricot à écosser', 5.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Haricot Flageolet rouge', 'Haricot à écosser', 5.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Haricot Morgane', 'Haricot mangetout', 5.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Haricot Tarbais (maïs)', 'Haricot à écosser', 5.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Haricot Émerite', 'Haricot mangetout', 5.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Laitue Crisp Mint', 'Laitue', 800.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Laitue Feuille de chêne rouge', 'Laitue', 800.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Laitue Great Lakes', 'Laitue', 800.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Laitue Grenobloise', 'Laitue', 1109.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Laitue Pasquier', 'Laitue', 800.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Laitue Pommée d''hiver Triolaise', 'Laitue', 800.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Laitue Romaine d''hiver rouge', 'Laitue', 800.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Laitue pommée du bon jardinier', 'Laitue', 800.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Laitue à couper feuille de chêne rouge', 'Laitue', 800.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Maïs Grand Roux', 'Maïs', 4.5, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Melon Cantaloup Charentais', 'Melon', 35.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Moutarde', 'Moutarde', 130.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Mâche verte', 'Mâche', 660.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Mâche verte à coeur plein', 'Mâche', 660.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Mélilot', 'Mélilot', 120.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Navet Globe à col violet', 'Navet', 600.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Navet Jaune Petrowski', 'Navet', 600.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Oignon Bulbilles Red Baron', 'Oignon', 227.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Oignon Bulbilles Sturon', 'Oignon', 264.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Oignon Jaune de Stuttgart', 'Oignon', 264.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Oignon Mazères', 'Oignon', 264.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Oignon Rouge de Brunswick', 'Oignon', 264.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Oignon Saint Turjan', 'Oignon', 264.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Panais Etroit amélioré', 'Panais', 220.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Panais Turga Pré germé', 'Panais', 30.8, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Panais long holkruin', 'Panais', 220.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Patate douce', 'Patate douce', NULL, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Persil Géant d''Italie', 'Persil', 570.5, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Persil à feuilles frisées vert foncé', 'Persil', 600.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Phacélie population', 'Phacélie', 550.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Physalis Alkékenge jaune doux', 'Physalis', 300.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Piment Doux des landes', 'Poivron piment', 150.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Poireau Gros long d''été', 'Poireau', 300.3, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Poireau Monstrueux de Carentan', 'Poireau', 300.3, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Poireau de Liège', 'Poireau', 300.3, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Poivron Jaune carré d''Asti', 'Poivron piment', 150.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Poivron Wisconsin Lake', 'Poivron piment', 150.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Poivron Yolo wonder', 'Poivron piment', 150.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Pomme de terre Artémis', 'Pomme de terre', NULL, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Pomme de terre Ditta', 'Pomme de terre', NULL, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Pomme de terre Désirée', 'Pomme de terre', NULL, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Radis Rond écarlate', 'Radis', 120.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Radis-rave Daikon', 'Radis', 120.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Radis-rave Rond noir', 'Radis', 120.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Ricin New Zealand Purple', 'Ricin', 1.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Roquette', 'Roquette', 660.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Sorgo Koral', 'Sorgho', 27.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate Ananas', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate Andines cornues', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate Cerise petit moineau', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate Homestead', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate Merveille des marchés', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate Roma', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate Rose de Berne', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate Saint Pierre', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate San Marzano', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate Udagorri', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate Yellow Feather', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate cerise rouge', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tomate coeur de boeuf', 'Tomate', 350.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Topinambour Rose', 'Topinambour', NULL, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Tournesol', 'Tournesol', 11.36, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Trèfle violet', 'Trèfle violet', 40.0, NULL, NULL, NULL, NULL);
INSERT INTO Variétés (Variété, Espèce, Nb_graines_g, Qté_stock, Qté_cde, Fournisseur, Notes) VALUES ('Vesce hiver', 'Vesce', 45.0, NULL, NULL, NULL, NULL);

INSERT INTO Rotations (Rotation, Type_planche, Année_1, Nb_années, Notes) VALUES ('Grande culture', 'Extérieur', 2025, 4, NULL);
INSERT INTO Rotations (Rotation, Type_planche, Année_1, Nb_années, Notes) VALUES ('Potager', 'Extérieur', 2025, 5, NULL);

INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (1, 'Potager', 1, 'Haricot SD-Ext', 100.0, NULL, NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (2, 'Potager', 1, 'EV automne Seigle Vesce Moutarde Phacélie', 100.0, NULL, NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (3, 'Potager', 5, 'Blette', 100.0, 'C', NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (5, 'Potager', 5, 'Laitue plant. printemps', 100.0, 'AB', NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (6, 'Potager', 5, 'Laitue plant. été', 100.0, 'D', NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (7, 'Potager', 5, 'Laitue plant. automne ext', 100.0, 'AB', NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (8, 'Potager', 3, 'Laitue plant. printemps précoce', 100.0, NULL, NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (9, 'Potager', 2, 'Betterave plant. printemps', 100.0, NULL, NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (10, 'Potager', 3, 'Poireau plant. printemps', 100.0, NULL, NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (12, 'Potager', 4, 'Chou pommé plant. printemps', 100.0, NULL, NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (14, 'Grande culture', 2, 'Pomme de terre conservation', 100.0, NULL, NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (15, 'Grande culture', 1, 'Maïs', 100.0, 'BD', NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (16, 'Grande culture', 1, 'Courge', 100.0, 'AC', NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (17, 'Grande culture', 1, 'Haricot Tarbais (maïs)', 100.0, 'BD', NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (18, 'Grande culture', 3, 'EV LD printemps Sorgho Trèfle Mélilot Sarrasin', 100.0, NULL, NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (19, 'Grande culture', 4, 'EV LD printemps Sorgho Trèfle Mélilot Sarrasin', 100.0, NULL, NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (81, 'Grande culture', 2, 'Ricin', 100.0, NULL, NULL);
INSERT INTO Rotations_détails (ID, Rotation, Année, IT_plante, Pc_planches, Fi_planches, Notes) VALUES (83, 'Potager', 1, 'Ricin', 100.0, NULL, NULL);

-- COMMIT TRANSACTION;

)#");
