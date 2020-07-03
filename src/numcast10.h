// SPDX-License-Identifier: BSL-1.0

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#if !defined(NUMCAST10_H_INCLUDED)
#define NUMCAST10_H_INCLUDED

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * private
 */

#define NUMCAST10_IMPL_HAVE_ULONG (ULONG_MAX > UINT_MAX)
#define NUMCAST10_IMPL_HAVE_ULLONG (ULLONG_MAX > ULONG_MAX)
#define NUMCAST10_IMPL_HAVE_UINTMAX (UINTMAX_MAX > ULLONG_MAX)

/* string from uint ("backward function") */

#define NUMCAST10_IMPL_BW_FUNC_NAME(name) \
	numcast10_impl_bw_##name

#define NUMCAST10_IMPL_BW_FUNC_PROTOTYPE(name, T) \
	size_t NUMCAST10_IMPL_BW_FUNC_NAME(name)( \
		char *dst, \
		T src)

#define NUMCAST10_IMPL_BW_FUNC_DECL(name, T) \
	NUMCAST10_IMPL_BW_FUNC_PROTOTYPE(name, T);

#define NUMCAST10_IMPL_BW_FUNC_DUMMY(name, T) \
	static inline \
	NUMCAST10_IMPL_BW_FUNC_PROTOTYPE(name, T) \
	{ \
		(void)dst; \
		(void)src; \
		assert(0); \
		errno = ENOSYS; \
		return 0; \
	}

NUMCAST10_IMPL_BW_FUNC_DECL(uint, unsigned int)

#if NUMCAST10_IMPL_HAVE_ULONG
NUMCAST10_IMPL_BW_FUNC_DECL(ulong, unsigned long)
#else
NUMCAST10_IMPL_BW_FUNC_DUMMY(ulong, unsigned long)
#endif

#if NUMCAST10_IMPL_HAVE_ULLONG
NUMCAST10_IMPL_BW_FUNC_DECL(ullong, unsigned long long)
#else
NUMCAST10_IMPL_BW_FUNC_DUMMY(ullong, unsigned long long)
#endif

#if NUMCAST10_IMPL_HAVE_UINTMAX
NUMCAST10_IMPL_BW_FUNC_DECL(uintmax, uintmax_t)
#else
NUMCAST10_IMPL_BW_FUNC_DUMMY(uintmax, uintmax_t)
#endif

// dst: char * | dst.count >= NUMCAST10_SIZE_OF(U),
// src: U
#define NUMCAST10_IMPL_BW_FUNC_EXEC_IF_FITS(name, T, U) \
	if (sizeof(U) <= sizeof(T)) \
	{ \
		return NUMCAST10_IMPL_BW_FUNC_NAME(name)( \
				dst, \
				(T)src); \
	}

/* string to uint ("forward function") */

#define NUMCAST10_IMPL_FW_FUNC_NAME(name) \
	numcast10_impl_fw_##name

#define NUMCAST10_IMPL_FW_FUNC_PROTOTYPE(name, T) \
	int NUMCAST10_IMPL_FW_FUNC_NAME(name)( \
		T *p_dst, \
		const char *src, size_t n_src, size_t *p_n_consumed, \
		T max_value, T max_value_10)

#define NUMCAST10_IMPL_FW_FUNC_DECL(name, T) \
	NUMCAST10_IMPL_FW_FUNC_PROTOTYPE(name, T);

#define NUMCAST10_IMPL_FW_FUNC_DUMMY(name, T) \
	static inline \
	NUMCAST10_IMPL_FW_FUNC_PROTOTYPE(name, T) \
	{ \
		(void)p_dst; \
		(void)src; \
		(void)n_src; \
		(void)p_n_consumed; \
		(void)max_value; \
		(void)max_value_10; \
		assert(0); \
		errno = ENOSYS; \
		return -1; \
	}

NUMCAST10_IMPL_FW_FUNC_DECL(uint, unsigned int)

