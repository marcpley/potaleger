QString sDDLTriggers = QStringLiteral(R"#(
BEGIN TRANSACTION;;

DROP TRIGGER IF EXISTS Cultures_INSERT_Planifier;;
CREATE TRIGGER Cultures_INSERT_Planifier AFTER INSERT ON Cultures
          WHEN (NEW.Terminée ISNULL) AND
               ((NEW."D_planif" ISNULL)     )AND -- OR PlanifAnneeSpecifiee(NEW."D_planif")) AND
               (NEW.IT_plante NOTNULL)
BEGIN
    SELECT '<PlanifCulture>'; --PlanifCulture(NEW.Culture, NEW.IT_plante, NEW.D_planif)
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

DROP TRIGGER IF EXISTS Cultures_UPDATE_Planifier;;
CREATE TRIGGER Cultures_UPDATE_Planifier AFTER UPDATE ON Cultures
          WHEN (NEW.Terminée ISNULL) AND
               ((NEW."D_planif" ISNULL)     )AND -- OR PlanifAnneeSpecifiee(NEW."D_planif")) AND
               (NEW.IT_plante NOTNULL)
BEGIN
    SELECT '<PlanifCulture>'; --PlanifCulture(NEW.Culture, NEW.IT_plante, NEW.D_planif)
END;;

DROP TRIGGER IF EXISTS Cultures__non_terminées_UPDATE;;
CREATE TRIGGER Cultures__non_terminées_UPDATE INSTEAD OF UPDATE ON Cultures__non_terminées
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
        Longueur=NEW.Longueur,
        Nb_rangs=NEW.Nb_rangs,
        Espacement=NEW.Espacement,
        Nb_rangs=NEW.Nb_rangs,
        Notes=NEW.Notes
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS ITP_UPDATE_FinsPériodes;;
CREATE TRIGGER ITP_UPDATE_FinsPériodes AFTER UPDATE ON ITP
          WHEN (NEW.Déb_semis NOTNULL AND NEW.Fin_semis ISNULL) OR
               (NEW.Déb_semis ISNULL AND NEW.Fin_semis NOTNULL) OR
               (NEW.Déb_plantation NOTNULL AND NEW.Fin_plantation ISNULL) OR
               (NEW.Déb_plantation ISNULL AND NEW.Fin_plantation NOTNULL) OR
               (NEW.Déb_récolte NOTNULL AND NEW.Fin_récolte ISNULL)
BEGIN
    SELECT '<ItpFinsPeriodes>'; --ItpFinsPeriodes(NEW.IT_plante)
END;;

DROP TRIGGER IF EXISTS "Rotations_détails_UPDATE";;
CREATE TRIGGER "Rotations_détails_UPDATE" AFTER UPDATE ON Rotations_détails
BEGIN
    UPDATE Rotations
       SET Nb_années=(SELECT max(Année)
                        FROM Rotations_détails
                       WHERE Rotation=NEW.Rotation)
     WHERE Rotation=NEW.Rotation;
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
        Notes=NEW.Notes
     WHERE Variété=NEW.Variété;

     UPDATE Espèces SET
         FG=NEW.FG,
         Nb_graines_g=NEW.Nb_graines_g,
        Notes=NEW.N_espèce
     WHERE Espèce=NEW.Espèce;

     UPDATE Familles SET
        Notes=NEW.N_famille
     WHERE Famille=NEW.Famille;
END;;

COMMIT TRANSACTION;;
)#");
