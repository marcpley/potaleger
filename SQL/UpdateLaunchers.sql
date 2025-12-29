-- Colors

UPDATE fda_l_schema AS FL SET
    color=(SELECT FT.color FROM fda_t_schema FT WHERE FT.name=FL.name)
WHERE (color ISNULL)AND(name NOTNULL);

UPDATE fda_l_schema AS FL SET
    color=(SELECT color FROM fda_l_schema FL2 WHERE FL2.parent=FL.launcher_name) -- color of 1rst menu entries.
WHERE (color ISNULL)AND(type='Submenu')AND(parent!='Main menu');
