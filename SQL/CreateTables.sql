CREATE TABLE Analyses_de_sol ( --- Analyses de sol effectuées.
                               --- #Analyse
    ---row_summary Analyse,Date,Planche,Organisme
    Analyse TEXT PRIMARY KEY, ---
    Planche TEXT REFERENCES Planches (Planche) ON UPDATE CASCADE, ---
    Date DATE NOT NULL, ---
    Sable_grossier REAL, --- Pourcentage de sable grossier (0,2 - 2 mm).
        ---unit %
    Sable_fin REAL, --- Pourcentage de sable fin (0,05 - 0,2 mm).
        ---unit %
    Limon_grossier REAL, --- Pourcentage de limon grossier (0,02 - 0.05 mm)").
                         --- Favorise la battance.
        ---unit %
    Limon_fin REAL, --- Pourcentage de limon fin (0,002 - 0,02 mm).
                    --- Favorise la battance.
        ---unit %
    Argile REAL, --- Pourcentage d'argile (0 - 0,002 mm).
        ---unit %
    pH REAL, --- Potentiel hydrogène, à l'eau, du prélèvement.
             --- Valeurs de référence: 6,5 < pH < 7,5
        ---cond_formats (::>=8)||(::<=6) #darkRed# #lightRed#
    MO REAL, --- Teneur en matière organique du prélèvement.
             --- Valeurs de référence: 15 < MO < 20
        ---unit g/kg
        ---cond_formats ::<=15 #darkRed# #lightRed#,::>=20 #darkGreen# #lightGreen#
    IB REAL, --- Indice de battance.
             --- Risque de désagrégation du sol sous l'action de la pluie et de formation d'une croûte superficielle lors du ressuyage.
             --- Valeurs de référence: IB < 1,4
        ---cond_formats ::>=1.4 #darkRed# #lightRed#,::<=0.7 #darkGreen# #lightGreen#
    CEC REAL, --- Capacité d’Echange Cationique du prélèvement.
              --- Mesure le pouvoir fixateur de cations du sol.
              --- Valeurs de référence: 10 < CEC < 20
        ---unit cmol+/kg
        ---cond_formats ::<10 #darkRed# #lightRed#,::>15 #darkGreen# #lightGreen#
    Cations REAL, --- Teneur en cations du prélèvement.
                  --- Valeurs de référence: Cations >= CEC
        ---unit cmol+/kg
    N REAL, --- Teneur en azote (Kjeldhal) du prélèvement.
            --- Valeurs de référence: 0,9 < N < 1,1
        ---unit g/kg
        ---cond_formats ::<=0.9 #darkRed# #lightRed#,::>=1.1 #darkGreen# #lightGreen#
    ☆N TEXT AS (CASE WHEN N >= 1.1 THEN 'Elevé' WHEN N <= 0.9 THEN 'Faible' WHEN N NOTNULL THEN 'Moyen' END), --- Teneur qualitative du sol.
        ---cond_formats "::"=="Faible" #darkRed# #lightRed#,"::"=="Elevé" #darkGreen# #lightGreen#
    P REAL, --- Teneur en phosphore du prélèvement.
            --- Valeurs de référence: 0,08 < P2O5 < 0,12
        ---unit g/kg
        ---cond_formats ::<=0.08 #darkRed# #lightRed#,::>=0.12 #darkGreen# #lightGreen#
    ☆P TEXT AS (CASE WHEN P >= 0.12 THEN 'Elevé' WHEN P <= 0.08 THEN 'Faible' WHEN P NOTNULL THEN 'Moyen' END), --- Teneur qualitative du sol.
        ---cond_formats "::"=="Faible" #darkRed# #lightRed#,"::"=="Elevé" #darkGreen# #lightGreen#
    K REAL, --- Potassium.
            --- Teneur en potasse du prélèvement.
            --- Valeurs de référence: 0,12 < K2O < 0,15
        ---unit g/kg
        ---cond_formats ::<=0.12 #darkRed# #lightRed#,::>=0.15 #darkGreen# #lightGreen#
    ☆K TEXT AS (CASE WHEN K >= 0.15 THEN 'Elevé' WHEN K <= 0.12 THEN 'Faible' WHEN K NOTNULL THEN 'Moyen' END), --- Teneur qualitative du sol.
        ---cond_formats "::"=="Faible" #darkRed# #lightRed#,"::"=="Elevé" #darkGreen# #lightGreen#
    C REAL, --- Teneur en carbone du prélèvement.
            --- Valeurs de référence: 9 < C < 11
        ---unit g/kg
        ---cond_formats ::<=9 #darkRed# #lightRed#,::>=11 #darkGreen# #lightGreen#
    CN REAL, --- Rapport Carbone/Azote.
             --- Valeurs de référence: 8 < C/N < 11
        ---cond_formats (::<=8)||(::>=11) #darkRed# #lightRed#
    Ca REAL, --- Calcium
             --- Teneur en chaux du prélèvement.
             --- Valeurs de référence: 3,7 < CaO < 3,9
        ---unit g/kg
        ---cond_formats ::<=3.7 #darkRed# #lightRed#,::>=3.9 #darkGreen# #lightGreen#
    Mg REAL, --- Magnésium
             --- Teneur en magnésie du prélèvement.
             --- Valeurs de référence: 0,09 < MgO < 0,14
        ---unit g/kg
        ---cond_formats ::<=0.09 #darkRed# #lightRed#,::>=0.14 #darkGreen# #lightGreen#
    Na REAL, --- Sodium
             --- Teneur en oxyde de sodium du prélèvement.
             --- Valeurs de référence: 0.02 < Na2O < 0,24
        ---unit g/kg
        ---cond_formats ::>=0.24 #darkRed# #lightRed#,::<=0.02 #darkGreen# #lightGreen#
    Interprétation TEXT, ---multiline
    Référence TEXT, ---
    Organisme TEXT, ---
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    WITHOUT ROWID;

CREATE TABLE Associations_détails ( --- Associations d'espèces ou de familles de plante.
                                    --- Permettent de détecter les associations entre :
                                    --- - annuelles des plans de rotation
                                    --- - annuelles planifiées la prochaine saison et vivaces en place *
                                    --- - annuelles et vivaces en place *
                                    --- *: en fonction de l'influence des planches entre elles (onglet 'Planches').
    ---can_open_tab SELECT (count(*)>2) FROM Espèces
    IdxAsReEsGrFa TEXT PRIMARY KEY, --- Clé calculée: Association||iif(Requise NOTNULL,'0'||Requise,'1 ')||'-'||coalesce(Espèce,Groupe,Famille)
    Association TEXT NOT NULL, --- Nom de l'association.
                               --- Terminer par ' +' pour signifier que l'association est favorable.
                               --- Terminer par ' !' pour signifier que l'association est défavorable.
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE, ---
    Groupe TEXT, --- Espèces dont le nom commence par.
    Famille TEXT REFERENCES Familles (Famille) ON UPDATE CASCADE, ---
        ---fk_filter (coalesce(:Espèce:,'x')='x')OR(Famille=(SELECT Famille FROM Espèces WHERE Espèce=:Espèce:))
    Requise BOOL, --- L'espèce doit obligatoirement être présente pour que l'association fasse son effet.
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    ;
UPDATE fada_f_schema SET base_data='x' WHERE name='Associations_détails';

CREATE TABLE Consommations ( --- Quantités de légume sorties du stock.
    ---can_open_tab SELECT (count(*)>0) FROM Espèces WHERE Conservation NOTNULL
    ID INTEGER PRIMARY KEY AUTOINCREMENT, ---
    Date DATE NOT NULL, --- Date de sortie de stock.
                        --- Laisser vide pour avoir automatiquement la date du jour.
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL, ---
        ---fk_filter Conservation NOTNULL
    Quantité REAL NOT NULL, --- Quantité sortie de stock.
        ---unit kg
    Prix REAL, --- Prix total pour cette consommassion.
        ---money
    Destination TEXT REFERENCES Destinations (Destination) ON UPDATE CASCADE, --- -- ON DELETE SET NULL
        ---fk_filter Active NOTNULL
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    ;

CREATE TABLE Cultures ( --- Une 'culture' c'est une plante (espèce,variété) sur une planche avec un itinéraire technique.
                        --- Si la même plante est présente sur plusieurs planches, il y a une culture (numérotée) par planche.
    ---no_data_text Saisissez au moins une espèce avant de saisir des cultures.
    ---can_open_tab SELECT (count(*)>0) FROM Espèces
    ---row_summary Culture,Planche,It_plante,Variété,Type,Etat,Date_semis|semis: :: - ,Date_plantation|plantation: :: -
    Culture INTEGER PRIMARY KEY AUTOINCREMENT, --- Numéro unique de la culture (pas de remise à zéro tous les ans).
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL, ---
    IT_plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE, --- Itinéraire technique: une espèce de plante et une manière de la cultiver.
                                                                 --- Sur une variété, vous pouvez sélectionner parmis les ITP de l'espèce concernée ou parmis les ITP qui n'ont pas d'espèce.
                                                                 --- Dans un plan de rotation, vous pouvez sélectionner parmis les ITP qui ont une espèce (annuelle ou vivace).
                                                                 --- Pour une culture, vous pouvez sélectionner parmis les ITP de l'espèce cultivée ou parmis les ITP qui n'ont pas d'espèce.
        ---fk_filter (Espèce ISNULL)OR(Espèce=:Espèce:)
    Variété TEXT REFERENCES Variétés (Variété) ON UPDATE CASCADE, ---
        ---fk_filter Espèce=:Espèce:
    Fournisseur TEXT REFERENCES Fournisseurs (Fournisseur)  ON UPDATE CASCADE, ---
    Planche TEXT REFERENCES Planches (Planche) ON UPDATE CASCADE, ---
    Type TEXT AS --- Automatique, en fonction des date de semis, plantation et récolte.
        (CASE WHEN (Date_plantation < Date_semis) OR (Début_récolte < Date_semis) OR (Fin_récolte < Date_semis) OR (Début_récolte < Date_plantation) OR (Fin_récolte < Date_plantation) OR (Fin_récolte < Début_récolte) THEN 'Erreur dates !'
              WHEN Terminée LIKE 'v%' THEN 'Vivace'
              WHEN Date_semis NOTNULL AND Date_plantation NOTNULL AND Début_récolte NOTNULL AND Fin_récolte NOTNULL THEN 'Semis pépinière'
              WHEN Date_semis ISNULL  AND Date_plantation NOTNULL AND Début_récolte NOTNULL AND Fin_récolte NOTNULL THEN 'Plant'
              WHEN Date_semis NOTNULL AND Date_plantation ISNULL  AND Début_récolte NOTNULL AND Fin_récolte NOTNULL THEN 'Semis en place'
              WHEN Date_semis NOTNULL AND Date_plantation NOTNULL AND Début_récolte ISNULL  AND Fin_récolte NOTNULL THEN 'Compagne'
              WHEN Date_semis NOTNULL AND Date_plantation ISNULL  AND Début_récolte ISNULL THEN 'Engrais vert'
              ELSE '?' END),
    Saison TEXT AS (CASE --- Annuelle: année de mise en place sur la planche (date de plantation ou de semis en place)
                         --- Vivace: année de début récolte, à défaut année de mise en place.
                         WHEN coalesce(Terminée,'') NOT LIKE 'v%' -- Anuelle
                         THEN substr(coalesce(Date_plantation,Date_semis,Début_récolte,Fin_récolte),1,4)
                         ELSE substr(coalesce(Début_récolte,Date_plantation,Date_semis,Fin_récolte),1,4) -- Vivace
                         END),
    Etat TEXT AS (CASE --- Automatique, en fonction des semis, plantation et récolte faites ou pas.
                       --- Détermine la couleur de la ligne dans les tableaux de données.
                       WHEN coalesce(Terminée,'') NOT LIKE 'v%'  -- Anuelle
                       THEN (CASE WHEN Terminée NOTNULL THEN 'Terminée' --gris
                                  WHEN Récolte_faite LIKE 'x%' THEN 'A terminer' --bleu
                                  WHEN Récolte_faite NOTNULL THEN 'Récolte' --violet
                                  WHEN Plantation_faite NOTNULL THEN 'En place' --vert
                                  WHEN Semis_fait NOTNULL THEN iif(Date_plantation IS NULL,'En place','Pépinière') --vert rouge
                                  ELSE 'Prévue'
                                  END)
                        -- Vivace
                        ELSE (CASE WHEN (Terminée != 'v')AND(Terminée != 'V') THEN 'Terminée' --gris
                                   WHEN Récolte_faite LIKE 'x%' THEN 'En place' --vert
                                   WHEN Récolte_faite NOTNULL THEN 'Récolte' --violet
                                   WHEN Plantation_faite NOTNULL THEN 'En place' --vert
                                   WHEN Semis_fait NOTNULL THEN iif(Date_plantation IS NULL,'En place','Pépinière') --vert rouge
                                   ELSE 'Prévue'
                                   END)
                        END),
    color TEXT AS (CASE --- Identique à Etat.
                       WHEN coalesce(Terminée,'') NOT LIKE 'v%'  -- Anuelle
                       THEN (CASE WHEN Terminée NOTNULL THEN '#808080' --Terminée
                                  WHEN Récolte_faite LIKE 'x%' THEN '#007aff' --A terminer
                                  WHEN Récolte_faite NOTNULL THEN '#bf00ff' --Récolte
                                  WHEN Plantation_faite NOTNULL THEN '#76c801' --En place
                                  WHEN Semis_fait NOTNULL THEN iif(Date_plantation IS NULL,'#76c801','#ff6000') --En place ou Pépinière
                                  ELSE NULL
                                  END)
                        -- Vivace
                        ELSE (CASE WHEN (Terminée != 'v')AND(Terminée != 'V') THEN '#808080' --Terminée
                                   WHEN Récolte_faite LIKE 'x%' THEN 'En place' --vert
                                   WHEN Récolte_faite NOTNULL THEN '#bf00ff' --Récolte
                                   WHEN Plantation_faite NOTNULL THEN '#76c801' --En place
                                   WHEN Semis_fait NOTNULL THEN iif(Date_plantation IS NULL,'#76c801','#ff6000') --En place ou Pépinière
                                   ELSE NULL
                                   END)
                        END),

    D_planif TEXT, --- Date de calcul des dates de semis, plantation et récolte (planification). -- Format TEXT pour pouvoir mettre une année simple quand on veut forcer un recalcul de planif.
                   --- La planification des cultures est faite en fonction de l'ITP et éventuellement de la variété pour les dates de récolte.
                   --- Les dates sont calées sur les débuts de période de l'ITP/variété.
                   ---
                   --- Effacer 'D_planif' et valider pour recalculer les dates non renseignées sur la culture.
                   --- Pour planifier la culture sur une saison particulière, saisissez dans 'D_planif' l'année sur 4 chiffres (ex 2025).
    Date_semis DATE, --- Date réelle ou prévue.
                     --- Laisser la date prévue même si l'opération ne sera pas faite, pour que le type de culture soit correctement calculé.
                     --- Vide pour plant acheté.
    Semis_fait BOOL, --- 'x' ou commence par 'x' : le semis est réussi.
                     --- Autre valeur : le semis est en cours.
                     --- Vide : le semis est à faire.
    Date_plantation DATE, --- Date réelle ou prévue.
                          --- Laisser la date prévue même si l'opération ne sera pas faite, pour que le type de culture soit correctement calculé.
                          --- Vide pour semis en place et engrais vert.
    Plantation_faite BOOL, --- 'x' ou commence par 'x' : la plantation est réussie.
                           --- Autre valeur : la plantation est en cours.
                           --- Vide : la plantation est à faire.
    Début_récolte DATE, --- Date réelle ou prévue.
                        --- Laisser la date prévue même si l'opération ne sera pas faite, pour que le type de culture soit correctement calculé.
                        --- Va être mis à jour lors de la saisie des récoltes.
                        --- Vide pour compagne et engrais vert.
    Fin_récolte DATE, --- Date réelle ou prévue.
                      --- Laisser la date prévue même si l'opération ne sera pas faite, pour que le type de culture soit correctement calculé.
                      --- Va être mis à jour lors de la saisie des récoltes.
                      --- Date de destruction compagne et engrais vert.
    Récolte_faite BOOL, --- 'x' ou commence par 'x' : la récolte est terminée.
                        --- Autre valeur : la récolte est en cours.
                        --- Vide : la récolte n'est pas commencée.
                        ---
                        --- Va être mis à jour lors de la saisie des récoltes ou lors du passage de la culture à terminée.
                        --- Saisir '?' pour forcer le calcul de dates de début et fin de récolte.
    Terminée BOOL, --- 'v' : à récolter aussi les années suivantes (vivace).
                   --- Autre valeur : la planche est fermée et disponible pour la culture suivante.
                   ---
                   --- Commence par 'v' : culture de vivace terminée.
                   --- Le 'v' est automatiquement ajouté lors de la saisie d'une récolte si l'espèce est 'Vivace'.
                   ---
                   --- Ajoutez 'NS' (non significative) à la fin si la culture ne doit pas être prise en compte dans les analyses.
    Longueur REAL, --- Longueur de la culture sur la planche.
                   --- Utilisé pour calculer le poids de semence nécessaire.
        ---unit m
    Nb_rangs REAL, --- Nombre de rangs cultivés sur la planche.
                   --- Utilisé pour calculer le poids de semence nécessaire.
        ---unit rangs
    Espacement INTEGER, --- Espacement entre 2 plants dans un rang de culture, après éclaircicement éventuel.
                        --- Utilisé pour calculer le poids de semence nécessaire.
        ---unit cm
    A_faire TEXT, ---multiline
    Notes TEXT) --- ::Familles.Notes
        ---multiline
;

CREATE TABLE Destinations ( --- Destinations des légumes sortis du stock.
    ---row_summary Destination,Type,Date_RAZ
    Destination TEXT PRIMARY KEY, ---
    Type TEXT, --- Liste de choix paramétrable (paramétre 'Combo_Destinations_Type').
        ---combo Combo_Destinations_Type
    Adresse TEXT, ---multiline
    Site_web TEXT, ---
    Date_RAZ DATE, --- Date à partir de laquelle sont prises en compte les sorties de stock.
    Active BOOL DEFAULT ('x'), ---
    Interne BOOL, --- Auto-consommation.
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    WITHOUT ROWID;

CREATE TABLE Espèces ( --- Plante pouvant se reproduire et engendrer une descendance viable et féconde.
                       --- Permet d'enregistrer:
                       --- Les caractéristiques des graines (pour les annuelles).
                       --- L'amendement nécessaire.
                       --- Les objectifs de production.
                       --- Des informations utiles pour les cultures.
    ---row_summary Espèce|:: ,Famille|(::) ,Catégories
    Espèce TEXT PRIMARY KEY, --- Espèce botanique.
    Famille TEXT REFERENCES Familles (Famille) ON UPDATE CASCADE, ---
    Catégories TEXT, --- Ce qui nous intéresse chez cette espèce.\nLes lettres suivantes seront remplacées par des symboles:\nLégume racine 🥕 (ra)\nLégume bulbe 🧅 (bu)\nLégume feuille et branche 🌿 (fb)\nLégume fleur 🌼 (fl)\nLégume fruit 🍆 (lf)\nGrain 🌽 (gr)\nPAM 🌺 (am)\nPetit fruit 🍓 (pf)\nFruitier 🍎 (fr)\nAgrume 🍊 (ag)\nEngrais vert 🟩 (ev)\nMellifère 🐝 (me)\nBois 🪓 (bo)\nArbre 🌳 (ar)
    Rendement REAL, --- Production théorique moyenne de l'espèce.
        ---unit kg/m²
    Niveau TEXT, --- Niveau de difficulté.
                 --- Pour information, non utilisé pour le moment.
    Densité REAL, --- Nombre de plants par m² si l'espèce est seule sur la planche.
                  --- Les besoins NPK pour une culture tiennent compte de la densité réelle de la culture.
        ---unit plants/m²
    Dose_semis REAL, --- Quantité de semence.
                     --- Valeur par défaut pour les itinéraires techniques.
        ---unit g/m²
    Nb_graines_g REAL, --- Nombre de graines par gramme.
                       --- Valeur par défaut pour les variétés.
        ---unit graines/g
    FG REAL, --- Faculté germinative.
             --- Durée de conservation des graines (années).
             --- Pour information, non utilisé pour le moment.
    T_germ TEXT, --- Température de germination (min-max °C).
                 --- Pour information, non utilisé pour le moment.
    Levée REAL, --- Temps de levée.--UNIT(jours)
                --- Pour information, non utilisé pour le moment.
    Irrig TEXT, --- Irrigation nécessaire (paramétre 'Combo_Espèces_Irrig').
        ---combo Combo_Espèces_Irrig
    Conservation BOOL, --- Espèce pouvant se conserver. Elles apparaissent dans l'onglet Stock.
    A_planifier BOOL DEFAULT ('x'), --- Espèce à inclure dans les rotations (à cultiver l'année prochaine).
    Vivace BOOL, --- Espèce pouvant être récoltée tout les ans, après la période de juvénilité (noté sur la variété).
                 --- Pendant la période de juvénilité, le champ 'Type' des cultures est identique aux cultures annuelles (semis, plant).
                 --- A partir de la 1ère récolte, le champ 'Type' des cultures devient 'Vivace' (le champ 'Terminée'='v').
    Besoins TEXT, ---
        ---multiline
    S_taille INTEGER CONSTRAINT 'S_taille, semaine 1 à 52' CHECK (S_taille ISNULL OR S_taille BETWEEN 1 AND 52), --- N° de semaine (1 à 52) du début de la période de taille.
    Effet TEXT, --- Effet sur la croissance et la productionsur plantes proches (association).
    Usages TEXT, --- Propriété et usages de l'espèce (plantes aromatiques et médicinales).
        ---multiline
    Obj_annuel REAL, --- Objectif de production annuel.
        ---unit kg
    N REAL, --- Besoin en azote.
            --- Les besoins NPK pour une culture tiennent compte de la densité réelle de la culture.
        ---unit g/m²
    ★N TEXT AS (CASE WHEN N > 10 THEN 'Elevé' WHEN N < 5 THEN 'Faible' WHEN N NOTNULL THEN 'Moyen' END), --- Besoin qualitatif en azote.
                                                                                                         --- Valeurs de référence: 5 < N < 10
        ---cond_formats "::"=="Elevé" #darkRed# #lightRed#,"::"=="Faible" #darkGreen# #lightGreen#
    P REAL, --- Besoin en phosphore.
            --- Les besoins NPK pour une culture tiennent compte de la densité réelle de la culture.
        ---unit g/m²
    ★P TEXT AS (CASE WHEN P > 5 THEN 'Elevé' WHEN P < 2.55 THEN 'Faible' WHEN P NOTNULL THEN 'Moyen' END), --- Besoin qualitatif en phosphore.
                                                                                                           --- Valeurs de référence: 2.5 < P < 5
        ---cond_formats "::"=="Elevé" #darkRed# #lightRed#,"::"=="Faible" #darkGreen# #lightGreen#
    K REAL, --- Besoin en potassium.
            --- Les besoins NPK pour une culture tiennent compte de la densité réelle de la culture.
        ---unit g/m²
    ★K TEXT AS (CASE WHEN K > 12 THEN 'Elevé' WHEN K < 7 THEN 'Faible' WHEN K NOTNULL THEN 'Moyen' END), --- Besoin qualitatif en potassium.
                                                                                                         --- Valeurs de référence: 7 < K < 12
        ---cond_formats "::"=="Elevé" #darkRed# #lightRed#,"::"=="Faible" #darkGreen# #lightGreen#
    Date_inv DATE, --- Date d'inventaire.
    Inventaire REAL, --- Quantité en stock à la date d'inventaire.
        ---unit kg
    Prix_kg REAL, ---
        ---money
        ---unit €/kg
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    WITHOUT ROWID;
UPDATE fada_f_schema SET base_data='x' WHERE (name='Espèces')AND(field_name IN('Espèce','Famille','Catégories','Rendement','Niveau','Densité','Dose_semis','Nb_graines_g','FG','T_germ','Levée','Vivace','Besoins','S_taille','Effet','Usages','N','P','K'));

CREATE TABLE Familles ( --- Espèces ayant une certaine proximité phylogénétique.
                        --- Permet de d'enregistrer l'intervale de temps minimum entre 2 cultures d'une même famille.
    ---row_summary Famille,Intervalle,Effet
    Famille TEXT PRIMARY KEY, --- Famille botanique.
    Intervalle REAL DEFAULT (4), --- Nombre d'années nécessaires entre 2 cultures consécutives sur la même planche.
        ---unit ans
    Effet TEXT, ---
    Notes TEXT) --- Notes pour cette ligne.
                ---
                --- Format Markdown: https://www.markdownguide.org
                --- # Titre
                --- ## Sous-titre
                --- ### ...
                --- **gras**
                --- *italique*
                --- ...
        ---multiline
    WITHOUT ROWID;
UPDATE fada_f_schema SET base_data='x' WHERE (name='Familles');

CREATE TABLE Fertilisants ( --- Engrais, ammendements et paillages pour la fertilisation des planches de culture.
                            --- Les engrais vert ne sont pas à saisir ici, ils sont gérés comme les autres cultures.
                            ---
                            --- Conditions optimales:
                            --- - sol neutre, bien structuré, bien pourvu en matière organique
                            --- - bonne humidité
                            --- - bonne activité biologique
                            --- - apport bien intégré.
    ---row_summary Fertilisant,Type,N|::-,P|::-,K|::-
    Fertilisant TEXT PRIMARY KEY, ---
    Type TEXT, --- Liste de choix paramétrable (paramétre 'Combo_Fertilisants_Type').
        ---combo Combo_Fertilisants_Type
    Fonction TEXT, ---
        ---multiline
    Utilisation TEXT, ---
        ---multiline
    Délai INTEGER, --- Temps depuis la date de fertilisation avant libération des nutriments.
                   --- 1 semaine si vide.
        ---unit semaines
    Durée INTEGER, --- Temps de disponibilité des nutriments après date de fertilisation + délai.
                   --- 1 semaine si vide.
        ---unit semaines
    pH REAL, --- Potentiel hydrogène.
             --- pH < 7 : acide
             --- pH > 7 : basique
             ---
             --- La disponibilité des nutriments pour les plantes dépend directement du pH du sol.
    N REAL, --- Teneur en azote du fertilisant (%). -- Pas d'unité enregistrée pour que l'affichage N-P-K soit bon.
    N_coef REAL, --- Coefficient de disponibilité de l'azote en conditions optimales.
        ---unit %
    N_disp_pc REAL AS (round(N*N_coef/100,2)), --- Teneur en azote disponible pour les plantes (% du poids de fertilisant) en conditions optimales.
                                               --- Diponible=teneur x coefficient / 100
                                               --- Un autre coefficient relatif au sol et aux pratiques culturales sera appliqué pour estimer les quantités de fertilisant nécessaires (paramètre 'Ferti_coef_...').
                                               --- Valeurs de référence: 0.2 < N disp < 1
        ---unit %
        ---cond_formats ::<0.2 #darkRed# #lightRed#,::>1 #darkGreen# #lightGreen#
    P REAL, --- Teneur en phosphore du fertilisant (%).
    P_coef REAL, --- Coefficient de disponibilité du phosphore en conditions optimales.
        ---unit %
    P_disp_pc REAL AS (round(P*P_coef/100,2)), --- Teneur en phosphore disponible pour les plantes (% du poids de fertilisant) en conditions optimales.
                                               --- Diponible=teneur x coefficient / 100
                                               --- Un autre coefficient relatif au sol et aux pratiques culturales sera appliqué pour estimer les quantités de fertilisant nécessaires (paramètre 'Ferti_coef_...').
                                               --- Valeurs de référence: 0.1 < P disp < 0.5
        ---unit %
        ---cond_formats ::<0.1 #darkRed# #lightRed#,::>0.5 #darkGreen# #lightGreen#
    K REAL, --- Teneur en potassium du fertilisant (%).
    K_coef REAL, --- Coefficient de disponibilité du potassium en conditions optimales.
        ---unit %
    K_disp_pc REAL AS (round(K*K_coef/100,2)), --- Teneur en potassium disponible pour les plantes (% du poids de fertilisant) en conditions optimales.
                                               --- Diponible=teneur x coefficient / 100
                                               --- Un autre coefficient relatif au sol et aux pratiques culturales sera appliqué pour estimer les quantités de fertilisant nécessaires (paramètre 'Ferti_coef_...').
                                               --- Valeurs de référence: 0.4 < K disp < 2
        ---unit %
        ---cond_formats ::<0.4 #darkRed# #lightRed#,::>2 #darkGreen# #lightGreen#
    Ca REAL, --- Teneur en calcium du fertilisant (%).
    Ca_coef REAL, --- Coefficient de disponibilité (%) du calcium en conditions optimales.
    Ca_disp_pc REAL AS (round(Ca*Ca_coef/100,2)), --- Teneur en calcium disponible pour les plantes (% du poids de fertilisant) en conditions optimales.
                                                  --- Diponible=teneur x coefficient / 100
                                                  --- Un autre coefficient relatif au sol et aux pratiques culturales sera appliqué pour estimer les quantités de fertilisant nécessaires (paramètre 'Ferti_coef_...').
                                                  --- Valeurs de référence: 0.4 < Ca disp < 2
        ---unit %
        ---cond_formats ::<0.4 #darkRed# #lightRed#,::>2 #darkGreen# #lightGreen#
    Fe REAL, --- Teneur en fer du fertilisant (%).
    Fe_coef REAL, --- Coefficient de disponibilité du fer en conditions optimales.
        ---unit %
    Fe_disp_pc REAL AS (round(Fe*Fe_coef/100,2)), --- Teneur en fer disponible pour les plantes (% du poids de fertilisant) en conditions optimales.
                                                  --- Diponible=teneur x coefficient / 100
                                                  --- Un autre coefficient relatif au sol et aux pratiques culturales sera appliqué pour estimer les quantités de fertilisant nécessaires (paramètre 'Ferti_coef_...').
                                                  --- Valeurs de référence: 0.02 < Fe disp < 0.1
        ---unit %
        ---cond_formats ::<0.02 #darkRed# #lightRed#,::>0.1 #darkGreen# #lightGreen#
    Mg REAL, --- Teneur en magnésium du fertilisant (%).
    Mg_coef REAL, --- Coefficient de disponibilité du magnésium en conditions optimales.
        ---unit %
    Mg_disp_pc REAL AS (round(Mg*Mg_coef/100,2)), --- Teneur en magnésium disponible pour les plantes (% du poids de fertilisant) en conditions optimales.
                                                  --- Diponible=teneur x coefficient / 100
                                                  --- Un autre coefficient relatif au sol et aux pratiques culturales sera appliqué pour estimer les quantités de fertilisant nécessaires (paramètre 'Ferti_coef_...').
                                                  --- Valeurs de référence: 0.2 < Mg disp < 1
        ---unit %
        ---cond_formats ::<0.2 #darkRed# #lightRed#,::>1 #darkGreen# #lightGreen#
    Na REAL, --- Teneur en sodium du fertilisant (%).
    Na_coef REAL, --- Coefficient de disponibilité du sodium en conditions optimales.
        ---unit %
    Na_disp_pc REAL AS (round(Na*Na_coef/100,2)), --- Teneur en sodium disponible pour les plantes (% du poids de fertilisant) en conditions optimales.
                                                  --- Diponible=teneur x coefficient / 100
                                                  --- Un autre coefficient relatif au sol et aux pratiques culturales sera appliqué pour estimer les quantités de fertilisant nécessaires (paramètre 'Ferti_coef_...').
                                                  --- Valeurs de référence: 0.04 < Na disp < 0.2
        ---unit %
        ---cond_formats ::>0.2 #darkRed# #lightRed#,::<0.04 #darkGreen# #lightGreen#
    S REAL, --- Teneur en souffre du fertilisant (%).
    S_coef REAL, --- Coefficient de disponibilité du souffre en conditions optimales.
        ---unit %
    S_disp_pc REAL AS (round(S*S_coef/100,2)), --- Teneur en souffre disponible pour les plantes (% du poids de fertilisant) en conditions optimales.
                                               --- Diponible=teneur x coefficient / 100
                                               --- Un autre coefficient relatif au sol et aux pratiques culturales sera appliqué pour estimer les quantités de fertilisant nécessaires (paramètre 'Ferti_coef_...').
                                               --- Valeurs de référence: 0.04 < S disp < 0.2
        ---unit %
        ---cond_formats ::<0.04 #darkRed# #lightRed#,::>0.2 #darkGreen# #lightGreen#
    Si REAL, --- Teneur en silicium du fertilisant (%).
    Si_coef REAL, --- Coefficient de disponibilité du silicium en conditions optimales.
        ---unit %
    Si_disp_pc REAL AS (round(Si*Si_coef/100,2)), --- Teneur en silicium disponible pour les plantes (% du poids de fertilisant) en conditions optimales.
                                                  --- Diponible=teneur x coefficient / 100
                                                  --- Un autre coefficient relatif au sol et aux pratiques culturales sera appliqué pour estimer les quantités de fertilisant nécessaires (paramètre 'Ferti_coef_...').
                                                  --- Valeurs de référence: 0.1 < Si disp < 0.5
        ---unit %
        ---cond_formats ::<0.1 #darkRed# #lightRed#,::>0.5 #darkGreen# #lightGreen#
    Date_inv DATE, ---
    Inventaire REAL, ---
        ---unit kg
    Prix_kg REAL, ---
        ---money
        ---unit €/kg
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    WITHOUT ROWID;
UPDATE fada_f_schema SET base_data='x' WHERE (name='Fertilisants')AND(field_name IN('Fertilisant','Type','Fonction','pH','N','N_coef','P','P_coef','K','K_coef','Ca','Ca_coef','Fe','Fe_coef','Mg','Mg_coef','Na','Na_coef','S','S_coef','Si','Si_coef'));

CREATE TABLE Fertilisations ( --- Quantités de fertilisant apportées par planche.
    ---row_summary Date,Planche,Fertilisant,Quantité
    -- ---can_open_tab SELECT (count(*)>0) FROM Cu_répartir_fertilisation WHERE (DATE('now') BETWEEN Début_fertilisation_possible AND Fin_fertilisation_possible)
    ID INTEGER PRIMARY KEY AUTOINCREMENT, ---
    Date DATE NOT NULL, --- Date de fertilisation.
                        --- Laisser vide pour avoir automatiquement la date du jour.
    Planche TEXT REFERENCES Planches (Planche) ON UPDATE CASCADE, ---
    -- Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL, --- Les espèces possibles pour saisir une fertilisation sont celles pour qui il existe des cultures à venir ou en place avant récolte.
    --                                                                     --- Voir infobulle 'Culture'.
    --     ---fk_filter Espèce IN (SELECT Espèce FROM Cu_répartir_fertilisation
    --     ---fk_filter            WHERE (DATE(coalesce(:Date:,'now')) BETWEEN Début_fertilisation_possible AND Fin_fertilisation_possible))
    -- Culture INTEGER REFERENCES Cultures (Culture) ON UPDATE CASCADE NOT NULL, --- Les cultures possibles pour saisir une fertilisation sont celles qui:
    --                                                                           --- - Date de mise en place (semis en place ou plantation) <= date du jour plus avance de fertilisation (paramètre 'Ferti_avance')
    --                                                                           --- - Début de récolte (Début_récolte) >= date du jour moins délai de saisie de fertilisation (paramètre 'Ferti_retard')
    --                                                                           ---
    --                                                                           --- Le paramètre 'Ferti_avance' permet de saisir des fertilisations avant la date de mise en place de la culture.
    --                                                                           --- Le paramètre 'Ferti_retard' permet de saisir les fertilisation après le début de récolte (Début_récolte).
    --     ---fk_filter Culture IN (SELECT Culture FROM Cu_répartir_fertilisation
    --     ---fk_filter             WHERE (Espèce=:Espèce:)AND
    --     ---fk_filter                   (DATE(coalesce(:Date:,'now')) BETWEEN Début_fertilisation_possible AND Fin_fertilisation_possible))
    --     ---fk_sort_field Planche
    Fertilisant TEXT REFERENCES Fertilisants (Fertilisant) ON UPDATE CASCADE NOT NULL, ---
    Quantité REAL NOT NULL, --- Quantité apportée sur la planche.
        ---unit kg
    N REAL, --- Apport en azote sur la planche.
            --- Quantité x N disp fertilisant x Coefficient de disponibilité relatif au sol (paramètre 'Ferti_coef_N')
            --- ATTENTION valeur calculée lors de la saisie et non mise à jour si le coefficient ou la teneur du fertilisant est modifié.
        ---unit g
    P REAL, --- Apport en phosphore sur la planche.
            --- Quantité x P disp fertilisant x Coefficient de disponibilité relatif au sol (paramètre 'Ferti_coef_P')
            --- ATTENTION valeur calculée lors de la saisie et non mise à jour si le coefficient ou la teneur du fertilisant est modifié.
        ---unit g
    K REAL, --- Apport en potassium sur la planche.
            --- Quantité x K disp fertilisant x Coefficient de disponibilité relatif au sol (paramètre 'Ferti_coef_K')
            --- ATTENTION valeur calculée lors de la saisie et non mise à jour si le coefficient ou la teneur du fertilisant est modifié.
        ---unit g
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    ;

CREATE TABLE Fournisseurs ( --- Fournisseurs, notamment des semences.
    Fournisseur TEXT PRIMARY KEY, ---
    Type TEXT, --- Liste de choix paramétrable (paramétre 'Combo_Fournisseurs_Type').
        ---combo Combo_Fournisseurs_Type
    Priorité INTEGER, --- Chez qui commander en priorité.
    Adresse TEXT, ---
        ---multiline
    Site_web TEXT, ---
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    WITHOUT ROWID;
UPDATE fada_f_schema SET base_data='x' WHERE (name='Fournisseurs')AND(field_name IN('Fournisseur','Type','Site_web','Notes'));

CREATE TABLE ITP ( ---
    ---row_summary IT_plante,Type_planche,Type_culture
    ---can_open_tab SELECT (count(*)>0) FROM Espèces
    IT_plante TEXT PRIMARY KEY, --- ::Cultures.IT_plante
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE, ---
    Type_planche TEXT, --- Liste de choix paramétrable (paramétre 'Combo_Planches_Type').
        ---combo Combo_Planches_Type
    Type_culture TEXT AS ( --- Automatique, en fonction des débuts de période de semis, plantation et récolte.
       CASE WHEN S_semis NOTNULL AND S_plantation NOTNULL AND S_récolte NOTNULL AND D_récolte NOTNULL THEN 'Semis pépinière'
            WHEN S_semis ISNULL  AND S_plantation NOTNULL AND S_récolte NOTNULL AND D_récolte NOTNULL THEN 'Plant'
            WHEN S_semis NOTNULL AND S_plantation ISNULL  AND S_récolte NOTNULL AND D_récolte NOTNULL THEN 'Semis en place'
            WHEN S_semis NOTNULL AND S_plantation NOTNULL AND S_récolte NOTNULL AND D_récolte ISNULL THEN 'Compagne'
            WHEN S_semis NOTNULL AND S_plantation ISNULL     AND D_récolte ISNULL THEN 'Engrais vert'
            -- WHEN S_récolte NOTNULL AND D_récolte NOTNULL THEN 'Vivace'
            ELSE '?' END),
    S_semis INTEGER CONSTRAINT 'S_semis, semaine 1 à 52' CHECK (S_semis ISNULL OR S_semis BETWEEN 1 AND 52), --- N° de semaine (1 à 52) du début de la période de semis.
                                                                                                             --- Vide pour plant acheté.
    S_plantation INTEGER CONSTRAINT 'S_plantation, semaine 1 à 52' CHECK (S_plantation ISNULL OR S_plantation BETWEEN 1 AND 52), --- N° de semaine (1 à 52) du début de la période de plantation.
                                                                                                                                 --- Vide pour semis en place et engrais vert.
    S_récolte INTEGER CONSTRAINT 'S_récolte, semaine 1 à 52' CHECK (S_récolte ISNULL OR S_récolte BETWEEN 1 AND 52), --- N° de semaine (1 à 52) du début de la période de récolte.
                                                                                                                     --- Si 'D_récolte' est vide (compagne, engrais vert), semaine de destruction de la culture.
    D_récolte INTEGER CONSTRAINT 'D_récolte, 1 à 52 semaines' CHECK (D_récolte ISNULL OR D_récolte BETWEEN 1 AND 52), --- Durée de récolte en semaines (1 à 52).
                                                                                                                      --- Vide pour compagne et engrais vert.
    Décal_max INTEGER CONSTRAINT 'Décal_max, 0 à 52 semaines' CHECK (Décal_max ISNULL OR Décal_max BETWEEN 0 AND 52), --- Décalage maximum possible des semis, plantation et récolte, pour que l'ITP reste viable.
        ---unit semaines
    Espacement INTEGER, --- ::Cultures.Espacement
        ---unit cm
    Esp_rangs INTEGER, --- Espacement entre les rangs cultivés sur une planche.
                       --- Utilisé pour calculer le nb de rangs pour les cultures.
        ---unit cm
    Nb_graines_plant REAL, --- Nombre de graines par poquet ou par plant si semis en ligne continue avec éclaircissement.
                           --- Utilisé pour calculer le poids de semence nécessaire.
        ---unit graines/plant
    Dose_semis REAL, --- Quantité de semence.
                     --- Utilisé pour calculer le poids de semence nécessaire SI ESPACEMENT=0.
        ---unit g/m²
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    WITHOUT ROWID;
UPDATE fada_f_schema SET base_data='x' WHERE (name='ITP')AND(field_name IN('IT_plante','Espèce','Type_planche','S_semis','S_plantation','S_récolte','D_récolte','Décal_max','Espacement','Esp_rangs','Nb_graines_plant','Dose_semis','Notes'));

CREATE TABLE Notes ( --- Notes utilisateurs.
    --- row_summary ID,Type,Description
    ID INTEGER PRIMARY KEY AUTOINCREMENT, ---
    Date_création DATE, ---
    Date_modif DATE, ---
    Type TEXT, --- Type de note (paramétre 'Combo_Notes_Type').
        ---combo Combo_Notes_Type
    Description TEXT, ---
    Texte TEXT) --- Texte de la note (Ctrl+Enter pour éditer).
        ---multiline
    ;

CREATE TABLE Planches ( --- Planches de cultures.\n"
                        --- Les 1ers caractères (nb paramétrable) indiquent l'ilot dans lequel se trouve la planche.
                        --- Les planches doivent être affectées à des rotations pour que la planification puisse générer les cultures.
                        --- Si vous créez vos cultures manuellement, il n'est pas nécessaire d'affecter les planches.
                        ---
                        --- Le nom d'une planche doit avoir la forme <Ilot><unité de production><planche>.
                        --- Exemple 'Sud1A' pour la planche A de l'unité de production 1 de l'ilot 'Sud'.
    ---row_summary Planche,Type,Longueur|::,Largeur| x ::
    Planche TEXT PRIMARY KEY, ---
    Type TEXT, --- Liste de choix paramétrable (paramétre 'Combo_Planches_Type').
        ---combo Combo_Planches_Type
    Longueur REAL, --- Longueur de la planche.
        ---unit m
    Largeur REAL, --- Largeur de la planche.
        ---unit m
    Irrig TEXT, --- Irrigation actuellement installée sur la planche (paramétre 'Combo_Planches_Irrig').
        ---combo Combo_Planches_Irrig
    Rotation TEXT REFERENCES Rotations (Rotation) ON UPDATE CASCADE, --- Rotation de cultures qui utilisent cette planche.
        ---fk_filter Type_planche=:Type:
    Année INTEGER, ---
    Analyse TEXT REFERENCES Analyses_de_sol (Analyse) ON DELETE SET NULL ON UPDATE CASCADE, ---
    Planches_influencées TEXT, --- Planches pouvant être influencées par une culture de vivace sur la planche courante (associations).
                               --- Noms de planche séparés par des virgules (,).
                               ---
                               --- Ctrl+Alt+V pour coller une sélection de cellules dans la cellule courante (en une seule ligne).
        ---multiline
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    WITHOUT ROWID;
UPDATE fada_f_schema SET base_data='Example' WHERE (name='Planches')AND(field_name IN('Planche','Type','Longueur','Largeur','Rotation','Année'));

CREATE TABLE Rotations ( --- Plan de rotations.
                         --- Ici vous ne saisissez que l'entête des rotations, avec un type de planche et une année de départ.
                         --- La liste des ITP qui constituent la rotation est à saisir dans l'onglet 'Détails des rotations'.
    ---row_summary Rotation,Type_planche,Nb_années
    Rotation TEXT PRIMARY KEY, --- Ensemble d'ITP qui vont se succéder sur un groupe de planches (ilot)..
    Type_planche TEXT, --- ::Planches.Type
        ---combo Combo_Planches_Type
    Année_1 INTEGER, --- Année de début de la rotation.
                     --- Cette valeur ne doit pas être changée d'une année sur l'autre tant que la rotation se poursuit.
    Nb_années INTEGER, --- Automatique, en fonction des détails de la rotation.
                       --- 3 à 6 ans en général.
        ---unit ans
    Active BOOL DEFAULT ('x'), --- Sera utilisée pour générer les cultures (planification).
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    WITHOUT ROWID;
UPDATE fada_f_schema SET base_data='Example' WHERE (name='Rotations')AND(field_name IN('Rotation','Type_planche','Année_1','Nb_années'));
UPDATE fada_f_schema SET readonly='x' WHERE (name='Rotations')AND(field_name IN('Nb_années'));


CREATE TABLE Rotations_détails ( ---
    ---row_summary Rotation,Année|Année :: - ,IT_plante
    ---can_open_tab SELECT (SELECT (count(*)>0) FROM Rotations)AND(SELECT (count(*)>0) FROM ITP JOIN Espèces E USING(Espèce) WHERE E.Vivace ISNULL)
    ID INTEGER PRIMARY KEY AUTOINCREMENT, ---
    Rotation TEXT REFERENCES Rotations (Rotation) ON UPDATE CASCADE NOT NULL, ---
    Année INTEGER DEFAULT (1) NOT NULL ON CONFLICT REPLACE, --- Année de culture dans la rotation.
                                                            --- Nombre entier entre 1 et 5 si la rotation est sur 5 ans.
    IT_plante TEXT REFERENCES ITP (IT_plante) ON UPDATE CASCADE, --- ::Cultures.IT_plante
        ---fk_filter Espèce IN(SELECT E.Espèce FROM Espèces E WHERE E.Vivace ISNULL)
    Pc_planches REAL DEFAULT (100) NOT NULL ON CONFLICT REPLACE, --- Pourcentage d'occupation des planches de l'UdP.
                                                                 --- Exemple: planche de 10m de long occupée à 50%
                                                                 --- - la culture occupera 10m avec un rang sur 2 si 'Occupation' commence par 'R'.
                                                                 --- - la culture occupera 10m avec un espacement entre plants 2 fois plus grand si 'Occupation' commence par 'E'.
                                                                 --- - la culture occupera 5m dans les autres cas (pas d'association de plante).
                                                                 ---
                                                                 --- Pour créer des associations de plante, utilisez toute la longueur de planche :
                                                                 --- 'Occupation' égal à 'R' (rang) ou 'E' (espacement).
        ---unit %
    Occupation TEXT, --- Comment les cultures occupent les planches de l'UdP si le pourcentage d'occupation est inférieur à 100%.
                     --- L: la culture n'occupera qu'une partie de la longueur de planche, toute la largeur de planche et des rangs pleins.
                     --- R: la culture occupera toute la longeur, pas toute la largeur (moins de rangs) et des rangs pleins.
                     --- E: la culture occupera toute la longeur, toute la largeur et des rangs partagés avec d'autres cultures (espacement plus grand).
                     ---
                     --- Pour créer des associations de plante, utilisez toute la longueur de planche :
                     --- 'Occupation' égal à 'R' (rang) ou 'E' (espacement).
    Fi_planches TEXT, --- Filtrage des planches de l'UdP sur le dernier caractère du nom de planche.
                      --- Exemple: l'ilot AA contient une UdP (1) de 4 planches (A, B, C et D).
                      --- Les planches sont: AA1A, AA1B, AA1C et AA1D.
                      --- Fi_planches=AC -> seule les planches AA1A et AA1C seront occupées par les cultures.
        ---col_width 40
    Décalage INTEGER, --- Décalage (en semaines) de la culture, par rapport aux semaines au plus tôt de l'ITP, dans la limte de 'Décal_max' sur l'ITP.
        ---unit semaines
                      --- Si vide, l'affichage montre les périodes possibles de semis, plantation et récolte, et les dates de cultures seront au plus tôt.
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    ;
UPDATE fada_f_schema SET base_data='Example' WHERE (name='Rotations_détails')AND(field_name IN('Rotation','Année','IT_plante','Pc_planches','Occupation','Fi_planches','Décalage'));

CREATE TABLE Planif_validations (
    IdxIdPl TEXT,
    Validée BOOL);

CREATE TABLE Récoltes ( --- Récoltes pour chaque culture.
    ---row_summary Date,Espèce,Culture,Quantité
    ---can_open_tab SELECT (count(*)>0) FROM Cultures__à_récolter
    ID INTEGER PRIMARY KEY AUTOINCREMENT, ---
    Date DATE NOT NULL, --- Date de récolte.
                        --- Laisser vide pour avoir automatiquement la date de fin de récolte de la culture, ou la date du jour si la date de fin de récolte est dans le futur.
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL, --- Les espèces possibles pour saisir une récolte sont celles pour qui il existe des cultures en cours de récolte.
                                                                        --- Voir infobulle 'Culture'.
        ---fk_filter Espèce IN (SELECT Espèce FROM Cu_répartir_récolte
        ---fk_filter            WHERE (DATE(coalesce(:Date:,'now')) BETWEEN Début_récolte_possible AND Fin_récolte_possible))
    Culture INTEGER REFERENCES Cultures (Culture) ON UPDATE CASCADE NOT NULL, --- Les cultures possibles pour saisir une récolte sont celles qui:
                                                                              --- - ont des dates de début et fin de récolte (réelles ou prévues)
                                                                              --- - Début récolte <= date du jour plus avance de saisie de récolte (paramètre 'C_récolte_avance')
                                                                              --- - Fin récolte >= date du jour moins délai de saisie de récolte (paramètre 'C_récolte_prolongation')
                                                                              ---
                                                                              --- Le paramètre 'C_récolte_avance' permet de saisir des récoltes faites avant leur date prévue.
                                                                              --- Le paramètre 'C_récolte_prolongation' permet de saisir les récoltes après que celles-ci aient été faites.
        ---fk_filter Culture IN (SELECT Culture FROM Cu_répartir_récolte
        ---fk_filter             WHERE (Espèce=:Espèce:)AND
        ---fk_filter                   (DATE(coalesce(:Date:,'now')) BETWEEN Début_récolte_possible AND Fin_récolte_possible))
        ---fk_sort_field Planche
    Quantité REAL NOT NULL, --- Quantité récoltée sur la planche.
        ---unit kg
    Réc_ter BOOL, --- La récolte est terminée pour cette culture.\n\nIl suffit d'une seule ligne de récolte avec une valeur non vide pour que le champ 'Récolte faite' de la culture passe à 'x'.
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    ;

CREATE TABLE Variétés ( --- Variété cultivée (cultivar) qui a été sélectionné et choisi pour certaines de ses caractéristiques.
                        --- Permet de gérer les stocks et commandes de semence.
    ---row_summary Variété,Fournisseur
    Variété TEXT PRIMARY KEY, ---
    Espèce TEXT REFERENCES Espèces (Espèce) ON UPDATE CASCADE NOT NULL, ---
    Nb_graines_g REAL, --- Nombre de graines par gramme.
        ---unit graines/g
                       --- Utilisé pour calculer le poids de semence nécessaire.
    Qté_stock REAL, --- Semence en stock.
        ---unit g
    Qté_cde REAL, --- Semence commandée.
                  --- A réception, mettre à 0 et ajouter la quantité à la quantité en stock.
        ---unit g
    Fournisseur TEXT REFERENCES Fournisseurs (Fournisseur) ON UPDATE CASCADE, ---
    S_récolte INTEGER CONSTRAINT 'S_récolte, semaine 1 à 52' CHECK (S_récolte ISNULL OR S_récolte BETWEEN 1 AND 52), --- N° de semaine (1 à 52).
                                                                                                                     --- Laisser vide pour que la valeur saisie sur l'itinéraire technique soit utilisée.
    D_récolte INTEGER CONSTRAINT 'D_récolte, 1 à 52 semaines' CHECK (D_récolte ISNULL OR D_récolte BETWEEN 1 AND 52), --- Durée de récolte en semaines (1 à 52).
                                                                                                                      --- Laisser vide pour que la valeur saisie sur l'itinéraire technique soit utilisée.
    PJ INTEGER, --- Période de juvénilité: Nombre d'années avant la 1ère récolte.
                --- 0 pour les plantes à récolter moins de 12 mois après la date de plantation.
                --- 1 pour les bisannuelles.
        ---unit ans
    Notes TEXT) --- ::Familles.Notes
        ---multiline
    WITHOUT ROWID;
UPDATE fada_f_schema SET base_data='x' WHERE (name='Variétés')AND(field_name IN('Variété','Espèce','Nb_graines_g'));



