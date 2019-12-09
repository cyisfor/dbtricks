#include "db/base.h"

int main(int argc, char *argv[])
{
    basedb db = basedb_open("/tmp/deletethis.sqlite", false);
	basedb_exec(db, "CREATE TABLE IF NOT EXISTS foo (id INTEGER PRIMARY KEY, val INTEGER)");
	basedb_stmt insert = basedb_prepare(db, "INSERT INTO foo (val) VALUES (?)");
	basedb_bind_int(insert, 1, 42);
	basedb_once(insert);
	basedb_finalize(insert);
	basedb_close(db);
    return 0;
}
