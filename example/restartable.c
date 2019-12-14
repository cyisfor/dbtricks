#include "db/base.h"
#include "db/transaction.h"

#include <unistd.h> // sleep, usleep
#include <stdio.h> // 

static int counter = 0;

static
result bar_in_transaction(basedb db, basedb_stmt insert, int i, int val, char val2) {
	result cleanup(void) {
		printf("%d cleanup for retrying %d %d\n", getpid(), counter, i);
		usleep(500);
		return result_busy;
	}

	basedb_bind_int(insert, 1, val);
	basedb_bind_int(insert, 2, val2);
	basedb_bind_int(insert, 3, i);
	/* XXX: check busy after every return? */
	transdb_check(basedb_once(insert));
	transdb_check(basedb_once(insert));
	transdb_check(basedb_once(insert));
	sleep(1);
	printf("%d inserted %d %d\n", getpid(), counter, i);
	counter = 0;
}

#include "foo.c"

int main(int argc, char *argv[])
{
    basedb db = basedb_open("/tmp/deletethis.sqlite", false);
	basedb_exec(db, "CREATE TABLE IF NOT EXISTS foo (id INTEGER PRIMARY KEY, val INTEGER)");
	basedb_stmt insert = basedb_prepare(db, "INSERT INTO foo (val) SELECT ?+?+?");
	transdb trans = transdb_open(db);
	int num = atoi(getenv("num"));
	if(!num) num = 10;
	int i;
	int pids[num];
	for(i=0;i<num;++i) {
		int pid = fork();
		if(pid == 0) {
			for(i=0;i<10;++i) {
				printf("%d trying %d %d\n", getpid(), ++counter, i);
				bar(trans, DEFERRED_TRANSACTION,
					insert, i, 23, 42);
				printf("%d done %d %d\n", getpid(), counter, i);
				sleep(1);
			}
			break;
		} else {
			pids[i] = pid;
			if(++i == num) {
				for(i=0;i<num;++i) {
					int status = 0;
					int pid = waitpid(i, &status, 0);
					printf("%d status %x\n", pid, status);
				}
		}		
	}
		
	transdb_close(trans);
	basedb_finalize(insert);
	basedb_close(db);
    return 0;
}
