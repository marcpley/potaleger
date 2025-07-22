
// Math

QString sCeil = QStringLiteral(R"#(
iif(:Valeur>0,CAST(:Valeur+1 AS INTEGER),CAST(:Valeur AS INTEGER))
)#");

QString sFloor = QStringLiteral(R"#(
iif(:Valeur>0,CAST(:Valeur AS INTEGER),CAST(:Valeur-1 AS INTEGER))
)#");

// Potaléger

QString sPlanifCultureCalcDate = QStringLiteral(R"#(
-- :D_min (YYYY-MM-DD)
-- :Déb_période (MM-DD)
iif(strftime(''%Y'',:D_min)||''-''||:Déb_période < DATE(:D_min,''-''||(SELECT Valeur FROM Params WHERE Paramètre=''Planifier_retard'')||'' days''),
    DATE(strftime(''%Y'',:D_min)||''-''||:Déb_période,''+1 years''),
    iif(strftime(''%Y'',:D_min)||''-''||:Déb_période<:D_min,
        DATE(:D_min,''+1 days''),
        strftime(''%Y'',:D_min)||''-''||:Déb_période))
)#");

QString sItpCompleteDFPeriode = QStringLiteral(R"#(
-- :dPeriode (MM)
iif((length(:dPeriode)<>5)AND(round(:dPeriode) BETWEEN 1 AND 12),
    format(''%02d-01'',round(:dPeriode)),
    :dPeriode)
-- out : début ou fin de période valide (MM-DD).
)#");

QString sItpTempoNJPeriode = QStringLiteral(R"#(
-- :dDeb1,:dFin1,:dDeb2 nb de jour depuis le début de l'année.
iif(:dDeb1 <= :dFin1,
    --Période 1 sur 1 année
    iif(:dDeb2 > :dDeb1,
        --Période 2 sur la même année
        iif(:dDeb2 >= :dFin1,:dFin1 - :dDeb1, -- Pas de chevauchement entre P1 et P2                 OK
                             :dDeb2 - :dDeb1),-- Chevauchement entre P1 et P2                          OK
        -- Période 2 l'année suivante, pas de chevauchement entre P1 et P2         OK
        :dFin1 - :dDeb1),
    --Période 1 sur 2 années
    iif(:dDeb2 >= :dDeb1,
        --Période 2 sur l'année 1, chevauchement entre P1 et P2    OK
        :dDeb2 - :dDeb1,
        --Période 2 sur l'année 2
        iif(:dDeb2 >= :dFin1,
            :dFin1 + 365 - :dDeb1, -- Pas de chevauchement entre P1 et P2              OK
            :dDeb2 + 365 - :dDeb1))) -- Chevauchement entre P1 et P2                   OK

    -- '' dDeb1: ''||:dDeb1||'' - dFin1: ''||:dFin1||'' - dDeb2: ''||:dDeb2||'' - dDeb2+365-dDeb1 = ''||CAST(:dDeb2+365-:dDeb1 AS TEXT) -- Chevauchement entre P1 et P2 BUG :dDeb2+365-:dDeb1
-- out : nb de jour
)#");

QString sItpTempoNJInterPe = QStringLiteral(R"#(
-- :dDeb1,:dFin1,:dDeb2 nb de jour depuis le début de l'année.
iif(:dDeb1 <= :dFin1, --Période 1 sur 1 année
    iif(:dDeb2 >= :dDeb1, --Période 2 sur la même année
        iif(:dDeb2 >= :dFin1,:dDeb2 - :dFin1, -- Pas de chevauchement entre P1 et P2
                           0),-- Chevauchement entre P1 et P2
        :dDeb2 + 365 - :dFin1), -- Période 2 l'année suivante, pas de chevauchement entre P1 et P2
                    --Période 1 sur 2 années
    iif(:dDeb2 > :dDeb1,0, --Période 2 sur l'année 1, chevauchement entre P1 et P2
                      iif(:dDeb2 > :dFin1,     --Période 2 sur l'année 2
                          :dDeb2 - :dFin1, -- Pas de chevauchement entre P1 et P2
                          0))) -- Chevauchement entre P1 et P2
-- out : nb de jour
)#");

