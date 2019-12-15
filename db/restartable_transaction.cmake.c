#include "db/transaction.struct.h"
#include "result.h"

result @WRAPPER_NAME@(basedb db@ARGUMENTS@);

result @FUNCTION_NAME@(struct transdb* db,
						 enum transaction_type type@ARGUMENTS@) {
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
		result res = @WRAPPER_NAME@(db->conn@VALUES@);
		switch(res) {
		case result_busy:
			if(!db->rollback) {
				PREPARE(db, rollback, "ROLLBACK");
			}
			basedb_once(db->rollback);
			continue;
		case result_success:
		case result_pending:
			basedb_once(db->commit);
		default:
			if(!db->rollback) {
				PREPARE(db, rollback, "ROLLBACK");
			}
			basedb_once(db->rollback);
		};
		return res;
	}
}
