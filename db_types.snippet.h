#ifndef TYPE
#error wha
#endif

/* cpp sucks... */
#define BIND_NAME(TYPE) CONCATSYM(db_bind_, TYPE)
#define COL_NAME(TYPE) CONCATSYM(db_column_, TYPE)

#ifdef IMPLEMENTATION
#define SQLITE_BIND_NAME(TYPE) CONCATSYM(sqlite3_bind_, TYPE)
#define SQLITE_COL_NAME(TYPE) CONCATSYM(sqlite3_column_, TYPE)

EXPORT
int BIND_NAME(TYPE)(db_stmt stmt, int col, BIND_ARGS) {
	return SQLITE_BIND_NAME(TYPE)(stmt->sqlite, col, BIND_PARAMS);
}
EXPORT
COLUMN_RETURN COL_NAME(TYPE)(db_stmt stmt, int col) {
	return SQLITE_COL_NAME(TYPE)(stmt->sqlite, col);
}
#undef SQLITE_BIND_NAME
#undef SQLITE_COL_NAME

#else  /* IMPLEMENTATION */
/* interface... */
int BIND_NAME(TYPE)(db_stmt, int col, BIND_ARGS);
COLUMN_RETURN COL_NAME(TYPE)(db_stmt, int col);
#endif
#undef TYPE
#undef BIND_NAME
#undef COL_NAME
/* leave the rest for db_all_types.snippet.h to redefine */
