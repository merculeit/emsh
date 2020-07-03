// SPDX-License-Identifier: BSL-1.0

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "emsh.h"
#include "numcast10.h"
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

/*
 * basic io
 */

static int _fgetc(FILE *fp)
{
#if 0
	return fgetc(fp);
#else
	struct termios old_opts;
	tcgetattr(fileno(fp), &old_opts);

	struct termios new_opts;
	memcpy(&new_opts, &old_opts, sizeof(new_opts));
	new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
	tcsetattr(fileno(fp), TCSANOW, &new_opts);
	int c = fgetc(fp);

	tcsetattr(fileno(fp), TCSANOW, &old_opts);
	return c;
#endif
}

static int console_read_char(void)
{
	return _fgetc(stdin);
}

static void console_write_char(int ch)
{
	fputc(ch, stdout);
}

static void console_write_strn(const char *str, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		console_write_char(str[i]);
	}
}

static void console_write_str(const char *str)
{
	while (*str != '\0')
	{
		console_write_char(*str);
		++str;
	}
}

/*
 * console
 */

typedef enum console_state
{
	CONSOLE_STATE_INIT,
	CONSOLE_STATE_SHELL,
	CONSOLE_STATE_COMMAND,
} console_state_t;

typedef struct console
{
	int running;
	console_state_t state;
	emsh_t emsh;

	struct
	{
		size_t index;
		union
		{
			struct
			{
				unsigned int count;
			} sleep;
		} context;
	} command;
} console_t;

static emsh_block_t console_emsh_blocks[EMSH_MAX_HIST_SIZE];
static console_t g_console;

static void _console_write_char(uintptr_t cookie, char ch);
static void _console_write_strn(uintptr_t cookie, const char *str, size_t len);
static void _console_exec(uintptr_t cookie, int argc, const char **argv);

static const emsh_conf_t console_emsh_conf = {
	.cookie = (uintptr_t)&g_console,
	.ops = {
		.write_char = &_console_write_char,
		.write_strn = &_console_write_strn,
		.exec = &_console_exec,
	},
	.blocks = console_emsh_blocks,
};

/*
 * command list
 */

#define CONSOLE_COMMAND_TASK_DONE 0
#define CONSOLE_COMMAND_TASK_CONT 1

typedef struct console_command
{
	const char *name;
	int (*entry)(int argc, const char **argv);
	int (*task)(void);
} console_command_t;

static int console__echo(int argc, const char **argv);
static int console__sleep(int argc, const char **argv);
static int console__sleep_task(void);
static int console__greet(int argc, const char **argv);
static int console__exit(int argc, const char **argv);

// keep sorted by name
static const console_command_t console_commands[] = {
	{"echo", &console__echo, NULL},
	{"exit", &console__exit, NULL},
	{"greet", &console__greet, NULL},
	{"sleep", &console__sleep, &console__sleep_task},
};

#define CONSOLE_COMMANDS_SIZE (sizeof(console_commands)/sizeof(*console_commands))

/*
 * framework functions
 */

static void _console_write_char(uintptr_t cookie, char ch)
{
	(void)cookie;
	console_write_char(ch);
}

static void _console_write_strn(uintptr_t cookie, const char *str, size_t len)
{
	(void)cookie;
	console_write_strn(str, len);
}

static void console_check_preconditions(void)
{
#if 1
	if (CONSOLE_COMMANDS_SIZE > 1)
	{
		for (size_t i = 0; i < CONSOLE_COMMANDS_SIZE - 1; ++i)
		{
			assert(strcmp(console_commands[i].name, console_commands[i+1].name) < 0);
		}
	}
#endif
}

static int console_find_command(const char *name, size_t *p_index)
{
	size_t i_begin = 0;
	size_t i_end = CONSOLE_COMMANDS_SIZE;
	size_t i_mid = i_end / 2;
	while (i_begin < i_end)
	{
		int cmp = strcmp(name, console_commands[i_mid].name);
		if (cmp < 0)
		{
			i_end = i_mid;
		}
		else if (cmp > 0)
		{
			i_begin = i_mid + 1;
		}
		else
		{
			*p_index = i_mid;
			return 0;
		}
		i_mid = (i_end - i_begin) / 2 + i_begin;
	}
	return -1;
}

