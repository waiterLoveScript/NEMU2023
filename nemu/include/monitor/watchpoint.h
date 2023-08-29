#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	uint32_t val;
	char expr [32];
	struct watchpoint *next;

	/* TODO: Add more members if necessary */


} WP;

#endif
