#ifndef TYPE
#error wha
#endif	/* TYPE */

/* cpp sucks... */
#define BIND_NAME(TYPE) CONCATSYM(basedb_bind_, TYPE)
#ifdef COLUMN_RETURN
#define COL_NAME(TYPE) CONCATSYM(basedb_column_, TYPE)
#endif	/* COLUMN_RETURN */

#ifdef IMPLEMENTATION
#define SQLITE_BIND_NAME(TYPE) CONCATSYM(sqlite3_bind_, TYPE)
#define SQLITE_COL_NAME(TYPE) CONCATSYM(sqlite3_column_, TYPE)

EXPORT
result BIND_NAME(TYPE)(basedb_stmt stmt, int col, BIND_ARGS) {
	return check(stmt->db,
				 SQLITE_BIND_NAME(TYPE)(stmt->sqlite, col, BIND_PARAMS));
}
#ifdef COLUMN_RETURN
EXPORT
COLUMN_RETURN COL_NAME(TYPE)(basedb_stmt stmt, int col) {
	return SQLITE_COL_NAME(TYPE)(stmt->sqlite, col);
}
#endif	/* COLUMN_RETURN */
#undef SQLITE_BIND_NAME
#undef SQLITE_COL_NAME

#else  /* IMPLEMENTATION */
/* interface... */
result BIND_NAME(TYPE)(basedb_stmt, int col, BIND_ARGS);
#ifdef COLUMN_RETURN
COLUMN_RETURN COL_NAME(TYPE)(basedb_stmt, int col);
#endif	/* COLUMN_RETURN */
#endif	/* IMPLEMENTATION */
#undef TYPE
#undef BIND_NAME
#undef BIND_ARGS
#undef BIND_PARAMS
#ifdef COLUMN_RETURN
#undef COL_NAME
#undef COLUMN_RETURN
#endif /* COLUMN_RETURN */
/* leave the rest for basedb_all_types.snippet.h to redefine */
