CREATE TABLE functions (
id INTEGER PRIMARY KEY,
name TEXT NOT NULL UNIQUE,
column_return TEXT
);

CREATE TABLE parameters (
function INTEGER NOT NULL REFERENCES functions(id) ON DELETE CASCADE,
position INTEGER NOT NULL,
type TEXT NOT NULL,
name TEXT NOT NULL,
PRIMARY KEY(function, position)) WITHOUT ROWID;
BEGIN;

INSERT INTO functions (name, column_return) VALUES ('blob', 'const void*');
CREATE TEMPORARY TABLE funcid AS SELECT last_insert_rowid() AS id;
CREATE TEMPORARY TABLE params (
id INTEGER PRIMARY KEY,
function INTEGER,
type TEXT,
name TEXT);
INSERT INTO params (function, type, name) SELECT
	   id,
	   'const void*',
	   'val' FROM funcid;
INSERT INTO params (function, type, name) SELECT
	   id,
	   'int',
	   'len' FROM funcid;
INSERT INTO params (function, type, name) SELECT
	   id,
	   'destructor',
	   'destruct' FROM funcid;
INSERT INTO parameters (position, function, type, name) SELECT id-1, function, type, name FROM params;
delete from params;
delete from funcid;

	   


COMMIT;
