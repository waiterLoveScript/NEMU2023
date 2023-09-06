#include "cpu/exec/template-start.h"

#define instr js

static void do_execute() {
	DATA_TYPE_S disp = op_src->val;
    if(cpu.eflags.SF) cpu.eip += disp;
	print_asm_template1();
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"
