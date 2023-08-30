#include "monitor/watchpoint.h"
#include "monitor/expr.h"
//#include "nemu.h"
#include <stdlib.h>

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

/*WP* new_wp()
{
	WP *f,*p;
	f = free_;
	free_ = free_->next;
	f->next = NULL;
	p = head;
	if (p == NULL){head = f;p = head;}
	else {
		while (p->next!=NULL)p=p->next;
		p->next = f;
		}
	return f;
}
void free_wp (WP *wp)
{
	WP *f,*p;
	p = free_;
	if (p == NULL){free_ = wp;p = free_;}
	else {
		while (p->next!=NULL)p=p->next;
		p->next = wp;
	}
	f = head;
	//printf ("%d %d %d\n",f->NO,f->next->NO,wp->NO);
	if (head == NULL)assert (0);
	if (head->NO == wp->NO)
	{
		head = head->next;
	}
	else 
	{
	while (f->next != NULL && f->next->NO != wp->NO)f = f->next;
	if (f->next == NULL && f->NO == wp->NO)printf ("what ghost!");
	else if (f->next->NO == wp->NO)f->next = f->next->next;
	else assert (0);
	//if (head == NULL)printf ("*NULL\n");
	}
	wp->next = NULL;
	wp->val = 0;
	wp->b = 0;
	wp->expr[0] = '\0';
}
bool check_wp()
{
	WP *f;
	f = head;
	bool key = true;
	bool suc;
	while (f != NULL)
	{
		uint32_t tmp_expr = expr (f->expr,&suc);
		if (!suc)assert (1);
		if (tmp_expr != f->val)
		{
			key = false;
			if (f->b)
			{
				printf ("Hit breakpoint %d at 0x%08x\n",f->b,cpu.eip);
				f = f->next;
				continue;
			}
			printf ("Watchpoint %d: %s\n",f->NO,f->expr);
			printf ("Old value = %d\n",f->val);
			printf ("New value = %d\n",tmp_expr);
			f->val = tmp_expr;
		}
		f = f->next;
	}
	return key;
}
void delete_wp(int num)
{
	WP *f;
	f = &wp_pool[num];
	free_wp (f);
}
void info_wp()
{
	WP *f;
	f=head;
	while (f!=NULL)
	{
		printf ("Watchpoint %d: %s = %d\n",f->NO,f->expr,f->val);
		f = f->next;
	}
}*/

static WP* new_WP() {
	assert(free_ != NULL);
	WP *p = free_;
	free_ = free_->next;
	return p;
}

static void free_WP(WP *p) {
	assert(p >= wp_pool && p < wp_pool + NR_WP);
	free(p->expr);
	p->next = free_;
	free_ = p;
}

int set_watchpoint(char *e) {
	uint32_t val;
	bool success;
	val = expr(e, &success);
	if(!success) return -1;

	WP *p = new_WP();
	p->expr = strdup(e);
	p->old_val = val;

	p->next = head;
	head = p;

	return p->NO;
}

bool delete_watchpoint(int NO) {
	WP *p, *prev = NULL;
	for(p = head; p != NULL; prev = p, p = p->next) {
		if(p->NO == NO) { break; }
	}

	if(p == NULL) { return false; }
	if(prev == NULL) { head = p->next; }
	else { prev->next = p->next; }

	free_WP(p);
	return true;
}

void list_watchpoint() {
	if(head == NULL) {
		printf("No watchpoints\n");
		return;
	}

	printf("%8s\t%8s\t%8s\n", "NO", "Address", "Enable");
	WP *p;
	for(p = head; p != NULL; p = p->next) {
		printf("%8d\t%s\t%#08x\n", p->NO, p->expr, p->old_val);
	}
}

WP* scan_watchpoint() {
	WP *p;
	for(p = head; p != NULL; p = p->next) {
		bool success;
		p->new_val = expr(p->expr, &success);
		if(p->old_val != p->new_val) {
			return p;
		}
	}

	return NULL;
}

/*WP* new_wp() {

	if (free_ == NULL) return NULL;

	WP *f_top, *h_tail;
	f_top = free_;
	h_tail = head;
	free_ = free_ -> next;

	f_top -> next = NULL;

	if (h_tail == NULL) head = f_top;
	else {
		while (h_tail -> next != NULL)
			h_tail = h_tail -> next;
		h_tail -> next = f_top;
	}
	return f_top;
}

void free_wp(WP *wp) {

	WP *h, *f;
	h = head;
	f = free_;
	if (h == wp) {
		head = wp -> next;
	} else {
		while (h != NULL && h -> next != wp) {
			h = h -> next;
		}
		h -> next = h -> next -> next;
	}
	wp -> next = free_;
	free_ = wp;
	wp -> val = 0;
	wp -> expr[0] = '\0';
}

void print_w() {
	WP *h = head;
	while (h != NULL) {
		printf("[Watchpoint NO.%d]\tExpression: %s\tValue: %d\n", h -> NO, h -> expr, h -> val);
		h = h -> next;
	}
}

WP* delete_wp(int id, bool* f) {
	WP* ret = head;
	while (ret != NULL && ret -> NO != id) {
		ret = ret -> next;
	}
	if (ret == NULL) *f = false;
	return ret;
}

void delete_wp(int num)
{
	WP *f;
	f = &wp_pool[num];
	free_wp (f);
}

void check_wp(bool* f) {
	WP* h = head;
	while (h != NULL) {
		bool tmp = true;
		uint32_t nxtv = expr(h->expr, &tmp);
		if (nxtv != h -> val) {
			printf("[Watchpoint NO.%d]\tExpression: %s\tOrigin Value: 0x%x\tNew Value: 0x%x\n", h -> NO, h -> expr, h -> val, nxtv);
			h -> val = nxtv;
			*f = true;
		}
		h = h -> next;
	}
}*/


