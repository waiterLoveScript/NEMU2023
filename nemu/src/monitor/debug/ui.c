#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_info(char *args);

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "One step", cmd_si},
	{ "info", "Display register status or monitoring point information", cmd_info},
	{ "x", "Display memory content", cmd_x},
	{ "p", "Expression evaluation", cmd_p},
	{ "w", "Add a WatchPoint", cmd_w},
	{ "d", "Delete a WatchPoint", cmd_d}

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

static int cmd_si(char *args) {
	char *arg = strtok(NULL, " ");
	int step = 0, i;
	if(arg == NULL) {
		cpu_exec(1);
		return 0;
	}
	sscanf(arg, "%d", &step);
	if(step <= 0) {
		printf("Invalid step number\n");
		return 0;
	}
	for(i = 0; i < step; i ++) {
		cpu_exec(1);
	}
	return 0;
}

static int cmd_info(char *args) {
	char *arg = strtok(NULL, " ");
	printf("%s\n", arg);
	if(strcmp(arg, "r") == 0) {
		printf("eax is %x\n", cpu.eax);
		printf("ebx is %x\n", cpu.ebx);
		printf("ecx is %x\n", cpu.ecx);
		printf("edx is %x\n", cpu.edx);
		printf("esi is %x\n", cpu.esi);
		printf("edi is %x\n", cpu.edi);
		printf("ebp is %x\n", cpu.ebp);
		printf("esp is %x\n", cpu.esp);
		printf("-------------------------\n");
	}
	else {
		printf("Invalid Input\n");
	}
	return 0;
}

static int cmd_x(char *args) {
	/*char *arg1 = strtok(NULL, " ");
	char *arg2 = strtok(NULL, " ");
	int step, i, j = 0;
	swaddr_t sw_addr;
	sscanf(arg1, "%d", &step);
	sscanf(arg2, "%x", &sw_addr);
	for(i = 0; i < step; i ++) {
		if(j%4 == 0) {
			printf("0x%x:", sw_addr);
		}
		printf("0x%08x ", swaddr_read(sw_addr, 4));
		sw_addr += 4;
		j++;
		if(j%4 == 0) {
			printf("\n");
		}
	}
	printf("------------------------\n");
	return 0;*/
	int n;
	swaddr_t start_address;
	int i;
	bool suc;
	char *cmd = strtok(args, " ");
	sscanf (cmd,"%d",&n);
	args = cmd + strlen(cmd) + 1;
	start_address = expr (args,&suc);
	if (!suc)assert (1);
	printf ("0x%08x: ",start_address);
	for (i=1;i<=n;i++)
	{
		printf ("0x%08x ",swaddr_read (start_address,4));
		start_address+=4;
	}
	printf ("\n");
	return 0;
}

static int cmd_p(char *args) {
	if(args == NULL) return 0;
	//char *arg = strtok(NULL, " ");
	bool success = true;
	uint32_t EXPR = expr(args, &success);
	if(!success) {
		printf("Expression Error!\n");
		return 0;
	}
	printf("0x%x\n", EXPR);
	return 0;
}

static int cmd_w(char *args) {
	if(args == NULL) return 0;
	WP *wp;
	bool success = true;
	uint32_t value;
	value = expr(args, &success);
	if(!success) {
		printf("Expression Error!\n");
		return 0;
	}
	wp = new_wp();
	strcpy(wp->expr, args);
	wp->val = value;
	printf("WatchPoint %d : %s is set.\n", wp->NO, wp->expr);
	printf("Value : 0x%x\n", wp->val);
	return 0;
}

static int cmd_d(char *args) {
	int num;
	if (args==NULL) {
		printf("Need more arguments, please inset an integer to appoint the WatchPoint.\n");
	} else {
		num = atoi(args);
		delete_wp(num);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
