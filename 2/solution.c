#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


static void execute_command_line(const struct command_line *line)
{
	/* REPLACE THIS CODE WITH ACTUAL COMMAND EXECUTION */

	assert(line != NULL);
	
	const struct expr *e = line->head;
	while (e != NULL) {
		if (e->type == EXPR_TYPE_COMMAND) {
			
		} else if (e->type == EXPR_TYPE_PIPE) {
			
		} else if (e->type == EXPR_TYPE_AND) {
			
		} else if (e->type == EXPR_TYPE_OR) {
			
		} else {
			assert(false);
		}
		e = e->next;
	}
}

int
main(void)
{
	const size_t buf_size = 1024;
	char buf[buf_size];
	int rc;
	struct parser *p = parser_new();
	while ((rc = read(STDIN_FILENO, buf, buf_size)) > 0) {
		parser_feed(p, buf, rc);
		struct command_line *line = NULL;
		while (true) {
			enum parser_error err = parser_pop_next(p, &line);
			if (err == PARSER_ERR_NONE && line == NULL)
				break;
			if (err != PARSER_ERR_NONE) {
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
