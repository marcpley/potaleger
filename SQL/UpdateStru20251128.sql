ALTER TABLE Rotations ADD COLUMN Active BOOL;
UPDATE Rotations SET Active='x';

ALTER TABLE Fertilisations ADD COLUMN Planche TEXT;
UPDATE Fertilisations SET Planche=(SELECT C.Planche FROM Cultures C WHERE C.Culture=Fertilisations.Culture);