QString sItpTempoNJ = QStringLiteral(R"#(
-- :date (MM-DD)
iif(:date NOT NULL,CAST(strftime(''%j'',''2001-''||:date)AS INTEGER),0) -- 2000 est bisextile
-- out : nb de jour depuis le début de l'année.
)#");

QString sItpTempo = QStringLiteral(R"#(
-- :Type : type_culture
-- :dXxx nb de jour depuis le début de l'année.
iif(:Type=''Semis pépinière'',(:dSem-1)||'':''|| -- Attente
                               ItpTempoNJPeriode(:dSem,:fSem,:dPlant)||'':''|| -- Durée semis
                               ItpTempoNJInterPe(:dSem,:fSem,:dPlant)||'':''|| -- Semis fait attente plantation
                               ItpTempoNJPeriode(:dPlant,:fPlant,:dRec)||'':''|| -- Durée plantation
                               ItpTempoNJInterPe(:dPlant,:fPlant,:dRec)||'':''|| -- Plantation faite attente récolte
                               ItpTempoNJPeriode(:dRec,:fRec,:fRec), -- Durée récolte
iif(:Type=''Semis en place''    ,(:dSem-1)||'':''|| -- Attente
                               ItpTempoNJPeriode(:dSem,:fSem,:dRec)||'':''||
                               '':''||
                               '':''||
                               ItpTempoNJInterPe(:dSem,:fSem,:dRec)||'':''||
                               ItpTempoNJPeriode(:dRec,:fRec,:fRec),
iif(:Type=''Plant''           ,(:dPlant-1)||'':''|| -- Attente
                               '':''||
                               '':''||
                               ItpTempoNJPeriode(:dPlant,:fPlant,:dRec)||'':''||
                               ItpTempoNJInterPe(:dPlant,:fPlant,:dRec)||'':''||
                               ItpTempoNJPeriode(:dRec,:fRec,:fRec),
iif(:Type=''Engrais vert''    ,(:dSem-1)||'':''|| -- Attente
                               ItpTempoNJPeriode(:dSem,:fSem,coalesce(:fRec,365))||'':''||
                               '':''||
                               '':''||
                               ItpTempoNJInterPe(:dSem,:fSem,coalesce(:fRec,365))||
                               '':'',
iif(:Type=''Sans récolte''    ,(:dSem-1)||'':''|| -- Attente
                               ItpTempoNJPeriode(:dSem,:fSem,:dPlant)||'':''||
                               ItpTempoNJInterPe(:dSem,:fSem,:dPlant)||'':''||
                               ItpTempoNJPeriode(:dPlant,:fPlant,:dRec)||'':''||
                               ItpTempoNJInterPe(:dPlant,:fPlant,coalesce(:fRec,365))||
                               '':'',NULL
)))))
-- out : 15:15:15:15:15:15
)#");

QString sCul_Espece = QStringLiteral(R"#(
-- SELECT C.Culture Cu, -- Impossible de garder le nom original, erreur "duplicate column name: Culture"
--        coalesce(V.Espèce,I.Espèce) Espèce,
--        C.Variété,
--        C.Saison,
--        (SELECT E.Vivace FROM Espèces E WHERE E.Espèce=coalesce(V.Espèce,I.Espèce)) Vivace,
--        C.Planche,
--        C.Date_semis,
--        C.Date_plantation,
--        C.Début_récolte,
--        C.Fin_récolte,
--        -- C.Récolte_faite,
--        C.Terminée,
--        C.Longueur
-- FROM Cultures C
-- LEFT JOIN ITP I USING(IT_plante)
-- LEFT JOIN Variétés V USING(Variété)
-- WHERE (:Culture ISNULL)OR(C.Culture=:Culture)
)#");