#if NUMCAST10_IMPL_HAVE_ULONG
NUMCAST10_IMPL_FW_FUNC_DECL(ulong, unsigned long)
#else
NUMCAST10_IMPL_FW_FUNC_DUMMY(ulong, unsigned long)
#endif

#if NUMCAST10_IMPL_HAVE_ULLONG
NUMCAST10_IMPL_FW_FUNC_DECL(ullong, unsigned long long)
#else
NUMCAST10_IMPL_FW_FUNC_DUMMY(ullong, unsigned long long)
#endif

#if NUMCAST10_IMPL_HAVE_UINTMAX
NUMCAST10_IMPL_FW_FUNC_DECL(uintmax, uintmax_t)
#else
NUMCAST10_IMPL_FW_FUNC_DUMMY(uintmax, uintmax_t)
#endif

// p_dst: U *
// src: const char *      | src.count >= n_src
// n_src: size_t
// p_n_consumed: size_t * | nullable
#define NUMCAST10_IMPL_FW_FUNC_EXEC_IF_FITS(name, T, U, max_value) \
	if (sizeof(U) <= sizeof(T)) \
	{ \
		T tmp; \
		int r; \
\
		if (p_dst == NULL) \
		{ \
			errno = EINVAL; \
			return -1; \
		} \
\
		r = NUMCAST10_IMPL_FW_FUNC_NAME(name)( \
				&tmp, \
				src, n_src, p_n_consumed, \
				(T)(max_value), (T)(max_value) / 10); \
		*p_dst = (U)tmp; \
		return r; \
	}

/*
 * public
 */

// enough buffer size to represent a value of T as a decimal number (doesn't include terminator)
#define NUMCAST10_SIZE_OF(T) ((size_t)(CHAR_BIT * sizeof(T) * 0.3010299956639812 + 1))

#define NUMCAST10_DEFINE_BW_FUNC(name, T) \
	static inline \
	size_t numcast10_from_##name( \
			char dst[NUMCAST10_SIZE_OF(T)], \
			T src) \
	{ \
		NUMCAST10_IMPL_BW_FUNC_EXEC_IF_FITS(uint, unsigned int, T) \
		else \
		NUMCAST10_IMPL_BW_FUNC_EXEC_IF_FITS(ulong, unsigned long, T) \
		else \
		NUMCAST10_IMPL_BW_FUNC_EXEC_IF_FITS(ullong, unsigned long long, T) \
		else \
		NUMCAST10_IMPL_BW_FUNC_EXEC_IF_FITS(uintmax, uintmax_t, T) \
		else \
		{ \
			assert(0); \
			errno = ENOTSUP; \
			return 0; \
		} \
	}

NUMCAST10_DEFINE_BW_FUNC(uchar, unsigned char)
NUMCAST10_DEFINE_BW_FUNC(ushort, unsigned short)
NUMCAST10_DEFINE_BW_FUNC(uint, unsigned int)
NUMCAST10_DEFINE_BW_FUNC(ulong, unsigned long)
NUMCAST10_DEFINE_BW_FUNC(ullong, unsigned long long)

NUMCAST10_DEFINE_BW_FUNC(size_t, size_t)
NUMCAST10_DEFINE_BW_FUNC(ptrdiff_t, ptrdiff_t)

NUMCAST10_DEFINE_BW_FUNC(uint8_t, uint8_t)
NUMCAST10_DEFINE_BW_FUNC(uint16_t, uint16_t)
NUMCAST10_DEFINE_BW_FUNC(uint32_t, uint32_t)
NUMCAST10_DEFINE_BW_FUNC(uint64_t, uint64_t)

NUMCAST10_DEFINE_BW_FUNC(uint_fast8_t, uint_fast8_t)
NUMCAST10_DEFINE_BW_FUNC(uint_fast16_t, uint_fast16_t)
NUMCAST10_DEFINE_BW_FUNC(uint_fast32_t, uint_fast32_t)
NUMCAST10_DEFINE_BW_FUNC(uint_fast64_t, uint_fast64_t)

