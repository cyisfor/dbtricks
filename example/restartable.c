#include "db/base.h"
#include "db/transaction.h"

#include <sys/wait.h> // waitpid

#include <unistd.h> // sleep, usleep
#include <stdio.h> 
#include <stdlib.h> // drand48
#include <math.h> // pow

static int counter = 0;
static int max_counter = 0;

static
result bar_in_transaction(basedb db, basedb_stmt insert, int which, int i, int val, char val2) {
	result cleanup(void) {
		printf("%02d cleanup for retrying %d %d\n", which, ++counter, i);
		if(counter > max_counter) {
			max_counter = counter;
			printf("%02d setting timeout to %d\n", which, max_counter);
			basedb_busy_timeout(db,  max_counter * 1000);
		}
		return result_busy;
	}

	basedb_bind_int(insert, 1, val);
	basedb_bind_int(insert, 2, val2);
	basedb_bind_int(insert, 3, i);
	/* XXX: check busy after every return? */
	transdb_check(basedb_once(insert));
	transdb_check(basedb_once(insert));
	transdb_check(basedb_once(insert));
	usleep(1000000 * drand48() + 500000);
	printf("%02d inserted %d %d\n", which, counter, i);
}

#include "foo.c"

void do_one(int which, basedb_stmt insert, transdb trans) {
	usleep(which * 1000000 + drand48() * 3000000);
	int i;
	for(i=0;i<10;++i) {
		counter = 0;
		printf("%02d trying %d\n", which, i);
		bar(trans, DEFERRED_TRANSACTION,
			insert, which, i, 23, 42);
		printf("%02d done %d %d\n", which, counter, i);
		sleep(1);
	}
}

int main(int argc, char *argv[])
{
	int num = 20;
	if(getenv("num")) {
		num = atoi(getenv("num"));
	}
	basedb db = basedb_open("/tmp/deletethis.sqlite", false);
	basedb_busy_timeout(db,  num * 100);

	basedb_exec(db, "CREATE TABLE IF NOT EXISTS foo (id INTEGER PRIMARY KEY, val INTEGER)");
	basedb_stmt insert = basedb_prepare(db, "INSERT INTO foo (val) SELECT ?+?+?");
	transdb trans = transdb_open(db);
	int i = 0;
	int pids[num];
	for(;;) {
		int pid;
		if(++i == num+1) {
//			do_one(num, insert, trans);
			for(i=0;i<num;++i) {
				int status;
				int pid = waitpid(0, &status, 0);
				printf("%02d(%d) exited %d\n", i+1, pid, status);
			}
			break;
		} else {
			pid = fork();
			if(pid == 0) {
				usleep(i * 1000000 * drand48() + 500000);
				do_one(i, insert, trans);
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
