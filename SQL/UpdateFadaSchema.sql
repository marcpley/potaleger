--------------------------
--------------------------
-- Automatic fda schema --
--------------------------
--------------------------

------------------
-- Set tv_type --
------------------

--Update 'View' tv_type to 'View as table' if a INSTEAD OF UPDATE trigger exists.
UPDATE fada_t_schema SET
    tv_type='View as table'
WHERE (tv_type='View')AND
      ((SELECT count() FROM sqlite_schema
        WHERE (type='trigger')AND
              (tbl_name=fada_t_schema.tv_name)AND
              (sql LIKE 'CREATE TRIGGER % INSTEAD OF UPDATE ON '||fada_t_schema.tv_name||' %'))>0);

-- Set tv_type in fada_f_schema
UPDATE fada_f_schema SET
    tv_type=(SELECT T.tv_type FROM fada_t_schema T WHERE T.tv_name=fada_f_schema.tv_name);

--------------------
-- Set field_type --
--------------------

-- Set field_type of view fields with properties of real table fields
UPDATE fada_f_schema SET
    field_type=(SELECT A1.field_type FROM fada_f_schema A1
                WHERE (A1.tv_type='Table')AND
                      (A1.field_name=fada_f_schema.field_name)AND
                      (fada_f_schema.tv_name LIKE A1.tv_name||'__%'))
WHERE (field_type ISNULL)AND(tv_type LIKE 'View%');
-- Set field_type of View field with field_type in real tables if they return the same field_type.
UPDATE fada_f_schema SET
    field_type=(SELECT A1.field_type FROM fada_f_schema A1
                WHERE (A1.tv_type='Table')AND
                      (A1.field_name=fada_f_schema.field_name)AND
                      ((SELECT count() FROM (SELECT DISTINCT field_type
                        FROM fada_f_schema A2
                        WHERE (A2.tv_type='Table')AND
                              (A2.field_name=fada_f_schema.field_name)))=1))
WHERE (field_type ISNULL)AND(tv_type LIKE 'View%');
-- Set field_type for fields with draw property
UPDATE fada_f_schema SET
    field_type='DRAWED'
WHERE (draw NOTNULL);

------------------
-- Set readonly --
------------------

-- Set readonly false (NULL) if a INSTEAD OF UPDATE trigger exists with field_name=NEW.field_name. For more spécific update, the readonly must be set manually.
UPDATE fada_f_schema SET
    readonly=NULL
WHERE (tv_type='View as table')AND
      ((SELECT count() FROM sqlite_schema
        WHERE (type='trigger')AND
              (tbl_name=fada_f_schema.tv_name)AND
              ((sql LIKE '% INSTEAD OF UPDATE ON '||fada_f_schema.tv_name||'%NEW.'||fada_f_schema.field_name||' %')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fada_f_schema.tv_name||'%NEW.'||fada_f_schema.field_name||',%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fada_f_schema.tv_name||'%NEW.'||fada_f_schema.field_name||')%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fada_f_schema.tv_name||'%NEW.'||fada_f_schema.field_name||'+%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fada_f_schema.tv_name||'%NEW.'||fada_f_schema.field_name||'-%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fada_f_schema.tv_name||'%NEW.'||fada_f_schema.field_name||'*%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fada_f_schema.tv_name||'%NEW.'||fada_f_schema.field_name||'/%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fada_f_schema.tv_name||'%NEW.'||fada_f_schema.field_name||'||%')))>0);

-- Set type of readonly (x, Calculated) with the real table but don't set readonly to false (NULL).
UPDATE fada_f_schema SET
    readonly=coalesce((SELECT A1.readonly FROM fada_f_schema A1
                       WHERE (A1.tv_type='Table')AND
                             (A1.field_name=fada_f_schema.field_name)AND
                             (fada_f_schema.tv_name LIKE A1.tv_name||'__%')),fada_f_schema.readonly)
WHERE (tv_type='View as table');

-- Set readonly true for drawed fields.
UPDATE fada_f_schema SET
    readonly='x'
WHERE (draw NOTNULL)AND(readonly ISNULL);

------------------------------------------------------------------------------
-- Set other properties of view fields with properties of real table fields --
------------------------------------------------------------------------------

-- natural_sort
UPDATE fada_f_schema SET
    natural_sort=(SELECT A1.natural_sort FROM fada_f_schema A1
               WHERE (A1.tv_type='Table')AND
                     (A1.field_name=fada_f_schema.field_name)AND
                     (fada_f_schema.tv_name LIKE A1.tv_name||'__%'))
WHERE (natural_sort ISNULL)AND(tv_type LIKE 'View%');

-- base_data
UPDATE fada_f_schema SET
    base_data=(SELECT A1.base_data FROM fada_f_schema A1
               WHERE (A1.tv_type='Table')AND
                     (A1.field_name=fada_f_schema.field_name)AND
                     (fada_f_schema.tv_name LIKE A1.tv_name||'__%'))
