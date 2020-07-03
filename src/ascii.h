// SPDX-License-Identifier: BSL-1.0

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#if !defined(ASCII_H_INCLUDED)
#define ASCII_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

#define ASCII_C_NUL 0x00
#define ASCII_C_SOH 0x01
#define ASCII_C_STX 0x02
#define ASCII_C_ETX 0x03
#define ASCII_C_EOT 0x04
#define ASCII_C_ENQ 0x05
#define ASCII_C_ACK 0x06
#define ASCII_C_BEL 0x07
#define ASCII_C_BS  0x08
#define ASCII_C_HT  0x09
#define ASCII_C_LF  0x0A
#define ASCII_C_VT  0x0B
#define ASCII_C_FF  0x0C
#define ASCII_C_CR  0x0D
#define ASCII_C_SO  0x0E
#define ASCII_C_SI  0x0F
#define ASCII_C_DLE 0x10
#define ASCII_C_DC1 0x11
#define ASCII_C_DC2 0x12
#define ASCII_C_DC3 0x13
#define ASCII_C_DC4 0x14
#define ASCII_C_NAK 0x15
#define ASCII_C_SYN 0x16
#define ASCII_C_ETB 0x17
#define ASCII_C_CAN 0x18
#define ASCII_C_EM  0x19
#define ASCII_C_SUB 0x1A
#define ASCII_C_ESC 0x1B
#define ASCII_C_FS  0x1C
#define ASCII_C_GS  0x1D
#define ASCII_C_RS  0x1E
#define ASCII_C_US  0x1F
#define ASCII_C_SP  0x20
#define ASCII_C_DEL 0x7F

#define ASCII_S_NUL "\x00"
#define ASCII_S_SOH "\x01"
#define ASCII_S_STX "\x02"
#define ASCII_S_ETX "\x03"
#define ASCII_S_EOT "\x04"
#define ASCII_S_ENQ "\x05"
#define ASCII_S_ACK "\x06"
#define ASCII_S_BEL "\x07"
#define ASCII_S_BS  "\x08"
#define ASCII_S_HT  "\x09"
#define ASCII_S_LF  "\x0A"
#define ASCII_S_VT  "\x0B"
#define ASCII_S_FF  "\x0C"
#define ASCII_S_CR  "\x0D"
#define ASCII_S_SO  "\x0E"
#define ASCII_S_SI  "\x0F"
#define ASCII_S_DLE "\x10"
#define ASCII_S_DC1 "\x11"
#define ASCII_S_DC2 "\x12"
#define ASCII_S_DC3 "\x13"
#define ASCII_S_DC4 "\x14"
#define ASCII_S_NAK "\x15"
#define ASCII_S_SYN "\x16"
#define ASCII_S_ETB "\x17"
#define ASCII_S_CAN "\x18"
#define ASCII_S_EM  "\x19"
#define ASCII_S_SUB "\x1A"
#define ASCII_S_ESC "\x1B"
#define ASCII_S_FS  "\x1C"
#define ASCII_S_GS  "\x1D"
#define ASCII_S_RS  "\x1E"
#define ASCII_S_US  "\x1F"
#define ASCII_S_SP  "\x20"
#define ASCII_S_DEL "\x7F"

static inline
int ascii_isascii(int c)
{
	return 0x00 <= c && c <= 0x7F;
}

static inline
int ascii_iscntrl(int c)
{
	return (0x00 <= c && c <= 0x1F) ||
	       (c == ASCII_C_DEL);
}

static inline
int ascii_isprint(int c)
{
	return 0x20 <= c && c <= 0x7E;
}

static inline
int ascii_isgraph(int c)
{
	return 0x21 <= c && c <= 0x7E;
}

static inline
int ascii_isspace(int c)
{
	return (c == ASCII_C_HT) ||
	       (c == ASCII_C_LF) ||
	       (c == ASCII_C_VT) ||
	       (c == ASCII_C_FF) ||
	       (c == ASCII_C_CR) ||
	       (c == ASCII_C_SP);
}

static inline
int ascii_isblank(int c)
{
	return (c == ASCII_C_HT) ||
	       (c == ASCII_C_SP);
}

static inline
int ascii_ispunct(int c)
{
	return (0x21 <= c && c <= 0x2F) ||
	       (0x3A <= c && c <= 0x40) ||
	       (0x5B <= c && c <= 0x60) ||
	       (0x7B <= c && c <= 0x7E);
}

static inline
int ascii_isdigit(int c)
{
	return 0x30 <= c && c <= 0x39;
}

static inline
int ascii_isupper(int c)
{
	return 0x41 <= c && c <= 0x5A;
}

static inline
int ascii_islower(int c)
{
	return 0x61 <= c && c <= 0x7A;
}

static inline
int ascii_isxdigit(int c)
{
	return ascii_isdigit(c) ||
	       (0x41 <= c && c <= 0x46) ||
	       (0x61 <= c && c <= 0x66);
}

static inline
int ascii_isalpha(int c)
{
	return ascii_isupper(c) ||
	       ascii_islower(c);
}

static inline
int ascii_isalnum(int c)
{
	return ascii_isalpha(c) ||
	       ascii_isdigit(c);
}

static inline
int ascii_tolower(int c)
{
	return ascii_isupper(c) ? (c | 0x20) : c;
}

static inline
int ascii_toupper(int c)
{
	return ascii_islower(c) ? (c & 0x5F) : c;
}

static inline
int ascii_toascii(int c)
{
	return c & 0x7F;
}

#define ASCII_CNTRL(c) (((c) - 0x40) & 0x7F)
#define ASCII_UNCNTRL(c) (((c) + 0x40) & 0x7F)

#if defined(__cplusplus)
}
#endif

#endif // ASCII_H_INCLUDED
