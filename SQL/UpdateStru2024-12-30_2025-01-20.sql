QString sDDL20250120 = QStringLiteral(R"#(
BEGIN TRANSACTION;

ALTER TABLE Rotations_détails DROP COLUMN Nb_planches;

COMMIT TRANSACTION;
)#");
