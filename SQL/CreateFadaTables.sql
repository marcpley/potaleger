----------------------
-- Tables and views --
----------------------

DROP TABLE IF EXISTS fada_t_schema;
CREATE TABLE fada_t_schema ( -- Additional information for tables and views (not parsed).
    name TEXT,
    tbl_type TEXT,
    description TEXT,
    row_summary TEXT,
    goto_last BOOL,
    can_open_tab TEXT,
    no_data_text TEXT,
    -- pixmap BLOB,
    color TEXT,
    PK_field_name TEXT,
    readonly BOOL,
    SQLite_field_count INT,
    FDA_field_count INT,
    Trigger_count INT,
    Internal_use_count INT,
    Menu_use_count INT,
    Total_use_count INT,
    Rec_count INT);

DROP VIEW IF EXISTS fada_t_schema__view;
CREATE VIEW fada_t_schema__view AS SELECT * FROM fada_t_schema ORDER BY name;

------------
-- Fields --
------------

DROP TABLE IF EXISTS fada_f_schema;
CREATE TABLE fada_f_schema ( -- Additional information for fields (not parsed).
    name TEXT,
    tbl_type TEXT,
    field_index INT,
    field_name TEXT,
    field_type TEXT,
    description TEXT,
    natural_sort INTEGER,
    base_data BOOL,
    hidden BOOL,
    col_width INTEGER,
    color TEXT,
    cond_formats TEXT,
    combo TEXT,
    fk_filter TEXT,
    fk_sort_field TEXT,
    master_table TEXT,
    master_field TEXT,
    money BOOL,
    multiline BOOL,
    draw TEXT,
    dyn_header TEXT,
    readonly BOOL,
    unit TEXT);

DROP VIEW IF EXISTS fada_f_schema__view;
CREATE VIEW fada_f_schema__view AS SELECT * FROM fada_f_schema ORDER BY name,field_index;

---------------
-- Launchers --
---------------

DROP TABLE IF EXISTS fada_launchers;
CREATE TABLE fada_launchers ( --- Additional information for menus and launchers. -- parsed ? todo
             ---readonly --Potaléger as C application.
        ID INTEGER PRIMARY KEY AUTOINCREMENT, ---hidden
        launcher_name TEXT, ---
        type TEXT, ---combo Menu item|Submenu|Button
        parent TEXT, --- 'Main menu' or launcher_name of another launcher.
        item_index INT, --- Horizontal order left to right or vertical order top to bottom.
        name TEXT, --- Table, view or script to launch.
        title TEXT, --- Title for the tab or dialogs created by the launcher.
                    --- launcher_name used if empty.
        description TEXT, --- Help description used for tooltip.
                          --- If empty, description of table or view will be used.
        filters TEXT, --- Predefined filters.
                      --- First line: Label on the left side of the combobox.
                      --- Next lines: '<combo label>|<SQL WHERE statement>'
            ---multiline
        graph TEXT, --- Display data as a graph. 23 params separated by "|":
                    --- 1: X-axis field name
                    --- 2: Grouping of values on the x-axis
                    --- - 'GroupSame' : same x-axis values grouped
                    --- - 'GroupFirstChar' : x-axis string values grouped if their first character is identical
                    --- - 'GroupFirstChar2' : x-axis string values grouped if their first 2 characters are identical
                    --- - ...
                    --- - 'GroupFirstChar9' : x-axis string values grouped if their first 9 characters are identical
                    --- - 'GroupYear' : x-axis dates values grouped by year
                    --- - 'GroupMonth' : x-axis dates values grouped by month
                    --- - 'GroupWeek' : x-axis dates values grouped by week
                    --- - 'GroupDay' : x-axis dates values grouped by day
                    --- - 'Group1000' : x-axis numeric values grouped by thousands
                    --- - 'Group100' : x-axis numeric values grouped by hundreds
                    --- - 'Group10' : x-axis numeric values grouped by tens
                    --- - 'Group1' : x-axis numeric values grouped by units
                    --- - 'Group1Dec' : x-axis numeric values grouped by rounding to 1 decimal place
                    --- - ...
                    --- - 'Group6Dec' : x-axis numeric values grouped by rounding to 6 decimal places
                    --- - empty or other : no grouping, all x-values displayed
                    --- 3: If not empty create one serie per year (x-axis values must be dates)
                    --- 4: Serie 1 field name
                    --- 5: Action on grouped values
                    --- - 'NotNull' : count not null values
                    --- - 'Distinct' : count distinct values
                    --- - 'First' : takes the first value
                    --- - 'Last' : takes the last value
                    --- - 'Average' 'Min' 'Max' 'Sum'
                    --- 6: Serie type 'Line' 'Points' 'NotNullPoints' 'Bars'
                    --- 7: Serie color like '#000000'
                    --- 8: If not empty use right y-axis
                    --- 9-13: idem for serie 2
                    --- 14-18: idem for serie 3
                    --- 19-23: idem for serie 4
            ---multiline
        color TEXT) --- If unset, takes color of table or view (name not empty) or takes color of 1rst menu item (Submenu).
            ---hidden -- color of table or view;