static void _console_exec(uintptr_t cookie, int argc, const char **argv)
{
	(void)cookie;

	int r = console_find_command(argv[0], &g_console.command.index);
	if (r == 0)
	{
		r = console_commands[g_console.command.index].entry(argc, argv);
		if (r == CONSOLE_COMMAND_TASK_CONT)
		{
			assert(console_commands[g_console.command.index].task != NULL);
			emsh_stop(&g_console.emsh);
		}
	}
	else
	{
		console_write_str("command not found" EMSH_S_NEWLINE);
	}
}

static int console_command_task(void)
{
	assert(g_console.command.index < CONSOLE_COMMANDS_SIZE);
	assert(console_commands[g_console.command.index].task != NULL);
	return console_commands[g_console.command.index].task();
}

static void console_init(void)
{
	console_check_preconditions();

	g_console.running = 1;
	g_console.state = CONSOLE_STATE_INIT;
	emsh_init(&g_console.emsh, &console_emsh_conf);
}

static void console_task(void)
{
	if (g_console.running)
	{
		switch (g_console.state)
		{
			case CONSOLE_STATE_INIT:
			{
				emsh_start(&g_console.emsh);
				g_console.state = CONSOLE_STATE_SHELL;
			}
			break;

			case CONSOLE_STATE_SHELL:
			{
				int c = console_read_char();
				if (c != EOF)
				{
					emsh_task(&g_console.emsh, c);
					if (!emsh_running(&g_console.emsh))
					{
						g_console.state = CONSOLE_STATE_COMMAND;
					}
				}
			}
			break;

			case CONSOLE_STATE_COMMAND:
			{
				int r = console_command_task();
				if (r == CONSOLE_COMMAND_TASK_DONE)
				{
					g_console.state = CONSOLE_STATE_INIT;
				}
			}
			break;
		}
	}
}

static int console_running(void)
{
	return g_console.running;
}

static void console_exit(void)
{
	emsh_stop(&g_console.emsh);
	g_console.running = 0;
}

/*
 * command implementations
 */

static int console__echo(int argc, const char **argv)
{
	if (argc >= 2)
	{
		fprintf(stdout, "%s", argv[1]);
	}
	for (int i = 2; i < argc; ++i)
	{
		fprintf(stdout, " %s", argv[i]);
	}
	fprintf(stdout, "\n");

	return CONSOLE_COMMAND_TASK_DONE;
}

static int console__sleep(int argc, const char **argv)
{
	if (argc < 2)
	{
		return CONSOLE_COMMAND_TASK_DONE;
	}

	size_t n;
	int r = numcast10_to_uint(&g_console.command.context.sleep.count, argv[1], SIZE_MAX, &n);
	if (r == -1 || argv[1][n] != '\0' || g_console.command.context.sleep.count == 0)
	{
		return CONSOLE_COMMAND_TASK_DONE;
	}

	return CONSOLE_COMMAND_TASK_CONT;
}

static int console__sleep_task(void)
{
	fprintf(stdout, "zzz...");
	fflush(stdout);
	sleep(1);

	if (--g_console.command.context.sleep.count != 0)
	{
		return CONSOLE_COMMAND_TASK_CONT;
	}
	else
	{
		fprintf(stdout, "\n");
		return CONSOLE_COMMAND_TASK_DONE;
	}
}

static int console__greet(int argc, const char **argv)
{
	const char *greeting = "Hi";
	const char *name = "Somebody";

	int opt;
	while ((opt = emsh_getopt(&g_console.emsh, argc, argv, "maenc:")) != -1)
	{
		switch (opt)
		{
		case 'm':
			greeting = "Good morning";
			break;
		case 'a':
			greeting = "Good afternoon";
			break;
		case 'e':
			greeting = "Good evening";
			break;
		case 'n':
			greeting = "Good night";
			break;

		case 'c':
			greeting = emsh_optarg;
			break;

		case '?':
			greeting = NULL;
			break;
		}
	}

	if (emsh_optind < argc)
	{
		name = argv[emsh_optind];
	}

	if (greeting != NULL)
	{
		fprintf(stdout, "%s, %s.\n", greeting, name);
	}

	return CONSOLE_COMMAND_TASK_DONE;
}

static int console__exit(int argc, const char **argv)
{
	(void)argc;
	(void)argv;
	console_exit();
	return CONSOLE_COMMAND_TASK_DONE;
}

/*
 * console thread
 */

int main()
{
	console_init();

	while (console_running())
	{
		console_task();
	}
}
