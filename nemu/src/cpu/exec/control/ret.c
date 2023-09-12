#include "cpu/exec/helper.h"

make_helper(ret){
	cpu.eip = swaddr_read(cpu.esp, 4) - 1;
	cpu.esp = cpu.esp + 4;
	print_asm("ret");
	return 1;
}

make_helper(ret_i_w){
	uint16_t temp = instr_fetch(eip + 1, 2);
	cpu.eip = swaddr_read(cpu.esp, 4) - 3;
	cpu.esp += 4 + temp;

	print_asm("ret $0x%x", temp);

	return 3;

}
