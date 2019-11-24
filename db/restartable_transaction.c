#include "symjoin.h"

#ifndef WRAPPER_NAME
#define WRAPPER_NAME symjoin(FUNCTION_NAME, _in_transaction)
#endif

#define PREPARE(self, member, sql)										\
	self->member = basedb_prepare(self->conn,									\
						  LITLEN(sql),									\
						  &self->member, NULL));

int FUNCTION_NAME(struct transdb* db, enum transaction_type type, ARGUMENTS) {
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
	}
	for(;;) {
		basedb_once(db->begin[type]);
		int res = WRAPPER_NAME(db, VALUES);
		switch(res) {
		case SQLITE_BUSY:
			basedb_once(db->rollback);
			continue;
		case SQLITE_OK:
		case SQLITE_DONE:
		case SQLITE_ROW:
			basedb_once(db->commit);
			return res;
		default:
			basedb_once(db->rollback);
			CHECK(res);
		};
	}
}
		
#undef WRAPPER_NAME
#undef FUNCTION_NAME
#undef ARGUMENTS
#undef VALUES
