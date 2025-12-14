DROP TABLE IF EXISTS fda_t_schema;
CREATE TABLE fda_t_schema ( -- Additional information for tables and views (not parsed).
    name TEXT,
    tbl_type TEXT,
    description TEXT,
    row_summary TEXT,
    goto_last BOOL,
    can_open_tab TEXT,
    no_data_text TEXT,
    pixmap BLOB,
    color TEXT);

DROP VIEW IF EXISTS fda_t_schema__view;
CREATE VIEW fda_t_schema__view AS SELECT * FROM fda_t_schema ORDER BY name;

DROP TABLE IF EXISTS fda_f_schema;
CREATE TABLE fda_f_schema ( -- Additional information for fields (not parsed).
    name TEXT,
    field_index int,
    field_name TEXT,
    field_type TEXT,
    description TEXT,
    tbl_type TEXT,
    natural_sort INTEGER,
    base_data BOOL,
    hidden BOOL,
    col_width INTEGER,
    color TEXT,
    combo TEXT,
    fk_filter TEXT,
    fk_sort_field TEXT,
    master_table TEXT,
    master_field TEXT,
    money BOOL,
    multiline BOOL,
    readonly BOOL,
    unit TEXT);

DROP VIEW IF EXISTS fda_f_schema__view;
CREATE VIEW fda_f_schema__view AS SELECT * FROM fda_f_schema ORDER BY name,field_index;

-----------------------------------------------------------------------
-- Set fda_t_schema properties manually because fda parser can't do it.
-----------------------------------------------------------------------

-- fda_t_schema properties

INSERT INTO fda_t_schema (name,tbl_type,description)
VALUES ('fda_t_schema__view','View','Additional information for tables and views.');

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_t_schema__view',1,'name','TEXT','Name of table or view.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_t_schema__view',2,'tbl_type','TEXT',
        'Table: part of the DB file where the data is stored in. data can be modified.'||x'0a0a'||
        'View: data extraction from tables or other views.'||x'0a0a'||
        'View as table: data can be modified through triggers.''.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_t_schema__view',3,'description','TEXT',
        'Comment in the SQL CREATE TABLE statement, used for tooltip in the app.',NULL,NULL,NULL,NULL);
UPDATE fda_f_schema SET multiline='x' WHERE (name='fda_t_schema__view')AND(field_name='description');

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_t_schema__view',4,'row_summary','TEXT',
        'Model for row summary. Include field
VALUES with: %field_name%.'||x'0a0a'||
        'Ex: ''%customer_name% (n° %customer_ID%) - %adress_town%''',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_t_schema__view',5,'goto_last','BOOL',
        'Go to last record when opening a tab.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_t_schema__view',6,'can_open_tab','TEXT',
        'SQL SELECT statement that return TRUE if tab can be open.'||x'0a0a'||
        'No statement means TRUE.'||x'0a0a'||
        'If FALSE, the no_data_text will be displayed.'||x'0a0a'||
        'Only for Table.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_t_schema__view',7,'no_data_text','TEXT',
        'Text displayed if :'||x'0a0a'||
        '-there is no record in the table or view'||x'0a0a'||
        '-can_open_tab returns FALSE',NULL,NULL,NULL,NULL);
UPDATE fda_f_schema SET multiline='x' WHERE (name='fda_t_schema__view')AND(field_name='no_data_text');

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_t_schema__view',8,'pixmap','BLOB',
        'Pixmap for menu entries.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_t_schema__view',9,'color','TEXT',
        'Background color for the column. If empty, the table color is used.',NULL,NULL,NULL,NULL);

-- fda_f_schema properties

INSERT INTO fda_t_schema (name,tbl_type,description)
VALUES ('fda_f_schema__view','View','Additional information for fields.');

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',1,'name','TEXT',
        'Name of table or view.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',2,'field_index','int',NULL,NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',3,'field_name','TEXT',NULL,NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',4,'field_type','TEXT',
        'Field data type if field_name not empty.'||x'0a0a'||
        'Can be ''AUTOINCREMENT'' instead of ''INTEGER''.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',5,'description','TEXT',
        'Comment in the SQL CREATE TABLE statement, used for tooltip in the app.',NULL,NULL,NULL,NULL);
UPDATE fda_f_schema SET multiline='x' WHERE (name='fda_f_schema__view')AND(field_name='description');

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',6,'tbl_type','TEXT',
        'Table: part of the DB file where the data is stored in. data can be modified.'||x'0a0a'||
        'View: data extraction from tables or other views.'||x'0a0a'||
        'View as table: data can be modified through triggers.''.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',7,'natural_sort','INTEGER',
        'Table PRIMARY KEY or view ORDER BY field. A blue down arow is displayed in column header if user did''nt set another sort.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',8,'base_data','BOOL',
        '''x'' : useful data for standard use. Can be reset (right mouse click).'||x'0a0a'||
        '''Example'' : can be deleted, can''t be reset.'||x'0a0a'||
        'Not empty value for table itself means all records will be deleted before reset.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',9,'hidden','BOOL',
        'Column does not appear in the grids when the tab is opened, but can be displayed by right-clicking.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',10,'col_width','INTEGER',
        'Default column width.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',11,'color','TEXT',
        'Background color for the column. If empty, the table color is used.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',12,'combo','TEXT',
        'Values for a combobox (separator = ''|'') or param key where user VALUES are stored.'||x'0a0a'||
        'No combobox if empty.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',13,'fk_filter','TEXT',
        'SQL WHERE statement to restrict possible foreign key VALUES.'||x'0a0a'||
        'No statement means no restriction.',NULL,NULL,NULL,NULL);
UPDATE fda_f_schema SET multiline='x' WHERE (name='fda_f_schema__view')AND(field_name='fk_filter');

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',14,'fk_sort_field','TEXT',
        'Sort field for the foreign table.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',15,'master_table','TEXT',
        'Table referenced by the field.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',16,'master_field','TEXT',
        'Field in the master table.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',17,'money','BOOL',
        'Display right aligned and with 2 decimal places if data is REAL (regardless the field data type).',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',18,'multiline','BOOL',
        'The multi-line text editor can be call (ctrl+N).',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',19,'readonly','BOOL',
        'User can''t modify the data via this table or view.',NULL,NULL,NULL,NULL);

INSERT INTO fda_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fda_f_schema__view',20,'unit','TEXT',
        'Unit of the data.',NULL,NULL,NULL,NULL);
