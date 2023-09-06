#include "cpu/exec/template-start.h"

#define instr ja

static void do_execute() {
	DATA_TYPE_S disp = op_src->val;
    if(cpu.eflags.ZF == 0 && cpu.eflags.CF == 0) 
		cpu.eip += disp;

	print_asm_template1();
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"
