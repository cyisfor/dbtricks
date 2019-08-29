result OPERATION(db public, result_handler on_res, string tail) {
	dbpriv priv = (dbpriv)public;
	sqlite3* c = priv->c;
	db_stmt stmt = NULL;
	const char* next = NULL;
	int i = 0;
	struct db_stmt dbstmt = {
		.db = priv;
		.sqlite = NULL;
	};
	for(;;++i) {
		string cur = {
			.base = tail.base,
			.len = 0;
		};
		int res = sqlite3_prepare_v2(priv->c,
									 tail.base, tail.len,
									 &stmt,
									 &next);
#define CHECK															\
		if(res != SQLITE_OK) {											\
			if(on_res) {												\
				dbstmt.sqlite = stmt;									\
				return on_res(res,i,&dbstmt,cur, sql);					\
			}															\
			return fail;												\
		}
		CHECK;
		if(stmt == NULL) return succeed; // just trailing comments, whitespace
		if(next != NULL) {
			cur.len = next - tail.base;
			tail.len -= cur.len;
			tail.base = next;
		}
		HANDLE_STATEMENT(stmt);
		if(on_res) {
			dbstmt.sqlite = stmt;
			if(fail == on_res(res,i,&dbstmt,cur,sql)) return fail;
		}
		if(next == NULL)
			return succeed;
		HANDLE_EXTRA(tail);
	}
}