;

-- DROP VIEW IF EXISTS fada_launchers__view;
-- CREATE VIEW fada_launchers__view AS SELECT * FROM fada_launchers ORDER BY name;

---------------
-- Scripts --
---------------

DROP TABLE IF EXISTS fada_scripts;
CREATE TABLE fada_scripts ( --- SQL and C-like scripts that can be fired via launchers.
             ---readonly --Potaléger as C application.
        ID INTEGER PRIMARY KEY AUTOINCREMENT, ---hidden
        name TEXT, ---
        dev BOOL, --- If true (not empty) script can't be executed via its launcher.
                  --- Warning message is displayed instead..
        script TEXT, ---
        created TIMESTAMP, ---
        modified TIMESTAMP, ---
        executed TIMESTAMP) ---
;

-----------------------------------------------------------------------
-- Set fada_t_schema properties manually because fda parser can't do it.
-----------------------------------------------------------------------

-- fada_t_schema properties (in both table t and f)

INSERT INTO fada_t_schema (name,tbl_type,description,readonly)
VALUES ('fada_t_schema__view','View','Additional information for tables and views.','x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,
                          description,
                          natural_sort,col_width,readonly,unit)
VALUES ('fada_t_schema__view',1,'name','TEXT',
        'Name of table or view.',
        0,NULL,'x',NULL),

       ('fada_t_schema__view',2,'tbl_type','TEXT',
        'Table: part of the DB file where the data is stored in. data can be modified.'||x'0a0a'||
        'View: data extraction from tables or other views.'||x'0a0a'||
        'View as table: data can be modified through triggers.''.',
        NULL,NULL,'x',NULL),

       ('fada_t_schema__view',3,'description','TEXT',
        'Help description used for tooltip in the app.',
        NULL,150,'x',NULL),

       ('fada_t_schema__view',4,'row_summary','TEXT',
        'Model used to calculate the row summary of the record.',
        NULL,100,'x',NULL),

       ('fada_t_schema__view',5,'goto_last','BOOL',
        'Go to last record when opening a tab.',
        NULL,NULL,'x',NULL),

       ('fada_t_schema__view',6,'can_open_tab','TEXT',
        'SQL SELECT statement that return TRUE if tab can be open.'||x'0a0a'||
        'No statement means TRUE.'||x'0a0a'||
        'If FALSE, the no_data_text will be displayed.'||x'0a0a'||
        'Only for Table.',
        NULL,100,'x',NULL),

       ('fada_t_schema__view',7,'no_data_text','TEXT',
        'Text displayed if :'||x'0a0a'||
        '-there is no record in the table or view and user can''t insert record.'||x'0a0a'||
        '-can_open_tab returns FALSE',
        NULL,150,'x',NULL),

       -- ('fada_t_schema__view',8,'pixmap','BLOB',
       --  'Pixmap for menu entries.',NULL,NULL,NULL,'x')

       ('fada_t_schema__view',9,'color','TEXT',
        'Background color for the column. If empty, the table color is used.',
        NULL,50,'x',NULL),

       ('fada_t_schema__view',10,'PK_field_name','TEXT',
        'Primary key.',
        NULL,NULL,'x',NULL),

       ('fada_t_schema__view',11,'SQLite_field_count','INT',
        'Number of field in the real SQLite table or view.',
        NULL,NULL,'x','fields'),

       ('fada_t_schema__view',12,'FDA_field_count','INT',
        'Number of fields with FDA properties.',
        NULL,NULL,'x','fields'),

       ('fada_t_schema__view',13,'Trigger_count','INT',
        'Number of triggers for this table or view.',
        NULL,NULL,'x','triggers'),

       ('fada_t_schema__view',14,'Internal_use_count','INT',
        'Number of calls of this table/view in triggers of other tables/views.',
        NULL,NULL,'x','calls'),

       ('fada_t_schema__view',15,'Menu_use_count','INT',
        'Number of menu calling this table/view.',
        NULL,NULL,'x','menus'),

       ('fada_t_schema__view',15,'Total_use_count','INT',
        'Number of calls of this table/view.',
        NULL,NULL,'x','menus'),

       ('fada_t_schema__view',17,'Rec_count','INT',
        'Number of records in the table/view.',
        NULL,NULL,'x','records') ;

UPDATE fada_f_schema SET cond_formats='::==0 #darkRed# #lightRed#' WHERE (name='fada_t_schema__view')AND(field_name='SQLite_field_count');
UPDATE fada_f_schema SET cond_formats='::==:SQLite_field_count: #darkGreen# #lightGreen#,::!=0 #darkRed# #lightRed#' WHERE (name='fada_t_schema__view')AND(field_name='FDA_field_count');
UPDATE fada_f_schema SET cond_formats='::==0 #darkRed# #lightRed#' WHERE (name='fada_t_schema__view')AND(field_name='Total_use_count');

