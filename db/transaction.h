#ifndef DB_TRANSACTIONS_H
#define DB_TRANSACTIONS_H

#include "db/base.h"

enum transaction_type {
	DEFERRED_TRANSACTION = 0,
	IMMEDIATE_TRANSACTION,
	EXCLUSIVE_TRANSACTION,
	TRANSACTION_TYPES
};

struct transdb* transdb;

typedef int (*transdb_handler)(basedb, void*);
extern int transaction(transdb, enum transaction_type,
					   transdb_handler, void*);

transdb transdb_open(basedb);
void transdb_close(transdb);

#endif /* DB_TRANSACTIONS_H */
