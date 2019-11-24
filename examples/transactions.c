#include "db/base.h"
#include "db/transaction.h"

struct db {
	struct transdb trans;
	basedb_stmt insert;
	basedb_stmt query;
	basedb base;
};

#define FUNCTION_NAME dothings
int dothings_in_transaction(struct transdb* db, struct db* db) {
	basedb_bind_int(db->insert,1,23);
	check_busy(basedb_step(db->insert));
	basedb_reset(db->insert);
	basedb_bind_int(db->insert,1,42);
	check_busy(basedb_step(db->insert));
	basedb_reset(db->insert);
	puts("herp");
	for(;;) {
		if(result_done == check_busy(basedb_step(db->query))) {
			break;
		}
		printf("derp %d\n", basedb_column_int(db->query, 0));
	}
	return SQLITE_OK;
}

#include "db/restartable_transaction.h"

int main(int argc, char *argv[])
{
	struct db db;
	db.base = basedb_open(
		.path = "test.sqlite",
		.readonly = false);
	basedb_exec(db.base,
				"CREATE TABLE stuff (\n"
				"value INTEGER NOT NULL\n"
				")");
	db.insert = basedb_prepare(db.base, "INSERT INTO stuff (value) VALUES (?)");
	db.query = basedb_prepare(db.base, "SELECT value FROM stuff ORDER BY value");
	dothings(&db.trans, DEFERRED_TRANSACTION, &db);
	transaction_finalize(&db.trans);
	basedb_finalize(db.insert);
	basedb_finalize(db.query);
	basedb_close(db.base);
    return 0;
}
