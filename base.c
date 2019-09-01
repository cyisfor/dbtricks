#include "record.h"
#include "base.h"
#include "itoa.h"
#include "mmapfile.h"
#include "mystring.h"

#include <sqlite3.h>


#include <sys/mman.h> // munmap
#include <string.h> // memchr

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <unistd.h> // sleep

#define N(a) CONCATSYM(basedb_, a)
#define T basedb

struct T {
	sqlite3* sqlite;
	sqlite3_stmt *begin, *commit, *rollback;
	sqlite3_stmt *has_table;
	int transaction_depth;
	int error;
};

typedef struct N(stmt) {
	struct T* db;
	sqlite3_stmt* sqlite;
} *N(stmt);

typedef struct T* T;

static
sqlite3_stmt* prepare(sqlite3* c, string sql) {
	sqlite3_stmt* stmt;
	const byte* next = NULL;
	int res = sqlite3_prepare_v2(
		c,
		sql.base,
		sql.len,
		&stmt,
		(const char**)&next);
	if(next && next - sql.base != sql.len) {
		string tail = {
			.base = next,
			.len = sql.len - (next - sql.base)
		};
		record(WARNING, "some sql wouldn't prepare #.*s",
			   STRING_FOR_PRINTF(tail));
	}
	if(res != SQLITE_OK) {
		record(ERROR, "preparing %.*s",
			   STRING_FOR_PRINTF(sql));
	}
	return stmt;
}

static
T open_with_flags(const char* path, int flags) {
	T db = calloc(1,sizeof(struct T));

	//chdir(getenv("FILEDB"));
	ensure_eq(
		SQLITE_OK,
		sqlite3_open_v2(
			path, &db->sqlite,
			flags | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_PRIVATECACHE,
			NULL));
	sqlite3_extended_result_codes(db->sqlite, 1);
	db->begin = prepare(db->sqlite, LITSTR("BEGIN"));
	db->commit = prepare(db->sqlite, LITSTR("COMMIT"));
	db->rollback = prepare(db->sqlite, LITSTR("ROLLBACK"));
	return db;
}

