#define DB_RESTARTABLE_TRANSACTION_IMPL
#include "db_transactions.h"
#define VALUES handler, udata
static
int in_transaction(struct transdb* db, ARGUMENTS) {
	return handler(db, udata);
}
#define WRAPPER_NAME in_transaction

#include "db_restartable_transaction.c"
