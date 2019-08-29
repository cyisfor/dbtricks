#include "result.h"
#include "ownable_string.h"
#include "mystring.h"

#include <sqlite3.h>
#include <stdlib.h> // size_t
#include <stdbool.h>

typedef sqlite3_int64 ident;

struct db {
	int dberr;
};
typedef struct db* db;

struct db_open_params {
	const char* path;
	bool readonly;
};
db db_open_f(struct db_open_params);
#define db_open(...) ({									\
		struct db_open_params params = {__VA_ARGS__};	\
		db_open_f(params);								\
	})
void db_close(db db);
size_t db_stmt_changes(db db);

typedef struct db_stmt *db_stmt;

#define db_prepare(lit) db_prepare_str(LITSTR(lit));
db_stmt db_prepare_str(string sql);
void db_reset(db_stmt stmt);
void db_finalize(db_stmt stmt);
void db_once(db_stmt stmt);
int db_step(db_stmt stmt);
size_t db_stmt_changes(db_stmt stmt);

static int db_change(db_stmt stmt) {
/* insert, update or delete */
	ensure_eq(SQLITE_DONE, db_step(stmt));
	return db_stmt_changes(stmt);
}

int db_check(int res);

#define db_exec(db, lit) db_exec_str(db, LITSTR(lit))
int db_exec_str(db db, string sql);

#define RESULT_HANDLER(name) \
	bool name(int res, int n, sqlite3_stmt* stmt, string sql, string tail)

typedef RESULT_HANDLER((*result_handler));

void db_execmany(db db, string sql, result_handler on_err);

void db_load(db db, const char* path, result_handler on_res);

ident db_lastrow(db db);

void db_savepoint(db db);
void db_release(db db);
void db_rollback(db db);
void db_retransaction(db db);

#include "defer.h"

#define TRANSACTION(db) db_savepoint(db); DEFER { db_release(db) }

bool db_has_table_str(db db, string table_name);
#define db_has_table(db, lit) db_has_table_str(db, LITSTR(lit))

#include "db_all_types.snippet.h"
ownable_string db_column_string(db db, int col);
