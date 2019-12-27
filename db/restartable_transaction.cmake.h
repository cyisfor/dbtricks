#include "base.h"
#include "result.h"

result @WRAPPER_NAME@(basedb db@ARGUMENTS@);

result @FUNCTION_NAME@(struct transdb* db,
						 enum transaction_type type@ARGUMENTS@) {
