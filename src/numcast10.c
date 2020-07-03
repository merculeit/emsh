// SPDX-License-Identifier: BSL-1.0

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "numcast10.h"

#define NUMCAST10_IMPL_DEFINE_BW_FUNC(name, T) \
	NUMCAST10_IMPL_BW_FUNC_PROTOTYPE(name, T) \
	{ \
		if (dst == NULL) \
		{ \
			errno = EINVAL; \
			return 0; \
		} \
\
		if (src != 0) \
		{ \
			size_t n = 0; \
\
			while (src != 0) \
			{ \
				dst[n] = (src % 10) + '0'; \
				src /= 10; \
				++n; \
			} \
\
			for (size_t i = 0; i < n / 2; ++i) \
			{ \
				char ch = dst[i]; \
				dst[i] = dst[n-i-1]; \
				dst[n-i-1] = ch; \
			} \
\
			return n; \
		} \
		else \
		{ \
			dst[0] = '0'; \
			return 1; \
		} \
	}

#define NUMCAST10_IMPL_DEFINE_FW_FUNC(name, T) \
	NUMCAST10_IMPL_FW_FUNC_PROTOTYPE(name, T) \
	{ \
		if (p_dst == NULL || (src == NULL && n_src != 0)) \
		{ \
			errno = EINVAL; \
			return -1; \
		} \
\
		size_t i = 0; \
		T dst = 0; \
		for (; i < n_src && ('0' <= src[i] && src[i] <= '9'); ++i) \
		{ \
			if (dst > max_value_10) \
			{ \
				errno = ERANGE; \
				return -1; \
			} \
			dst *= 10; \
\
			T a_i = src[i] - '0'; \
			if (dst > max_value - a_i) \
			{ \
				errno = ERANGE; \
				return -1; \
			} \
			dst += a_i; \
		} \
\
		if (i == 0) \
		{ \
			errno = EILSEQ; \
			return -1; \
		} \
		*p_dst = dst; \
\
		if (p_n_consumed != NULL) \
		{ \
			*p_n_consumed = i; \
		} \
\
		return 0; \
	}

NUMCAST10_IMPL_DEFINE_BW_FUNC(uint, unsigned int)
NUMCAST10_IMPL_DEFINE_FW_FUNC(uint, unsigned int)

#if NUMCAST10_IMPL_HAVE_ULONG
NUMCAST10_IMPL_DEFINE_BW_FUNC(ulong, unsigned long)
NUMCAST10_IMPL_DEFINE_FW_FUNC(ulong, unsigned long)
#endif

#if NUMCAST10_IMPL_HAVE_ULLONG
NUMCAST10_IMPL_DEFINE_BW_FUNC(ullong, unsigned long long)
NUMCAST10_IMPL_DEFINE_FW_FUNC(ullong, unsigned long long)
#endif

#if NUMCAST10_IMPL_HAVE_UINTMAX
NUMCAST10_IMPL_DEFINE_BW_FUNC(uintmax, uintmax_t)
NUMCAST10_IMPL_DEFINE_FW_FUNC(uintmax, uintmax_t)
#endif
