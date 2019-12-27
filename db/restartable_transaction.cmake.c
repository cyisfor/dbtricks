#include "db/transaction.struct.h"
#include "result.h"

result @WRAPPER_NAME@(basedb db@ARGUMENTS@);

result @FUNCTION_NAME@(struct transdb* db,
						 enum transaction_type type@ARGUMENTS@) {
	if(!db->begin[type]) {
		switch(type) {
		case DEFERRED_TRANSACTION:
			db->begin[type] = basedb_prepare(db->conn, "BEGIN");
			break;
		case IMMEDIATE_TRANSACTION:
			db->begin[type] = basedb_prepare(db->conn, "BEGIN IMMEDIATE TRANSACTION");			
			break;
		case EXCLUSIVE_TRANSACTION:
			db->begin[type] = basedb_prepare(db->conn, "BEGIN EXCLUSIVE TRANSACTION");			
			break;
		};
		if(!db->commit)
			db->commit = basedb_prepare(db->conn, "COMMIT");			
	}
	for(;;) {
		basedb_once(db->begin[type]);
		result res = @WRAPPER_NAME@(db->conn@VALUES@);
		switch(res) {
		case result_busy:
			if(!db->rollback) {
				db->rollback = basedb_prepare(db->conn, "ROLLBACK");			
			}
			basedb_once(db->rollback);
			int timeout = basedb_get_busy_timeout(db->conn);
			if(timeout == 1) timeout = 2;
			else if(timeout > 1) timeout *= 1 + drand48();
			else timeout = 100; // 100ms by default if conflict
			basedb_busy_timeout(db->conn, timeout);
			continue;
		case result_success:
		case result_pending:
			basedb_once(db->commit);
		default:
			if(!db->rollback) {
				db->rollback = basedb_prepare(db->conn, "ROLLBACK");
			}
			basedb_once(db->rollback);
		};
		return res;
	}
}
