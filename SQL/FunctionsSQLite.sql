



QString sCulNbRecoltesTheo = QStringLiteral(R"#( -- A tester
-- :Ter Cultures.Terminée
-- :fRec date de fin de récolte de la culture (prochaine ou dernière si culture terminée)
-- :NbJRec nb de jour de récolte de l'ITP.
-- :Rec1 date de la 1ère récolte.
    ceil2((julianday(min(:fRec,DATE(''now'',''+''||:NbJRec||'' days'')))- --fonction native ceil pas reconnue.
          julianday(:Rec1))/365)
-- out : nb de récoltes théorique pour cette culture.
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







