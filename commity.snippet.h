#include "concatsym.h" // strify

#ifdef INCREMENT
#define VALUE_TRIGGER 0
#define VALUE_FIRST 1
#else
#define VALUE_TRIGGER 0
#define VALUE_FIRST 0
#endif

static
int FUNCNAME(T db) {
	int res = SQLITE_ERROR;
#ifdef INCREMENT
	++db->transaction_depth;
#else
	assert(db->transaction_depth > 0);
#endif
	if(db->transaction_depth == VALUE_TRIGGER) {
//		record(DEBUG, STRIFY(FUNCNAME) " TRIGGER %d ", VALUE_TRIGGER);
		res = sqlite3_step(db->FULL_COMMIT);
		sqlite3_reset(db->FULL_COMMIT);
		if(res == SQLITE_DONE)
			db->transaction_depth = VALUE_FIRST;
		return res;
	}
	char buf[0x100] = COMMIT_PREFIX;
	string sql = {
		.base = buf,
		.len = LITSIZ(COMMIT_PREFIX) + itoa(
			buf+LITSIZ(COMMIT_PREFIX),
			0x100-LITSIZ(COMMIT_PREFIX),
			db->transaction_depth)
	};

	sqlite3_stmt* stmt = prepare(db->sqlite, sql);
	if(stmt) {
//		record(DEBUG, STRIFY(FUNCNAME) " %d ", VALUE_TRIGGER);
		res = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		check(db, res);
#ifndef INCREMENT
		--db->transaction_depth;
#endif
	} else {
		record(ERROR, "Could not prepare " STRIFY(COMMIT_PREFIX) " %s",
			   sqlite3_errstr(res));
	}
	return res;
}

EXPORT
result CONCATSYM(basedb_, FUNCNAME)(T self) {
	return check(self, FUNCNAME(self));
}
/* defines basedb_release, basedb_savepoint, basedb_rollback */

#undef FUNCNAME
#undef FULL_COMMIT
#undef COMMIT_PREFIX
#undef VALUE_TRIGGER
#undef VALUE_FIRST
#undef INCREMENT
