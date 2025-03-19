QString sTest = QStringLiteral(R"#(
''a '' || :a || '' b '' || :b
)#");


QString sPlanifCultureCalcDate = QStringLiteral(R"#(
-- :D_min (YYYY-MM-DD)
-- :Déb_période (MM-DD)
iif(strftime(''%Y'',:D_min)||''-''||:Déb_période<:D_min,
    DATE(strftime(''%Y'',:D_min)||''-''||:Déb_période,''+1 years''),
    strftime(''%Y'',:D_min)||''-''||:Déb_période)
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
iif(:dDeb1 <= :dFin1, --Période 1 sur 1 année
    iif(:dDeb2 > :dDeb1, --Période 2 sur la même année
        iif(:dDeb2 >= :dFin1,:dFin1 - :dDeb1, -- Pas de chevauchement entre P1 et P2                 OK
                           :dDeb2 - :dDeb1),-- Chevauchement entre P1 et P2                          OK
        :dFin1 - :dDeb1), -- Période 2 l'année suivante, pas de chevauchement entre P1 et P2         OK
                    --Période 1 sur 2 années
    iif(:dDeb2 >= :dDeb1,:dDeb2 - :dDeb1, --Période 2 sur l'année 1, chevauchement entre P1 et P2    OK
                      iif(:dDeb2 >= :dFin1,     --Période 2 sur l'année 2
                          :dFin1 + 365 - :dDeb1, -- Pas de chevauchement entre P1 et P2              OK
                          :dDeb2 + 365 - :dDeb1))) -- Chevauchement entre P1 et P2                   OK

--                          '' dDeb2 ''||:dDeb2||'' - dDeb1 ''||:dDeb1||'' - dFin1 ''||:dFin1||'' - dDeb2+365-dDeb1 = ''||:dDeb2+365-:dDeb1))) -- Chevauchement entre P1 et P2 BUG :dDeb2+365-:dDeb1
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
iif(:date NOT NULL,strftime(''%j'',''2000-''||:date),0)
-- out : nb de jour depuis le début de l'année.
)#");

QString sItpTempo = QStringLiteral(R"#(
-- :Type : type_culture
-- :dXxx nb de jour depuis le début de l'année.
iif(:Type=''Semis sous abris'',(:dSem-1)||'':''|| -- Attente
                               ItpTempoNJPeriode(:dSem,:fSem,:dPlant)||'':''|| -- Durée semis
                               ItpTempoNJInterPe(:dSem,:fSem,:dPlant)||'':''|| -- Semis fait attente plantation
                               ItpTempoNJPeriode(:dPlant,:fPlant,:dRec)||'':''|| -- Durée plantation
                               ItpTempoNJInterPe(:dPlant,:fPlant,:dRec)||'':''|| -- Plantation faite attente récolte
                               ItpTempoNJPeriode(:dRec,:fRec,:fRec), -- Durée récolte
iif(:Type=''Semis direct''    ,(:dSem-1)||'':''|| -- Attente
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
iif(:Type=''Sans récolte''    ,(:dSem-1) ||'':''|| -- Attente
                               ItpTempoNJPeriode(:dSem,:fSem,:dPlant) ||'':''||
                               ItpTempoNJInterPe(:dSem,:fSem,:dPlant) ||'':''||
                               ItpTempoNJPeriode(:dPlant,:fPlant,:dRec) ||'':''||
                               ItpTempoNJInterPe(:dPlant,:fPlant,coalesce(:fRec,365)) ||
                               '':'',NULL
)))))
-- out : 15:15:15:15:15:15
)#");

