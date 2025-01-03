QString sDDLTriggers = QStringLiteral(R"#(
BEGIN TRANSACTION;;

DROP TRIGGER IF EXISTS Planifier_UPDATE;;
CREATE TRIGGER Planifier_UPDATE AFTER UPDATE ON Cultures
          WHEN (NEW.Terminée ISNULL) AND
               ((NEW."D_planif" ISNULL)     )AND -- OR PlanifAnneeSpecifiee(NEW."D_planif")) AND
               (NEW.IT_plante NOTNULL)
BEGIN
    SELECT '<PlanifCulture>'; --PlanifCulture(NEW.Culture, NEW.IT_plante, NEW.D_planif)
END;;

DROP TRIGGER IF EXISTS Planifier_INSERT;;
CREATE TRIGGER Planifier_INSERT AFTER INSERT ON Cultures
          WHEN (NEW.Terminée ISNULL) AND
               ((NEW."D_planif" ISNULL)     )AND -- OR PlanifAnneeSpecifiee(NEW."D_planif")) AND
               (NEW.IT_plante NOTNULL)
BEGIN
    SELECT '<PlanifCulture>'; --PlanifCulture(NEW.Culture, NEW.IT_plante, NEW.D_planif)
END;;

DROP TRIGGER IF EXISTS Longueur;;
CREATE TRIGGER Longueur AFTER UPDATE ON Cultures
          WHEN (NEW.Longueur ISNULL) AND (NEW.Planche NOTNULL)
BEGIN
    UPDATE Cultures
       SET Longueur=coalesce( (SELECT Longueur
                                 FROM Planches P
                                WHERE P.Planche=NEW.Planche), 0)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Nb_rangs;;
CREATE TRIGGER Nb_rangs AFTER UPDATE ON Cultures
          WHEN (NEW.Nb_rangs ISNULL) AND (NEW.IT_plante NOTNULL)
BEGIN
    UPDATE Cultures
       SET Nb_rangs=coalesce( (SELECT Nb_rangs
                                 FROM ITP I
                                WHERE I.IT_plante=NEW.IT_plante), 0)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS Espacement;;
CREATE TRIGGER Espacement AFTER UPDATE ON Cultures
          WHEN (NEW.Espacement ISNULL) AND (NEW.IT_plante NOTNULL)
BEGIN
    UPDATE Cultures
       SET Espacement=coalesce( (SELECT Espacement
                                   FROM ITP I
                                  WHERE I.IT_plante=NEW.IT_plante), 0)
     WHERE Culture=NEW.Culture;
END;;

DROP TRIGGER IF EXISTS FinsPériodes;;
CREATE TRIGGER FinsPériodes AFTER UPDATE ON ITP
          WHEN (NEW.Déb_semis NOTNULL AND NEW.Fin_semis ISNULL) OR
               (NEW.Déb_semis ISNULL AND NEW.Fin_semis NOTNULL) OR
               (NEW.Déb_plantation NOTNULL AND NEW.Fin_plantation ISNULL) OR
               (NEW.Déb_plantation ISNULL AND NEW.Fin_plantation NOTNULL) OR
               (NEW.Déb_récolte NOTNULL AND NEW.Fin_récolte ISNULL)
BEGIN
    SELECT '<ItpFinsPeriodes>'; --ItpFinsPeriodes(NEW.IT_plante)
END;;

DROP TRIGGER IF EXISTS "";;
CREATE TRIGGER "" AFTER UPDATE ON Rotations_détails
BEGIN
    UPDATE Rotations
       SET Nb_années=(SELECT max(Année)
                        FROM Rotations_détails
                       WHERE Rotation=NEW.Rotation)
     WHERE Rotation=NEW.Rotation;
END;;

DROP TRIGGER IF EXISTS Nb_graines_g;;
CREATE TRIGGER Nb_graines_g AFTER UPDATE ON Variétés
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

COMMIT TRANSACTION;;
)#");
