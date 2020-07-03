// SPDX-License-Identifier: BSL-1.0

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#if !defined(BYTEARRAY_H_INCLUDED)
#define BYTEARRAY_H_INCLUDED

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef unsigned char bytearray_datum_t;

/*
 * |<--                capacity                -->|
 * |<-- offset -->|<--    effective capacity   -->|
 *                |<--   size   -->|<--  room  -->|
 * |**************|xxxxxxxxxxxxxxxx|**************|
 *                 ^              ^
 *               front          back
 *               data
 *
 *  x := valid element
 *  * := invalid element
 */
typedef struct bytearray
{
	size_t capacity;
	size_t offset;
	size_t size;
	bytearray_datum_t *data;
} bytearray_t;

#define BYTEARRAY_INITIALIZER(_n, _p)\
	{.capacity = (_n), .offset = 0, .size = 0, .data = (_p)}

#define BYTEARRAY_INITIALIZER_CONST(_n, _p)\
	{.capacity = (_n), .offset = 0, .size = (_n), .data = (bytearray_datum_t *)(_p)}

static inline
void bytearray_init(bytearray_t *self, size_t n, bytearray_datum_t *p)
{
	assert(n != 0);
	assert(p != NULL);

	self->capacity = n;
	self->offset = 0;
	self->size = 0;
	self->data = p;
}

static inline
void bytearray_destroy(bytearray_t *self)
{
	self->capacity = 0;
	self->offset = 0;
	self->size = 0;
	self->data = NULL;
}

static inline
bytearray_datum_t *bytearray_data(bytearray_t *self)
{
	return self->data;
}

static inline
const bytearray_datum_t *bytearray_data_const(const bytearray_t *self)
{
	return self->data;
}

static inline
size_t bytearray_size(const bytearray_t *self)
{
	return self->size;
}

static inline
bool bytearray_empty(const bytearray_t *self)
{
	return bytearray_size(self) == 0;
}

static inline
size_t bytearray_capacity(const bytearray_t *self)
{
	return self->capacity;
}

static inline
size_t bytearray_offset(const bytearray_t *self)
{
	return self->offset;
}

static inline
size_t bytearray_effective_capacity(const bytearray_t *self)
{
	return bytearray_capacity(self) - bytearray_offset(self);
}

static inline
size_t bytearray_room(const bytearray_t *self)
{
	return bytearray_effective_capacity(self) - bytearray_size(self);
}

static inline
void bytearray_clear(bytearray_t *self)
{
	self->size = 0;
}

static inline
void bytearray_reset(bytearray_t *self)
{
	self->data -= self->offset;
	self->offset = 0;
	self->size = 0;
}

static inline
void bytearray_resize(bytearray_t *self, size_t n)
{
	assert(n <= bytearray_effective_capacity(self));

	self->size = n;
}

static inline
void bytearray_push_back(bytearray_t *self, bytearray_datum_t val)
{
	assert(bytearray_room(self) > 0);

	self->data[self->size] = val;
	++self->size;
}

static inline
void bytearray_push_back_n(bytearray_t *self, const bytearray_datum_t *data, size_t n)
{
	assert(bytearray_room(self) >= n);

	memcpy(&self->data[self->size], data, n);
	self->size += n;
}

static inline
void bytearray_pop_back(bytearray_t *self)
{
	assert(bytearray_size(self) >= 1);

	--self->size;
}

static inline
void bytearray_pop_back_n(bytearray_t *self, size_t n)
{
	assert(bytearray_size(self) >= n);

	self->size -= n;
}

static inline
void bytearray_push_front(bytearray_t *self, bytearray_datum_t val)
{
	assert(bytearray_offset(self) > 0);

	--self->offset;
	--self->data;
	++self->size;
	self->data[0] = val;
}

static inline
void bytearray_push_front_n(bytearray_t *self, const bytearray_datum_t *data, size_t n)
{
	assert(bytearray_offset(self) >= n);

	self->offset -= n;
	self->data -= n;
	self->size += n;
	memcpy(&self->data[0], data, n);
}

static inline
void bytearray_pop_front(bytearray_t *self)
{
	assert(bytearray_size(self) >= 1);

	++self->offset;
	++self->data;
	--self->size;
}

static inline
void bytearray_pop_front_n(bytearray_t *self, size_t n)
{
	assert(bytearray_size(self) >= n);

	self->offset += n;
	self->data += n;
	self->size -= n;
}

static inline
void bytearray_insert(bytearray_t *self, size_t pos, bytearray_datum_t val)
{
	assert(bytearray_room(self) > 0);
	assert(pos <= bytearray_size(self));

	memmove(&self->data[pos + 1], &self->data[pos], self->size - pos);
	self->data[pos] = val;
	++self->size;
}

static inline
void bytearray_insert_n(bytearray_t *self, size_t pos, const bytearray_datum_t *data, size_t n)
{
	assert(bytearray_room(self) >= n);
	assert(pos <= bytearray_size(self));

	memmove(&self->data[pos + n], &self->data[pos], self->size - pos);
	memcpy(&self->data[pos], data, n);
	self->size += n;
}

static inline
void bytearray_erase(bytearray_t *self, size_t pos)
{
	assert(bytearray_size(self) > 0);
	assert(pos < bytearray_size(self));

	memmove(&self->data[pos], &self->data[pos + 1], self->size - pos - 1);
	--self->size;
}

static inline
void bytearray_erase_n(bytearray_t *self, size_t pos, size_t n)
{
	assert(bytearray_size(self) >= n);
	assert(pos <= bytearray_size(self) - n);

	memmove(&self->data[pos], &self->data[pos + n], self->size - pos - n);
	self->size -= n;
}

static inline
void bytearray_fill(bytearray_t *self, bytearray_datum_t val)
{
	memset(bytearray_data(self), val, bytearray_size(self));
}

static inline
void bytearray_copy(bytearray_t *self, const bytearray_t *other)
{
	assert(bytearray_capacity(self) >= bytearray_size(other));

	bytearray_reset(self);
	bytearray_resize(self, bytearray_size(other));
	memcpy(bytearray_data(self), bytearray_data_const(other), bytearray_size(other));
}

#if defined(__cplusplus)
}
#endif

#endif // BYTEARRAY_H_INCLUDED
