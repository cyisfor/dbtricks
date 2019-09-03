result OPERATION(T self, THING_HANDLER on_res, void* udata, string tail) {
	sqlite3* c = self->sqlite;
	const byte* next = NULL;
	int i = 0;
	struct N(stmt) stmt = {
		.db = self,
		.sqlite = NULL
	};
	START_OPERATION;
	for(;;++i) {
		string cur = {
			.base = tail.base,
			.len = 0
		};
		int res = sqlite3_prepare_v2(self->sqlite,
									 tail.base, tail.len,
									 &stmt.sqlite,
									 (const char**)&next);
#define CHECK															\
		if(res != SQLITE_OK && res != SQLITE_DONE) {					\
			if(on_res) {												\
				return CHECK_RESULT(udata, check(self,res),				\
									i,(&stmt),cur, tail);				\
			}															\
			return failure;												\
		}
		CHECK;
		if(stmt.sqlite == NULL)
			return success; // just trailing comments, whitespace
		if(next != NULL) {
			cur.len = next - tail.base;
			tail.len -= cur.len;
			tail.base = next;
		}
		HANDLE_STATEMENT(stmt);
		if(on_res) {
			if(failure == CHECK_RESULT(udata, check(self,res),i,(&stmt),cur,tail))
				return failure;
		}
		if(next == NULL)
			return success;
		HANDLE_EXTRA(tail);
	}
}
#undef START_OPERATION
#undef OPERATION
#undef HANDLE_EXTRA
#undef HANDLE_STATEMENT
#undef CHECK_RESULT
#undef THING_HANDLER
