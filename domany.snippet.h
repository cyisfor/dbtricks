result OPERATION(T self, N(result_handler) on_res, string tail) {
	sqlite3* c = self->sqlite;
	const byte* next = NULL;
	int i = 0;
	struct N(stmt) stmt = {
		.db = self,
		.sqlite = NULL
	};
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
		if(res != SQLITE_OK) {											\
			if(on_res) {												\
				return on_res(res,i,&stmt,cur, tail);					\
			}															\
			return fail;												\
		}
		CHECK;
		if(stmt.sqlite == NULL)
			return succeed; // just trailing comments, whitespace
		if(next != NULL) {
			cur.len = next - tail.base;
			tail.len -= cur.len;
			tail.base = next;
		}
		HANDLE_STATEMENT(stmt);
		if(on_res) {
			if(fail == on_res(res,i,&stmt,cur,tail)) return fail;
		}
		if(next == NULL)
			return succeed;
		HANDLE_EXTRA(tail);
	}
}
