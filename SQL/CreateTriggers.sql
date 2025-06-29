QString sDDLTriggers = QStringLiteral(R"#(
-- BEGIN TRANSACTION;;

DROP TRIGGER IF EXISTS Consommations__Saisies_INSERT;;
CREATE TRIGGER Consommations__Saisies_INSERT INSTEAD OF INSERT ON Consommations__Saisies
BEGIN
    INSERT INTO Consommations (Date,
                               Espèce,
                               Quantité,
                               Prix,
                               Destination,
                               Notes)
    VALUES (coalesce(NEW.Date,DATE('now')),
            NEW.Espèce,
            NEW.Quantité,
            coalesce(NEW.Prix,(SELECT Prix_kg FROM Espèces WHERE Espèce=NEW.Espèce)),
            NEW.Destination,
            NEW.Notes);
END;;

DROP TRIGGER IF EXISTS Consommations__Saisies_UPDATE;;
CREATE TRIGGER Consommations__Saisies_UPDATE INSTEAD OF UPDATE ON Consommations__Saisies
BEGIN
    UPDATE Consommations SET
        Date=coalesce(NEW.Date,DATE('now')),
        Espèce=NEW.Espèce,
        Quantité=NEW.Quantité,
        Prix=coalesce(NEW.Prix,(SELECT Prix_kg FROM Espèces WHERE Espèce=NEW.Espèce)),
        Destination=NEW.Destination,
        Notes=NEW.Notes
     WHERE Consommations.ID=OLD.ID;
END;;

DROP TRIGGER IF EXISTS Consommations__Saisies_DELETE;;
CREATE TRIGGER Consommations__Saisies_DELETE INSTEAD OF DELETE ON Consommations__Saisies
BEGIN
    DELETE FROM Consommations WHERE Consommations.ID=OLD.ID;
END;;


DROP TRIGGER IF EXISTS Cultures_INSERT_Planifier;;
CREATE TRIGGER Cultures_INSERT_Planifier AFTER INSERT ON Cultures
          WHEN (NOT CulTer(NEW.Terminée)) AND
               ((NEW.D_planif ISNULL) OR ((length(NEW.D_planif)=4)AND(CAST(NEW.D_planif AS INTEGER) BETWEEN 2000 AND 2100))) AND
               ((NEW.IT_plante NOTNULL)OR(NEW.Variété NOTNULL AND NEW.Terminée='v'))
BEGIN
    DELETE FROM Params WHERE Paramètre LIKE 'temp_%';
    INSERT INTO Params (Paramètre, Valeur)
    VALUES ('temp_date_planif', CASE
                                WHEN ((length(NEW.D_planif)=4)AND(CAST(NEW.D_planif AS INTEGER) BETWEEN 2000 AND 2100))
                                THEN    CASE
                                        WHEN (SELECT (ITP.Déb_plantation NOTNULL)AND(ITP.Déb_plantation<ITP.Déb_semis) FROM ITP WHERE ITP.IT_plante=NEW.IT_plante)
                                        THEN CAST(CAST(NEW.D_planif AS INTEGER)-1 AS TEXT)||'-01-01' -- Semis pépinière l'année précédent la mise en place.
                                        ELSE NEW.D_planif||'-01-01' END
                                ELSE DATE() END);

    UPDATE Cultures SET
        Date_semis=CASE WHEN Semis_fait NOTNULL THEN Date_semis
                        WHEN (SELECT (ITP.Déb_semis NOTNULL) FROM ITP WHERE ITP.IT_plante=NEW.IT_plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Déb_semis FROM ITP WHERE ITP.IT_plante=NEW.IT_plante))
                        ELSE NULL
                        END,
        D_planif=DATE()-- Nécessaire pour ne pas provoquer un appel récursif.
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=max((SELECT coalesce(Date_semis,Valeur) FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Date_plantation=CASE WHEN Plantation_faite NOTNULL THEN Date_plantation
                        WHEN (SELECT (ITP.Déb_plantation NOTNULL) FROM ITP WHERE ITP.IT_plante=NEW.IT_plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Déb_plantation FROM ITP WHERE ITP.IT_plante=NEW.IT_plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=max((SELECT coalesce(DATE(Date_plantation,'+'||coalesce((SELECT V.PJ FROM Variétés V WHERE V.Variété=NEW.Variété),0)||' years'),
                                                  DATE(Date_semis,'+'||coalesce((SELECT V.PJ FROM Variétés V WHERE V.Variété=NEW.Variété),0)||' years'),
                                                  Valeur) FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Début_récolte=  CASE WHEN Récolte_faite NOTNULL THEN Début_récolte
                        WHEN (SELECT (V.Déb_récolte NOTNULL) FROM Variétés V WHERE V.Variété=NEW.Variété)
                        THEN PlanifCultureCalcDate((SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                   (SELECT V.Déb_récolte FROM Variétés V WHERE V.Variété=NEW.Variété))
                        WHEN (SELECT (ITP.Déb_récolte NOTNULL) FROM ITP WHERE ITP.IT_plante=NEW.IT_plante)
                        THEN PlanifCultureCalcDate((SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                   (SELECT ITP.Déb_récolte FROM ITP WHERE ITP.IT_plante=NEW.IT_plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=max((SELECT coalesce(Début_récolte,Date_plantation,Date_semis,Valeur) FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Fin_récolte=  CASE WHEN Récolte_faite NOTNULL THEN Fin_récolte
                        WHEN (SELECT (V.Fin_récolte NOTNULL) FROM Variétés V WHERE V.Variété=NEW.Variété)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT V.Fin_récolte FROM Variétés V WHERE V.Variété=NEW.Variété))
                        WHEN (SELECT (ITP.Fin_récolte NOTNULL) FROM ITP WHERE ITP.IT_plante=NEW.IT_plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Fin_récolte FROM ITP WHERE ITP.IT_plante=NEW.IT_plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    DELETE FROM Params WHERE Paramètre LIKE 'temp_%';
END;;

DROP TRIGGER IF EXISTS Cultures_UPDATE_Planifier;;
CREATE TRIGGER Cultures_UPDATE_Planifier AFTER UPDATE ON Cultures
          WHEN (NOT CulTer(NEW.Terminée)) AND
               ((NEW.D_planif ISNULL) OR ((length(NEW.D_planif)=4)AND(CAST(NEW.D_planif AS INTEGER) BETWEEN 2000 AND 2100))) AND
               ((NEW.IT_plante NOTNULL)OR(NEW.Variété NOTNULL AND NEW.Terminée='v'))
BEGIN--Code identique à INSERT
    DELETE FROM Params WHERE Paramètre LIKE 'temp_%';
    INSERT INTO Params (Paramètre, Valeur)
    VALUES ('temp_date_planif', CASE
                                WHEN ((length(NEW.D_planif)=4)AND(CAST(NEW.D_planif AS INTEGER) BETWEEN 2000 AND 2100))
                                THEN    CASE
                                        WHEN (SELECT (ITP.Déb_plantation NOTNULL)AND(ITP.Déb_plantation<ITP.Déb_semis) FROM ITP WHERE ITP.IT_plante=NEW.IT_plante)
                                        THEN CAST(CAST(NEW.D_planif AS INTEGER)-1 AS TEXT)||'-01-01' -- Semis pépinière l'année précédent la mise en place.
                                        ELSE NEW.D_planif||'-01-01' END
                                ELSE DATE() END);

    UPDATE Cultures SET
        Date_semis=CASE WHEN Semis_fait NOTNULL THEN Date_semis
                        WHEN (SELECT (ITP.Déb_semis NOTNULL) FROM ITP WHERE ITP.IT_plante=NEW.IT_plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Déb_semis FROM ITP WHERE ITP.IT_plante=NEW.IT_plante))
                        ELSE NULL
                        END,
        D_planif=DATE()-- Nécessaire pour ne pas provoquer un appel récursif.
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=max((SELECT coalesce(Date_semis,Valeur) FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Date_plantation=CASE WHEN Plantation_faite NOTNULL THEN Date_plantation
                        WHEN (SELECT (ITP.Déb_plantation NOTNULL) FROM ITP WHERE ITP.IT_plante=NEW.IT_plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Déb_plantation FROM ITP WHERE ITP.IT_plante=NEW.IT_plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=max((SELECT coalesce(DATE(Date_plantation,'+'||coalesce((SELECT V.PJ FROM Variétés V WHERE V.Variété=NEW.Variété),0)||' years'),
                                                  DATE(Date_semis,'+'||coalesce((SELECT V.PJ FROM Variétés V WHERE V.Variété=NEW.Variété),0)||' years'),
                                                  Valeur) FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Début_récolte=  CASE WHEN Récolte_faite NOTNULL THEN Début_récolte
                        WHEN (SELECT (V.Déb_récolte NOTNULL) FROM Variétés V WHERE V.Variété=NEW.Variété)
                        THEN PlanifCultureCalcDate((SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                   (SELECT V.Déb_récolte FROM Variétés V WHERE V.Variété=NEW.Variété))
                        WHEN (SELECT (ITP.Déb_récolte NOTNULL) FROM ITP WHERE ITP.IT_plante=NEW.IT_plante)
                        THEN PlanifCultureCalcDate((SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                   (SELECT ITP.Déb_récolte FROM ITP WHERE ITP.IT_plante=NEW.IT_plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=max((SELECT coalesce(Début_récolte,Date_plantation,Date_semis,Valeur) FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Fin_récolte=  CASE WHEN Récolte_faite NOTNULL THEN Fin_récolte
                        WHEN (SELECT (V.Fin_récolte NOTNULL) FROM Variétés V WHERE V.Variété=NEW.Variété)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT V.Fin_récolte FROM Variétés V WHERE V.Variété=NEW.Variété))
                        WHEN (SELECT (ITP.Fin_récolte NOTNULL) FROM ITP WHERE ITP.IT_plante=NEW.IT_plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Fin_récolte FROM ITP WHERE ITP.IT_plante=NEW.IT_plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    DELETE FROM Params WHERE Paramètre LIKE 'temp_%';
END;;

DROP TRIGGER IF EXISTS Cultures_INSERT_Longueur;;
CREATE TRIGGER Cultures_INSERT_Longueur AFTER INSERT ON Cultures
          WHEN (NEW.Longueur ISNULL OR NEW.Longueur='?') AND (NEW.Planche NOTNULL)
BEGIN
    UPDATE Cultures
       SET Longueur=coalesce( (SELECT Longueur
                                 FROM Planches P
                                WHERE P.Planche=NEW.Planche), 0)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures_UPDATE_Longueur;;
CREATE TRIGGER Cultures_UPDATE_Longueur AFTER UPDATE ON Cultures
          WHEN (NEW.Longueur ISNULL OR NEW.Longueur='?') AND (NEW.Planche NOTNULL)
BEGIN
    UPDATE Cultures
       SET Longueur=coalesce( (SELECT Longueur
                                 FROM Planches P
                                WHERE P.Planche=NEW.Planche), 0)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures_INSERT_Espacement;;
CREATE TRIGGER Cultures_INSERT_Espacement AFTER INSERT ON Cultures
          WHEN (NEW.Espacement ISNULL OR NEW.Espacement='?') AND (NEW.IT_plante NOTNULL)
BEGIN
    UPDATE Cultures
       SET Espacement=coalesce( (SELECT Espacement
                                   FROM ITP I
                                  WHERE I.IT_plante=NEW.IT_plante), 0)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures_UPDATE_Espacement;;
CREATE TRIGGER Cultures_UPDATE_Espacement AFTER UPDATE ON Cultures
          WHEN (NEW.Espacement ISNULL OR NEW.Espacement='?') AND (NEW.IT_plante NOTNULL)
BEGIN
    UPDATE Cultures
       SET Espacement=coalesce( (SELECT Espacement
                                   FROM ITP I
                                  WHERE I.IT_plante=NEW.IT_plante), 0)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures_INSERT_Nb_rangs;;
CREATE TRIGGER Cultures_INSERT_Nb_rangs AFTER INSERT ON Cultures
          WHEN (NEW.Nb_rangs ISNULL OR NEW.Nb_rangs='?') AND (NEW.IT_plante NOTNULL)
BEGIN
    UPDATE Cultures
       SET Nb_rangs=coalesce( (SELECT Nb_rangs
                                 FROM ITP I
                                WHERE I.IT_plante=NEW.IT_plante), 0)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures_UPDATE_Nb_rangs;;
CREATE TRIGGER Cultures_UPDATE_Nb_rangs AFTER UPDATE ON Cultures
          WHEN (NEW.Nb_rangs ISNULL OR NEW.Nb_rangs='?') AND (NEW.IT_plante NOTNULL)
BEGIN
    UPDATE Cultures
       SET Nb_rangs=coalesce( (SELECT Nb_rangs
                                 FROM ITP I
                                WHERE I.IT_plante=NEW.IT_plante), 0)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__non_terminées_INSERT;;
CREATE TRIGGER Cultures__non_terminées_INSERT INSTEAD OF INSERT ON Cultures__non_terminées
BEGIN
    INSERT INTO  Cultures ( --Culture,
                            Planche,
                            Espèce,
                            IT_plante,
                            Variété,
                            Fournisseur,
                            D_planif,
                            Date_semis,
                            Semis_fait,
                            Date_plantation,
                            Plantation_faite,
                            Début_récolte,
                            -- Récolte_com,
                            Fin_récolte,
                            Récolte_faite,
                            Terminée,
                            Longueur,
                            Nb_rangs,
                            Espacement,
                            A_faire,
                            Notes)
    VALUES (--NEW.Culture,
            NEW.Planche,
            NEW.Espèce,
            NEW.IT_plante,
            NEW.Variété,
            NEW.Fournisseur,
            NEW.D_planif,
            NEW.Date_semis,
            NEW.Semis_fait,
            NEW.Date_plantation,
            NEW.Plantation_faite,
            NEW.Début_récolte,
            -- NEW.Récolte_com,
            NEW.Fin_récolte,
            NEW.Récolte_faite,
            NEW.Terminée,
            NEW.Longueur,
            NEW.Nb_rangs,
            NEW.Espacement,
            NEW.A_faire,
            NEW.Notes);
END;;


DROP TRIGGER IF EXISTS Cultures__non_terminées_UPDATE;;
CREATE TRIGGER Cultures__non_terminées_UPDATE INSTEAD OF UPDATE ON Cultures__non_terminées
BEGIN
    UPDATE Cultures SET
        Planche=NEW.Planche,
        Espèce=NEW.Espèce,
        IT_plante=NEW.IT_plante,
        Variété=NEW.Variété,
        Fournisseur=NEW.Fournisseur,
        D_planif=NEW.D_planif,
        Date_semis=NEW.Date_semis,
        Semis_fait=NEW.Semis_fait,
        Date_plantation=NEW.Date_plantation,
        Plantation_faite=NEW.Plantation_faite,
        Début_récolte=NEW.Début_récolte,
        -- Récolte_com=NEW.Récolte_com,
        Fin_récolte=NEW.Fin_récolte,
        Récolte_faite=NEW.Récolte_faite,
        Terminée=NEW.Terminée,
        Longueur=NEW.Longueur,
        Nb_rangs=NEW.Nb_rangs,
        Espacement=NEW.Espacement,
        A_faire=NEW.A_faire,
        Notes=NEW.Notes
    WHERE Culture=OLD.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__non_terminées_DELETE;;
CREATE TRIGGER Cultures__non_terminées_DELETE INSTEAD OF DELETE ON Cultures__non_terminées
BEGIN
    DELETE FROM Cultures WHERE Culture=OLD.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__à_semer_UPDATE;;
CREATE TRIGGER Cultures__à_semer_UPDATE INSTEAD OF UPDATE ON Cultures__à_semer
BEGIN
    UPDATE Cultures SET
        Planche=NEW.Planche,
        Variété=NEW.Variété,
        Fournisseur=NEW.Fournisseur,
        D_planif=NEW.D_planif,
        Date_semis=NEW.Date_semis,
        Semis_fait=NEW.Semis_fait,
        Longueur=NEW.Longueur,
        Nb_rangs=NEW.Nb_rangs,
        Espacement=NEW.Espacement,
        A_faire=NEW.A_faire,
        Notes=NEW.Notes
    WHERE Culture=OLD.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__à_semer_SA_UPDATE;;
DROP TRIGGER IF EXISTS Cultures__à_semer_pep_UPDATE;;
CREATE TRIGGER Cultures__à_semer_pep_UPDATE INSTEAD OF UPDATE ON Cultures__à_semer_pep
BEGIN
    -- Mise à jour semis sur toutes les cultures groupées.
    UPDATE Cultures SET
        Date_semis=NEW.Date_semis,
        Semis_fait=CASE WHEN NEW.Semis_fait NOTNULL AND(CAST(NEW.Semis_fait AS INTEGER)=NEW.Semis_fait) -- Valeur numérique entière, répartir au prorata de la longueur.
                        THEN NEW.Semis_fait /(SELECT sum(C.Longueur) FROM Cultures C WHERE instr(OLD.Cultures,C.Culture||' ')>0)
                                             *Cultures.Longueur
                        ELSE NEW.Semis_fait -- Même valeur pour toutes les cultures.
                        END
    WHERE instr(OLD.Cultures,Cultures.Culture||' ')>0;
    -- Mise à jour des notes uniquement sur les cultures qui avaient la même notes avant édition.
    UPDATE Cultures SET
        Notes=NEW.Notes
    WHERE (instr(OLD.Cultures,Cultures.Culture||' ')>0)AND
          (coalesce(Notes,'wdrsgvge')=coalesce(OLD.Notes,'wdrsgvge'));
    -- Mise à jour des notes uniquement sur les cultures qui avaient la même notes avant édition.
    UPDATE Cultures SET
        A_faire=NEW.A_faire
    WHERE (instr(OLD.Cultures,Cultures.Culture||' ')>0)AND
          (coalesce(A_faire,'dfthhfws')=coalesce(OLD.A_faire,'dfthhfws'));
END;;

DROP TRIGGER IF EXISTS Cultures__à_semer_D_UPDATE;;
DROP TRIGGER IF EXISTS Cultures__à_semer_EP_UPDATE;;
CREATE TRIGGER Cultures__à_semer_EP_UPDATE INSTEAD OF UPDATE ON Cultures__à_semer_EP
BEGIN
    UPDATE Cultures SET
        Planche=NEW.Planche,
        Variété=NEW.Variété,
        Fournisseur=NEW.Fournisseur,
        D_planif=NEW.D_planif,
        Date_semis=NEW.Date_semis,
        Semis_fait=NEW.Semis_fait,
        Longueur=NEW.Longueur,
        Nb_rangs=NEW.Nb_rangs,
        Espacement=NEW.Espacement,
        A_faire=NEW.A_faire,
        Notes=NEW.Notes
    WHERE Culture=OLD.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__à_planter_UPDATE;;
CREATE TRIGGER Cultures__à_planter_UPDATE INSTEAD OF UPDATE ON Cultures__à_planter
BEGIN
    UPDATE Cultures SET
        Planche=NEW.Planche,
        Variété=NEW.Variété,
        Fournisseur=NEW.Fournisseur,
        D_planif=NEW.D_planif,
        Date_semis=NEW.Date_semis,
        Semis_fait=NEW.Semis_fait,
        Date_plantation=NEW.Date_plantation,
        Plantation_faite=NEW.Plantation_faite,
        Longueur=NEW.Longueur,
        Nb_rangs=NEW.Nb_rangs,
        Espacement=NEW.Espacement,
        A_faire=NEW.A_faire,
        Notes=NEW.Notes
    WHERE Culture=OLD.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__à_récolter_UPDATE;;
CREATE TRIGGER Cultures__à_récolter_UPDATE INSTEAD OF UPDATE ON Cultures__à_récolter
BEGIN
    UPDATE Cultures SET
        Date_semis=NEW.Date_semis,
        Semis_fait=NEW.Semis_fait,
        Date_plantation=NEW.Date_plantation,
        Plantation_faite=NEW.Plantation_faite,
        Début_récolte=NEW.Début_récolte,
        -- Récolte_com=NEW.Récolte_com,
        Fin_récolte=NEW.Fin_récolte,
        Récolte_faite=NEW.Récolte_faite,
        Terminée=NEW.Terminée,
        A_faire=NEW.A_faire,
        Notes=NEW.Notes
    WHERE Culture=OLD.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__à_terminer_UPDATE;;
CREATE TRIGGER Cultures__à_terminer_UPDATE INSTEAD OF UPDATE ON Cultures__à_terminer
BEGIN
    UPDATE Cultures SET
        Date_semis=NEW.Date_semis,
        Date_plantation=NEW.Date_plantation,
        Début_récolte=NEW.Début_récolte,
        -- Récolte_com=NEW.Récolte_com,
        Fin_récolte=NEW.Fin_récolte,
        Récolte_faite=NEW.Récolte_faite,
        Terminée=NEW.Terminée,
        A_faire=NEW.A_faire,
        Notes=NEW.Notes
     WHERE Culture=OLD.Culture;
END;;

-- Pas d'INSERT pour Cultures__vivaces, les cultures sont crées dans les annuelles qui sont passées en vivaces après plantation.
DROP TRIGGER IF EXISTS Cultures__vivaces_UPDATE;;
CREATE TRIGGER Cultures__vivaces_UPDATE INSTEAD OF UPDATE ON Cultures__vivaces
BEGIN
    UPDATE Cultures SET
        D_planif=NEW.D_planif,
        Début_récolte=NEW.Début_récolte,
        Fin_récolte=NEW.Fin_récolte,
        Récolte_faite=NEW.Récolte_faite,
        Terminée=CASE WHEN NEW.Terminée LIKE 'v%' THEN NEW.Terminée
                      WHEN NEW.Terminée ISNULL THEN 'v'
                      ELSE 'v'||NEW.Terminée END,
        A_faire=NEW.A_faire,
        Notes=NEW.Notes
    WHERE Culture=OLD.Culture;
END;;

DROP TRIGGER IF EXISTS Destinations__conso_INSERT;;
CREATE TRIGGER Destinations__conso_INSERT INSTEAD OF INSERT ON Destinations__conso
BEGIN
    INSERT INTO Destinations (
        Destination,
        Type,
        Adresse,
        Site_web,
        Date_RAZ,
        Active,
        Notes)
    VALUES (
        NEW.Destination,
        NEW.Type,
        NEW.Adresse,
        NEW.Site_web,
        NEW.Date_RAZ,
        NEW.Active,
        NEW.Notes);
END;;

DROP TRIGGER IF EXISTS Destinations__conso_UPDATE;;
CREATE TRIGGER Destinations__conso_UPDATE INSTEAD OF UPDATE ON Destinations__conso
BEGIN
    UPDATE Destinations SET
        Destination=NEW.Destination,
        Type=NEW.Type,
        Adresse=NEW.Adresse,
        Site_web=NEW.Site_web,
        Date_RAZ=NEW.Date_RAZ,
        Active=NEW.Active,
        Notes=NEW.Notes
    WHERE Destination=OLD.Destination;
END;;

DROP TRIGGER IF EXISTS Destinations__conso_DELETE;;
CREATE TRIGGER Destinations__conso_DELETE INSTEAD OF DELETE ON Destinations__conso
BEGIN
    DELETE FROM Destinations
    WHERE Destination=OLD.Destination;
END;;

DROP TRIGGER IF EXISTS Espèces__a_INSERT;;
CREATE TRIGGER Espèces__a_INSERT INSTEAD OF INSERT ON Espèces__a
BEGIN
    INSERT INTO Espèces (
       Espèce,
       Famille,
       Rendement,
       Niveau,
       Favorable,
       Défavorable,
       Densité,
       Dose_semis,
       Nb_graines_g,
       FG,
       T_germ,
       Levée,
       Irrig,
       Conservation,
       A_planifier,
       -- Vivace,
       Obj_annuel,
       N,
       P,
       K,
       Notes)
    VALUES (
       NEW.Espèce,
       NEW.Famille,
       NEW.Rendement,
       NEW.Niveau,
       NEW.Favorable,
       NEW.Défavorable,
       NEW.Densité,
       NEW.Dose_semis,
       NEW.Nb_graines_g,
       NEW.FG,
       NEW.T_germ,
       NEW.Levée,
       NEW.Irrig,
       NEW.Conservation,
       NEW.A_planifier,
       -- NEW.Vivace,
       NEW.Obj_annuel,
       NEW.N,
       NEW.P,
       NEW.K,
       NEW.Notes);
END;;

DROP TRIGGER IF EXISTS Espèces__a_UPDATE;;
CREATE TRIGGER Espèces__a_UPDATE INSTEAD OF UPDATE ON Espèces__a
BEGIN
    UPDATE Espèces SET
        Espèce=NEW.Espèce,
        Famille=NEW.Famille,
        Rendement=NEW.Rendement,
        Niveau=NEW.Niveau,
        Favorable=NEW.Favorable,
        Défavorable=NEW.Défavorable,
        Densité=NEW.Densité,
        Dose_semis=NEW.Dose_semis,
        Nb_graines_g=NEW.Nb_graines_g,
        FG=NEW.FG,
        T_germ=NEW.T_germ,
        Levée=NEW.Levée,
        Irrig=NEW.Irrig,
        Conservation=NEW.Conservation,
        A_planifier=NEW.A_planifier,
        Obj_annuel=NEW.Obj_annuel,
        N=NEW.N,
        P=NEW.P,
        K=NEW.K,
        Notes=NEW.Notes
     WHERE Espèce=OLD.Espèce;
END;;

DROP TRIGGER IF EXISTS Espèces__a_DELETE;;
CREATE TRIGGER Espèces__a_DELETE INSTEAD OF DELETE ON Espèces__a
BEGIN
    DELETE FROM Espèces
    WHERE Espèce=OLD.Espèce;
END;;

DROP TRIGGER IF EXISTS Espèces__couverture_UPDATE;;
CREATE TRIGGER Espèces__couverture_UPDATE INSTEAD OF UPDATE ON Espèces__couverture
BEGIN
    UPDATE Espèces SET
        Rendement=NEW.Rendement,
        Niveau=NEW.Niveau,
        Obj_annuel=NEW.Obj_annuel,
        Notes=NEW.Notes
     WHERE Espèce=OLD.Espèce;
END;;

DROP TRIGGER IF EXISTS Espèces__v_INSERT;;
CREATE TRIGGER Espèces__v_INSERT INSTEAD OF INSERT ON Espèces__v
BEGIN
    INSERT INTO Espèces (
       Espèce,
       Famille,
       Rendement,
       Favorable,
       Défavorable,
       Taille,
       Irrig,
       Conservation,
       A_planifier,
       Vivace,
       Obj_annuel,
       N,
       P,
       K,
       Notes)
    VALUES (
       NEW.Espèce,
       NEW.Famille,
       NEW.Rendement,
       NEW.Favorable,
       NEW.Défavorable,
       NEW.Taille,
       NEW.Irrig,
       NEW.Conservation,
       NEW.A_planifier,
       'x',
       NEW.Obj_annuel,
       NEW.N,
       NEW.P,
       NEW.K,
       NEW.Notes);
END;;

DROP TRIGGER IF EXISTS Espèces__v_UPDATE;;
CREATE TRIGGER Espèces__v_UPDATE INSTEAD OF UPDATE ON Espèces__v
BEGIN
    UPDATE Espèces SET
        Espèce=NEW.Espèce,
        Famille=NEW.Famille,
        Rendement=NEW.Rendement,
        Favorable=NEW.Favorable,
        Défavorable=NEW.Défavorable,
        Taille=NEW.Taille,
        Irrig=NEW.Irrig,
        Conservation=NEW.Conservation,
        A_planifier=NEW.A_planifier,
        Obj_annuel=NEW.Obj_annuel,
        N=NEW.N,
        P=NEW.P,
        K=NEW.K,
        Notes=NEW.Notes
     WHERE Espèce=OLD.Espèce;
END;;

DROP TRIGGER IF EXISTS Espèces__v_DELETE;;
CREATE TRIGGER Espèces__v_DELETE INSTEAD OF DELETE ON Espèces__v
BEGIN
    DELETE FROM Espèces
    WHERE Espèce=OLD.Espèce;
END;;

DROP TRIGGER IF EXISTS Espèces__inventaire_UPDATE;;
CREATE TRIGGER Espèces__inventaire_UPDATE INSTEAD OF UPDATE ON Espèces__inventaire
BEGIN
    UPDATE Espèces SET
        Date_inv=NEW.Date_inv,
        Inventaire=NEW.Inventaire,
        Prix_kg=NEW.Prix_kg,
        Notes=NEW.Notes
     WHERE Espèce=OLD.Espèce;
END;;

DROP TRIGGER IF EXISTS Fertilisants__inventaire_UPDATE;;
CREATE TRIGGER Fertilisants__inventaire_UPDATE INSTEAD OF UPDATE ON Fertilisants__inventaire
BEGIN
    UPDATE Fertilisants SET
        Date_inv=NEW.Date_inv,
        Inventaire=NEW.Inventaire,
        Prix_kg=NEW.Prix_kg,
        Notes=NEW.Notes
     WHERE Fertilisant=OLD.Fertilisant;
END;;

DROP TRIGGER IF EXISTS Fertilisations__Saisies_INSERT;;
CREATE TRIGGER Fertilisations__Saisies_INSERT INSTEAD OF INSERT ON Fertilisations__Saisies
BEGIN
    SELECT RAISE(ABORT,'NOT NULL constraint failed Fertilisations.Culture/Répartir unable to fetch row') WHERE (NEW.Culture ISNULL)AND(NEW.Répartir ISNULL);
    SELECT RAISE(ABORT,'Culture et Répartir non NULL') WHERE (NEW.Culture NOTNULL)AND(NEW.Répartir NOTNULL);
    --Saisie d'un Fertilisations pour une culture unique0
    INSERT INTO Fertilisations (Date,
                                Espèce,
                                Culture,
                                Fertilisant,
                                Quantité,
                                N,P,K,
                                Notes)
    SELECT coalesce(NEW.Date,DATE('now')),
           NEW.Espèce,
           C.Culture,
           NEW.Fertilisant,
           NEW.Quantité, -- kg
           NEW.Quantité*(SELECT N_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10 -- g
                       *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_N')/100,
           NEW.Quantité*(SELECT P_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10*10 -- g
                       *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_P')/100,
           NEW.Quantité*(SELECT K_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10*10 -- g
                       *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_K')/100,
           NEW.Notes
    FROM Cultures C
    WHERE (NEW.Répartir ISNULL)AND
          (C.Culture=NEW.Culture);
    --Répartition de la quantité de fertilisant.
    INSERT INTO Fertilisations (Date,
                                Espèce,
                                Culture,
                                Fertilisant,
                                Quantité,
                                N,P,K,
                                Notes)
    SELECT coalesce(NEW.Date,DATE('now')),
            C.Espèce, -- Si pas d'espèce saisie, répartition sur TOUTES les cultures.
            C.Culture,
            NEW.Fertilisant,
            round(NEW.Quantité/(SELECT sum(Surface) FROM Repartir_Fertilisation_sur(NEW.Répartir,NEW.Espèce,NEW.Date))*C.Surface,3),
            round(NEW.Quantité*(SELECT N_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10
                              *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_N')/100
                              /(SELECT sum(Surface) FROM Repartir_Fertilisation_sur(NEW.Répartir,NEW.Espèce,NEW.Date))*C.Surface,3),
            round(NEW.Quantité*(SELECT P_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10
                              *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_P')/100
                              /(SELECT sum(Surface) FROM Repartir_Fertilisation_sur(NEW.Répartir,NEW.Espèce,NEW.Date))*C.Surface,3),
            round(NEW.Quantité*(SELECT K_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10
                              *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_K')/100
                              /(SELECT sum(Surface) FROM Repartir_Fertilisation_sur(NEW.Répartir,NEW.Espèce,NEW.Date))*C.Surface,3),
            NEW.Notes
     FROM Repartir_Fertilisation_sur(NEW.Répartir,NEW.Espèce,NEW.Date) C;
END;;

DROP TRIGGER IF EXISTS Fertilisations__Saisies_UPDATE;;
CREATE TRIGGER Fertilisations__Saisies_UPDATE INSTEAD OF UPDATE ON Fertilisations__Saisies
BEGIN
    SELECT RAISE(ABORT,'Culture et Répartir non NULL') WHERE (NEW.Culture NOTNULL)AND(NEW.Répartir NOTNULL);
    --Mise à jour de la ligne fertilisation si pas de répartition.
    UPDATE Fertilisations SET
           Date=coalesce(NEW.Date,DATE('now')),
           Espèce=NEW.Espèce,
           Culture=NEW.Culture,
           Fertilisant=NEW.Fertilisant,
           Quantité=NEW.Quantité,
           N=NEW.Quantité*(SELECT N_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10
                         *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_N')/100,
           P=NEW.Quantité*(SELECT P_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10
                         *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_P')/100,
           K=NEW.Quantité*(SELECT K_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10
                         *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_K')/100,
           Notes=NEW.Notes
     WHERE (ID=OLD.ID)AND(NEW.Répartir ISNULL);
     --Suppression de la ligne fertilisation si répartition.
     DELETE FROM Fertilisations WHERE (ID=OLD.ID)AND(NEW.Répartir NOTNULL);
     --Répartition de la quantité de fertilisant.
     INSERT INTO Fertilisations (Date,
                                 Espèce,
                                 Culture,
                                 Fertilisant,
                                 Quantité,
                                 N,P,K,
                                 Notes)
     SELECT coalesce(NEW.Date,DATE('now')),
            C.Espèce, -- Si pas d'espèce saisie, répartition sur TOUTES les cultures.
            C.Culture,
            NEW.Fertilisant,
            round(NEW.Quantité/(SELECT sum(Surface) FROM Repartir_Fertilisation_sur(NEW.Répartir,NEW.Espèce,NEW.Date))*C.Surface,3),
            round(NEW.Quantité*(SELECT N_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10
                              *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_N')/100
                              /(SELECT sum(Surface) FROM Repartir_Fertilisation_sur(NEW.Répartir,NEW.Espèce,NEW.Date))*C.Surface,3),
            round(NEW.Quantité*(SELECT P_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10
                              *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_P')/100
                              /(SELECT sum(Surface) FROM Repartir_Fertilisation_sur(NEW.Répartir,NEW.Espèce,NEW.Date))*C.Surface,3),
            round(NEW.Quantité*(SELECT K_disp_pc FROM Fertilisants WHERE Fertilisant=NEW.Fertilisant)*10
                              *(SELECT Valeur FROM Params WHERE Paramètre='Ferti_coef_K')/100
                              /(SELECT sum(Surface) FROM Repartir_Fertilisation_sur(NEW.Répartir,NEW.Espèce,NEW.Date))*C.Surface,3),
            NEW.Notes
      FROM Repartir_Fertilisation_sur(NEW.Répartir,NEW.Espèce,NEW.Date) C;
END;;

DROP TRIGGER IF EXISTS Fertilisations__Saisies_DELETE;;
CREATE TRIGGER Fertilisations__Saisies_DELETE INSTEAD OF DELETE ON Fertilisations__Saisies
BEGIN
    DELETE FROM Fertilisations WHERE ID=OLD.ID;
END;;

DROP TRIGGER IF EXISTS Variétés_UPDATE_FinsPériodes;;
CREATE TRIGGER Variétés_UPDATE_FinsPériodes AFTER UPDATE ON Variétés
          WHEN (NEW.Déb_récolte NOTNULL AND NEW.Fin_récolte ISNULL)
BEGIN
    UPDATE Variétés SET
        Fin_récolte=coalesce(Fin_récolte,ItpPlusN(Déb_récolte,'1 months'))
        WHERE Variété=OLD.Variété;
END;;

DROP TRIGGER IF EXISTS ITP_UPDATE_FinsPériodes;;
CREATE TRIGGER ITP_UPDATE_FinsPériodes AFTER UPDATE ON ITP
          WHEN (NEW.Déb_semis NOTNULL AND NEW.Fin_semis ISNULL) OR
               (NEW.Déb_plantation NOTNULL AND NEW.Fin_plantation ISNULL) OR
               (NEW.Déb_récolte NOTNULL AND NEW.Fin_récolte ISNULL)
BEGIN
    UPDATE ITP SET
        Fin_semis=coalesce(Fin_semis,ItpPlusN(Déb_semis,'1 months')),
        Fin_plantation=coalesce(Fin_plantation,ItpPlusN(Déb_plantation,'1 months')),
        Fin_récolte=coalesce(Fin_récolte,ItpPlusN(Déb_récolte,'1 months'))
        WHERE IT_plante=OLD.IT_plante;
END;;

DROP TRIGGER IF EXISTS ITP__Tempo_INSERT;;
CREATE TRIGGER ITP__Tempo_INSERT INSTEAD OF INSERT ON ITP__Tempo
BEGIN
    INSERT INTO ITP (IT_plante,
                     Espèce,
                     Type_planche,
                     Déb_semis,
                     Fin_semis,
                     Déb_plantation,
                     Fin_plantation,
                     Déb_récolte,
                     Fin_récolte,
                     Nb_rangs,
                     Espacement,
                     Nb_graines_trou,
                     Dose_semis,
                     Notes)
    VALUES (NEW.IT_plante,
            NEW.Espèce,
            NEW.Type_planche,
            ItpCompleteDFPeriode(NEW.Déb_semis),
            ItpCompleteDFPeriode(NEW.Fin_semis),
            ItpCompleteDFPeriode(NEW.Déb_plantation),
            ItpCompleteDFPeriode(NEW.Fin_plantation),
            ItpCompleteDFPeriode(NEW.Déb_récolte),
            ItpCompleteDFPeriode(NEW.Fin_récolte),
            NEW.Nb_rangs,
            NEW.Espacement,
            NEW.Nb_graines_trou,
            NEW.Dose_semis,
            NEW.Notes);
END;;

DROP TRIGGER IF EXISTS ITP__Tempo_UPDATE;;
CREATE TRIGGER ITP__Tempo_UPDATE INSTEAD OF UPDATE ON ITP__Tempo
BEGIN
    UPDATE ITP SET
        IT_plante=NEW.IT_plante,
        Espèce=NEW.Espèce,
        Type_planche=NEW.Type_planche,
        Déb_semis=ItpCompleteDFPeriode(NEW.Déb_semis),
        Fin_semis=ItpCompleteDFPeriode(NEW.Fin_semis),
        Déb_plantation=ItpCompleteDFPeriode(NEW.Déb_plantation),
        Fin_plantation=ItpCompleteDFPeriode(NEW.Fin_plantation),
        Déb_récolte=ItpCompleteDFPeriode(NEW.Déb_récolte),
        Fin_récolte=ItpCompleteDFPeriode(NEW.Fin_récolte),
        Nb_rangs=NEW.Nb_rangs,
        Espacement=NEW.Espacement,
        Nb_graines_trou=NEW.Nb_graines_trou,
        Dose_semis=NEW.Dose_semis,
        Notes=NEW.Notes
     WHERE IT_plante=OLD.IT_plante;

     UPDATE Espèces SET
        Notes=NEW.N_espèce
     WHERE (Espèce=NEW.Espèce)AND(NEW.Espèce=OLD.Espèce);--If Espèce changed on ITP, can't know what Espèce.Notes have to be update.
END;;

DROP TRIGGER IF EXISTS ITP__Tempo_DELETE;;
CREATE TRIGGER ITP__Tempo_DELETE INSTEAD OF DELETE ON ITP__Tempo
BEGIN
    DELETE FROM ITP WHERE IT_plante=OLD.IT_plante;
END;;

DROP TRIGGER IF EXISTS Notes_INSERT;;
CREATE TRIGGER Notes_INSERT AFTER INSERT ON Notes
BEGIN
    UPDATE Notes SET
        Date_création=coalesce(NEW.Date_création,DATE('now'))
    WHERE ID=NEW.ID;
END;;

DROP TRIGGER IF EXISTS Notes_UPDATE;;
CREATE TRIGGER Notes_UPDATE AFTER UPDATE ON Notes
WHEN (coalesce(NEW.Date_modif,"")!=DATE('now'))
BEGIN
    UPDATE Notes SET
        Date_modif=DATE('now')
    WHERE (ID=NEW.ID)AND(SELECT Valeur!='Oui' FROM Params WHERE Paramètre='Notes_Modif_dates');
END;;

DROP TRIGGER IF EXISTS Planches_INSERT_Largeur;;
CREATE TRIGGER Planches_INSERT_Largeur AFTER INSERT ON Planches
          WHEN (NEW.Largeur ISNULL OR NEW.Largeur='?') AND (SELECT Valeur NOTNULL FROM Params WHERE Paramètre='Largeur_planches')
BEGIN
    UPDATE Planches
       SET Largeur=(SELECT Valeur FROM Params WHERE Paramètre='Largeur_planches')
     WHERE Planche=NEW.Planche;
END;;

DROP TRIGGER IF EXISTS Planches_UPDATE_Largeur;;
CREATE TRIGGER Planches_UPDATE_Largeur AFTER UPDATE ON Planches
          WHEN (NEW.Largeur ISNULL OR NEW.Largeur='?') AND (SELECT Valeur NOTNULL FROM Params WHERE Paramètre='Largeur_planches')
BEGIN
    UPDATE Planches
       SET Largeur=(SELECT Valeur FROM Params WHERE Paramètre='Largeur_planches')
     WHERE Planche=NEW.Planche;
END;;

DROP TRIGGER IF EXISTS "Rotations_détails_INSERT";;
CREATE TRIGGER "Rotations_détails_INSERT" AFTER INSERT ON Rotations_détails
BEGIN
     UPDATE Rotations
       SET Nb_années=(SELECT max(Année)
                        FROM Rotations_détails
                       WHERE Rotation=NEW.Rotation)
     WHERE Rotation=NEW.Rotation;
END;;

DROP TRIGGER IF EXISTS "Rotations_détails_UPDATE";;
CREATE TRIGGER "Rotations_détails_UPDATE" AFTER UPDATE ON Rotations_détails
BEGIN
     UPDATE Rotations
       SET Nb_années=(SELECT max(Année)
                        FROM Rotations_détails
                       WHERE Rotation=NEW.Rotation)
     WHERE Rotation=NEW.Rotation;
     UPDATE Rotations
        SET Nb_années=(SELECT max(Année)
                         FROM Rotations_détails
                        WHERE Rotation=OLD.Rotation)
      WHERE (NEW.Rotation!=OLD.Rotation)AND(Rotation=OLD.Rotation);
END;;

DROP TRIGGER IF EXISTS Rotations_détails__Tempo_INSERT;;
CREATE TRIGGER Rotations_détails__Tempo_INSERT INSTEAD OF INSERT ON Rotations_détails__Tempo
BEGIN
    INSERT INTO Rotations_détails (ID,
                                   Rotation,
                                   Année,
                                   IT_plante,
                                   Pc_planches,
                                   Fi_planches,
                                   Notes)
    VALUES (NEW.ID,
            NEW.Rotation,
            coalesce(NEW.Année,1),
            NEW.IT_plante,
            coalesce(NEW.Pc_planches,100),
            NEW.Fi_planches,
            NEW.Notes);
END;;

DROP TRIGGER IF EXISTS Rotations_détails__Tempo_UPDATE;;
CREATE TRIGGER Rotations_détails__Tempo_UPDATE INSTEAD OF UPDATE ON Rotations_détails__Tempo
BEGIN
    UPDATE Rotations_détails SET
        Rotation=NEW.Rotation,
        Année=NEW.Année,
        IT_plante=NEW.IT_plante,
        Pc_planches=coalesce(NEW.Pc_planches,100),
        Fi_planches=NEW.Fi_planches,
        Notes=NEW.Notes
     WHERE ID=OLD.ID;
END;;

DROP TRIGGER IF EXISTS Rotations_détails__Tempo_DELETE;;
CREATE TRIGGER Rotations_détails__Tempo_DELETE INSTEAD OF DELETE ON Rotations_détails__Tempo
BEGIN
    DELETE FROM Rotations_détails WHERE ID=OLD.ID;
END;;

DROP TRIGGER IF EXISTS Récoltes__Saisies_INSERT;;
CREATE TRIGGER Récoltes__Saisies_INSERT INSTEAD OF INSERT ON Récoltes__Saisies
BEGIN
    SELECT RAISE(ABORT,'NOT NULL constraint failed Récoltes.Culture/Répartir unable to fetch row') WHERE (NEW.Culture ISNULL)AND(NEW.Répartir ISNULL);
    SELECT RAISE(ABORT,'Culture et Répartir non NULL') WHERE (NEW.Culture NOTNULL)AND(NEW.Répartir NOTNULL);
    --Saisie d'un récolte pour une culture unique0
    INSERT INTO Récoltes (Date,
                          Espèce,
                          Culture,
                          Quantité,
                          Notes)
    SELECT coalesce(NEW.Date,min(Début_récolte,DATE('now'))),
          NEW.Espèce,
          C.Culture,
          NEW.Quantité,
          NEW.Notes
    FROM Cultures C
    WHERE (NEW.Répartir ISNULL)AND
          (C.Culture=NEW.Culture);
    --Répartition de la quantité récoltée.
    INSERT INTO Récoltes (Date,
                          Espèce,
                          Culture,
                          Quantité,
                          Notes)
    SELECT coalesce(NEW.Date,min(Début_récolte,DATE('now'))),
            NEW.Espèce,
            C.Culture,
            round(NEW.Quantité/(SELECT sum(Longueur) FROM Repartir_Recolte_sur(NEW.Répartir,NEW.Espèce,NEW.Date))*C.Longueur,3),
            NEW.Notes
     FROM Repartir_Recolte_sur(NEW.Répartir,NEW.Espèce,NEW.Date) C;
END;;

DROP TRIGGER IF EXISTS Récoltes__Saisies_UPDATE;;
CREATE TRIGGER Récoltes__Saisies_UPDATE INSTEAD OF UPDATE ON Récoltes__Saisies
BEGIN
    SELECT RAISE(ABORT,'Culture et Répartir non NULL') WHERE (NEW.Culture NOTNULL)AND(NEW.Répartir NOTNULL);
    --Mise à jour de la ligne récolte si pas de répartition.
    UPDATE Récoltes SET
        Date=coalesce(NEW.Date,(SELECT min(Début_récolte,DATE('now')) FROM Cultures WHERE Culture=NEW.Culture),DATE('now')),
        Espèce=NEW.Espèce,
        Culture=NEW.Culture,
        Quantité=NEW.Quantité,
        Notes=NEW.Notes
     WHERE (ID=OLD.ID)AND(NEW.Répartir ISNULL);
     --Suppression de la ligne de récolte si répartition.
     DELETE FROM Récoltes WHERE (ID=OLD.ID)AND(NEW.Répartir NOTNULL);
     --Répartition de la quantité récoltée.
     INSERT INTO Récoltes (Date,
                           Espèce,
                           Culture,
                           Quantité,
                           Notes)
     SELECT coalesce(NEW.Date,min(Début_récolte,DATE('now'))),
             NEW.Espèce,
             C.Culture,
             round(NEW.Quantité/(SELECT sum(Longueur) FROM Repartir_Recolte_sur(NEW.Répartir,NEW.Espèce,NEW.Date))*C.Longueur,3),
             NEW.Notes
      FROM Repartir_Recolte_sur(NEW.Répartir,NEW.Espèce,NEW.Date) C;
END;;

DROP TRIGGER IF EXISTS Récoltes__Saisies_DELETE;;
CREATE TRIGGER Récoltes__Saisies_DELETE INSTEAD OF DELETE ON Récoltes__Saisies
BEGIN
    DELETE FROM Récoltes WHERE ID=OLD.ID;
END;;

DROP TRIGGER IF EXISTS "Récoltes_INSERT";;
CREATE TRIGGER "Récoltes_INSERT" AFTER INSERT ON Récoltes
BEGIN
     UPDATE Cultures
       SET Début_récolte=(SELECT min(Date) FROM Recoltes_cul(NEW.Culture,Terminée,Début_récolte,Fin_récolte)),
           Récolte_faite=(CASE WHEN (SELECT count(*) FROM Recoltes_cul(NEW.Culture,Terminée,Début_récolte,Fin_récolte))>0
                               THEN coalesce(Récolte_faite,'-') -- Récolte commencée
                               ELSE NULL
                               END),
           Fin_récolte=max((SELECT max(Date) FROM Recoltes_cul(NEW.Culture,Terminée,Début_récolte,Fin_récolte)),
                           CASE WHEN (coalesce(Récolte_faite,'') NOT LIKE 'x%') THEN Fin_récolte ELSE 0 END), -- Si la culture n'est pas finie de récolter, ne pas effacer la date de fin de récolte prévue.
           Terminée=iif((coalesce(Terminée,'') NOT LIKE 'v%')AND(SELECT (E.Vivace NOTNULL) FROM Espèces E -- Passer la culture à Vivace.
                                                                 WHERE E.Espèce=Cultures.Espèce),
                        iif(Terminée ISNULL,'v','v'||Terminée),
                        Terminée)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS "Récoltes_UPDATE";;
CREATE TRIGGER "Récoltes_UPDATE" AFTER UPDATE ON Récoltes
BEGIN
     UPDATE Cultures
       SET Début_récolte=(SELECT min(Date) FROM Recoltes_cul(NEW.Culture,Terminée,Début_récolte,Fin_récolte)),
           Récolte_faite=(CASE WHEN (SELECT count(*) FROM Recoltes_cul(NEW.Culture,Terminée,Début_récolte,Fin_récolte))>0
                               THEN coalesce(Récolte_faite,'-')
                               ELSE NULL
                               END),
           Fin_récolte=max((SELECT max(Date) FROM Recoltes_cul(NEW.Culture,Terminée,Début_récolte,Fin_récolte)),
                           CASE WHEN (coalesce(Récolte_faite,'') NOT LIKE 'x%')
                                THEN Fin_récolte -- Si la culture n'est pas finie de récolter, ne pas effacer la date de fin de récolte prévue.
                                ELSE DATE('2000-01-01') END),
           Terminée=iif((coalesce(Terminée,'') NOT LIKE 'v%')AND(SELECT (E.Vivace NOTNULL) FROM Espèces E -- Passer la culture à Vivace.
                                                                 WHERE E.Espèce=Cultures.Espèce),
                        iif(Terminée ISNULL,'v','v'||Terminée),
                        Terminée)
     WHERE Culture=NEW.Culture;

     UPDATE Cultures
       SET Début_récolte=(SELECT min(Date)
                          FROM Recoltes_cul(OLD.Culture,Terminée,Début_récolte,Fin_récolte)),
           Récolte_faite=(CASE WHEN (SELECT count(*) FROM Recoltes_cul(OLD.Culture,Terminée,Début_récolte,Fin_récolte))>0
                               THEN coalesce(Récolte_faite,'-')
                               ELSE NULL
                               END),
           Fin_récolte=max((SELECT max(Date) FROM Recoltes_cul(OLD.Culture,Terminée,Début_récolte,Fin_récolte)),
                           CASE WHEN (coalesce(Récolte_faite,'') NOT LIKE 'x%')
                                THEN Fin_récolte
                                ELSE DATE('2000-01-01') END) -- Si la culture n'est pas finie de récolter, ne pas effacer la date de fin de récolte prévue.
     WHERE (NEW.Culture!=OLD.Culture)AND(Culture=OLD.Culture);
END;;

DROP TRIGGER IF EXISTS "Récoltes_DELETE";;
CREATE TRIGGER "Récoltes_DELETE" AFTER DELETE ON Récoltes
BEGIN
     UPDATE Cultures
     SET Début_récolte=CASE WHEN (SELECT count() FROM Recoltes_cul(OLD.Culture,Terminée,Début_récolte,Fin_récolte))>0
                            THEN (SELECT min(Date) FROM Recoltes_cul(OLD.Culture,Terminée,Début_récolte,Fin_récolte))
                            WHEN (SELECT (V.Déb_récolte NOTNULL) FROM Variétés V WHERE V.Variété=Cultures.Variété)
                            THEN PlanifCultureCalcDate(DATE(coalesce(Date_plantation,Date_semis),'+'||coalesce((SELECT V.PJ FROM Variétés V WHERE V.Variété=Cultures.Variété),0)||' years'),
                                                       (SELECT V.Déb_récolte FROM Variétés V WHERE V.Variété=Cultures.Variété))
                            WHEN (SELECT (ITP.Déb_récolte NOTNULL) FROM ITP WHERE ITP.IT_plante=Cultures.IT_plante)
                            THEN PlanifCultureCalcDate(DATE(coalesce(Date_plantation,Date_semis),'+'||coalesce((SELECT V.PJ FROM Variétés V WHERE V.Variété=Cultures.Variété),0)||' years'),
                                                       (SELECT ITP.Déb_récolte FROM ITP WHERE ITP.IT_plante=Cultures.IT_plante))
                            ELSE NULL
                            END,
         Récolte_faite=(CASE WHEN (SELECT count(*) FROM Recoltes_cul(OLD.Culture,Terminée,Début_récolte,Fin_récolte))>0
                             THEN coalesce(Récolte_faite,'-')
                             ELSE NULL
                             END)
     WHERE Culture=OLD.Culture;

     UPDATE Cultures
     SET Fin_récolte=CASE WHEN (SELECT count() FROM Recoltes_cul(OLD.Culture,Terminée,Début_récolte,Fin_récolte))>0
                          THEN max((SELECT max(Date) FROM Recoltes_cul(OLD.Culture,Terminée,Début_récolte,Fin_récolte)),
                                   CASE WHEN (coalesce(Récolte_faite,'') NOT LIKE 'x%') THEN Fin_récolte ELSE 0 END) -- Si la culture n'est pas finie de récolter, ne pas effacer la date de fin de récolte prévue.
                          WHEN (SELECT (V.Fin_récolte NOT NULL) FROM Variétés V WHERE V.Variété=Cultures.Variété)
                          THEN PlanifCultureCalcDate(coalesce(Début_récolte,Date_plantation,Date_semis),
                                                     (SELECT V.Fin_récolte FROM Variétés V WHERE V.Variété=Cultures.Variété))
                          WHEN (SELECT ITP.Fin_récolte FROM ITP WHERE ITP.IT_plante=Cultures.IT_plante)
                          THEN PlanifCultureCalcDate(coalesce(Début_récolte,Date_plantation,Date_semis),
                                                     (SELECT ITP.Fin_récolte FROM ITP WHERE ITP.IT_plante=Cultures.IT_plante))
                          ELSE NULL
                          END
     WHERE Culture=OLD.Culture;
END;;

DROP TRIGGER IF EXISTS Variétés_INSERT_Nb_graines_g;;
CREATE TRIGGER Variétés_INSERT_Nb_graines_g AFTER INSERT ON Variétés
          WHEN (NEW.Nb_graines_g ISNULL OR NEW.Nb_graines_g='?') AND (NEW.Espèce NOTNULL) AND
               (SELECT Nb_graines_g NOTNULL
                  FROM Espèces E
                 WHERE E.Espèce=NEW.Espèce)
BEGIN
    UPDATE Variétés
       SET Nb_graines_g=coalesce( (SELECT Nb_graines_g
                                     FROM Espèces E
                                    WHERE E.Espèce=NEW.Espèce), 0)
     WHERE Variété=NEW.Variété;
END;;

DROP TRIGGER IF EXISTS Variétés_UPDATE_Nb_graines_g;;
CREATE TRIGGER Variétés_UPDATE_Nb_graines_g AFTER UPDATE ON Variétés
          WHEN (NEW.Nb_graines_g ISNULL OR NEW.Nb_graines_g='?') AND (NEW.Espèce NOTNULL) AND
               (SELECT Nb_graines_g NOTNULL
                  FROM Espèces E
                 WHERE E.Espèce=NEW.Espèce)
BEGIN
    UPDATE Variétés
       SET Nb_graines_g=coalesce( (SELECT Nb_graines_g
                                     FROM Espèces E
                                    WHERE E.Espèce=NEW.Espèce), 0)
     WHERE Variété=NEW.Variété;
END;;

DROP TRIGGER IF EXISTS Variétés__inv_et_cde;;
CREATE TRIGGER Variétés__inv_et_cde_UPDATE INSTEAD OF UPDATE ON Variétés__inv_et_cde
BEGIN
    UPDATE Variétés SET
        Qté_stock=NEW.Qté_stock,
        Qté_cde=NEW.Qté_cde,
        Fournisseur=NEW.Fournisseur,
        Nb_graines_g=NEW.Nb_graines_g,
        Notes=NEW.Notes
     WHERE Variété=OLD.Variété;

     UPDATE Espèces SET
         FG=NEW.FG,
        Notes=NEW.N_espèce
     WHERE (Espèce=NEW.Espèce)AND(NEW.Espèce=OLD.Espèce);

     UPDATE Familles SET
        Notes=NEW.N_famille
     WHERE Famille=NEW.Famille;
END;;

-- COMMIT TRANSACTION;;
)#");
