#include "parser.h"
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"


typedef struct buffer
{
	int last_read_desc;
	int last_write_desc;
	enum expr_type last_operand;
} buffer;

static void execute_command_line(const struct command_line *line)
{
	/* REPLACE THIS CODE WITH ACTUAL COMMAND EXECUTION */

	assert(line != NULL);
	const struct expr *e = line->head;

	int std_descs[2];
	pipe(std_descs);
	dup2(std_descs[0], 0);
	
	if (line->out_type==1){
		int filedesc = open(line->out_file,"a");
		dup2(std_descs[1],filedesc);
	}else{
		dup2(std_descs[1],1);
	}

	buffer buf;
	buf.last_read_desc = std_descs[0];
	buf.last_write_desc = std_descs[1];
	buf.last_operand = EXPR_TYPE_COMMAND;

	while (e != NULL)
	{
		if (e->type == EXPR_TYPE_COMMAND)
		{
			char *arguments[e->cmd.arg_count];
			int i = 0;
			for (char *arg = e->cmd.args[i++]; i < (int)e->cmd.arg_count; arg = e->cmd.args[i++])
			{
				// printf("%c", arg[0]);
				arguments[i++] = arg;
			}
			// int p_descs[2];
			// pipe(p_descs);
			
			// надо чтобы печатать мог только один поток
			if (fork() == 0)
			{
				// buf.last_read_desc = &p_descs[0];
				// buf.last_write_desc = &p_descs[1];
				printf("DBG %s\n", e->cmd.exe);

				execvp(e->cmd.exe, arguments);

				return 0;
			}
			
		}
		else if (e->type == EXPR_TYPE_PIPE)
		{

			printf("DBG pipe %d\n", e->cmd.arg_count);
		}
		else if (e->type == EXPR_TYPE_AND)
		{
			int ret = wait(NULL);
			// if !ret = 0 do not execute next command
			printf("DBG and\n");
		}
		else if (e->type == EXPR_TYPE_OR)
		{
			int ret = wait(NULL);
			//if !ret = 1 do not execute next command
			printf("DBG or\n");
		}
		else
		{

			assert(false);
		}
		buf.last_operand = e->type;
		e = e->next;
	}
	wait(NULL);
}

int main(void)
{
	const size_t buf_size = 1024;
	char buf[buf_size];
	int rc;
	struct parser *p = parser_new();
	while ((rc = read(STDIN_FILENO, buf, buf_size)) > 0)
	{
		parser_feed(p, buf, rc);
		struct command_line *line = NULL;
		while (true)
		{
			enum parser_error err = parser_pop_next(p, &line);
			if (err == PARSER_ERR_NONE && line == NULL)
				break;
			if (err != PARSER_ERR_NONE)
			{
				printf("Error: %d\n", (int)err);
				continue;
			}
			execute_command_line(line);
			command_line_delete(line);
		}
	}
	parser_delete(p);
	return 0;
}
