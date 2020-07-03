// SPDX-License-Identifier: BSL-1.0

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#if !defined(EMSH_H_INCLUDED)
#define EMSH_H_INCLUDED

#include "ascii.h"
#include "ctlseq.h"
#include "list.h"
#include "bytearray.h"
#include <stdint.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(EMSH_MAX_HIST_SIZE)
  #define EMSH_MAX_HIST_SIZE 10
#endif

#if !defined(EMSH_MAX_LINE_SIZE)
  #define EMSH_MAX_LINE_SIZE (80 - 1 - 2)
#endif

#if !defined(EMSH_S_PROMPT)
  #define EMSH_S_PROMPT "> "
#endif

#if !defined(EMSH_MAX_N_ARGS)
  #define EMSH_MAX_N_ARGS 10
#endif

#if !defined(EMSH_S_NEWLINE)
  #define EMSH_S_NEWLINE ASCII_S_LF
#endif

#if !defined(EMSH_ENABLE_GETOPT)
  #define EMSH_ENABLE_GETOPT 1
#endif

///@internal
typedef struct emsh_buf
{
	bytearray_t array;
	size_t pos;
} emsh_buf_t;

///@internal
typedef struct emsh_block
{
	list_node_t node;
	char mem[EMSH_MAX_LINE_SIZE+1];
} emsh_block_t;

///@internal
typedef struct emsh_hist
{
	size_t capacity;
	list_t free_list;
	list_t list;
	size_t size;
	unsigned int pos;
	list_node_t *cur;
} emsh_hist_t;

///@internal
typedef struct emsh_ops
{
	void (*write_char)(uintptr_t cookie, char ch);
	void (*write_strn)(uintptr_t cookie, const char *str, size_t len);
	void (*exec)(uintptr_t cookie, int argc, const char **argv);
} emsh_ops_t;

///@internal
typedef struct emsh
{
	uintptr_t cookie;
	emsh_ops_t ops;

	bool running;
	emsh_hist_t hist;
	emsh_buf_t buf;

	struct
	{
		ctlseq_st_t st;
		char param_byte;
		char interm_byte;
	} ctlseq;

	struct
	{
#if EMSH_ENABLE_GETOPT
		size_t optpos;
#endif
		int argc;
		const char *argv[EMSH_MAX_N_ARGS];
	} cmd;
} emsh_t;

typedef struct emsh_conf
{
	uintptr_t cookie; ///< arbitrary data for ops
	emsh_ops_t ops;
	emsh_block_t *blocks; ///< EMSH_MAX_HIST_SIZE elements
} emsh_conf_t;

void emsh_init(emsh_t *self, const emsh_conf_t *conf);
void emsh_start(emsh_t *self);
void emsh_task(emsh_t *self, int c);
void emsh_stop(emsh_t *self);

#define EMSH_DEFINE_WRITE_STRN(_name, _write_char)            \
	void _name(uintptr_t cookie, const char *str, size_t len) \
	{                                                         \
		for (size_t i = 0; i < len; ++i)                      \
		{                                                     \
			(_write_char)(cookie, str[i]);                    \
		}                                                     \
	}

static inline
bool emsh_running(const emsh_t *self)
{
	return self->running;
}

#if EMSH_ENABLE_GETOPT
int emsh_getopt(emsh_t *self, int argc, const char **argv, const char *optstring);
extern const char *emsh_optarg;
extern int emsh_opterr, emsh_optind, emsh_optopt;
#endif

#if defined(__cplusplus)
}
#endif

#endif // EMSH_H_INCLUDED
