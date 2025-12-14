ALTER TABLE Rotations ADD COLUMN Active BOOL;
UPDATE Rotations SET Active='x';
