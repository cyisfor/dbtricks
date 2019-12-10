struct transdb {
	basedb conn;
	basedb_stmt begin[TRANSACTION_TYPES];
	basedb_stmt rollback;
	basedb_stmt commit;
};
