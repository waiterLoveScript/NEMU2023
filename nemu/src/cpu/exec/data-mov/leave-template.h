#include "cpu/exec/template-start.h"

#define instr leave

make_helper(leave){
    cpu.esp=cpu.ebp;
    /*only 32*/
    cpu.ebp=MEM_R(cpu.esp);
    cpu.esp+=4;
    print_asm_template1();
    return 1;
}

#include "cpu/exec/template-end.h"