QString sCulTempoNJPeriode = QStringLiteral(R"#(
-- :dDeb :dFin date (YYYY-MM-DD)
iif((:dDeb ISNULL)OR(:dFin ISNULL),0,
     replace(-julianday(:dDeb)+julianday(:dFin),''.0'',''''))
-- out : nb de jour de la période.
)#");

QString sCulTempo = QStringLiteral(R"#(
-- :Type : type_culture
-- :dXxx date.
iif(:Type=''Semis pépinière'',CulTempoNJPeriode(strftime(''%Y'',:dSem)||''-01-01'',:dSem)||'':''|| -- Attente
                               ''4:''|| -- Durée semis
                               (CulTempoNJPeriode(:dSem,:dPlant)-4) ||'':''|| -- Semis fait attente plantation
                               ''4:''|| -- Durée plantation
                               (CulTempoNJPeriode(:dPlant,:dRec)-4) ||'':''|| -- Plantation faite attente récolte
                               CulTempoNJPeriode(:dRec,:fRec), -- Durée récolte
iif(:Type=''Semis en place'',CulTempoNJPeriode(strftime(''%Y'',:dSem)||''-01-01'',:dSem)||'':''|| -- Attente
                           ''4:''|| -- Durée semis
                           ''0:''|| -- Semis fait attente récolte
                           ''0:''||
                           (CulTempoNJPeriode(:dSem,:dRec)-4) ||'':''|| -- Semis fait attente récolte
                           CulTempoNJPeriode(:dRec,:fRec), -- Durée récolte
iif(:Type=''Plant'',CulTempoNJPeriode(strftime(''%Y'',:dPlant)||''-01-01'',:dPlant)||'':''|| -- Attente
                    ''0:''|| -- Durée semis
                    ''0:''||
                    ''4:''|| -- Durée plantation
                    (CulTempoNJPeriode(:dPlant,:dRec)-4) ||'':''|| -- Plantation faite attente récolte
                    CulTempoNJPeriode(:dRec,:fRec), -- Durée récolte
iif(:Type=''Engrais vert'',CulTempoNJPeriode(strftime(''%Y'',:dSem)||''-01-01'',:dSem)||'':''|| -- Attente
                           ''4:''|| -- Durée semis
                           ''0:''||
                           ''0:''|| -- Durée plantation
                           (CulTempoNJPeriode(:dSem,coalesce(:fRec,strftime(''%Y'',:dSem)||''-12-31''))-4) ||'':''|| -- Plantation faite attente récolte
                           ''0'', -- Durée récolte
iif(:Type=''Sans récolte'',CulTempoNJPeriode(strftime(''%Y'',:dSem)||''-01-01'',:dSem)||'':''|| -- Attente
                           ''4:''|| -- Durée semis
                           (CulTempoNJPeriode(:dSem,:dPlant)-4) ||'':''|| -- Semis fait attente plantation
                           ''4:''|| -- Durée plantation
                           (CulTempoNJPeriode(:dPlant,coalesce(:fRec,strftime(''%Y'',:dSem)||''-12-31''))-4) ||'':''|| -- Plantation faite attente fin de culture
                           ''0'', NULL -- Durée récolte
)))))
-- out : 15:15:15:15:15:15
)#");

QString sCulIncDate = QStringLiteral(R"#(
-- :a : Année de semis ou plantation de la culture (YYYY)
-- :c : date de la culture (YYYY-MM-DD)
-- :i : début ou fin de période de l'IT_plante (MM-DD)
-- :test : 'D' ou 'F'
-- :tol : nb de jour de tolérance de sortie de la période.
iif((:a NOTNULL)AND(:i0 NOTNULL)AND(:c NOTNULL)AND(:i NOTNULL),
    iif(:test=''D'',julianday(DATE(iif(:i<:i0,:a+1,:a)||''-''||:i,-max(:tol,0)||'' days''))-julianday(:c),
                    julianday(:c)-julianday(DATE(iif(:i<:i0,:a+1,:a)||''-''||:i,max(:tol,0)||'' days''))),
0)
-- out : nb de jour de sortie de la période+tolérance.
)#");

QString sCulIncDates = QStringLiteral(R"#(
-- :annee : Année de semis ou plantation de la culture (YYYY)
-- :i0 : Début de semis ou Début de plantation de l'ITP
-- :cXxx : dates de la culture (YYYY-MM-DD)
-- :iDXxx et iFXxx : débuts et fins de période de l'IT_plante (MM-DD)
iif(CulIncDate(:annee,:i0,:cSem,:iDSem,''D'',    (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_A_semis''))>0,     ''Semis trop tôt'',
iif(CulIncDate(:annee,:i0,:cSem,:iFSem,''F'',    (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_R_semis''))>0,     ''Semis trop tard'',
iif(CulIncDate(:annee,:i0,:cPlant,:iDPlant,''D'',(SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_A_plantation''))>0,''Plant. trop tôt'',
iif(CulIncDate(:annee,:i0,:cPlant,:iFPlant,''F'',(SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_R_plantation''))>0,''Plant. trop tard'',
iif(CulIncDate(:annee,:i0,:cDRec,:iDRec,''D'',   (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_A_récolte''))>0,   ''Récolte trop tôt'',
iif(CulIncDate(:annee,:i0,:cFRec,:iFRec,''F'',   (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_R_récolte''))>0,   ''Récolte trop tard'',
NULL))))))
-- out : Texte qui dit où est l'incohérence.
)#");

QString sCulNbRecoltesTheo = QStringLiteral(R"#(
-- :Ter Cultures.Terminée
-- :fRec date de fin de récolte de la culture (prochaine ou dernière si culture terminée)
-- :NbJRec nb de jour de récolte de l'ITP.
-- :Rec1 date de la 1ère récolte.
iif(coalesce(:Ter,'''') NOT LIKE ''v%'', --OR(:fRec=:Culture),
    1, -- Culture d'une espèce annuelle.
    ceil2((julianday(min(:fRec,DATE(''now'',''+''||:NbJRec||'' days'')))- --fonction native ceil pas reconnue.
          julianday(:Rec1))/365))
-- out : nb de récoltes théorique pour cette culture.
)#");

QString sCulTer = QStringLiteral(R"#(
-- :Ter
(:Ter NOTNULL)AND(:Ter!=''v'')AND(:Ter!=''V'')
-- out : vrai ou faux.
)#");

QString sRotDecalDateMeP = QStringLiteral(R"#(
-- :DateMeP date (YYYY-MM-DD)
iif(substr(:DateMeP,6,2)=''01'',DATE(:DateMeP,''-5 month''),
iif(substr(:DateMeP,6,2)=''02'',DATE(:DateMeP,''-6 month''),
iif(substr(:DateMeP,6,2)=''03'',DATE(:DateMeP,''-7 month''),
iif(substr(:DateMeP,6,2)=''04'',DATE(:DateMeP,''-8 month''),
iif(substr(:DateMeP,6,2)=''05'',DATE(:DateMeP,''-8 month''),
iif(substr(:DateMeP,6,2)=''06'',DATE(:DateMeP,''-7 month''),
iif(substr(:DateMeP,6,2)=''07'',DATE(:DateMeP,''-6 month''),
iif(substr(:DateMeP,6,2)=''08'',DATE(:DateMeP,''-5 month''),
iif(substr(:DateMeP,6,2)=''09'',DATE(:DateMeP,''-4 month''),
iif(substr(:DateMeP,6,2)=''10'',DATE(:DateMeP,''-4 month''),
iif(substr(:DateMeP,6,2)=''11'',DATE(:DateMeP,''-4 month''),
iif(substr(:DateMeP,6,2)=''12'',DATE(:DateMeP,''-5 month''),:DateMeP
))))))))))))
-- out : date décallée de plusieurs mois, décalage en fonction de la saison.
)#");

QString sRotTempo = QStringLiteral(R"#(
-- :Type : type_culture
-- :dXxx nb de jour depuis le début de l'année.
iif(:Type=''xxx'',:dSem||:fSem, --to force params oder
iif(:Type=''Semis pépinière'',(:dPlant-1) ||'':''|| -- Attente
                               '':''|| -- Durée semis
                               '':''|| -- Semis fait attente plantation
                               ItpTempoNJPeriode(:dPlant,:fPlant,:dRec) ||'':''|| -- Durée plantation
                               ItpTempoNJInterPe(:dPlant,:fPlant,:dRec) ||'':''|| -- Plantation faite attente récolte
                               ItpTempoNJPeriode(:dRec,:fRec,:fRec), -- Durée récolte
iif(:Type=''Semis en place''    ,(:dSem-1) ||'':''|| -- Attente
                               ItpTempoNJPeriode(:dSem,:fSem,:dRec) ||'':''||
                               '':''||
                               '':''||
                               ItpTempoNJInterPe(:dSem,:fSem,:dRec) ||'':''||
                               ItpTempoNJPeriode(:dRec,:fRec,:fRec),
iif(:Type=''Plant''           ,(:dPlant-1) ||'':''|| -- Attente
                               '':''||
                               '':''||
                               ItpTempoNJPeriode(:dPlant,:fPlant,:dRec) ||'':''||
                               ItpTempoNJInterPe(:dPlant,:fPlant,:dRec) ||'':''||
                               ItpTempoNJPeriode(:dRec,:fRec,:fRec),
iif(:Type=''Engrais vert''    ,(:dSem-1) ||'':''|| -- Attente
                               ItpTempoNJPeriode(:dSem,:fSem,:dPlant) ||'':''||
                               '':''||
                               '':''||
                               ItpTempoNJInterPe(:dSem,:fSem,coalesce(:fRec,365)) ||
                               '':'',
iif(:Type=''Sans récolte''    ,(:dPlant-1) ||'':''|| -- Attente
                               '':''||
                               '':''||
                               ItpTempoNJPeriode(:dPlant,:fPlant,:dRec) ||'':''||
                               ItpTempoNJInterPe(:dPlant,:fPlant,coalesce(:fRec,365)) ||
                               '':'',NULL
))))))
-- out : 15:15:15:15:15:15
)#");

QString sRF_trop_proches2 = QStringLiteral(R"#(
-- SELECT RF.Famille || ' année ' || format('%i',RF.Année) result
--                                          FROM R_famille RF
--                                         WHERE (RF.Rotation=(SELECT Rotation FROM R_famille WHERE ID=:ID))AND --ITP de la rotation
--                                               (RF.Année<>(SELECT Année FROM R_famille WHERE ID=:ID))AND -- que les ITP des autres années de la rotation
--                                               (RF.Famille=(SELECT Famille FROM R_famille WHERE ID=:ID))AND -- que les ITP de la même famille
--                                               (((SELECT Année FROM R_famille WHERE ID=:ID) BETWEEN RF.Année-RF.Intervalle+1 AND
--                                                                                                    RF.Année+RF.Intervalle-1)OR -- en conflit lors du 1er cycle
--                                                ((SELECT Année FROM R_famille WHERE ID=:ID) BETWEEN RF.Année-RF.Intervalle+1+RF.Nb_années AND
--                                                                                                    RF.Année+RF.Intervalle-1+RF.Nb_années)) -- en conflit lors du 2ème cycle
)#");

QString sRF_trop_proches = QStringLiteral(R"#(
SELECT RF.Famille || ' année ' || format('%i',RF.Année) result
                                         FROM R_famille RF
                                        WHERE (RF.Rotation=:Rot)AND --ITP de la rotation
                                              (RF.Année<>:Ann)AND -- que les ITP des autres années de la rotation
                                              (RF.Famille=:Fam)AND -- que les ITP de la même famille
                                              ((:Ann BETWEEN RF.Année-RF.Intervalle+1 AND
                                                             RF.Année+RF.Intervalle-1)OR -- en conflit lors du 1er cycle
                                               (:Ann BETWEEN RF.Année-RF.Intervalle+1+RF.Nb_années AND
                                                             RF.Année+RF.Intervalle-1+RF.Nb_années)) -- en conflit lors du 2ème cycle
)#");

QString sR_ITP_CAnt = QStringLiteral(R"#(
-- culture antérieure dans la rotation (si la courante n'est pas la 1ère de la rotation)
SELECT Date_Ferm,Pc_planches,Fi_planches FROM R_ITP
WHERE (Rotation=:Rot)AND(Rotation||Date_MEP < :Rot || :DateMEP)
ORDER BY Ind DESC
LIMIT 1
)#");

QString sR_ITP_CDer = QStringLiteral(R"#(
-- dernière culture de la rotation
SELECT DATE(Date_Ferm,'-'||:NbAnn||' years') Date_Ferm,Pc_planches,Fi_planches FROM R_ITP
WHERE (Rotation=:Rot)
ORDER BY Ind DESC
LIMIT 1
)#");

QString sItpPlus15jours = QStringLiteral(R"#(
iif(:dPeriode ISNULL,NULL,
iif(substr(:dPeriode,4,2)=''01'',substr(:dPeriode,1,3)||''15'', -- xx-01 > xx-15
iif(substr(:dPeriode,1,2)=''09'',''10-01'', -- 09-15 > 10-01
iif(substr(:dPeriode,1,2)=''10'',''11-01'', -- 10-15 > 11-01
iif(substr(:dPeriode,1,2)=''11'',''12-01'', -- 11-15 > 12-01
iif(substr(:dPeriode,1,2)=''12'',''01-01'', -- 12-15 > 01-01
                                 ''0''||(substr(:dPeriode,2,1)+1)||''-01'' -- xx-15 > xx+1-01
))))))
)#");


QString sItpPlusN = QStringLiteral(R"#(
-- :dPeriode (MM-DD)
iif(:dPeriode ISNULL,NULL,
    substr(DATE(''2000-''||:dPeriode,:N),6,5))
)#");

QString sRepartir_Recolte_sur = QStringLiteral(R"#(
SELECT C.Culture,C.Espèce,C.Longueur,C.Début_récolte,C.Fin_récolte
FROM Cultures C
WHERE (:Repartir NOTNULL)AND
      ((:Espece ISNULL)OR(C.Espèce=:Espece))AND
      (DATE(C.Début_récolte,'-'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_avance')||' days') <= coalesce(:Date,DATE('now')))AND
      (DATE(C.Fin_récolte,'+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_prolongation')||' days') >= coalesce(:Date,DATE('now')))AND
      (DATE(coalesce(C.Date_plantation,C.Date_semis),'+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_après_MEP')||' days') <= coalesce(:Date,DATE('now')))AND --1.1b1
      ((:Repartir='*')OR
       (C.Planche LIKE :Repartir||'%'))
)#");

QString sAnalyse_de_sol_proche = QStringLiteral(R"#(
-- SELECT 1 Proximité,* -- Trés lent !
-- FROM Analyses_de_sol
-- WHERE Planche=:Pl
-- UNION
-- SELECT 2 Proximité,*
-- FROM Analyses_de_sol
-- WHERE (:Pl LIKE substr(Planche,-1,-100)||'%')AND(length(Planche)>(SELECT Valeur FROM Params WHERE Paramètre='Ilot_nb_car'))
-- UNION
-- SELECT 3 Proximité,*
-- FROM Analyses_de_sol
-- WHERE (:Pl LIKE substr(Planche,-2,-100)||'%')AND(length(Planche)-1>(SELECT Valeur FROM Params WHERE Paramètre='Ilot_nb_car'))
-- UNION
-- SELECT 4 Proximité,*
-- FROM Analyses_de_sol
-- WHERE (:Pl LIKE substr(Planche,-3,-100)||'%')AND(length(Planche)-2>(SELECT Valeur FROM Params WHERE Paramètre='Ilot_nb_car'))
-- UNION
-- SELECT 5 Proximité,*
-- FROM Analyses_de_sol
-- WHERE (:Pl LIKE substr(Planche,-4,-100)||'%')AND(length(Planche)-3>(SELECT Valeur FROM Params WHERE Paramètre='Ilot_nb_car'))
-- ORDER BY Proximité,Date DESC
)#");

QString sRecoltes_cul = QStringLiteral(R"#(
SELECT R.Date,R.Quantité
FROM Récoltes R
WHERE (R.Culture=:Culture)AND
      ((coalesce(:Ter,'') NOT LIKE 'v%')OR(:dRec ISNULL)OR(:fRec ISNULL)OR -- Toutes les récoltes pour les annuelles.
        -- Récolte de l'année pour les vivaces.
       ((R.Date >= DATE(:dRec,'-'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_avance')||' days'))AND
        (R.Date <= DATE(:fRec,'+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_récolte_prolongation')||' days'))))
)#");

QString sRepartir_Fertilisation_sur = QStringLiteral(R"#(
SELECT C.Culture,C.Espèce,C.Longueur*P.Largeur Surface,C.Début_récolte,C.Fin_récolte
FROM Cultures C
JOIN Planches P USING(Planche)
WHERE (:Repartir NOTNULL)AND
      ((:Espece ISNULL)OR(C.Espèce=:Espece))AND
      (DATE(coalesce(C.Date_plantation,C.Date_semis),'-'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='Ferti_avance')||' days') <= coalesce(:Date,DATE('now')))AND
      (DATE(C.Début_récolte,'+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='Ferti_retard')||' days') >= coalesce(:Date,DATE('now')))AND
      ((:Repartir='*')OR
       (C.Planche LIKE :Repartir||'%'))
)#");
