// SPDX-License-Identifier: BSL-1.0

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "emsh.h"

#include "ascii.h"
#include "ctlseq.h"
#include "numcast10.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

/*
 * Buffer
 */

/// capacity doesn't include terminator (NUL byte)
static void emsh_buf_init(emsh_buf_t *self, char *data, size_t capacity)
{
	bytearray_init(&self->array, capacity, (bytearray_datum_t *)data);
	bytearray_resize(&self->array, strlen(data));
	self->pos = bytearray_size(&self->array);
}

static size_t emsh_buf_capacity(const emsh_buf_t *self)
{
	return bytearray_capacity(&self->array);
}

static size_t emsh_buf_size(const emsh_buf_t *self)
{
	return bytearray_size(&self->array);
}

static const char *emsh_buf_data_const(const emsh_buf_t *self)
{
	return (const char *)bytearray_data_const(&self->array);
}

static char *emsh_buf_data(emsh_buf_t *self)
{
	return (char *)bytearray_data(&self->array);
}

static size_t emsh_buf_pos(const emsh_buf_t *self)
{
	return self->pos;
}

static void emsh_buf_set_pos(emsh_buf_t *self, size_t pos)
{
	assert(pos <= emsh_buf_size(self));

	self->pos = pos;
}

static void emsh_buf_dec_pos(emsh_buf_t *self)
{
	if (self->pos > 0)
	{
		--self->pos;
	}
}

static void emsh_buf_inc_pos(emsh_buf_t *self)
{
	if (self->pos < emsh_buf_size(self))
	{
		++self->pos;
	}
}

static void emsh_buf_insert(emsh_buf_t *self, char ch)
{
	assert(emsh_buf_size(self) != emsh_buf_capacity(self));

	bytearray_insert(&self->array, self->pos, ch);
	emsh_buf_data(self)[emsh_buf_size(self)] = '\0';
	++self->pos;
}

static void emsh_buf_erase(emsh_buf_t *self)
{
	assert(self->pos != emsh_buf_size(self));

	bytearray_erase(&self->array, self->pos);
	emsh_buf_data(self)[emsh_buf_size(self)] = '\0';
}

/*
 * History
 */

static void emsh_hist_init(emsh_hist_t *self, emsh_block_t *blocks, size_t n_blocks)
{
	assert(n_blocks != 0);

	self->capacity = n_blocks;
	list_init(&self->free_list);
	for (size_t i = 0; i < n_blocks; ++i)
	{
		list_node_init(&blocks[i].node);
		blocks[i].mem[0] = '\0';
		list_push_back(&self->free_list, &blocks[i].node);
	}

	list_init(&self->list);
	list_push_front(&self->list, list_pop_front(&self->free_list));
	self->size = 1;

	self->pos = 0;
	self->cur = list_front(&self->list);
}

static char *emsh_hist_current(emsh_hist_t *self)
{
	return list_entry_of(self->cur, emsh_block_t, node)->mem;
}

static void emsh_hist_commit(emsh_hist_t *self)
{
	if (self->pos != 0)
	{
		// remove the draft
		list_push_back(&self->free_list, list_pop_front(&self->list));
		--self->size;

		// bring the current entry to the front
		list_node_unlink(self->cur);
		list_push_front(&self->list, self->cur);
	}
	else if (self->size == self->capacity)
	{
		// retire the oldest entry
		list_push_back(&self->free_list, list_pop_back(&self->list));
		--self->size;
	}

	// add a new draft
	list_push_front(&self->list, list_pop_front(&self->free_list));
	++self->size;

	// move focus to the new draft
	self->pos = 0;
	self->cur = list_front(&self->list);
	emsh_hist_current(self)[0] = '\0';
}

static void emsh_hist_move_backward(emsh_hist_t *self)
{
	if (self->pos < self->size - 1)
	{
		++self->pos;
		self->cur = list_node_next(self->cur);
	}
}