NUMCAST10_DEFINE_BW_FUNC(uint_least8_t, uint_least8_t)
NUMCAST10_DEFINE_BW_FUNC(uint_least16_t, uint_least16_t)
NUMCAST10_DEFINE_BW_FUNC(uint_least32_t, uint_least32_t)
NUMCAST10_DEFINE_BW_FUNC(uint_least64_t, uint_least64_t)

NUMCAST10_DEFINE_BW_FUNC(uintmax_t, uintmax_t)

NUMCAST10_DEFINE_BW_FUNC(uintptr_t, uintptr_t)

#define NUMCAST10_DEFINE_FW_FUNC(name, T, max_value) \
	static inline \
	int numcast10_to_##name( \
			T *p_dst, \
			const char *src, size_t n_src, size_t *p_n_consumed) \
	{ \
		NUMCAST10_IMPL_FW_FUNC_EXEC_IF_FITS(uint, unsigned int, T, max_value) \
		else \
		NUMCAST10_IMPL_FW_FUNC_EXEC_IF_FITS(ulong, unsigned long, T, max_value) \
		else \
		NUMCAST10_IMPL_FW_FUNC_EXEC_IF_FITS(ullong, unsigned long long, T, max_value) \
		else \
		NUMCAST10_IMPL_FW_FUNC_EXEC_IF_FITS(uintmax, uintmax_t, T, max_value) \
		else \
		{ \
			assert(0); \
			errno = ENOTSUP; \
			return -1; \
		} \
	}

NUMCAST10_DEFINE_FW_FUNC(uchar, unsigned char, UCHAR_MAX)
NUMCAST10_DEFINE_FW_FUNC(ushort, unsigned short, USHRT_MAX)
NUMCAST10_DEFINE_FW_FUNC(uint, unsigned int, UINT_MAX)
NUMCAST10_DEFINE_FW_FUNC(ulong, unsigned long, ULONG_MAX)
NUMCAST10_DEFINE_FW_FUNC(ullong, unsigned long long, ULLONG_MAX)

NUMCAST10_DEFINE_FW_FUNC(size_t, size_t, SIZE_MAX)
NUMCAST10_DEFINE_FW_FUNC(ptrdiff_t, ptrdiff_t, PTRDIFF_MAX)

NUMCAST10_DEFINE_FW_FUNC(uint8_t, uint8_t, UINT8_MAX)
NUMCAST10_DEFINE_FW_FUNC(uint16_t, uint16_t, UINT16_MAX)
NUMCAST10_DEFINE_FW_FUNC(uint32_t, uint32_t, UINT32_MAX)
NUMCAST10_DEFINE_FW_FUNC(uint64_t, uint64_t, UINT64_MAX)

NUMCAST10_DEFINE_FW_FUNC(uint_fast8_t, uint_fast8_t, UINT_FAST8_MAX)
NUMCAST10_DEFINE_FW_FUNC(uint_fast16_t, uint_fast16_t, UINT_FAST16_MAX)
NUMCAST10_DEFINE_FW_FUNC(uint_fast32_t, uint_fast32_t, UINT_FAST32_MAX)
NUMCAST10_DEFINE_FW_FUNC(uint_fast64_t, uint_fast64_t, UINT_FAST64_MAX)

NUMCAST10_DEFINE_FW_FUNC(uint_least8_t, uint_least8_t, UINT_LEAST8_MAX)
NUMCAST10_DEFINE_FW_FUNC(uint_least16_t, uint_least16_t, UINT_LEAST16_MAX)
NUMCAST10_DEFINE_FW_FUNC(uint_least32_t, uint_least32_t, UINT_LEAST32_MAX)
NUMCAST10_DEFINE_FW_FUNC(uint_least64_t, uint_least64_t, UINT_LEAST64_MAX)

NUMCAST10_DEFINE_FW_FUNC(uintmax_t, uintmax_t, UINTMAX_MAX)

NUMCAST10_DEFINE_FW_FUNC(uintptr_t, uintptr_t, UINTPTR_MAX)

#if defined(__cplusplus)
}
#endif

#endif // NUMCAST10_H_INCLUDED
