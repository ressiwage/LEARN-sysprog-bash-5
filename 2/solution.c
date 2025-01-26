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

	// int std_out[2];
	// pipe(std_out);
	int out_descriptor = line->out_type==1?open(line->out_file, O_APPEND | O_CREAT | O_RDWR):STDOUT_FILENO; //дескриптор записи
	
	printf("out_file %d\n", e->cmd.arg_count);
	
	
	buffer buf;
	// buf.last_read_desc = std_descs[0];
	// buf.last_write_desc = std_descs[1];
	buf.last_operand = EXPR_TYPE_COMMAND;
	while (e != NULL)
	{
		if (e->type == EXPR_TYPE_COMMAND)
		{
			char buf[1024];
			
			char **arguments[e->cmd.arg_count+2];
			int i = 0;
			arguments[0] = e->cmd.exe;
			for (int i =0; i < (int)e->cmd.arg_count; i++)
			{
				// arguments[1+i++] = arg;
				char *arg = e->cmd.args[i];
				// printf("%d, %s\n",i, arg);
				// sprintf(arguments[1+i++], "%s", *arg);
				arguments[i+1] = arg;
			}
			arguments[i+2] = NULL;

			
			if (fork() == 0)
			{
				close(STDIN_FILENO);
				dup2(out_descriptor, STDOUT_FILENO);
				// sprintf(buf, "DBG %s\n", e->cmd.exe);
				
				// printf("%d", write(out_descriptor, buf, strlen(buf)));
				// printf("std %s", buf);
				// printf("\n%s|", e->cmd.exe);
				// for(int i=0; i < sizeof(arguments)/sizeof(arguments[0]) ; i++)
				//     printf(" %s|", arguments[i]);
				execvp(e->cmd.exe, arguments);
				return 0;
			}
			wait(NULL);
			close(STDOUT_FILENO);
			close(out_descriptor);
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
