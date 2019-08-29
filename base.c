#include "db.h"
#include "itoa.h"
#include "mmapfile.h"
#include "mystring.h"

#include <sys/mman.h> // munmap
#include <string.h> // memchr

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <unistd.h> // sleep

struct dbpriv {
	struct db public;
	sqlite3* sqlite;
	sqlite3_stmt *begin, *commit, *rollback;
	sqlite3_stmt *has_table;
	int transaction_depth;
};

typedef struct db_stmt {
	struct dbpriv* db;
	sqlite3_stmt* sqlite;
} *db_stmt;

typedef struct dbpriv* dbpriv;

static
sqlite3_stmt* prepare(sqlite3* c, string sql) {
	sqlite3_stmt* stmt
	const char* db_next = NULL;
	int res = sqlite3_prepare_v2(
		c,
		sql.base,
		sql.len,
		&stmt,
		&db_next);
	if(db_next && db_next - sql.base != sql.len) {
		string tail = {
			.base = db_next,
			.len = sql.len - (db_next - sql.base)
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
db open_with_flags(const char* path, int flags) {
	dbpriv db = calloc(1,sizeof(struct dbpriv));

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
	DB_OK(db);
	return (db)db;
}

db db_open_f(struct db_params params) {
	return open_with_flags(params.path,
						   params.readonly ?
						   SQLITE_OPEN_READONLY :
						   (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
}

#define FUNCNAME base_rollback
#define FULL_COMMIT rollback
#define COMMIT_PREFIX "ROLLBACK TO s"
#include "db_commity.snippet.h"

int rollback(dbpriv db) {
	db->error = 0;
	return base_rollback(db);
}

#define FUNCNAME base_release
#define FULL_COMMIT commit
#define COMMIT_PREFIX "RELEASE TO s"
#include "db_commity.snippet.h"

int release(dbpriv db) {
	if(db->error) {
		return rollback(db);
	}
	return base_release(db);
}

#define FUNCNAME savepoint
#define FULL_COMMIT begin
#define COMMIT_PREFIX "SAVEPOINT s"
#define INCREMENT
#include "db_commity.snippet.h"

static
int full_commit(dbpriv db) {
	if(db->transaction_level == 0) {
		record(ERROR, "No transaction, so why are we committing?");
	}
	int res = sqlite3_step(db->commit);
	sqlite3_reset(db->commit);
	db->transaction_level = 0;
	return res;
}

EXPORT
void db_full_commit(db db) {
	db_check((dbpriv)db, full_commit((dbpriv)db));
}

int db_check(dbpriv db, int res)
{
	switch(res) {
	case SQLITE_OK:
	case SQLITE_ROW:
	case SQLITE_DONE:
		return res;
	};
	if(db->transaction_depth > 0) {
		int res = release(db);
		if(res != SQLITE_DONE) {
			record(WARNING, "Couldn't release! %d %s", res,
				   sqlite3_errmsg(res));
		}
	}
	record(ERROR, "sqlite error %s (%s)\n",
			sqlite3_errstr(res), sqlite3_errmsg(db->sqlite));
	return res;
}

void db_once(db_stmt stmt) {
	int res = db_check(stmt->db, sqlite3_step(stmt->sqlite));
	assert(res != SQLITE_ROW);
	sqlite3_reset(stmt->sqlite);
}

void db_retransaction(db db) {
	if(((dbpriv)db)->transaction_level == 0) {
		return;
	}
	db_release(db);
	db_savepoint(db);
}

void db_close(db db) {
	dbpriv priv = (dbpriv)db;
	if(priv->transaction_level > 0) {
		full_commit(priv);
	}

	sqlite3_finalize(priv->begin);
	sqlite3_finalize(priv->commit);
	sqlite3_finalize(priv->rollback);

	int attempt = 0;
	for(;attempt<10;++attempt) {
		int res = sqlite3_close(priv->c);
		if(res == SQLITE_OK) {
			free(priv);
			return;
		}
		record(WARNING, "sqlite close error %s %s",
			   sqlite3_errstr(res), sqlite3_errmsg(priv->c));
		if(attempt > 1) {
			sleep(attempt);
		}
		sqlite3_stmt* stmt = NULL;
		while((stmt = sqlite3_next_stmt(priv->c, stmt))) {
			record(WARNING,
				   "closing statement\n%s\n",sqlite3_sql(stmt));
			db_check(sqlite3_finalize(stmt));
		}
	}
	record(ERROR,"could not close the database");
}

void db_load(db db, result_handler on_res, const char* path) {
	size_t len = 0;
	ncstring sql = {};
	sql.base = mmapfile(path,&sql.len);
	db_execmany(db, on_res, sql);
	munmap((void*)sql.base, sql.len);
}

int db_exec_str(db db, string sql) {
	sqlite3_stmt* stmt = prepare(db->sqlite, sql);
	int res = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return res;
}

#define HANDLE_STATEMENT(stmt)					\
	res = sqlite3_step(stmt);					\
	CHECK;										\
	res = sqlite3_finalize(stmt);				\
	CHECK;
#define HANDLE_EXTRA(tail)
#define OPERATION db_execmany
#include "domany.snippet.h"

void db_preparemany(db public, prepare_handler on_res, string sql) {
	
}

result db_execmany(db public, result_handler on_res, string tail, bool finalize) {
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
			dbstmt.sqlite = stmt;
			if(fail == on_res(res,i,&dbstmt,cur,sql)) return fail;
		}
		if(next == NULL)
			return succeed;
	}
}

db_stmt db_prepare_str(db public, string sql) {
	sqlite3* c = ((dbpriv)public)->c;
	sqlite3_stmt* stmt = prepare(c, sql);

	db_stmt dbstmt = calloc(1, sizeof(*dbstmt));
	dbstmt->sqlite = stmt;
	dbstmt->db = (dbpriv)public;
	return dbstmt;
}

int db_step(db_stmt stmt) {
	if(stmt->db->error) {
		record(ERROR, "tried to do something without rolling back!");
	}
	return db_check(stmt->db, sqlite3_step(stmt->sqlite));
}

void db_reset(db_stmt stmt) {
	db_check(stmt->db,sqlite3_reset(stmt->sqlite));
}
void db_finalize(db_stmt stmt) {
	db_check(stmt->db,sqlite3_finalize(stmt->sqlite));
	free(stmt);
}

ident db_lastrow(db db) {
	return sqlite3_last_insert_rowid(((dbpriv)db)->c);
}

bool db_has_table_str(db db, const char* table, size_t n) {
	dbpriv priv = (dbpriv)db;
	if(!db->has_table) {
		db->has_table = prepare(priv->c,
								LITSTR("SELECT 1 FROM sqlite_master "
									   "WHERE type='table' AND name=?"));
	}
	sqlite3_bind_text(db->has_table,1,table,n,NULL);
	// can't use db_once because we expect SQLITE_ROW
	int res = db_check(priv, sqlite3_step(db->has_table));
	sqlite3_reset(db->has_table);
	return res == SQLITE_ROW;
}

size_t db_stmt_changes(db_stmt stmt) {
	return sqlite3_changes(stmt->db->sqlite);
}

size_t db_total_changes(db db) {
	return sqlite3_total_changes(((dbpriv)db)->sqlite);
}

#define IMPLEMENTATION
#include "all_types.snippet.h"

string db_column_string(db_stmt stmt, int col) {
	string ret = {
		.base = sqlite3_column_blob(stmt->sqlite),
		.len = sqlite3_column_bytes(stmt->sqlite)
	};
	return ret;
}
