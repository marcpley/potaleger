----------------------
----------------------
-- Model fda schema --
----------------------
----------------------



-- No color fields
UPDATE fada_f_schema SET color=NULL
WHERE --(field_name LIKE 'TEMPO%')OR
      (field_name LIKE 'Prod_%')OR
      (field_name LIKE 'Couv_%')OR
      (field_name LIKE 'Graph%');

-- Standard colors for Graph fields
-- UPDATE fada_f_schema SET cond_formats=replace(cond_formats,'#darkRed#','#b70000');

UPDATE fada_f_schema SET draw=replace(draw,'GraphTitres12Mois',(SELECT value FROM Prm_draw WHERE name='GraphTitres12Mois'))
WHERE (field_name='Graph');
UPDATE fada_f_schema SET draw=replace(draw,'GraphTitres24Mois',(SELECT value FROM Prm_draw WHERE name='GraphTitres24Mois'))
WHERE (field_name='Graph');
UPDATE fada_f_schema SET draw=replace(draw,'GraphTitres36Mois',(SELECT value FROM Prm_draw WHERE name='GraphTitres36Mois'))
WHERE (field_name='Graph');
