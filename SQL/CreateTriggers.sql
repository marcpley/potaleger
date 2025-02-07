QString sDDLTriggers = QStringLiteral(R"#(
-- BEGIN TRANSACTION;;

DROP TRIGGER IF EXISTS Cultures_INSERT_Planifier;;
CREATE TRIGGER Cultures_INSERT_Planifier AFTER INSERT ON Cultures
          WHEN (NEW.Terminée ISNULL) AND
               ((NEW.D_planif ISNULL) OR ((length(NEW.D_planif)=4)AND(CAST(NEW.D_planif AS INTEGER) BETWEEN 2000 AND 2100))) AND
               (NEW.IT_plante NOTNULL)
BEGIN
    DELETE FROM Params WHERE Paramètre LIKE 'temp_%';
    INSERT INTO Params (Paramètre, Valeur)
    VALUES ('temp_date_planif', CASE
                                WHEN ((length(NEW.D_planif)=4)AND(CAST(NEW.D_planif AS INTEGER) BETWEEN 2000 AND 2100))
                                THEN    CASE
                                        WHEN (SELECT (ITP.Déb_plantation NOTNULL)AND(ITP.Déb_plantation<ITP.Déb_semis) FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante)
                                        THEN CAST(CAST(NEW.D_planif AS INTEGER)-1 AS TEXT)||'-01-01' -- Semis sous abris l'année précédent la mise en place.
                                        ELSE NEW.D_planif||'-01-01' END
                                ELSE DATE() END);

    UPDATE Cultures SET
        Date_semis=CASE WHEN Semis_fait NOTNULL THEN Date_semis
                        WHEN (SELECT (ITP.Déb_semis NOTNULL) FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Déb_semis FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante))
                        ELSE NULL
                        END,
        D_planif=DATE()-- Nécessaire pour ne pas provoquer un appel récursif.
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=coalesce((SELECT Date_semis FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Date_plantation=CASE WHEN Plantation_faite NOTNULL THEN Date_plantation
                        WHEN (SELECT (ITP.Déb_plantation NOTNULL) FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Déb_plantation FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=coalesce((SELECT coalesce(Date_plantation,Date_semis) FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Début_récolte=  CASE WHEN Récolte_faite NOTNULL THEN Début_récolte
                        WHEN (SELECT (ITP.Déb_récolte NOTNULL) FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Déb_récolte FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=coalesce((SELECT coalesce(Début_récolte,Date_plantation,Date_semis) FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Fin_récolte=  CASE WHEN Récolte_faite NOTNULL THEN Fin_récolte
                        WHEN (SELECT (ITP.Fin_récolte NOTNULL) FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Fin_récolte FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    DELETE FROM Params WHERE Paramètre LIKE 'temp_%';
END;;

DROP TRIGGER IF EXISTS Cultures_UPDATE_Planifier;;
CREATE TRIGGER Cultures_UPDATE_Planifier AFTER UPDATE ON Cultures
          WHEN (NEW.Terminée ISNULL) AND
               ((NEW.D_planif ISNULL) OR ((length(NEW.D_planif)=4)AND(CAST(NEW.D_planif AS INTEGER) BETWEEN 2000 AND 2100))) AND
               (NEW.IT_plante NOTNULL)
BEGIN--Code identique à INSERT
    DELETE FROM Params WHERE Paramètre LIKE 'temp_%';
    INSERT INTO Params (Paramètre, Valeur)
    VALUES ('temp_date_planif', CASE
                                WHEN ((length(NEW.D_planif)=4)AND(CAST(NEW.D_planif AS INTEGER) BETWEEN 2000 AND 2100))
                                THEN    CASE
                                        WHEN (SELECT (ITP.Déb_plantation NOTNULL)AND(ITP.Déb_plantation<ITP.Déb_semis) FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante)
                                        THEN CAST(CAST(NEW.D_planif AS INTEGER)-1 AS TEXT)||'-01-01' -- Semis sous abris l'année précédent la mise en place.
                                        ELSE NEW.D_planif||'-01-01' END
                                ELSE DATE() END);

    UPDATE Cultures SET
        Date_semis=CASE WHEN Semis_fait NOTNULL THEN Date_semis
                        WHEN (SELECT (ITP.Déb_semis NOTNULL) FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Déb_semis FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante))
                        ELSE NULL
                        END,
        D_planif=DATE()-- Nécessaire pour ne pas provoquer un appel récursif.
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=coalesce((SELECT Date_semis FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Date_plantation=CASE WHEN Plantation_faite NOTNULL THEN Date_plantation
                        WHEN (SELECT (ITP.Déb_plantation NOTNULL) FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Déb_plantation FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=coalesce((SELECT coalesce(Date_plantation,Date_semis) FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Début_récolte=  CASE WHEN Récolte_faite NOTNULL THEN Début_récolte
                        WHEN (SELECT (ITP.Déb_récolte NOTNULL) FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Déb_récolte FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    UPDATE Params SET Valeur=coalesce((SELECT coalesce(Début_récolte,Date_plantation,Date_semis) FROM Cultures WHERE Culture=NEW.Culture),Valeur)
    WHERE Paramètre='temp_date_planif';

    UPDATE Cultures SET
        Fin_récolte=  CASE WHEN Récolte_faite NOTNULL THEN Fin_récolte
                        WHEN (SELECT (ITP.Fin_récolte NOTNULL) FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante)
                        THEN PlanifCultureCalcDate( (SELECT Valeur FROM Params WHERE Paramètre='temp_date_planif'),
                                                    (SELECT ITP.Fin_récolte FROM ITP WHERE ITP.IT_Plante=NEW.IT_Plante))
                        ELSE NULL
                        END
    WHERE Culture=NEW.Culture;

    DELETE FROM Params WHERE Paramètre LIKE 'temp_%';
END;;

DROP TRIGGER IF EXISTS Cultures_UPDATE_Espacement;;
CREATE TRIGGER Cultures_UPDATE_Espacement AFTER UPDATE ON Cultures
          WHEN (NEW.Espacement ISNULL) AND (NEW.IT_plante NOTNULL)
BEGIN
    UPDATE Cultures
       SET Espacement=coalesce( (SELECT Espacement
                                   FROM ITP I
                                  WHERE I.IT_plante=NEW.IT_plante), 0)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures_UPDATE_Longueur;;
CREATE TRIGGER Cultures_UPDATE_Longueur AFTER UPDATE ON Cultures
          WHEN (NEW.Longueur ISNULL) AND (NEW.Planche NOTNULL)
BEGIN
    UPDATE Cultures
       SET Longueur=coalesce( (SELECT Longueur
                                 FROM Planches P
                                WHERE P.Planche=NEW.Planche), 0)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures_UPDATE_Nb_rangs;;
CREATE TRIGGER Cultures_UPDATE_Nb_rangs AFTER UPDATE ON Cultures
          WHEN (NEW.Nb_rangs ISNULL) AND (NEW.IT_plante NOTNULL)
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
                            IT_plante,
                            Variété,
                            Fournisseur,
                            D_planif,
                            Date_semis,
                            Semis_fait,
                            Date_Plantation,
                            Plantation_faite,
                            Début_récolte,
                            Fin_récolte,
                            Récolte_faite,
                            Terminée,
                            Longueur,
                            Nb_rangs,
                            Espacement,
                            Notes)
    VALUES (--NEW.Culture,
            NEW.Planche,
            NEW.IT_plante,
            NEW.Variété,
            NEW.Fournisseur,
            NEW.D_planif,
            NEW.Date_semis,
            NEW.Semis_fait,
            NEW.Date_Plantation,
            NEW.Plantation_faite,
            NEW.Début_récolte,
            NEW.Fin_récolte,
            NEW.Récolte_faite,
            NEW.Terminée,
            NEW.Longueur,
            NEW.Nb_rangs,
            NEW.Espacement,
            NEW.Notes);
END;;


DROP TRIGGER IF EXISTS Cultures__non_terminées_UPDATE;;
CREATE TRIGGER Cultures__non_terminées_UPDATE INSTEAD OF UPDATE ON Cultures__non_terminées
BEGIN
    UPDATE Cultures SET
        Planche=NEW.Planche,
        IT_plante=NEW.IT_plante,
        Variété=NEW.Variété,
        Fournisseur=NEW.Fournisseur,
        D_planif=NEW.D_planif,
        Date_semis=NEW.Date_semis,
        Semis_fait=NEW.Semis_fait,
        Date_Plantation=NEW.Date_Plantation,
        Plantation_faite=NEW.Plantation_faite,
        Début_récolte=NEW.Début_récolte,
        Fin_récolte=NEW.Fin_récolte,
        Récolte_faite=NEW.Récolte_faite,
        Terminée=NEW.Terminée,
        Longueur=NEW.Longueur,
        Nb_rangs=NEW.Nb_rangs,
        Espacement=NEW.Espacement,
        Notes=NEW.Notes
    WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__non_terminées_DELETE;;
CREATE TRIGGER Cultures__non_terminées_DELETE INSTEAD OF DELETE ON Cultures__non_terminées
BEGIN
    DELETE FROM Cultures WHERE Culture=OLD.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__Semis_à_faire_UPDATE;;
CREATE TRIGGER Cultures__Semis_à_faire_UPDATE INSTEAD OF UPDATE ON Cultures__Semis_à_faire
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
        Notes=NEW.Notes
    WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__Plantations_à_faire_UPDATE;;
CREATE TRIGGER Cultures__Plantations_à_faire_UPDATE INSTEAD OF UPDATE ON Cultures__Plantations_à_faire
BEGIN
    UPDATE Cultures SET
        Planche=NEW.Planche,
        Variété=NEW.Variété,
        Fournisseur=NEW.Fournisseur,
        D_planif=NEW.D_planif,
        Date_semis=NEW.Date_semis,
        Semis_fait=NEW.Semis_fait,
        Date_Plantation=NEW.Date_Plantation,
        Plantation_faite=NEW.Plantation_faite,
        Longueur=NEW.Longueur,
        Nb_rangs=NEW.Nb_rangs,
        Espacement=NEW.Espacement,
        Notes=NEW.Notes
    WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__Récoltes_à_faire_UPDATE;;
CREATE TRIGGER Cultures__Récoltes_à_faire_UPDATE INSTEAD OF UPDATE ON Cultures__Récoltes_à_faire
BEGIN
    UPDATE Cultures SET
        Date_semis=NEW.Date_semis,
        Semis_fait=NEW.Semis_fait,
        Date_Plantation=NEW.Date_Plantation,
        Plantation_faite=NEW.Plantation_faite,
        Début_récolte=NEW.Début_récolte,
        Fin_récolte=NEW.Fin_récolte,
        Récolte_faite=NEW.Récolte_faite,
        Terminée=NEW.Terminée,
        Notes=NEW.Notes
    WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Cultures__à_terminer_UPDATE;;
CREATE TRIGGER Cultures__à_terminer_UPDATE INSTEAD OF UPDATE ON Cultures__à_terminer
BEGIN
    UPDATE Cultures SET
        Date_semis=NEW.Date_semis,
        Date_Plantation=NEW.Date_Plantation,
        Début_récolte=NEW.Début_récolte,
        Fin_récolte=NEW.Fin_récolte,
        Récolte_faite=NEW.Récolte_faite,
        Terminée=NEW.Terminée,
        Notes=NEW.Notes
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS ITP_UPDATE_FinsPériodes;;
CREATE TRIGGER ITP_UPDATE_FinsPériodes AFTER UPDATE ON ITP
          WHEN (NEW.Déb_semis NOTNULL AND NEW.Fin_semis ISNULL) OR
               (NEW.Déb_plantation NOTNULL AND NEW.Fin_plantation ISNULL) OR
               (NEW.Déb_récolte NOTNULL AND NEW.Fin_récolte ISNULL)
BEGIN
    UPDATE ITP SET
        Fin_semis=coalesce(Fin_semis,ItpPlus15jours(Déb_semis)),
        Fin_plantation=coalesce(Fin_plantation,ItpPlus15jours(Déb_plantation)),
        Fin_récolte=coalesce(Fin_récolte,ItpPlus15jours(Déb_récolte))
        WHERE IT_plante=NEW.IT_plante;
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
            NEW.Déb_semis,
            NEW.Fin_semis,
            NEW.Déb_plantation,
            NEW.Fin_plantation,
            NEW.Déb_récolte,
            NEW.Fin_récolte,
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
        Déb_semis=NEW.Déb_semis,
        Fin_semis=NEW.Fin_semis,
        Déb_plantation=NEW.Déb_plantation,
        Fin_plantation=NEW.Fin_plantation,
        Déb_récolte=NEW.Déb_récolte,
        Fin_récolte=NEW.Fin_récolte,
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
      WHERE Rotation=OLD.Rotation;
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
            NEW.Année,
            NEW.IT_plante,
            NEW.Pc_planches,
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
        Pc_planches=NEW.Pc_planches,
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
    INSERT INTO Récoltes (ID,
                          Date,
                          Espèce,
                          Culture,
                          Quantité,
                          Notes)
    VALUES (NEW.ID,
            coalesce(NEW.Date,DATE('now')),
            NEW.Espèce,
            NEW.Culture,
            NEW.Quantité,
            NEW.Notes);
END;;

DROP TRIGGER IF EXISTS Récoltes__Saisies_UPDATE;;
CREATE TRIGGER Récoltes__Saisies_UPDATE INSTEAD OF UPDATE ON Récoltes__Saisies
BEGIN
    UPDATE Récoltes SET
        Date=NEW.Date,
        Espèce=NEW.Espèce,
        Culture=NEW.Culture,
        Quantité=NEW.Quantité,
        Notes=NEW.Notes
     WHERE ID=OLD.ID;
END;;

DROP TRIGGER IF EXISTS Récoltes__Saisies_DELETE;;
CREATE TRIGGER Récoltes__Saisies_DELETE INSTEAD OF DELETE ON Récoltes__Saisies
BEGIN
    DELETE FROM Récoltes WHERE ID=OLD.ID;
END;;

DROP TRIGGER IF EXISTS Variétés_UPDATE_Nb_graines_g;;
CREATE TRIGGER Variétés_UPDATE_Nb_graines_g AFTER UPDATE ON Variétés
          WHEN (NEW.Nb_graines_g ISNULL) AND (NEW.Espèce NOTNULL) AND
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
     WHERE Variété=NEW.Variété;

     UPDATE Espèces SET
         FG=NEW.FG,
        Notes=NEW.N_espèce
     WHERE Espèce=NEW.Espèce;

     UPDATE Familles SET
        Notes=NEW.N_famille
     WHERE Famille=NEW.Famille;
END;;

-- COMMIT TRANSACTION;;
)#");