static void emsh_hist_move_forward(emsh_hist_t *self)
{
	if (self->pos > 0)
	{
		--self->pos;
		self->cur = list_node_prev(self->cur);
	}
}

/*
 * Write
 */

static void emsh_write_char(emsh_t *self, char ch)
{
	self->ops.write_char(self->cookie, ch);
}

static void emsh_write_strn(emsh_t *self, const char *str, size_t len)
{
	self->ops.write_strn(self->cookie, str, len);
}

// str should be a compile-time constant
#define emsh_write_str(self, str) emsh_write_strn(self, str, strlen(str))

static void emsh_write_prompt(emsh_t *self)
{
	emsh_write_str(self, EMSH_S_PROMPT);
}

static void emsh_write_cr(emsh_t *self)
{
	emsh_write_char(self, ASCII_C_CR);
}

static void emsh_write_newline(emsh_t *self)
{
	emsh_write_str(self, EMSH_S_NEWLINE);
}

static void emsh_write_ctlseq_1(emsh_t *self, uint_fast32_t param_val, char interm_byte, char final_byte)
{
	char buf[2+NUMCAST10_SIZE_OF(uint_fast32_t)+1+1]; // CSI P ... P (I) F
	size_t len = 0;

	// CSI
	buf[len + 0] = CTLSEQ_C_CSI_1;
	buf[len + 1] = CTLSEQ_C_CSI_2;
	len += 2;

	// P ... P
	len += numcast10_from_uint_fast32_t(&buf[len], param_val);

	// (I)
	if (interm_byte != 0x00)
	{
		buf[len] = interm_byte;
		++len;
	}

	// F
	buf[len] = final_byte;
	++len;

	emsh_write_strn(self, buf, len);
}

static void emsh_write_ctlseq_el(emsh_t *self, int param)
{
	switch (param)
	{
	case 0:
		emsh_write_str(self, CTLSEQ_S_CSI CTLSEQ_S_EL);
		break;
	case 1:
		emsh_write_str(self, CTLSEQ_S_CSI "1" CTLSEQ_S_EL);
		break;
	case 2:
		emsh_write_str(self, CTLSEQ_S_CSI "2" CTLSEQ_S_EL);
		break;
	default:
		assert(0);
		break;
	}
}

static void emsh_write_ctlseq_cuf(emsh_t *self, uint_fast32_t n)
{
	if (n == 0)
	{
		// nothing to do
	}
	else if (n == 1)
	{
		emsh_write_str(self, CTLSEQ_S_CSI CTLSEQ_S_CUF);
	}
	else
	{
		emsh_write_ctlseq_1(self, n, 0x00, CTLSEQ_C_CUF);
	}
}

static void emsh_write_ctlseq_cub(emsh_t *self, uint_fast32_t n)
{
	if (n == 0)
	{
		// nothing to do
	}
	else if (n == 1)
	{
		emsh_write_str(self, CTLSEQ_S_CSI CTLSEQ_S_CUB);
	}
	else
	{
		emsh_write_ctlseq_1(self, n, 0x00, CTLSEQ_C_CUB);
	}
}

/*
 * Display
 */

static void emsh_disp_refresh_cur_to_eol(emsh_t *self)
{
	size_t size = emsh_buf_size(&self->buf);
	size_t pos = emsh_buf_pos(&self->buf);

	emsh_write_ctlseq_el(self, 0);
	emsh_write_strn(self, emsh_buf_data_const(&self->buf) + pos, size - pos);
	emsh_write_ctlseq_cub(self, (uint_fast32_t)(size - pos));
}

static void emsh_disp_refresh_line(emsh_t *self)
{
	emsh_buf_init(&self->buf, emsh_hist_current(&self->hist), EMSH_MAX_LINE_SIZE);

	emsh_write_cr(self);
	emsh_write_ctlseq_el(self, 0);
	emsh_write_prompt(self);
	emsh_write_strn(self, emsh_buf_data_const(&self->buf), emsh_buf_size(&self->buf));
}

