#include "result.h"
#include "mystring.h"
#include "myint.h" // identifier
#include "concatsym.h"
#include <stdlib.h> // size_t
#include <stdbool.h>

#define N(a) CONCATSYM(basedb_, a)
#define T basedb

struct T;
typedef struct T *T;

enum N(state) {
	BASEDB_ERROR,
		BASEDB_OK,
		BASEDB_ROW,
		BASEDB_DONE
		};

struct N(open)_params {
	const char* path;
	bool readonly;
};
T N(open)_f(struct N(open)_params);
#define basedb_open(...) ({								\
		struct N(open)_params params = {__VA_ARGS__};	\
		N(open)_f(params);								\
	})
void N(close)(T db);
size_t N(stmt)_changes(T db);

typedef struct N(stmt) *N(stmt);

#define N(prepare)(lit) N(prepare)_str(LITSTR(lit));
N(stmt) N(prepare)_str(string sql);
void N(reset)(N(stmt) stmt);
void N(finalize)(N(stmt) stmt);
void N(once)(N(stmt) stmt);
enum N(state) N(step)(N(stmt) stmt);
size_t N(stmt)_changes(N(stmt) stmt);

static int N(change)(N(stmt) stmt) {
/* insert, update or delete */
	ensure_eq(SQLITE_DONE, N(step)(stmt));
	return N(stmt)_changes(stmt);
}

#define N(exec)(db, lit) N(exec)_str(db, LITSTR(lit))
enum N(state) N(exec)_str(T db, string sql);

#define RESULT_HANDLER(name) \
	bool name(enum N(state) res, int n, N(stmt) stmt, string sql, string tail)

#define PREPARE_HANDLER(name) \
	bool name(enum N(state) res, int n, N(stmt) stmt, string name, string sql, string tail)

typedef RESULT_HANDLER((*N(result_handler)));
typedef PREPARE_HANDLER((*N(prepare_handler)));

enum N(state) N(execmany)(T db, N(result_handler) on_err, string sql);
enum N(state) N(preparemany)(T public, N(prepare_handler) on_res, string sql);

enum N(state) N(load)(T db, N(result_handler) on_res, const char* path);
enum N(state) N(preparemany)_from_file(T public, N(prepare_handler) on_res,
								const char* path);

identifer N(lastrow)(T db);

void N(savepoint)(T db);
void N(release)(T db);
void N(rollback)(T db);
void N(retransaction)(T db);

#include "defer.h"

#define TRANSACTION(db) N(savepoint)(db); DEFER { N(release)(db) }

bool N(has_table_str)(T db, string table_name);
#define basedb_table(db, lit) N(has_table_str)(db, LITSTR(lit))

#include "all_types.snippet.h"
ownable_string N(column_string)(T db, int col);
#define basedb_column_identifier N(column_int64)

#undef N
#undef T
