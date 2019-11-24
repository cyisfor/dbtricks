#ifndef DB_TRANSACTIONS_H
#define DB_TRANSACTIONS_H

#include "db/base.h"

enum transaction_type {
	DEFERRED_TRANSACTION = 0,
	IMMEDIATE_TRANSACTION,
	EXCLUSIVE_TRANSACTION,
	TRANSACTION_TYPES
};

struct transdb {
	basedb conn;
	basedb_stmt begin[TRANSACTION_TYPES];
	basedb_stmt rollback;
	basedb_stmt commit;
};

#define FUNCTION_NAME transaction
#define ARGUMENTS int (*handler)(struct transdb*, void*), void* udata)
extern int transaction(struct transdb*, enum transaction_type, ARGUMENTS);
#include "db_restartable_transaction.h"

#define check_busy(expr) ({ result res = (expr); if(res

#endif /* DB_TRANSACTIONS_H */