/*
 * Cursor
 */

static void emsh_cur_move_forward(emsh_t *self)
{
	assert(emsh_buf_pos(&self->buf) < emsh_buf_size(&self->buf));

	emsh_buf_inc_pos(&self->buf);
	emsh_write_ctlseq_cuf(self, 1);
}

static void emsh_cur_move_backward(emsh_t *self)
{
	assert(emsh_buf_pos(&self->buf) > 0);

	emsh_buf_dec_pos(&self->buf);
	emsh_write_ctlseq_cub(self, 1);
}

static void emsh_cur_set_pos(emsh_t *self, size_t pos)
{
	assert(pos <= emsh_buf_size(&self->buf));

	emsh_write_cr(self);
	emsh_write_ctlseq_cuf(self, (uint_fast32_t)(strlen(EMSH_S_PROMPT) + pos));
	emsh_buf_set_pos(&self->buf, pos);
}

/*
 * Command processor
 */

/// updates cmd.argc and cmd.argv
static void emsh_cmd_split(emsh_t *self)
{
	size_t pos = 0;
	size_t size = emsh_buf_size(&self->buf);
	char *data = emsh_buf_data(&self->buf);

	self->cmd.argc = 0;

	// skip leading spaces
	while (pos < size && data[pos] == ' ') ++pos;
	if (pos == size)
	{
		return;
	}

	// split arguments
	while (pos < size)
	{
		if (self->cmd.argc == EMSH_MAX_N_ARGS)
		{
			++self->cmd.argc;
			break;
		}

		self->cmd.argv[self->cmd.argc] = &data[pos];
		while (pos < size && data[pos] != ' ') ++pos;
		data[pos] = '\0';
		++pos;
		while (pos < size && data[pos] == ' ') ++pos;

		++self->cmd.argc;
	}
}

static void emsh_cmd_exec(emsh_t *self)
{
#if EMSH_ENABLE_GETOPT
	self->cmd.optpos = 1;
	emsh_optind = 1;
#endif
	self->ops.exec(self->cookie, self->cmd.argc, self->cmd.argv);
}

static void emsh_cmd_restore(emsh_t *self)
{
	size_t size = emsh_buf_size(&self->buf);
	char *data = emsh_buf_data(&self->buf);

	for (size_t i = 0; i < size; ++i)
	{
		if (data[i] == '\0')
		{
			data[i] = ' ';
		}
	}
}

static bool emsh_cmd_run(emsh_t *self)
{
	emsh_cmd_split(self);
	if (self->cmd.argc == 0)
	{
		// ignore
	}
	else if (self->cmd.argc <= EMSH_MAX_N_ARGS)
	{
		emsh_cmd_exec(self);
	}
	else
	{
		emsh_write_str(self, "emsh: Argument list too long." EMSH_S_NEWLINE);
	}
	emsh_cmd_restore(self);

	return self->cmd.argc != 0;
}

/*
 * Actions
 */

static void emsh_do_commit(emsh_t *self)
{
	emsh_write_newline(self);

	if (emsh_cmd_run(self))
	{
		emsh_hist_commit(&self->hist);
		emsh_buf_init(&self->buf, emsh_hist_current(&self->hist), EMSH_MAX_LINE_SIZE);
	}

	if (self->running)
	{
		emsh_write_prompt(self);
	}
}

static void emsh_do_insert(emsh_t *self, int c)
{
	if (emsh_buf_size(&self->buf) < emsh_buf_capacity(&self->buf))
	{
		emsh_buf_insert(&self->buf, c);
		emsh_write_char(self, (char)c);
		emsh_disp_refresh_cur_to_eol(self);
	}
}

static void emsh_do_erase(emsh_t *self)
{
	if (emsh_buf_pos(&self->buf) != emsh_buf_size(&self->buf))
	{
		emsh_buf_erase(&self->buf);
		emsh_disp_refresh_cur_to_eol(self);
	}
}