T N(open_f)(struct N(open_params) params) {
	return open_with_flags(params.path,
						   params.readonly ?
						   SQLITE_OPEN_READONLY :
						   (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
}

static
result check(T db, int res);



#define FUNCNAME base_rollback
#define FULL_COMMIT rollback
#define COMMIT_PREFIX "ROLLBACK TO s"
#include "commity.snippet.h"

int rollback(T db) {
	db->error = 0;
	return base_rollback(db);
}

#define FUNCNAME base_release
#define FULL_COMMIT commit
#define COMMIT_PREFIX "RELEASE TO s"
#include "commity.snippet.h"

int release(T db) {
	if(db->error) {
		return rollback(db);
	}
	return base_release(db);
}

#define FUNCNAME savepoint
#define FULL_COMMIT begin
#define COMMIT_PREFIX "SAVEPOINT s"
#define INCREMENT
#include "commity.snippet.h"

static
int full_commit(T db) {
	if(db->transaction_depth == 0) {
		record(ERROR, "No transaction, so why are we committing?");
	}
	int res = sqlite3_step(db->commit);
	sqlite3_reset(db->commit);
	db->transaction_depth = 0;
	return res;
}

EXPORT
void N(full_commit)(T self) {
	check(self, full_commit(self));
}

result check(T db, int res)
{
	switch(res) {
	case SQLITE_OK:
		return succeed;
	case SQLITE_ROW:
		return succeed;		
	case SQLITE_DONE:
		return done;
	};
	if(db->transaction_depth > 0) {
		int res = release(db);
		if(res != SQLITE_DONE) {
			record(WARNING, "Couldn't release! %d %s", res,
				   sqlite3_errstr(res));
		}
	}
	db->error = res;
	record(ERROR, "sqlite error %s (%s)\n",
			sqlite3_errstr(res), sqlite3_errmsg(db->sqlite));
	return fail;
}

void N(once)(N(stmt) stmt) {
	int res = check(stmt->db, sqlite3_step(stmt->sqlite));
	assert(res != SQLITE_ROW);
	sqlite3_reset(stmt->sqlite);
}

void N(retransaction)(T self) {
	if(self->transaction_depth == 0) {
		return;
	}
	N(release)(self);
	N(savepoint)(self);
}

void N(close)(T self) {
	if(self->transaction_depth > 0) {
		full_commit(self);
	}

	sqlite3_finalize(self->begin);
	sqlite3_finalize(self->commit);
	sqlite3_finalize(self->rollback);

	int attempt = 0;
	for(;attempt<10;++attempt) {
		int res = sqlite3_close(self->sqlite);
		if(res == SQLITE_OK) {
			free(self);
			return;
		}
		record(WARNING, "sqlite close error %s %s",
			   sqlite3_errstr(res), sqlite3_errmsg(self->sqlite));
		if(attempt > 1) {
			sleep(attempt);
		}
		sqlite3_stmt* stmt = NULL;
		while((stmt = sqlite3_next_stmt(self->sqlite, stmt))) {
			record(WARNING,
				   "closing statement\n%s\n",sqlite3_sql(stmt));
			check(self, sqlite3_finalize(stmt));
		}
	}
	record(ERROR,"could not close the database");
}

result N(load)(T db, N(result_handler) on_res, const char* path) {
	size_t len = 0;
	ncstring sql = {};
	sql.base = mmapfile(path,&sql.len);
	int ret = N(execmany)(db, on_res, STRING(sql));
	munmap((void*)sql.base, sql.len);
	return check(db, ret);
}

result N(exec_str)(T db, string sql) {
	sqlite3_stmt* stmt = prepare(db->sqlite, sql);
	int res = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return check(db, res);
}

#define START_OPERATION
#define HANDLE_STATEMENT(stmt)					\
	res = sqlite3_step(stmt->sqlite);					\
	CHECK;										\
	res = sqlite3_finalize(stmt->sqlite);				\
	CHECK;
#define HANDLE_EXTRA(tail)
#define OPERATION N(execmany)
#define CHECK_RESULT on_res
#define THING_HANDLER N(result_handler)
#include "domany.snippet.h"

#define START_OPERATION string name;
#define HANDLE_STATEMENT(stmt)
#define HANDLE_EXTRA(tail) {										\
		const char* newline = memchr(tail.base, '\n', tail.len);	\
		if(!newline) {												\
			/* no more */											\
			return success;											\
		}															\
		name.base = tail.base;										\
		name.len = (newline - tail.base);							\
		tail.base += name.len + 1;									\
		tail.len -= name.len + 1;									\
	}
#define OPERATION N(preparemany)
#define CHECK_RESULT(res,i,stmt,cur,sql) on_res(res,i,stmt,name,cur,tail)
#define THING_HANDLER N(prepare_handler)
#include "domany.snippet.h"

result N(prepare_many_from_file)(T self, N(prepare_handler) on_res,
										const char* path) {
	size_t len = 0;
	ncstring sql = {};
	sql.base = mmapfile(path,&sql.len);
	result ret = N(preparemany)(self, on_res, sql);
	munmap((void*)sql.base, sql.len);
	return ret;
}
	
result N(execmany)(T self, N(result_handler) on_res,
						  string tail, bool finalize) {
	sqlite3* c = self->sqlite;
	const char* next = NULL;
	int i = 0;
	struct N(stmt) stmt = {
		.db = self;
		.sqlite = NULL;
	};
	for(;;++i) {
		string cur = {
			.base = tail.base,
			.len = 0;
		};
		int res = sqlite3_prepare_v2(self->sqlite,
									 tail.base, tail.len,
									 &stmt->sqlite,
									 &next);
#define CHECK															\
		if(res != SQLITE_OK) {											\
			if(on_res) {												\
				return on_res(res,i,&stmt,cur, sql);					\
			}															\
			return fail;												\
		}
		CHECK;
		if(stmt == NULL) return true; // just trailing comments, whitespace
		if(next != NULL) {
			cur.len = next - tail.base;
			tail.len -= cur.len;
			tail.base = next;
		}
		res = sqlite3_step(stmt);
		CHECK;
		res = sqlite3_finalize(stmt);
		CHECK;
		if(on_res) {
			if(fail == on_res(res,i,&stmt,cur,sql)) return fail;
		}
		if(next == NULL)
			return succeed;
	}
}

N(stmt) N(prepare_str)(T self, string sql) {
	sqlite3* c = self->sqlite;
	sqlite3_stmt* stmt = prepare(c, sql);

	N(stmt) dbstmt = calloc(1, sizeof(*dbstmt));
	dbstmt->sqlite = stmt;
	dbstmt->db = self;
	return dbstmt;
}

int N(step)(N(stmt) stmt) {
	if(stmt->db->error) {
		record(ERROR, "tried to do something without rolling back!");
	}
	return check(stmt->db, sqlite3_step(stmt->sqlite));
}

void N(reset)(N(stmt) stmt) {
	check(stmt->db,sqlite3_reset(stmt->sqlite));
}

void N(finalize)(N(stmt) stmt) {
	check(stmt->db,sqlite3_finalize(stmt->sqlite));
	free(stmt);
}

ident N(lastrow)(T self) {
	return sqlite3_last_insert_rowid(self->sqlite);
}

bool N(has_table_str)(T self, const char* table, size_t n) {
	if(!self->has_table) {
		self->has_table = prepare(self->sqlite,
								LITSTR("SELECT 1 FROM sqlite_master "
									   "WHERE type='table' AND name=?"));
	}
	sqlite3_bind_text(self->has_table,1,table,n,NULL);
	// can't use N(once) because we expect SQLITE_ROW
	int res = check(self, sqlite3_step(self->has_table));
	sqlite3_reset(db->has_table);
	return res == SQLITE_ROW;
}

size_t N(stmt_changes)(N(stmt) stmt) {
	return sqlite3_changes(stmt->db->sqlite);
}

int N(change)(N(stmt) stmt) {
/* insert, update or delete */
	ensure_eq(SQLITE_DONE, N(step)(stmt));
	return N(stmt_changes)(stmt);
}

size_t N(total_changes)(T self) {
	return sqlite3_total_changes(self->sqlite);
}

#define IMPLEMENTATION
#include "all_types.snippet.h"

string N(column_string)(N(stmt) stmt, int col) {
	string ret = {
		.base = sqlite3_column_blob(stmt->sqlite),
		.len = sqlite3_column_bytes(stmt->sqlite)
	};
	return ret;
}
