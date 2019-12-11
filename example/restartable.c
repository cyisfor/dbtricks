#include "db/base.h"
#include "db/transaction.h"

#include <unistd.h> // sleep
#include <stdio.h> // 


static
result bar_in_transaction(basedb db, basedb_stmt insert, int val, char val2) {
	result cleanup(void) {
		puts("cleanup for retrying");
		return result_busy;
	}

	basedb_bind_int(insert, 1, val);
	basedb_bind_int(insert, 2, val2);
	int i;
	for(i=0;i<10;++i) {
		basedb_bind_int(insert, 3, i);
		/* XXX: check busy after every return? */
		transdb_check(basedb_once(insert));
		sleep(1);
		printf("inserted %d\n", i);
	}
}

#include "foo.c"

int main(int argc, char *argv[])
{
    basedb db = basedb_open("/tmp/deletethis.sqlite", false);
	basedb_exec(db, "CREATE TABLE IF NOT EXISTS foo (id INTEGER PRIMARY KEY, val INTEGER)");
	basedb_stmt insert = basedb_prepare(db, "INSERT INTO foo (val) SELECT ?+?+?");
	transdb trans = transdb_open(db);
	bar(trans, DEFERRED_TRANSACTION,
		insert, 23, 42);

	transdb_close(trans);
	basedb_finalize(insert);
	basedb_close(db);
    return 0;
}
