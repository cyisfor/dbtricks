#include "db/base.h"
#include "db/transaction.h"

static
int foo_in_transaction(basedb db, basedb_stmt insert, int val, int val2) {
	basedb_bind_int(insert, 1, val);
	basedb_bind_int(insert, 2, val2);
	int i;
	for(i=0;i<10;++i) {
		basedb_bind_int(insert, 3, i);
		/* XXX: check busy after every return? */
		basedb_once(insert);
		sleep(1);
	}
}

#include "foo.c"

int main(int argc, char *argv[])
{
    basedb db = basedb_open("/tmp/deletethis.sqlite", false);
	basedb_exec(db, "CREATE TABLE IF NOT EXISTS foo (id INTEGER PRIMARY KEY, val INTEGER)");
	basedb_stmt insert = basedb_prepare(db, "INSERT INTO foo (val) SELECT ?+?+?");
	struct transdb trans = {
		.db = db
	};
	foo(&trans, insert, 23, 42);
	basedb_bind_int(insert, 1, 42);
	basedb_once(insert);

	basedb_finalize(insert);
	basedb_close(db);
    return 0;
}