QString sCulTempoNJPeriode = QStringLiteral(R"#(
-- :dDeb :dFin date (YYYY-MM-DD)
replace(-julianday(:dDeb)+julianday(:dFin),''.0'','''')
-- out : nb de jour de la période.
)#");

QString sCulTempo = QStringLiteral(R"#(
-- :Type : type_culture
-- :dXxx date.
iif(:Type=''Semis sous abris'',CulTempoNJPeriode(strftime(''%Y'',:dSem)||''-01-01'',:dSem)||'':''|| -- Attente
                               ''2:''|| -- Durée semis
                               (CulTempoNJPeriode(:dSem,:dPlant)-2) ||'':''|| -- Semis fait attente plantation
                               ''2:''|| -- Durée plantation
                               (CulTempoNJPeriode(:dPlant,:dRec)-2) ||'':''|| -- Plantation faite attente récolte
                               CulTempoNJPeriode(:dRec,:fRec), -- Durée récolte
iif(:Type=''Semis direct'',CulTempoNJPeriode(strftime(''%Y'',:dSem)||''-01-01'',:dSem)||'':''|| -- Attente
                           ''2:''|| -- Durée semis
                           ''0:''|| -- Semis fait attente récolte
                           ''0:''||
                           (CulTempoNJPeriode(:dSem,:dRec)-2) ||'':''|| -- Semis fait attente récolte
                           CulTempoNJPeriode(:dRec,:fRec), -- Durée récolte
iif(:Type=''Plant'',CulTempoNJPeriode(strftime(''%Y'',:dPlant)||''-01-01'',:dPlant)||'':''|| -- Attente
                    ''0:''|| -- Durée semis
                    ''0:''||
                    ''2:''|| -- Durée plantation
                    (CulTempoNJPeriode(:dPlant,:dRec)-2) ||'':''|| -- Plantation faite attente récolte
                    CulTempoNJPeriode(:dRec,:fRec), -- Durée récolte
iif(:Type=''Engrais vert'',CulTempoNJPeriode(strftime(''%Y'',:dSem)||''-01-01'',:dSem)||'':''|| -- Attente
                           ''2:''|| -- Durée semis
                           ''0:''||
                           ''0:''|| -- Durée plantation
                           (CulTempoNJPeriode(:dSem,coalesce(:fRec,strftime(''%Y'',:dSem)||''-12-31''))-2) ||'':''|| -- Plantation faite attente récolte
                           ''0'', -- Durée récolte
iif(:Type=''Sans récolte'',CulTempoNJPeriode(strftime(''%Y'',:dSem)||''-01-01'',:dSem)||'':''|| -- Attente
                           ''2:''|| -- Durée semis
                           (CulTempoNJPeriode(:dSem,:dPlant)-2) ||'':''|| -- Semis fait attente plantation
                           ''2:''|| -- Durée plantation
                           CulTempoNJPeriode(:dPlant,:dRec)-2 ||'':''|| -- Plantation faite attente récolte
                           ''0'', NULL -- Durée récolte
)))))
-- out : 15:15:15:15:15:15
)#");

QString sZeroSiErrPlusDe5mois = QStringLiteral(R"#(
iif(:err BETWEEN 0 AND 150, :err,0)
)#");

QString sCulIncDate = QStringLiteral(R"#(
-- :c : date de la culture (YYYY-MM-DD)
-- :i : début ou fin de période de l'IT_plante (MM-DD)
-- :test : 'D' ou 'F'
-- :tol : nb de jour de tolérance de sortie de la période.
iif((:c NOTNULL)AND(:i NOTNULL),
    iif(:test=''D'',ZeroSiErrPlusDe5mois(julianday(DATE(substr(:c,1,5)||:i,-max(:tol,0)||'' days''))-julianday(:c)),
                    ZeroSiErrPlusDe5mois(julianday(:c)-julianday(DATE(substr(:c,1,5)||:i,max(:tol,0)||'' days'')))),
0)
-- out : nb de jour de sortie de la période+tolérance.
)#");

QString sCulIncDates = QStringLiteral(R"#(
-- :cXxx : dates de la culture (YYYY-MM-DD)
-- :iDXxx et iFXxx : débuts et fins de période de l'IT_plante (MM-DD)
iif(CulIncDate(:cSem,:iDSem,''D'',    (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_A_semis''))>0,     ''Semis trop tôt'',
iif(CulIncDate(:cSem,:iFSem,''F'',    (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_R_semis''))>0,     ''Semis trop tard'',
iif(CulIncDate(:cPlant,:iDPlant,''D'',(SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_A_plantation''))>0,''Plant. trop tôt'',
iif(CulIncDate(:cPlant,:iFPlant,''F'',(SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_R_plantation''))>0,''Plant. trop tard'',
iif(CulIncDate(:cDRec,:iDRec,''D'',   (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_A_récolte''))>0,   ''Récolte trop tôt'',
iif(CulIncDate(:cFRec,:iFRec,''F'',   (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_R_récolte''))>0,   ''Récolte trop tard'',
NULL))))))
-- out : Texte qui dit où est l'incohérence.
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
iif(:Type=''Semis sous abris'',(:dPlant-1) ||'':''|| -- Attente
                               ''0:''|| -- Durée semis
                               ''0:''|| -- Semis fait attente plantation
                               ItpTempoNJPeriode(:dPlant,:fPlant,:dRec) ||'':''|| -- Durée plantation
                               ItpTempoNJInterPe(:dPlant,:fPlant,:dRec) ||'':''|| -- Plantation faite attente récolte
                               ItpTempoNJPeriode(:dRec,:fRec,:fRec), -- Durée récolte
iif(:Type=''Semis direct''    ,(:dSem-1) ||'':''|| -- Attente
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
iif(:Type=''Sans récolte''    ,(:dSem-1) ||'':''|| -- Attente
                               ItpTempoNJPeriode(:dSem,:fSem,:dPlant) ||'':''||
                               ItpTempoNJInterPe(:dSem,:fSem,:dPlant) ||'':''||
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
SELECT C.Culture,I.Espèce,C.Longueur,C.Début_récolte,C.Fin_récolte
FROM Cultures C JOIN ITP I USING(IT_plante)
WHERE (:Repartir NOTNULL)AND
      -- (C.Début_récolte <= DATE('now','+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_avance_saisie_récolte')||' days'))AND
      -- (C.Fin_récolte >= DATE('now','-'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_retard_saisie_récolte')||' days'))AND
      ((:Espece ISNULL)OR(I.Espèce=:Espece))AND
      (DATE(C.Début_récolte,'-'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_avance_saisie_récolte')||' days') <= coalesce(:Date,DATE('now')))AND
      (DATE(C.Fin_récolte,'+'||(SELECT max(Valeur,0) FROM Params WHERE Paramètre='C_retard_saisie_récolte')||' days') >= coalesce(:Date,DATE('now')))AND
      ((:Repartir='*')OR
       (C.Planche LIKE :Repartir||'%'))

)#");
