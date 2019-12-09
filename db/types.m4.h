m4_ifdef({{IMPLEMENTATION}}, {{m4_dnl;
EXPORT
result basedb_bind_{{}}TYPE{{}}(N(stmt) stmt, int col, BIND_ARGS) {
	return check(stmt->db,
				 sqlite3_bind_{{}}TYPE{{}}(stmt->sqlite, col, BIND_PARAMS));
}
m4_ifdef({{COLUMN_RETURN}}, {{m4_dnl;
EXPORT
COLUMN_RETURN basedb_column_{{}}TYPE(N(stmt) stmt, int col) {
	return sqlite3_column_{{}}TYPE(stmt->sqlite, col);
}
}})
}},{{
m4_dnl; /* else not IMPLEMENTATION */
result basedb_bind_{{}}TYPE{{}}(N(stmt), int col, BIND_ARGS);
m4_ifdef({{COLUMN_RETURN}}, {{m4_dnl
COLUMN_RETURN basedb_column_{{}}TYPE{{}}(N(stmt), int col);
}})m4_dnl; 						/* COLUMN_RETURN */
}})m4_dnl;					/* not IMPLEMENTATION */
