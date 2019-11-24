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
	puts("derp");
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
