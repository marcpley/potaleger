--------------------------
--------------------------
-- Automatic fda schema --
--------------------------
--------------------------

------------------
-- Set tbl_type --
------------------

--Update 'View' tbl_type to 'View as table' if a INSTEAD OF UPDATE trigger exists.
UPDATE fda_t_schema SET
    tbl_type='View as table'
WHERE (tbl_type='View')AND
      ((SELECT count() FROM sqlite_schema
        WHERE (type='trigger')AND
              (tbl_name=fda_t_schema.name)AND
              (sql LIKE 'CREATE TRIGGER % INSTEAD OF UPDATE ON '||fda_t_schema.name||' %'))>0);

-- Set tbl_type in fda_f_schema
UPDATE fda_f_schema SET
    tbl_type=(SELECT T.tbl_type FROM fda_t_schema T WHERE T.name=fda_f_schema.name);

------------------
-- Set readonly --
------------------

-- Set readonly false (NULL) if a INSTEAD OF UPDATE trigger exists with field_name=NEW.field_name. For more spécific update, the readonly must be set manually.
UPDATE fda_f_schema SET
    readonly=NULL
WHERE (tbl_type='View as table')AND
      ((SELECT count() FROM sqlite_schema
        WHERE (type='trigger')AND
              (tbl_name=fda_f_schema.name)AND
              ((sql LIKE '% INSTEAD OF UPDATE ON '||fda_f_schema.name||'%NEW.'||fda_f_schema.field_name||' %')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fda_f_schema.name||'%NEW.'||fda_f_schema.field_name||',%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fda_f_schema.name||'%NEW.'||fda_f_schema.field_name||')%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fda_f_schema.name||'%NEW.'||fda_f_schema.field_name||'+%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fda_f_schema.name||'%NEW.'||fda_f_schema.field_name||'-%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fda_f_schema.name||'%NEW.'||fda_f_schema.field_name||'*%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fda_f_schema.name||'%NEW.'||fda_f_schema.field_name||'/%')OR
               (sql LIKE '% INSTEAD OF UPDATE ON '||fda_f_schema.name||'%NEW.'||fda_f_schema.field_name||'||%')))>0);

-- Set type of readonly (x, Calculated) with the real table but don't set readonly to false (NULL);
UPDATE fda_f_schema SET
    readonly=coalesce((SELECT A1.readonly FROM fda_f_schema A1
                       WHERE (A1.tbl_type='Table')AND
                             (A1.field_name=fda_f_schema.field_name)AND
                             (fda_f_schema.name LIKE A1.name||'__%')),fda_f_schema.readonly)
WHERE (tbl_type='View as table');

------------------------------------------------------------------------
-- Set properties of view fields with properties of real table fields --
------------------------------------------------------------------------

-- field_type
UPDATE fda_f_schema SET
    field_type=(SELECT A1.field_type FROM fda_f_schema A1
                WHERE (A1.tbl_type='Table')AND
                      (A1.field_name=fda_f_schema.field_name)AND
                      (fda_f_schema.name LIKE A1.name||'__%'))
WHERE (field_type ISNULL)AND(tbl_type LIKE 'View%');

-- col_width
UPDATE fda_f_schema SET
    col_width=(SELECT A1.col_width FROM fda_f_schema A1
               WHERE (A1.tbl_type='Table')AND
                     (A1.field_name=fda_f_schema.field_name)AND
                     (fda_f_schema.name LIKE A1.name||'__%'))
WHERE (col_width ISNULL)AND(tbl_type LIKE 'View%');

-- combo
UPDATE fda_f_schema SET
    combo=(SELECT A1.combo FROM fda_f_schema A1
           WHERE (A1.tbl_type='Table')AND
                 (A1.field_name=fda_f_schema.field_name)AND
                 (fda_f_schema.name LIKE A1.name||'__%'))
WHERE (combo ISNULL)AND(tbl_type LIKE 'View%');

-- money
UPDATE fda_f_schema SET
    money=(SELECT A1.money FROM fda_f_schema A1
           WHERE (A1.tbl_type='Table')AND
                 (A1.field_name=fda_f_schema.field_name)AND
                 (fda_f_schema.name LIKE A1.name||'__%'))
WHERE (money ISNULL)AND(tbl_type LIKE 'View%');

-- multiline
UPDATE fda_f_schema SET
    multiline=(SELECT A1.multiline FROM fda_f_schema A1
               WHERE (A1.tbl_type='Table')AND
                     (A1.field_name=fda_f_schema.field_name)AND
                     (fda_f_schema.name LIKE A1.name||'__%'))
WHERE (multiline ISNULL)AND(tbl_type LIKE 'View%');

-- unit
UPDATE fda_f_schema SET
    unit=(SELECT A1.unit FROM fda_f_schema A1
           WHERE (A1.tbl_type='Table')AND
                 (A1.field_name=fda_f_schema.field_name)AND
                 (fda_f_schema.name LIKE A1.name||'__%'))
WHERE (unit ISNULL)AND(tbl_type LIKE 'View%');

--------------------
-- Other settings --
--------------------

-- Set field_type of View field with field_type in real tables if they return the same field_type.
UPDATE fda_f_schema SET
    field_type=(SELECT A1.field_type FROM fda_f_schema A1
                WHERE (A1.tbl_type='Table')AND
                      (A1.field_name=fda_f_schema.field_name)AND
                      ((SELECT count() FROM (SELECT DISTINCT field_type
                        FROM fda_f_schema A2
                        WHERE (A2.tbl_type='Table')AND
                              (A2.field_name=fda_f_schema.field_name)))=1))
WHERE (field_type ISNULL)AND(tbl_type LIKE 'View%');
