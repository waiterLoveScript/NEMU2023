#include "cpu/exec/template-start.h"

#define make_set_helper(temp) \
	make_helper(concat(set, temp)) { \
		int len = decode_rm_b(eip + 1); \
		write_operand_b(op_src, concat(check_jcc_, temp)()); \
		print_asm(str(concat(set, temp)) " %s", op_src->str); \
		return len + 1; \
	}

make_set_helper(ne);
make_set_helper(e);
#include "cpu/exec/template-end.h"
