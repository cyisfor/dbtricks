m4_divert(`-1')
m4_include(`m4/c.m4')
m4_define({{FUNCTION_NAME}}, {{testfoo}})
m4_divert{{}}m4_dnl
m4_include({{db/restartable_transaction.m4.c}})
