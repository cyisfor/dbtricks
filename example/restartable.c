#include "db/base.h"
#include "db/transaction.h"

#include <sys/wait.h> // waitpid

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

void do_one(basedb_stmt insert, transdb trans) {
	int i;
	for(i=0;i<10;++i) {
		printf("%d trying %d %d\n", getpid(), ++counter, i);
		bar(trans, DEFERRED_TRANSACTION,
			insert, i, 23, 42);
		printf("%d done %d %d\n", getpid(), counter, i);
		sleep(1);
	}
}

int main(int argc, char *argv[])
{
    basedb db = basedb_open("/tmp/deletethis.sqlite", false);
	basedb_exec(db, "CREATE TABLE IF NOT EXISTS foo (id INTEGER PRIMARY KEY, val INTEGER)");
	basedb_stmt insert = basedb_prepare(db, "INSERT INTO foo (val) SELECT ?+?+?");
	transdb trans = transdb_open(db);
	int num = atoi(getenv("num"));
	if(!num) num = 10;
	int i = 0;
	int pids[num];
	for(;;) {
		int pid;
		if(++i == num) {
			do_one(insert, trans);
			for(i=0;i<num;++i) {
				int status;
				int pid = waitpid(pids[num], &status, 0);
				printf("%d(%d) exited %d\n", pid, pids[num], status);
			}
			break;
		} else {
			pid = fork();
			if(pid == 0) {
				do_one(insert, trans);
				break;
			} else {
				pids[num] = pid;
			}
		}
	}

	
	transdb_close(trans);
	basedb_finalize(insert);
	basedb_close(db);
    return 0;
}
