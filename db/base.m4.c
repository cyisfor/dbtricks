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

m4_divert(`-1');
m4_include(`c.m4')
m4_define({{N}}, {{basedb_$1}})
m4_define({{T}}, {{basedb}})
m4_divert{{}}m4_dnl ;
struct T {
	sqlite3* sqlite;
	sqlite3_stmt *begin, *commit, *rollback;
	sqlite3_stmt *has_table;
	int transaction_depth;
	int error;
	void* udata;
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
		record(ERROR, "preparing %.*s\n%s\n%s",
			   STRING_FOR_PRINTF(sql),
			sqlite3_errstr(res),
			sqlite3_errmsg(c));
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

result rollback(T db) {
	db->error = 0;
	return base_rollback(db);
}

#define FUNCNAME base_release
#define FULL_COMMIT commit
#define COMMIT_PREFIX "RELEASE s"
#include "commity.snippet.h"

EXPORT
result N(release)(T db) {
	if(db->error) {
		return check(db,rollback(db));
	}
	return check(db,base_release(db));
}

#define FUNCNAME savepoint
#define FULL_COMMIT begin
#define COMMIT_PREFIX "SAVEPOINT s"
#define INCREMENT
#include "commity.snippet.h"

static
result full_commit(T db) {
	if(db->transaction_depth == 0) {
		record(ERROR, "No transaction, so why are we committing?");
	}
	int res = sqlite3_step(db->commit);
	sqlite3_reset(db->commit);
	db->transaction_depth = 0;
	return check(db, res);
}

EXPORT
result N(full_commit)(T self) {
	return check(self,full_commit(self));
}

result check(T db, int res) {
	switch(res) {
	case SQLITE_OK:
		return success;
	case SQLITE_ROW:
		return result_pending;		
	case SQLITE_DONE:
		return success;
	};
#if 0
	if(db->transaction_depth > 0) {
		int relres = base_release(db);
		if(relres != SQLITE_DONE) {
			record(WARNING, "Couldn't release! %d\n%s\n%s", relres,
				   sqlite3_errstr(res),
				   sqlite3_errmsg(db->sqlite));
		}
	}
#endif
	db->error = res;
	record(ERROR, "sqlite error %d %s (%s)\n",
		   res,
			sqlite3_errstr(res), sqlite3_errmsg(db->sqlite));
	return failure;
}

result N(once)(N(stmt) stmt) {
	result res = check(stmt->db, sqlite3_step(stmt->sqlite));
	assert(res != SQLITE_ROW);
	sqlite3_reset(stmt->sqlite);
	return res;
}

result N(retransaction)(T self) {
	if(self->transaction_depth == 0) {
		return success;
	}
	ensure_ne(failure, N(release)(self));
	return N(savepoint)(self);
}

void N(close)(T self) {
	if(self->transaction_depth > 0) {
		full_commit(self);
	}

	sqlite3_finalize(self->begin);
	sqlite3_finalize(self->commit);
	sqlite3_finalize(self->rollback);
	if(self->has_table) {
		sqlite3_finalize(self->has_table);
		self->has_table = NULL;
	}
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
				   "force finalizing statement\n%s\n",sqlite3_sql(stmt));
			check(self, sqlite3_finalize(stmt));
		}
	}
	record(ERROR,"could not close the database");
}

void* N(data)(T db) {
	return db->udata;
}
void N(set_data)(T db, void* udata) {
	db->udata = udata;
}

result N(load)(T db, N(result_handler) on_res, void* udata, const char* path) {
	size_t len = 0;
	ncstring sql = {};
	sql.base = mmapfile(path,&sql.len);
	int ret = N(execmany)(db, on_res, udata, STRING(sql));
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
	res = sqlite3_step(stmt.sqlite);			\
	CHECK;										\
	res = sqlite3_finalize(stmt.sqlite);		\
	CHECK;
#define HANDLE_EXTRA(tail)
#define OPERATION N(execmany)
#define CHECK_RESULT on_res
#define THING_HANDLER N(result_handler)
#include "domany.snippet.h"

#define START_OPERATION string name;
#define HANDLE_STATEMENT(stmt)
#define HANDLE_EXTRA(tail) {										\
		const byte* newline = memchr(tail.base, '\n', tail.len);	\
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
#define CHECK_RESULT(udata,res,i,stmt,cur,sql) on_res(udata,res,i,stmt,name,cur,tail)
#define THING_HANDLER N(prepare_handler)
#include "domany.snippet.h"

result N(prepare_many_from_file)(T self, N(prepare_handler) on_res,
								 void* udata,
								 const char* path) {
	size_t len = 0;
	ncstring sql = {};
	sql.base = mmapfile(path,&sql.len);
	result ret = N(preparemany)(self, on_res, udata, STRING(sql));
	munmap((void*)sql.base, sql.len);
	return ret;
}
	
N(stmt) N(prepare_str)(T self, string sql) {
	sqlite3* c = self->sqlite;
	sqlite3_stmt* stmt = prepare(c, sql);

	N(stmt) dbstmt = calloc(1, sizeof(*dbstmt));
	dbstmt->sqlite = stmt;
	dbstmt->db = self;
	return dbstmt;
}

result N(step)(N(stmt) stmt) {
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

identifier N(lastrow)(T self) {
	return sqlite3_last_insert_rowid(self->sqlite);
}

bool N(has_table_str)(T self, string table_name) {
	if(!self->has_table) {
		self->has_table = prepare(self->sqlite,
								LITSTR("SELECT 1 FROM sqlite_master "
									   "WHERE type='table' AND name=?"));
	}
	sqlite3_bind_text(self->has_table,1,table_name.base,table_name.len,NULL);
	// can't use N(once) because we expect SQLITE_ROW
	int res = sqlite3_step(self->has_table);
	sqlite3_reset(self->has_table);
	check(self, res);
	return res == SQLITE_ROW;
}

size_t N(stmt_changes)(N(stmt) stmt) {
	return sqlite3_changes(stmt->db->sqlite);
}

int N(change)(N(stmt) stmt) {
/* insert, update or delete */
	ensure_eq(success, N(step)(stmt));
	int ret = N(stmt_changes)(stmt);
	sqlite3_reset(stmt->sqlite);
}

size_t N(total_changes)(T self) {
	return sqlite3_total_changes(self->sqlite);
}

#include "all_types.c"

result N(bind_string)(N(stmt) stmt, int col, string value) {
	return check(stmt->db,
				 sqlite3_bind_blob(stmt->sqlite, col,
								   value.base, value.len, NULL));
}

string N(column_string)(N(stmt) stmt, int col) {
	string ret = {
		.base = sqlite3_column_blob(stmt->sqlite, col),
		.len = sqlite3_column_bytes(stmt->sqlite, col)
	};
	return ret;
}
