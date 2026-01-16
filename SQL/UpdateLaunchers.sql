-- Colors

UPDATE fada_launchers AS FL SET
    color=(SELECT FT.color FROM fada_t_schema FT WHERE FT.name=FL.name)
WHERE (color ISNULL)AND(name NOTNULL);

UPDATE fada_launchers AS FL SET
    color=(SELECT color FROM fada_launchers FL2 WHERE FL2.parent=FL.launcher_name) -- color of 1rst menu entries.
WHERE (color ISNULL)AND(type='Submenu')AND(parent!='Main menu');
