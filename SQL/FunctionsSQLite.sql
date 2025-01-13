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
iif(:Type=''Semis sous abris'',(:dSem-1) ||'':''|| -- Attente
                               ItpTempoNJPeriode(:dSem,:fSem,:dPlant) ||'':''|| -- Durée semis
                               ItpTempoNJInterPe(:dSem,:fSem,:dPlant) ||'':''|| -- Semis fait attente plantation
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
                               '':'',''''
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
                           CulTempoNJPeriode(:dSem,:dRec)-2 ||'':''|| -- Semis fait attente récolte
                           CulTempoNJPeriode(:dRec,:fRec), -- Durée récolte

iif(:Type=''Plant'',CulTempoNJPeriode(strftime(''%Y'',:dPlant)||''-01-01'',:dPlant)||'':''|| -- Attente
                    ''0:''|| -- Durée semis
                    ''0:''||
                    ''2:''|| -- Durée plantation
                    CulTempoNJPeriode(:dPlant,:dRec)-2 ||'':''|| -- Plantation faite attente récolte
                    CulTempoNJPeriode(:dRec,:fRec), -- Durée récolte

iif(:Type=''Engrais vert'',CulTempoNJPeriode(strftime(''%Y'',:dSem)||''-01-01'',:dSem)||'':''|| -- Attente
                           ''2:''|| -- Durée semis
                           ''0:''||
                           ''0:''|| -- Durée plantation
                           CulTempoNJPeriode(:dSem,coalesce(:fRec,strftime(''%Y'',:dSem)||''-12-31''))-2 ||'':''|| -- Plantation faite attente récolte
                           ''0'', -- Durée récolte

iif(:Type=''Sans récolte'',CulTempoNJPeriode(strftime(''%Y'',:dSem)||''-01-01'',:dSem)||'':''|| -- Attente
                           ''2:''|| -- Durée semis
                           CulTempoNJPeriode(:dSem,:dPlant)-2 ||'':''|| -- Semis fait attente plantation
                           ''2:''|| -- Durée plantation
                           CulTempoNJPeriode(:dPlant,:dRec)-2 ||'':''|| -- Plantation faite attente récolte
                           ''0'', '''' -- Durée récolte
)))))
-- out : 15:15:15:15:15:15
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
iif(:Type=''to force params oder'',:dSem||:fSem,
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
                               '':'',''''
))))))
-- out : 15:15:15:15:15:15
)#");

QString sRF_trop_proches = QStringLiteral(R"#(
SELECT RF.Famille || ' année ' || format('%i',RF.Année) result
                                         FROM R_famille RF
                                        WHERE (RF.Rotation=(SELECT Rotation FROM R_famille WHERE ID=:ID))AND --ITP de la rotation
                                              (RF.Année<>(SELECT Année FROM R_famille WHERE ID=:ID))AND -- que les ITP des autres années de la rotation
                                              (RF.Famille=(SELECT Famille FROM R_famille WHERE ID=:ID))AND -- que les ITP de la même famille
                                              (((SELECT Année FROM R_famille WHERE ID=:ID) BETWEEN RF.Année-RF.Intervalle+1 AND
                                                                                                   RF.Année+RF.Intervalle-1)OR -- en conflit lors du 1er cycle
                                               ((SELECT Année FROM R_famille WHERE ID=:ID) BETWEEN RF.Année-RF.Intervalle+1+RF.Nb_années AND
                                                                                                   RF.Année+RF.Intervalle-1+RF.Nb_années)) -- en conflit lors du 2ème cycle
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

