#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include "elf.c"

enum {
	NOTYPE = 256, EQ, NEQ, AND, OR, MINUS, POINTER, NUMBER, HNUMBER, REGISTER, MARK, NOT, PLUS, DIV, NEG, LB, RB, TIMES

	/* TODO: Add more tokens types */

};

static struct rule {
	char *regex;
	int token_type;
	int priority;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE, 0},							// spaces
	{"\\b[0-9]+\\b",NUMBER, 0},					// number
	{"\\b0[xX][0-9a-fA-F]+\\b",HNUMBER, 0},		// 16 number
	{"\\$[a-zA-Z]+",REGISTER, 0},				// register
	{"\\b[a-zA-Z_0-9]+" , MARK, 0},				// mark
	{"!=", NEQ, 3},								// not equal	
	{"!", NOT, 6},								// not
	{"\\*", TIMES, 5},							// mul
	{"/", DIV, 5},								// div
	{"	+", NOTYPE, 0},							// tabs
	{"\\+", PLUS, 4},							// plus
	{"==", EQ, 3},								// equal
	{"-", '-', 4},								// sub
	{"&&", AND, 2},								// and
	{"\\|\\|", OR, 1},							// or
	{"\\(", LB, 7},                       		// left bracket   
	{"\\)", RB, 7},                        		// right bracket

};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct tokens {
	int type;
	char str[32];
	int priority;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;

				char *tmp = e + position + 1;

				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new tokens is recognized with rules[i]. Add codes
				 * to record the tokens in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case NOTYPE: break;

					case REGISTER:
						tokens[nr_token].type = rules[i].token_type;
						tokens[nr_token].priority = rules[i].priority; 
						strncpy (tokens[nr_token].str,tmp,substr_len-1);
						tokens[nr_token].str[substr_len-1]='\0';
						nr_token ++;
						break;

					default:
						tokens[nr_token].type = rules[i].token_type;
						tokens[nr_token].priority = rules[i].priority;
						strncpy (tokens[nr_token].str,substr_start,substr_len);
						tokens[nr_token].str[substr_len]='\0';
						nr_token ++;
						panic("please implement me");
				}
				position += substr_len;
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

bool check_parentheses (int l,int r)
{
	int i;
	if (tokens[l].type == '(' && tokens[r].type ==')')
	{
		int lc = 0, rc = 0;
		for (i = l + 1; i < r; i ++)
		{
			if (tokens[i].type == '(')lc ++;
			if (tokens[i].type == ')')rc ++;
			if (rc > lc)return false;	
		}
		if (lc == rc)return true;
	}
	return false;
}
int dominant_operator (int l,int r)
{
	int i,j;
	int min_priority = 10;
	int oper = l;
	for (i = l; i <= r;i ++)
	{
		if (tokens[i].type == NUMBER || tokens[i].type == HNUMBER || tokens[i].type == REGISTER || tokens[i].type == MARK)
			continue;
		int cnt = 0;
		bool key = true;
		for (j = i - 1; j >= l ;j --)
		{ 
			if (tokens[j].type == '(' && !cnt){key = false;break;}
			if (tokens[j].type == '(')cnt --;
			if (tokens[j].type == ')')cnt ++; 
		}
		if (!key)continue;
		if (tokens[i].priority <= min_priority){min_priority = tokens[i].priority;oper = i;}
 	}
	return oper;
}

uint32_t eval(int l, int r, bool *success) {

	if(l > r) return *success = false;
	if(l == r) {
		uint32_t tmp;
		if(tokens[l].type == HNUMBER) {
			sscanf(tokens[l].str, "%x", &tmp);
			return tmp;
		}else if(tokens[l].type == NUMBER) {
			sscanf(tokens[l].str, "%d", &tmp);
			return tmp;
		}else if(tokens[l].type == REGISTER) {
			const char *RE[] = {"$eax", "$ecx", "$edx", "$ebx", "$esp", "$ebp", "$esi", "$edi"};
			const char *REB[] = {"$EAX", "$ECX", "$EDX", "$EBX", "$ESP", "$EBP", "$ESI", "$EDI"};
			int i;
			if(strcmp(tokens[l].str, "$eip") == 0 || strcmp(tokens[l].str, "$EIP") == 0) return cpu.eip;
			for(i = 0; i < 8; i++)
				if(strcmp(tokens[l].str, RE[i]) == 0 || strcmp(tokens[l].str, REB[i]) == 0)
					return cpu.gpr[i]._32;
			return *success = false;
		}
	}

	bool flag = check_parentheses(l, r);
	if(!*success) 	return 0;
	if(flag)
		return eval(l + 1, r - 1, success);
	int nxtPro = 10, i, cnt = 0, nxt = l;
	for(i = l; i <= r; i++) {
		if(tokens[i].type == LB) cnt++;
		if(tokens[i].type == RB) cnt--;
		if(cnt == 0) {
			if(tokens[i].type >= PLUS && tokens[i].type < LB && tokens[i].priority <= nxtPro)
				nxt = i, nxtPro = tokens[i].priority;
		}
	}
	//Log("%d %d", nxt, tokens[nxt].priority);
	assert(cnt == 0);
	if (l == nxt || tokens[nxt].type == POINTER || tokens[nxt].type == NEG || tokens[nxt].type == NOT) {
		uint32_t val = eval(l + 1, r, success);
		switch (tokens[l].type) {
			case POINTER:
				return swaddr_read(val, 4);
			case NEG:
				return -val;
			case NOT:
				return !val;
			default :
				break;
		}
		panic("error 1");
	}
	uint32_t a = eval(l, nxt - 1, success);
	uint32_t b = eval(nxt + 1, r, success);
	switch (tokens[nxt].type) {
		case PLUS:
			return a + b;
			break;
		case MINUS:
			return a - b;
			break;
		case TIMES:
			return a * b;
			break;
		case DIV:
			return a / b;
			break;
		case EQ:
			return a == b;
			break;
		case NEQ:
			return a != b;
			break;
		case AND:
			return a && b;
			break;
		case OR:
			return a || b;
			break;
	}
	panic("error 2");
	return 0;
}


uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	int i;
	for (i = 0;i < nr_token; i ++) {
 		if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != NUMBER && tokens[i - 1].type != HNUMBER && tokens[i - 1].type != REGISTER && tokens[i - 1].type != MARK && tokens[i - 1].type !=')'))) {
			tokens[i].type = POINTER;
			tokens[i].priority = 6;
		}
		if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != NUMBER && tokens[i - 1].type != HNUMBER && tokens[i - 1].type != REGISTER && tokens[i - 1].type != MARK && tokens[i - 1].type !=')'))) {
			tokens[i].type = MINUS;
			tokens[i].priority = 6;
 		}
  	}

	/* TODO: Insert codes to evaluate the expression. */
	panic("please implement me");
	*success = true;
	return 0;
}

