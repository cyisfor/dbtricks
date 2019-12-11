#ifndef DB_TRANSACTIONS_H
#define DB_TRANSACTIONS_H

#include "db/base.h"
#include "result.h"

enum transaction_type {
	DEFERRED_TRANSACTION = 0,
	IMMEDIATE_TRANSACTION,
	EXCLUSIVE_TRANSACTION,
	TRANSACTION_TYPES
};

typedef struct transdb* transdb;

typedef result (*transdb_handler)(basedb, void*);
extern result transaction(transdb, enum transaction_type,
						transdb_handler, void*);

transdb transdb_open(basedb);
void transdb_close(transdb);

#define transdb_check(a) if(a == result_busy) {
	return cleanup();  \
}

#endif /* DB_TRANSACTIONS_H */
