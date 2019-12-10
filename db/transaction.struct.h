#ifndef TRANSACTION_STRUCT_H
#define TRANSACTION_STRUCT_H

struct transdb {
	basedb conn;
	basedb_stmt begin[TRANSACTION_TYPES];
	basedb_stmt rollback;
	basedb_stmt commit;
};


#endif /* TRANSACTION_STRUCT_H */
