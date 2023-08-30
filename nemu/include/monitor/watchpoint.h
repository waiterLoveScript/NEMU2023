#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	/*int NO;
	uint32_t val;
	char expr [32];
	struct watchpoint *next;
	int b;*/

	/* TODO: Add more members if necessary */
	int NO;
	struct watchpoint *next;
	char *expr;
	uint32_t new_val;
	uint32_t old_val;

} WP;

/*WP* new_wp ();
void free_wp(WP *);
bool check_wp();
void delete_wp(int );
void info_wp();*/

int set_watchpoint(char *e);
bool delete_watchpoint(int NO);
void list_watchpoint();
WP* scan_watchpoint();

#endif
