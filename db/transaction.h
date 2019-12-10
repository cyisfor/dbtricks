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

typedef transdb_handler int (*handler)(struct transdb*, void*);
extern int transaction(struct transdb*, enum transaction_type,  transdb_handler, void*);

#endif /* DB_TRANSACTIONS_H */
