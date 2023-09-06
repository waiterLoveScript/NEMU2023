#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat3(instr,_i_,SUFFIX)){
    int len = concat(decode_i_,SUFFIX)(eip+1);
    len++;
    REG(R_ESP) -= 4;
    swaddr_write(reg_l(R_ESP), 4,eip + len);

    DATA_TYPE_S disp = op_src->val;
    print_asm("call " str(SUFFIX) " 0x%x",eip +len +disp);
    cpu.eip += disp;
    //printf("cpu.eip = %x\n",cpu.eip);
    return len;
}
make_helper(concat3(instr,_rm_,SUFFIX)){
    int len = concat(decode_rm_,SUFFIX)(eip+1);
    len++;
    REG(R_ESP) -= 4;
    swaddr_write(reg_l(R_ESP), 4, eip + len);
    DATA_TYPE_S disp = op_src-> val;
    print_asm("call " str(SUFFIX) " 0x%x",disp);
    cpu.eip = disp;
   return 0;
}

#include "cpu/exec/template-end.h"