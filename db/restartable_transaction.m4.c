m4_divert(`-1');
m4_include(`c.m4');
m4_ifdef({{WRAPPER_NAME}},{{}},{{
		m4_define(WRAPPER_NAME, FUNCTION_NAME{{}}_in_transaction);
		}});
m4_define({{PREPARE}}, {{$1}}->{{$2}} = basedb_prepare_str({{$1}}->conn, LITSTR({{$3}})));
m4_divert{{}}m4_dnl ;
m4_dnl;
int WRAPPER_NAME{{}}(basedb db{{}}ARGUMENTS);

#include "db/transaction.struct.h"
#include <sqlite3.h>

int FUNCTION_NAME{{}}(struct transdb* db, enum transaction_type type{{}}ARGUMENTS) {
	if(!db->begin[type]) {
		switch(type) {
		case DEFERRED_TRANSACTION:
			PREPARE(db, begin[type], "BEGIN");
			break;
		case IMMEDIATE_TRANSACTION:
			PREPARE(db, begin[type], "BEGIN IMMEDIATE TRANSACTION");
			break;
		case EXCLUSIVE_TRANSACTION:
			PREPARE(db, begin[type], "BEGIN EXCLUSIVE TRANSACTION");
			break;
		};
		if(!db->commit)
			PREPARE(db, commit, "COMMIT");
	}
	for(;;) {
		basedb_once(db->begin[type]);
		int res = WRAPPER_NAME{{}}(db->conn{{}}VALUES);
		switch(res) {
		case SQLITE_BUSY:
			if(!db->rollback) {
				PREPARE(db, rollback, "ROLLBACK");
			}
			basedb_once(db->rollback);
			continue;
		case SQLITE_OK:
		case SQLITE_DONE:
		case SQLITE_ROW:
			basedb_once(db->commit);
			return res;
		default:
			if(!db->rollback) {
				PREPARE(db, rollback, "ROLLBACK");
			}
			basedb_once(db->rollback);
			return res;
		};
	}
}
