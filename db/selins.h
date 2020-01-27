#include "base.h"

enum selins_bind_mode {
	SELINS_SELECT,
	SELINS_UPDATE,
	SELINS_INSERT
};

struct selins {
	basedb db;
	basedb_stmt select;
	basedb_stmt insert;
	basedb_stmt update;
	int identparam;
};

static struct selins create_selins(basedb db, const string select, const string insert, const string update, int identparam) {
	struct selins ret = {
		.db = db,
		.select = basedb_prepare_str(db, select),
		.insert = basedb_prepare_str(db, insert),
		.identparam = identparam,
	};
	if(update.len > 0) {
		ret.update = basedb_prepare_str(db, update);
	}
	return ret;
}
	
#define CREATE_SELINS(db, select, insert, update, identparam) create_selins(\
		db,																\
		LITSTR("SELECT id FROM " select),								\
		LITSTR("INSERT INTO " insert),									\
		( LITSIZ(update) == 0 ? (const string){} : LITSTR("UPDATE " update " WHERE id = ?" #identparam)), \
		identparam)
