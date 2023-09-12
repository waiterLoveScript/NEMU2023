#ifndef __EFLAGS_H__
#define __EFLAGS_H__

#include "common.h"

void update_eflags_pf_zf_sf(uint32_t);

static inline bool check_jcc_e(){
	if(cpu.eflags.ZF == 1)
		return 1;
	else
		return 0;
}

static inline bool check_jcc_be(){
	return cpu.eflags.CF | cpu.eflags.ZF;
}

static inline bool check_jcc_ne(){
	return !cpu.eflags.ZF;
}

static inline bool check_jcc_le(){
	if(cpu.eflags.ZF == 1 || cpu.eflags.SF != cpu.eflags.OF){
		return 1;
	}
	else 
		return 0;
}

static inline bool check_jcc_g(){
	if(cpu.eflags.ZF == 0 && cpu.eflags.SF == cpu.eflags.OF)
		return 1;
	else
		return 0;
}
static inline bool check_jcc_l(){
	if(cpu.eflags.SF != cpu.eflags.OF)
		return 1;
	else 
		return 0;
}
static inline bool check_jcc_s(){
	if(cpu.eflags.SF == 1)
		return 1;
	else
		return 0;
}
static inline bool check_jcc_ns(){
	if(cpu.eflags.SF == 0)
		return 1;
	else
		return 0;
}
static inline bool check_jcc_ge(){
	if(cpu.eflags.SF == cpu.eflags.OF)
		return 1;
	else
		return 0;
}
static inline bool check_jcc_a(){
	if(cpu.eflags.CF == 0 && cpu.eflags.ZF == 0)
		return 1;
	else
		return 0;
}
static inline bool check_jcc_b(){
	if(cpu.eflags.CF == 1)
		return 1;
	else
		return 0;
}
#endif
