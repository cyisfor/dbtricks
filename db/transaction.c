#include "transaction.h"
#include "transaction.struct.h"

/* #include <glib.h> */
#define g_slice_new0(type) calloc(1,sizeof(type))
#define g_slice_free(type, var) free(var)

transdb transdb_open(basedb conn) {
	transdb db = g_slice_new0(struct transdb);
	db->conn = conn;
	return db;
}
void transdb_close(transdb db) {
#define CLEANUP(member) \
	if(db->member) { \
		basedb_finalize(db->member); \
		db->member = NULL; \
	}
	CLEANUP(begin[DEFERRED_TRANSACTION]);
	CLEANUP(begin[IMMEDIATE_TRANSACTION]);
	CLEANUP(begin[EXCLUSIVE_TRANSACTION]);
	CLEANUP(rollback);
	CLEANUP(commit);
	g_slice_free(struct transdb, db);
}


static
result transaction_in_transaction(basedb db, transdb_handler handler, void* udata) {
	return handler(db, udata);
}
#define WRAPPER_NAME in_transaction

#include "runtime_transaction.c"
