#ifndef BASE_H
#define BASE_H

#include "export.h"

#include "ensure.h"
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
typedef struct N(stmt) *N(stmt);

struct N(open_params) {
	const char* path;
	bool readonly;
};
T N(open_f)(struct N(open_params));
#define basedb_open(...) ({								\
		struct basedb_open_params params = {__VA_ARGS__};	\
		basedb_open_f(params);									\
	})
void N(close)(T db);
size_t N(stmt_changes)(N(stmt) db);

void* N(data)(T db);
void N(set_data)(T db, void* udata);

#define basedb_prepare(db, ...) basedb_prepare_str(db, LITSTR(__VA_ARGS__));
N(stmt) N(prepare_str)(T db, string sql);
void N(reset)(N(stmt) stmt);
void N(finalize)(N(stmt) stmt);
result N(once)(N(stmt) stmt);
result N(step)(N(stmt) stmt);
size_t N(stmt_changes)(N(stmt) stmt);

int N(change)(N(stmt) stmt);
/* insert, update or delete */

#define basedb_exec(db, lit) basedb_exec_str(db, LITSTR(lit))
result N(exec_str)(T db, string sql);

#define RESULT_HANDLER(name) \
	bool name(void* udata, result res, int n, basedb_stmt stmt, string sql, string tail)

#define PREPARE_HANDLER(funcname) \
	bool funcname(void* udata, result res, int n, basedb_stmt stmt, string name, string sql, string tail)

typedef RESULT_HANDLER((*N(result_handler)));
typedef PREPARE_HANDLER((*N(prepare_handler)));

result N(execmany)(T db, N(result_handler) on_err, void* udata, string sql);
result N(preparemany)(T public, N(prepare_handler) on_res, void* udata, string sql);

result N(load)(T db, N(result_handler) on_res, void* udata, const char* path);
result N(preparemany_from_file)(T public, N(prepare_handler) on_res,
								void* udata, const char* path);

identifier N(lastrow)(T db);

result N(savepoint)(T db);
result N(release)(T db);
result N(rollback)(T db);
result N(full_commit)(T db);
result N(retransaction)(T db);

bool N(has_table_str)(T db, string table_name);
#define basedb_has_table(db, lit) basedb_has_table_str(db, LITSTR(lit))

#include "all_types.h"
result N(bind_string)(N(stmt) stmt, int col, string value);
string N(column_string)(N(stmt) stmt, int col);
#define basedb_bind_identifier basedb_bind_int64
#define basedb_column_identifier basedb_column_int64

#undef N
#undef T


#endif /* BASE_H */