-- fada_f_schema properties (in both table t and f)

INSERT INTO fada_t_schema (name,tbl_type,description,readonly)
VALUES ('fada_f_schema__view','View','Additional information for fields.','x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',1,'name','TEXT',
        'Name of table or view.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',2,'field_index','int',NULL,NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',3,'field_name','TEXT',NULL,NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',4,'field_type','TEXT',
        'Additional FDA field types:'||x'0a0a'||
        '''AUTOINCREMENT'' (sqlite ''INTEGER'')'||x'0a0a'||
        '''DRAWED'' (sqlite ''TEXT'')',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',5,'description','TEXT',
        'Comment in the SQL CREATE TABLE statement, used for tooltip in the app.',NULL,NULL,NULL,'x');
UPDATE fada_f_schema SET multiline='x' WHERE (name='fada_f_schema__view')AND(field_name='description');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',6,'tbl_type','TEXT',
        'Table: part of the DB file where the data is stored in. data can be modified.'||x'0a0a'||
        'View: data extraction from tables or other views.'||x'0a0a'||
        'View as table: data can be modified through triggers.''.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',7,'natural_sort','INTEGER',
        'Table PRIMARY KEY or view ORDER BY field. A blue down arow is displayed in column header if user did''nt set another sort.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',8,'base_data','BOOL',
        '''x'' : useful data for standard use. Can be reset (right mouse click).'||x'0a0a'||
        '''Example'' : can be deleted, can''t be reset.'||x'0a0a'||
        'Not empty value for table itself means all records will be deleted before reset.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',9,'hidden','BOOL',
        'Column does not appear in the grids when the tab is opened, but can be displayed by right-clicking.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',10,'col_width','INTEGER',
        'Default column width.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',11,'color','TEXT',
        'Background color for the column. If empty, the table color is used.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',12,'cond_formats','TEXT',
        'Conditional formats (color and font attributes) for data display.'||x'0a0a'||
        'Conditional format séparator = '','''||x'0a0a'||
        'Conditional format : ''condition color dark_theme_color font_attributes'' (separator is space)'||x'0a0a'||
        'color : #rgb'||x'0a0a'||
        'font attributes : string that may contain 50-200 (stretch value), ''b'' (bold), ''u'' (underline), ''s'' (strike out)'||x'0a0a0a0a'||
        'Exemple : '||x'0a0a'||
        '::Diag_pc>110 #b70000 #ff7171 150b,::Diag_pc<90 #009e00 #77ff77 b'||x'0a0a'||
        '''Diag_pc'' is a field name. ::Diag_pc will be replaced by the current line field value.'||x'0a0a'||
        'If field value is greater than 110, it will be displayed in red, bold and font width of 150%',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',13,'combo','TEXT',
        'Values for a combobox (separator = ''|'') or param key where user VALUES are stored.'||x'0a0a'||
        'No combobox if empty.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',14,'fk_filter','TEXT',
        'SQL WHERE statement to restrict possible foreign key VALUES.'||x'0a0a'||
        'No statement means no restriction.',NULL,NULL,NULL,'x');
UPDATE fada_f_schema SET multiline='x' WHERE (name='fada_f_schema__view')AND(field_name='fk_filter');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',15,'fk_sort_field','TEXT',
        'Sort field for the foreign table.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',16,'master_table','TEXT',
        'Table referenced by the field.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',17,'master_field','TEXT',
        'Field in the master table.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',18,'money','BOOL',
        'Display right aligned and with 2 decimal places if data is REAL (regardless the field data type).',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',19,'multiline','BOOL',
        'The multi-line text editor can be call (ctrl+N).',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',20,'draw','TEXT',
        'Field data will drawed as texts, lines, rectangles...'||x'0a0a'||
        'Origin is left bottom corner, unit is pixel.'||x'0a0a'||
        'Commands separator is space: "color(#000000,255) line(0,0,10,10) ..."'||x'0a0a'||
        'Color and alpha parameters are optional (#rgb,alpha), except for color command.'||x'0a0a'||
        'color(#rgb,alpha) : change color for folowing commands.'||x'0a0a'||
        'line(x1,y1,x2,y2,#rgb,alpha) : line from point 1 to point 2.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',20,'dyn_header','TEXT',
        'SQL statement used to calculate the title of column.',NULL,NULL,NULL,'x');


INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',21,'readonly','BOOL',
        'User can''t modify the data via this table or view.',NULL,NULL,NULL,'x');

INSERT INTO fada_f_schema (name,field_index,field_name,field_type,description,natural_sort,master_table,master_field,readonly)
VALUES ('fada_f_schema__view',22,'unit','TEXT',
        'Unit of the data.',NULL,NULL,NULL,'x');
