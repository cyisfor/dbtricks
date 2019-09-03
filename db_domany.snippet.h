result OPERATION(db public, THING_HANDLER on_res, string tail) {
	dbpriv priv = (dbpriv)public;
	sqlite3* c = priv->c;
	db_stmt stmt = NULL;
	const char* next = NULL;
	int i = 0;
	struct db_stmt dbstmt = {
		.db = priv;
		.sqlite = NULL;
	};
	START_OPERATION;
	for(;;++i) {
		HANDLE_EXTRA(tail);
		
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
				return CHECK_RESULT(res, i, &dbstmt, cur, tail);			\
			}															\
			return failure;												\
		}
		CHECK;
		if(stmt == NULL) return success; // just trailing comments, whitespace
		if(next != NULL) {
			cur.len = next - tail.base;
			tail.len -= cur.len;
			tail.base = next;
		}
		HANDLE_STATEMENT(stmt);
		if(on_res) {
			dbstmt.sqlite = stmt;
			if(failure == CHECK_RESULT(res,i,&dbstmt,cur,tail)) return failure;
		}
		if(next == NULL)
			return success;
	}
}

#undef HANDLE_STATEMENT
#undef HANDLE_EXTRA
#undef OPERATION
#undef START_OPERATION
#undef CHECK_RESULT
#undef THING_HANDLER
