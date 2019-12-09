m4_divert({{-1}});
m4_define(WRAPPER_NAME, FUNCTION_NAME{{}}_in_transaction);
m4_define(PREPARE, {{$1}}->{{$2}} = basedb_prepare({{$1}}->conn, LITLEN({{$3}})));
m4_divert{{}}m4_dnl ;
WRAPPER_NAME;
PREPARE(self, {{begin[type]}}, "BEGIN IMMEDIATE TRANSACTION");

#ifndef WRAPPER_NAME
#define WRAPPER_NAME symjoin(FUNCTION_NAME, _in_transaction)
#endif

#define PREPARE(self, member, sql)										\
	self->member = basedb_prepare(self->conn,									\
						  LITLEN(sql),									\
						  &self->member, NULL));

int FUNCTION_NAME{{}}(struct transdb* db, enum transaction_type type, ARGUMENTS) {
	if(!db->begin[type]) {
		switch(type) {
		case DEFERRED_TRANSACTION:
			PREPARE(db, begin[type], "BEGIN");
			break;
		case IMMEDIATE_TRANSACTION:
			PREPARE(db, begin[type], "BEGIN IMMEDIATE TRANSACTION");
			break;
		case EXCLUSIVE_TRANSACTION:
			PREPARE(db, begin[type], "BEGIN EXCLUSIVE TRANSACTION");
			break;
		};
		if(!db->commit)
			PREPARE(db, commit, "COMMIT");
	}
	for(;;) {
		basedb_once(db->begin[type]);
		int res = WRAPPER_NAME(db, VALUES);
		switch(res) {
		case SQLITE_BUSY:
			if(!db->rollback)
				PREPARE(db->rollback);
			basedb_once(db->rollback);
			continue;
		case SQLITE_OK:
		case SQLITE_DONE:
		case SQLITE_ROW:
			basedb_once(db->commit);
			return res;
		default:
			basedb_once(db->rollback);
			CHECK(res);
		};
	}
}
		
#undef WRAPPER_NAME
#undef FUNCTION_NAME
#undef ARGUMENTS
#undef VALUES
