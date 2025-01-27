#include "parser.h"
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>

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
	int l_r_d; // last read descriptor
	int l_w_d; // last write descriptor
	enum expr_type last_operand;
	int last_return;
} buffer;

static void execute_command_line(const struct command_line *line)
{
	/* REPLACE THIS CODE WITH ACTUAL COMMAND EXECUTION */

	assert(line != NULL);
	const struct expr *e = line->head;

	int std_output = STDOUT_FILENO;
	int out_descriptor = line->out_type == 2 ? open(line->out_file, O_APPEND | O_CREAT | O_RDWR,  S_IRUSR | S_IWUSR) : line->out_type==1? open(line->out_file, O_CREAT | O_RDWR,  S_IRUSR | S_IWUSR) : dup(STDOUT_FILENO); // дескриптор записи

	// buffer buf_proto;
	// buffer *buf = mmap(NULL, sizeof(buf_proto), PROT_READ | PROT_WRITE,
	// 				 MAP_ANON | MAP_SHARED, -1, 0); // размер буфера
	int command_i = 0;
	for (struct expr *tempe = line->head; tempe != NULL; tempe = tempe->next)
	{
		if (tempe->type == EXPR_TYPE_COMMAND)
		{
			command_i++;
		}
	}
	int read_descs[command_i];
	// int write_descs[command_i];
	command_i = 0;

	buffer *buf = malloc(sizeof(buffer));
	buf->l_r_d = STDIN_FILENO;
	buf->l_w_d = out_descriptor;
	buf->last_return = 0;

	buf->last_operand = EXPR_TYPE_COMMAND;
	while (e != NULL)
	{
		if (e->type == EXPR_TYPE_COMMAND)
		{
			if(strcmp( e->cmd.exe, "cd")==0){
				chdir(e->cmd.args[0]);
				goto continue_;
			}
			if(strcmp(e->cmd.exe, "exit")==0){
				wait(NULL);
				exit(0);
			}
			// парсинг аргументов
			char **arguments[e->cmd.arg_count + 2];
			int i = 0;
			arguments[0] = e->cmd.exe;
			for (int i = 0; i < (int)e->cmd.arg_count; i++)
			{
				char *arg = e->cmd.args[i];
				arguments[i + 1] = arg;
			}
			arguments[i + 2] = NULL;

			// дитя
			pid_t childpid;
			int child_descs[2];
			pipe(child_descs);
			read_descs[command_i] = child_descs[0];
			// write_descs[command_i] = child_descs[1];

			if (e->next != NULL && e->next->type == EXPR_TYPE_PIPE)
			{
				buf->l_r_d = child_descs[0];
			}

			if ((childpid = fork()) == 0)
			{
				if (buf->last_operand == EXPR_TYPE_PIPE)
				{
					// printf("c%d\n", buf->l_r_d);
					dup2(read_descs[command_i - 1], STDIN_FILENO); 
				}
				else
				{
					close(STDIN_FILENO);
				}

				if (e->next != NULL && e->next->type == EXPR_TYPE_PIPE)
				{
					// printf("p%d\n", child_descs[1]);
					// printf("pc%d\n", child_descs[0]);
					// close(child_descs[1]);
					dup2(child_descs[1], STDOUT_FILENO);
				}
				else
				{
					dup2(out_descriptor, STDOUT_FILENO); // меняем вывод ребенка на наш дескриптор вывода
				}
				if(e->cmd.arg_count>0)
					return execvp(e->cmd.exe, arguments);
				else 
				    return execlp(e->cmd.exe, NULL);
			}
			else // родитель
			{
				assert(childpid > 0); // else error forking

				int status;
				if (e->next != NULL && e->next->type != EXPR_TYPE_PIPE)
				{
					pid_t result = waitpid(childpid, &status, 0);
					buf->last_return = result == 0;
				}
				// char temp[1024];
				// sprintf(temp, "returned %d %d\n\n", result, status);
				// write(out_descriptor, temp, strlen(temp));
				// fprintf(stdout, "returned %d\n", result);
				// fflush(stdin);
			}
			command_i++;
		}
		else if (e->type == EXPR_TYPE_PIPE)
		{
		}
		else if (e->type == EXPR_TYPE_AND)
		{
			if (buf->last_return == 0)
			{
				e = e->next->next;
				continue;
			}
			// if !ret = 0 do not execute next command
		}
		else if (e->type == EXPR_TYPE_OR)
		{
			if (buf->last_return == 1)
			{
				e = e->next->next;
				continue;
			}
			// if !ret = 1 do not execute next command
		}
		else
		{

			assert(false);
		}
continue_:
		buf->last_operand = e->type;
		e = e->next;
	}
	wait(NULL);
	close(out_descriptor); // закрываем дескриптор вывода родителя
	// for (struct expr *tempe = line->head; tempe != NULL; tempe = tempe->next)
	// {
	// 	if (tempe->type == EXPR_TYPE_COMMAND)
	// 	{
	// 		close(read_descs[command_i]);
	// 		close(write_descs[command_i]);
	// 		command_i++;
	// 	}
	// }
	free(buf);
	// munmap(buf, sizeof(buf_proto));
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