WHERE (base_data ISNULL)AND(tv_type LIKE 'View%');

-- col_width
UPDATE fada_f_schema SET
    col_width=(SELECT A1.col_width FROM fada_f_schema A1
               WHERE (A1.tv_type='Table')AND
                     (A1.field_name=fada_f_schema.field_name)AND
                     (fada_f_schema.tv_name LIKE A1.tv_name||'__%'))
WHERE (col_width ISNULL)AND(tv_type LIKE 'View%');

-- cond_formats
UPDATE fada_f_schema SET
    cond_formats=(SELECT A1.cond_formats FROM fada_f_schema A1
                  WHERE (A1.tv_type='Table')AND
                        (A1.field_name=fada_f_schema.field_name)AND
                        (fada_f_schema.tv_name LIKE A1.tv_name||'__%'))
WHERE (cond_formats ISNULL)AND(tv_type LIKE 'View%');

-- combo
UPDATE fada_f_schema SET
    combo=(SELECT A1.combo FROM fada_f_schema A1
           WHERE (A1.tv_type='Table')AND
                 (A1.field_name=fada_f_schema.field_name)AND
                 (fada_f_schema.tv_name LIKE A1.tv_name||'__%'))
WHERE (combo ISNULL)AND(tv_type LIKE 'View%');

-- money
UPDATE fada_f_schema SET
    money=(SELECT A1.money FROM fada_f_schema A1
           WHERE (A1.tv_type='Table')AND
                 (A1.field_name=fada_f_schema.field_name)AND
                 (fada_f_schema.tv_name LIKE A1.tv_name||'__%'))
WHERE (money ISNULL)AND(tv_type LIKE 'View%');

-- multiline
UPDATE fada_f_schema SET
    multiline=(SELECT A1.multiline FROM fada_f_schema A1
               WHERE (A1.tv_type='Table')AND
                     (A1.field_name=fada_f_schema.field_name)AND
                     (fada_f_schema.tv_name LIKE A1.tv_name||'__%'))
WHERE (multiline ISNULL)AND(tv_type LIKE 'View%');

-- unit
UPDATE fada_f_schema SET
    unit=(SELECT A1.unit FROM fada_f_schema A1
           WHERE (A1.tv_type='Table')AND
                 (A1.field_name=fada_f_schema.field_name)AND
                 (fada_f_schema.tv_name LIKE A1.tv_name||'__%'))
WHERE (unit ISNULL)AND(tv_type LIKE 'View%');


------------
-- Colors --
------------
--Params table.
UPDATE fada_f_schema SET color='#7f7f7f'
WHERE ((field_name IN('Valeur'))AND(tv_name LIKE 'Params%'));

-- Set table color if not field color.
UPDATE fada_f_schema SET
    color=(SELECT F.color FROM fada_t_schema F WHERE F.tv_name=fada_f_schema.tv_name)
WHERE (color ISNULL);

-- Master color
UPDATE fada_f_schema SET
    color=(SELECT F.color FROM fada_t_schema F WHERE F.tv_name=fada_f_schema.master_table)
WHERE (master_table NOTNULL);

-- Hide color columns
UPDATE fada_f_schema SET hidden='x' WHERE (field_name='color')AND(tv_name!='fada_f_schema');

-- Standard colors for cond_formats
UPDATE fada_f_schema SET cond_formats=replace(cond_formats,'#darkRed#','#b70000');
UPDATE fada_f_schema SET cond_formats=replace(cond_formats,'#lightRed#','#ff7171');
UPDATE fada_f_schema SET cond_formats=replace(cond_formats,'#darkGreen#','#009e00');
UPDATE fada_f_schema SET cond_formats=replace(cond_formats,'#lightGreen#','#77ff77');
UPDATE fada_f_schema SET cond_formats=replace(cond_formats,'#darkBlue#','#0000ee');
UPDATE fada_f_schema SET cond_formats=replace(cond_formats,'#lightBlue#','#b5b5ff');

--------------------
-- Other settings --
--------------------

----------------------------------------------
-- SQLite tables and views not FDA declared --
----------------------------------------------

INSERT INTO fada_t_schema (tv_name,tv_type,description)
SELECT name,'Table','No FDA properties'
FROM sqlite_schema
WHERE (type='table')AND(name NOT LIKE 'sqlite_%')AND NOT(name IN(SELECT tv_name FROM fada_t_schema WHERE tv_type='Table'));

INSERT INTO fada_t_schema (tv_name,tv_type,description)
SELECT name,'View','No FDA properties'
FROM sqlite_schema
WHERE (type='view')AND NOT(name IN(SELECT tv_name FROM fada_t_schema WHERE tv_type LIKE 'View%'));
