{
	BIND(SELINS.select, SELINS_SELECT);
	result res = basedb_step(SELINS.select);
	identifier ident;
	if(res == result_pending) {
		ident = basedb_column_identifier(SELINS.select, 0);
		basedb_reset(SELINS.select);
		if(SELINS.update) {
			//BIND_UNIQUE(SELINS.update);
			BIND(SELINS.update, SELINS_UPDATE);
			basedb_bind_identifier(SELINS.update, SELINS.identparam, ident);
			basedb_once(SELINS.update);
		}
	} else {
		basedb_reset(SELINS.select);
		BIND(SELINS.insert, SELINS_INSERT);
		basedb_once(SELINS.insert);
		ident = basedb_lastrow(SELINS.db);
	}
	RETURN(ident);
}
