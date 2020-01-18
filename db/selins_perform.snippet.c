{
	BIND_UNIQUE(SELINS.select);
	result res = basedb_step(SELINS.select);
	identifier ident;
	if(res == result_pending) {
		ident = basedb_column_identifier(SELINS.select, 0);
		basedb_reset(SELINS.select);
		if(SELINS.update) {
			BIND_UNIQUE(SELINS.update, false);
			BIND_INSERT(SELINS.update);
			basedb_bind_identifier(stmt, SELINS.identparam, ident);
			basedb_once(SELINS.update);
		}
	} else {
		basedb_reset(SELINS.select);
		BIND_UNIQUE(SELINS.insert, true);
		BIND_INSERT(SELINS.insert);
		basedb_step(SELINS.insert);
		ident = basedb_last_insert(SELINS.db);
	}
	RETURN(ident);
}
