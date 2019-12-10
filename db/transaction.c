#include "transaction.h"
static
int transaction_in_transaction(basedb db, transdb_handler handler, void* udata) {
	return handler(db->conn, udata);
}
#define WRAPPER_NAME in_transaction

#include "runtime_transaction.c"
