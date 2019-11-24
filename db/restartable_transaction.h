/*

	sqlite3 has an interesting strategy for lock contention that doesn't play well with C programs.
	Basically if two transactions both need mutually blocking locks (as happens stupidly often any time you
	use exclusive transactions), then one will return SQLITE_BUSY from sqlite3_step or the like. The other one
	will either also return SQLITE_BUSY, or will block forever, freezing eternally. The idea is that the one that
	gets the SQLITE_BUSY will then rollback the transaction, so that the other one can proceed uncontested.

	In practice, that works out terribly, because the response to SQLITE_BUSY is to just kill the process, leaving
	tons of secretly uncommitted data lost because sqlite3_close never returned SQLITE_OK. So to do it properly,
	what we need is some way to rollback the transaction, then go back and start the code inside our transaction
	all over again. The process doesn't need to die then. We waste time repeating steps, but they should be repeated,
	since the underlying database likely changed while they were happening.

	So in other words, every transaction should be like this:
	LOOP:
	  BEGIN;
		do things;
		something returns BUSY, rollback wait, then goto LOOP
		something returns error, rollback and die
		everything finishes, commit

	NOT every savepoint! This is only non-nested transactions!
 */

int FUNCTION_NAME(struct transdb* db, enum transaction_type type, ARGUMENTS);

#ifndef DB_RESTARTABLE_TRANSACTION_IMPL
#undef FUNCTION_NAME
#undef ARGUMENTS
#endif