/// cursor up
static void emsh_do_cuu(emsh_t *self)
{
	emsh_hist_move_backward(&self->hist);
	emsh_disp_refresh_line(self);
}

/// cursor down
static void emsh_do_cud(emsh_t *self)
{
	emsh_hist_move_forward(&self->hist);
	emsh_disp_refresh_line(self);
}

/// cursor forward
static void emsh_do_cuf(emsh_t *self)
{
	if (emsh_buf_pos(&self->buf) < emsh_buf_size(&self->buf))
	{
		emsh_cur_move_forward(self);
	}
}

/// cursor back
static void emsh_do_cub(emsh_t *self)
{
	if (emsh_buf_pos(&self->buf) > 0)
	{
		emsh_cur_move_backward(self);
	}
}

/// backspace
static void emsh_do_bs(emsh_t *self)
{
	if (emsh_buf_pos(&self->buf) > 0)
	{
		emsh_do_cub(self);
		emsh_do_erase(self);
	}
}

/// start of line
static void emsh_do_sol(emsh_t *self)
{
	emsh_cur_set_pos(self, 0);
}

/// end of line
static void emsh_do_eol(emsh_t *self)
{
	emsh_cur_set_pos(self, emsh_buf_size(&self->buf));
}

/*
 * Public functions
 */

void emsh_init(emsh_t *self, const emsh_conf_t *conf)
{
	assert(conf->ops.write_char != NULL);
	assert(conf->ops.write_strn != NULL);
	assert(conf->ops.exec != NULL);
	assert(conf->blocks != NULL);

	self->cookie = conf->cookie;
	self->ops = conf->ops;

	emsh_hist_init(&self->hist, conf->blocks, EMSH_MAX_HIST_SIZE);
	emsh_buf_init(&self->buf, emsh_hist_current(&self->hist), EMSH_MAX_LINE_SIZE);

	self->ctlseq.st = CTLSEQ_ST_INIT;
	self->ctlseq.interm_byte = 0x00;
}

void emsh_start(emsh_t *self)
{
	self->running = true;
	emsh_write_prompt(self);
}

void emsh_task(emsh_t *self, int c)
{
	int psep;
	ctlseq_ev_t ev = ctlseq_sm(&self->ctlseq.st, c, &psep);
	if (self->ctlseq.st == CTLSEQ_ST_INIT)
	{
		switch (c)
		{
		case ASCII_C_CR:
			// ignore
			break;
		case ASCII_C_LF:
			emsh_do_commit(self);
			break;
		case ASCII_C_BS:
		case ASCII_C_DEL:
			emsh_do_bs(self);
			break;
#if 1
		case ASCII_CNTRL('A'):
			emsh_do_sol(self);
			break;
		case ASCII_CNTRL('B'):
			emsh_do_cub(self);
			break;
		case ASCII_CNTRL('D'):
			emsh_do_erase(self);
			break;
		case ASCII_CNTRL('E'):
			emsh_do_eol(self);
			break;
		case ASCII_CNTRL('F'):
			emsh_do_cuf(self);
			break;
		case ASCII_CNTRL('N'):
			emsh_do_cud(self);
			break;
		case ASCII_CNTRL('P'):
			emsh_do_cuu(self);
			break;
#endif
		default:
			if (ascii_isprint(c))
			{
				emsh_do_insert(self, c);
			}
		}
	}
	else
	{
		switch (ev)
		{
		case CTLSEQ_EV_NONE:
			if (self->ctlseq.st == CTLSEQ_ST_PARAM)
			{
				self->ctlseq.param_byte = 0xFF; // more than one parameter bytes
			}
			else if (self->ctlseq.st == CTLSEQ_ST_INTERM)
			{
				self->ctlseq.interm_byte = 0xFF; // more than one intermediate bytes
			}
			break;
		case CTLSEQ_EV_ESC:
			// init
			self->ctlseq.param_byte = 0x00; // no parameter byte
			self->ctlseq.interm_byte = 0x00; // no intermediate byte
			break;
		case CTLSEQ_EV_CSI:
			break;
		case CTLSEQ_EV_PARAM:
			self->ctlseq.param_byte = c; // the first parameter byte
			break;
		case CTLSEQ_EV_INTERM:
			self->ctlseq.interm_byte = c; // the first intermediate byte
			break;
		case CTLSEQ_EV_FINAL:
			switch (self->ctlseq.interm_byte)
			{
			case 0x00:
				switch (c)
				{
				case CTLSEQ_C_CUU:
					emsh_do_cuu(self);
					break;
				case CTLSEQ_C_CUD:
					emsh_do_cud(self);
					break;
				case CTLSEQ_C_CUF:
					emsh_do_cuf(self);
					break;
				case CTLSEQ_C_CUB:
					emsh_do_cub(self);
					break;
#if 1
				case 0x7E:
					switch (self->ctlseq.param_byte)
					{
					case 0x31:
						emsh_do_sol(self);
						break;
					case 0x32:
						// overwrite-mode
						break;
					case 0x33:
						emsh_do_erase(self);
						break;
					case 0x34:
						emsh_do_eol(self);
						break;
					}
					break;
#endif
				}
				break;
			case CTLSEQ_K_MAP_1:
				// nothing to do for now
				break;
			}
			break;
		case CTLSEQ_EV_ILSEQ:
			break;
		}
	}
}

