#ifndef TYPE
#error wha
#endif

/* cpp sucks... */
#define BIND_NAME(TYPE) CONCATSYM(basedb_bind_, TYPE)
#define COL_NAME(TYPE) CONCATSYM(basedb_column_, TYPE)

#ifdef IMPLEMENTATION
#define SQLITE_BIND_NAME(TYPE) CONCATSYM(sqlite3_bind_, TYPE)
#define SQLITE_COL_NAME(TYPE) CONCATSYM(sqlite3_column_, TYPE)

EXPORT
result BIND_NAME(TYPE)(basedb_stmt stmt, int col, BIND_ARGS) {
	return check(db, SQLITE_BIND_NAME(TYPE)(stmt->sqlite, col, BIND_PARAMS));
}
EXPORT
COLUMN_RETURN COL_NAME(TYPE)(basedb_stmt stmt, int col) {
	return SQLITE_COL_NAME(TYPE)(stmt->sqlite, col);
}
#undef SQLITE_BIND_NAME
#undef SQLITE_COL_NAME

#else  /* IMPLEMENTATION */
/* interface... */
result BIND_NAME(TYPE)(basedb_stmt, int col, BIND_ARGS);
COLUMN_RETURN COL_NAME(TYPE)(basedb_stmt, int col);
#endif
#undef TYPE
#undef BIND_NAME
#undef COL_NAME
/* leave the rest for basedb_all_types.snippet.h to redefine */
