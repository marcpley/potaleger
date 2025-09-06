
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
-- :Semaine (1 à 52)
iif(:D_min NOTNULL AND :Semaine NOTNULL,
iif(date(strftime(''%Y'',:D_min)||''-01-01'',''+''||(SemToNJ(:Semaine)-1)||'' days'') < DATE(:D_min,''-''||(SELECT Valeur FROM Params WHERE Paramètre=''Planifier_retard'')||'' days''),
    -- Planifier année N+1
    DATE(date(strftime(''%Y'',:D_min)||''-01-01'',''+''||(SemToNJ(:Semaine)-1)||'' days''),''+1 years''),
    -- Planifier année N
    iif(date(strftime(''%Y'',:D_min)||''-01-01'',''+''||(SemToNJ(:Semaine)-1)||'' days'')<:D_min,
        DATE(:D_min,''+1 days''),
        date(strftime(''%Y'',:D_min)||''-01-01'',(SemToNJ(:Semaine)-1)||'' days''))),
    NULL)
-- out : date planifiée.
)#");

QString sSemToNJ = QStringLiteral(R"#(
-- :Semaine
iif(:Semaine NOT NULL,CAST(:Semaine*7 AS INTEGER)-6,0)
-- out : nb de jour depuis le début de l'année du lundi de la semaine (si lundi semaine 1 = 1er janvier, comme pour l'année 2001).
)#");

QString sCulTempo = QStringLiteral(R"#(
-- in : dates
iif(:jdRef<>:jdRef,'''','''')|| -- Pour forcer l'ordre des paramètres
CAST(iif(:dSemis ISNULL,0,CAST(julianday(:dSemis)-:jdRef AS INTEGER)) AS TEXT)||'':''||
CAST(iif(:dSemis ISNULL,0,CAST(julianday(:dSemis)-:jdRef AS INTEGER)+4) AS TEXT)||'':''||
CAST(iif(:dPlant ISNULL,0,CAST(julianday(:dPlant)-:jdRef AS INTEGER)) AS TEXT)||'':''||
CAST(iif(:dPlant ISNULL,0,CAST(julianday(:dPlant)-:jdRef AS INTEGER)+4) AS TEXT)||'':''||
CAST(iif(:dRec ISNULL,0,CAST(julianday(:dRec)-:jdRef AS INTEGER)) AS TEXT)||'':''||
CAST(iif(:fRec ISNULL,0,CAST(julianday(:fRec)-:jdRef AS INTEGER)+iif(:dRec=:fRec,4,0)) AS TEXT)
-- out : 15:15:15:15:15:15
)#");

QString sCulIncDate = QStringLiteral(R"#(
-- :a : Année de semis ou plantation de la culture (YYYY)
-- :i0 : début semis ou plantation de l'IT_plante (semaine)
-- :c : date de la culture (YYYY-MM-DD)
-- :i : début ou fin de période de l'IT_plante (semaine)
-- :test : 'D' ou 'F'
-- :tol : nb de jour de tolérance de sortie de la période.
iif((:a NOTNULL)AND(:i0 NOTNULL)AND(:c NOTNULL)AND(:i NOTNULL),
    iif(:test=''D'',julianday(DATE(iif(:i<:i0,:a+1,:a)||''-01-01'',(SemToNJ(:i)-1-max(:tol,0))||'' days''))-julianday(:c),
                    julianday(:c)-julianday(DATE(iif(:i<:i0,:a+1,:a)||''-01-01'',(SemToNJ(:i)-1+max(:tol,0))||'' days''))),
0)
-- out : nb de jour de sortie de la période+tolérance.
)#");

QString sCulIncDates = QStringLiteral(R"#(
-- :annee : Année de semis ou plantation de la culture (YYYY)
-- :i0 : Début de semis ou Début de plantation de l'ITP
-- :cXxx : dates de la culture (YYYY-MM-DD)
-- :iSXxx, :iDecalM et :iDRec : périodes de l'IT_plante (semaines)
iif(CulIncDate(:annee,:i0,:cSemis,:iSSemis,''D'',             (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_A_semis''))>0,
    ''Semis trop tôt: ''   ||CAST(CulIncDate(:annee,:i0,:cSemis,:iSSemis,''D'',0)AS INTEGER)||'' j'',
iif(CulIncDate(:annee,:i0,:cSemis,:iSSemis+:iDecalM,''F'',    (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_R_semis''))>0,
    ''Semis trop tard: ''  ||CAST(CulIncDate(:annee,:i0,:cSemis,:iSSemis+:iDecalM,''F'',0)AS INTEGER)||'' j'',
iif(CulIncDate(:annee,:i0,:cPlant,:iSPlant,''D'',             (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_A_plantation''))>0,
    ''Plant. trop tôt: ''  ||CAST(CulIncDate(:annee,:i0,:cPlant,:iSPlant,''D'',0)AS INTEGER)||'' j'',
iif(CulIncDate(:annee,:i0,:cPlant,:iSPlant+:iDecalM,''F'',    (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_R_plantation''))>0,
    ''Plant. trop tard: '' ||CAST(CulIncDate(:annee,:i0,:cPlant,:iSPlant+:iDecalM,''F'',0)AS INTEGER)||'' j'',
iif(CulIncDate(:annee,:i0,:cDRec,:iSRec,''D'',                (SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_A_récolte''))>0,
    ''Récolte trop tôt: '' ||CAST(CulIncDate(:annee,:i0,:cDRec,:iSRec,''D'',0)AS INTEGER)||'' j'',
iif(CulIncDate(:annee,:i0,:cFRec,:iSRec+:iDRec+:iDecalM,''F'',(SELECT Valeur FROM Params WHERE Paramètre=''Tolérance_R_récolte''))>0,
    ''Récolte trop tard: ''||CAST(CulIncDate(:annee,:i0,:cFRec,:iSRec+:iDRec+:iDecalM,''F'',0)AS INTEGER)||'' j'',
    -- ||'' ''||coalesce(:annee,''null'')||'' ''||coalesce(:i0,''null'')||'' ''||coalesce(:iSRec+:iDRec+:iDecalM,''null''),
    -- ||coalesce(:cSemis,''null'')||''_''||coalesce(:iSSemis,''null'')||'' ''
    -- ||coalesce(:cPlant,''null'')||''_''||coalesce(:iSPlant,''null'')||'' ''
    -- ||coalesce(:cDRec,''null'')||''_''||coalesce(:iSRec,''null'')||'' ''||coalesce(:cFRec,''null'')||''_''||coalesce(:iDRec,''null'')||'' ''||coalesce(:iDecalM,''null''),
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

QString sRecoltes_cul = QStringLiteral(R"#(
SELECT R.Date,R.Quantité
FROM Récoltes R
WHERE (R.Culture=:Culture)AND
      ((coalesce(:Ter,'') NOT LIKE 'v%')OR(:dRec ISNULL)OR(:fRec ISNULL)OR -- Toutes les récoltes pour les annuelles ou les vivaces sans période de récolte.
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