void emsh_stop(emsh_t *self)
{
	self->running = false;
}

#if EMSH_ENABLE_GETOPT
static void _emsh_write_getopt_error(emsh_t *self, const char *name, const char *msg, size_t msglen)
{
#if 1
	if (emsh_opterr)
	{
		emsh_write_str(self, name);
		emsh_write_str(self, ": ");
		emsh_write_strn(self, msg, msglen);
		emsh_write_str(self, " -- ");
		emsh_write_char(self, emsh_optopt);
		emsh_write_newline(self);
	}
#else
	(void)self;
	(void)name;
	(void)msg;
	(void)msglen;
#endif
}

#define emsh_write_getopt_error(self, name, msg) _emsh_write_getopt_error(self, name, msg, strlen(msg))

int emsh_getopt(emsh_t *self, int argc, const char **argv, const char *optstring)
{
	if (emsh_optind >= argc ||
	    argv[emsh_optind] == NULL ||
	    argv[emsh_optind][0] != '-' ||
	    argv[emsh_optind][1] == '\0')
	{
		return -1;
	}
	else if (argv[emsh_optind][1] == '-' &&
	         argv[emsh_optind][2] == '\0')
	{
		++emsh_optind;
		return -1;
	}

	int c = argv[emsh_optind][self->cmd.optpos];
	const char *p = strchr(optstring, c);

	int last = 0;
	++self->cmd.optpos;
	if (argv[emsh_optind][self->cmd.optpos] == '\0')
	{
		++emsh_optind;
		self->cmd.optpos = 1;
		last = 1;
	}

	if (p != NULL)
	{
		if (p[1] == ':')
		{
			if (last && emsh_optind < argc)
			{
				emsh_optarg = argv[emsh_optind];
				++emsh_optind;
			}
			else
			{
				emsh_optopt = c;
				if (optstring[0] == ':')
				{
					c = ':';
				}
				else
				{
					c = '?';
					emsh_write_getopt_error(self, argv[0], "option requires an argument");
				}
			}
		}
	}
	else
	{
		emsh_optopt = c;
		c = '?';
		if (optstring[0] != ':')
		{
			emsh_write_getopt_error(self, argv[0], "illegal option");
		}
	}

	return c;
}

const char *emsh_optarg = NULL;
int emsh_opterr = 1, emsh_optind = 1, emsh_optopt = '\0';
#endif
