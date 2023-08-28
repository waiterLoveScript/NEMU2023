#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include "elf.c"

enum {
	NOTYPE = 256, EQ, NEQ, AND, OR, MINUS, POINTOR, NUMBER, HNUMBER, REGISTER, MARK, NOT

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
	{"\\*", '*', 5},							// mul
	{"/", '/', 5},								// div
	{"	+", NOTYPE, 0},							// tabs
	{"\\+", '+', 4},							// plus
	{"==", EQ, 3},								// equal
	{"-", '-', 4},								// sub
	{"&&", AND, 2},								// and
	{"\\|\\|", OR, 1},							// or
	{"\\(", '(', 7},                       		// left bracket   
	{"\\)", ')', 7},                        	// right bracket

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

int eval(int p, int q){
	int result = 0;
	int op;
	int val1, val2;
	if (p > q){
		assert(0);
	} else if (p == q){
		if (tokens[p].type == NUMBER){
			sscanf(tokens[p].str, "%d", &result);
			return result;
		} else if (tokens[p].type == HNUMBER){
			int i = 2;
			while(tokens[p].str[i] != 0){
				result *= 16;
				result += tokens[p].str[i] < 58 ? tokens[p].str[i] - '0' : tokens[p].str[i] - 'a' + 10;
				i++;
		}
		} else if (tokens[p].type == REGISTER){
			if (!strcmp(tokens[p].str, "$eax")){
					return cpu.eax;
				} else if (!strcmp(tokens[p].str, "$ecx")){
					return cpu.ecx;
				} else if (!strcmp(tokens[p].str, "$edx")){
					return cpu.edx;
				} else if (!strcmp(tokens[p].str, "$ebx")){
					return cpu.ebx;
				} else if (!strcmp(tokens[p].str, "$esp")){
					return cpu.esp;
				} else if (!strcmp(tokens[p].str, "$ebp")){
					return cpu.ebp;
				} else if (!strcmp(tokens[p].str, "$esi")){
					return cpu.esi;
				} else if (!strcmp(tokens[p].str, "$edi")){
					return cpu.edi;
				} else if (!strcmp(tokens[p].str, "$eip")){
					return cpu.eip;
				} else {
					return 0;
				}
		} else {
			assert(0);
			}
	 } else if (check_parentheses(p, q) == true){
	 	return eval(p + 1, q - 1);
	 } else {
		op = dominant_operator(p, q);
	 	if (op == -2){
			assert(0);
		} else if (tokens[p].type == '!'){
				sscanf(tokens[q].str, "%d", &result);
				return !result;
			} else if (tokens[p].type == REGISTER) {
				if (!strcmp(tokens[p].str, "$eax")){
					result = cpu.eax;
					return result;
				} else if (!strcmp(tokens[p].str, "$ecx")){
					result = cpu.ecx;
					return result;
				} else if (!strcmp(tokens[p].str, "$edx")){
					result = cpu.edx;
					return result;
				} else if (!strcmp(tokens[p].str, "$ebx")){
					result = cpu.ebx;
					return result;
				} else if (!strcmp(tokens[p].str, "$esp")){
					result = cpu.esp;
					return result;
				} else if (!strcmp(tokens[p].str, "$ebp")){
					result = cpu.ebp;
					return result;
				} else if (!strcmp(tokens[p].str, "$esi")){
					result = cpu.esi;
					return result;
				} else if (!strcmp(tokens[p].str, "$edi")){
					result = cpu.edi;
					return result;
				} else if (!strcmp(tokens[p].str, "$eip")){
					result = cpu.eip;
					return result;
				} else {
					assert(0);
					return 0;
				}
			}
		}
		val1 = eval(p, op - 1);
		val2 = eval(op + 1, q);

		switch (tokens[op].type){
			case '+' : return val1 + val2;	
			case '-' : return val1 - val2;
			case '*' : return val1 * val2;
			case '/' : return val1 / val2;
			case OR : return val1 || val2;
			case AND : return val1 && val2;
			case EQ : 
				   if (val1 == val2){
					return 1;
				   } else {
					return 0;
				   }
			case NEQ :
				   if (val1 != val2){
					return 1;
				    } else {
					return 0;
				    }
			default : assert(0);
		}
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
			tokens[i].type = POINTOR;
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

