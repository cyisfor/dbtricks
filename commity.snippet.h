#ifdef INCREMENT
#define ADJUST ++
#define VALUE_TRIGGER 0
#define VALUE_FIRST 1
#else
#define ADJUST --
#define VALUE_TRIGGER 1
#define VALUE_FIRST 0
#endif

static
int FUNCNAME(dbpriv db) {
	int res = SQLITE_ERROR;
#ifndef INCREMENT
	assert(db->transaction-depth > 0);
#endif
	if(db->transaction_depth == VALUE_TRIGGER) {
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
			db->transaction_depth);
	};

	sqlite3_stmt* stmt = prepare(db->sqlite, sql);
	if(stmt) {
		res = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if(res == SQLITE_DONE) {
			ADJUST db->transaction_depth;
		}	
	} else {
		record(ERROR, "Could not prepare " STRIFY(COMMIT_PREFIX) " %s",
			   sqlite3_errmsg(res));
	}
	return res;
}

EXPORT
void CONCATSYM(db_, FUNCNAME)(db db) {
	dbpriv priv = (dbpriv)db;
	db_check(priv, FUNCNAME(priv));
}
/* defines db_release, db_savepoint, db_rollback */

#undef FUNCNAME
#undef FULL_COMMIT
#undef COMMIT_PREFIX
#undef ADJUST
#undef VALUE_TRIGGER
#undef VALUE_FIRST
#undef INCREMENT
