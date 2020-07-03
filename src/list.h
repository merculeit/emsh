// SPDX-License-Identifier: BSL-1.0

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#if !defined(LIST_H_INCLUDED)
#define LIST_H_INCLUDED

#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(LIST_DEBUG)
  #if defined(NDEBUG)
    #define LIST_DEBUG 0
  #else
    #define LIST_DEBUG 1
  #endif
#endif

#if LIST_DEBUG
  #define list_assert(_cond) assert(_cond)
#else
  #define list_assert(_cond)
#endif

/*
 *              bidirectional linked list
 *
 *            node         node         node
 *         +--------+   +--------+   +--------+
 *   ...-->|  next--+-->|  next--+-->|  next--+-->...
 *   ...<--+--prev  |<--+--prev  |<--+--prev  |<--...
 *         +--------+   +--------+   +--------+
 *
 *
 *                intrusive linked list
 *
 *            back                     front
 *         +--------+                +--------+
 *         |        |                |        |
 *      +--+-[node]-+-----[list]-----+-[node]-+--+
 *      |  |        |                |        |  |
 *      |  +--------+                +--------+  |
 *      |                                        |
 *      +----------------- ... ------------------+
 *
 * list_entry_of() retrieves a pointer-to-user-data-structure from a pointer-to-node.
 */
struct list_node
{
	struct list_node *prev, *next;
};

typedef struct list_node list_node_t;
typedef struct list_node list_t;

static inline
void list_node_init(list_node_t *self)
{
	list_assert(self != NULL);

	self->prev = self->next = NULL;
}

#if LIST_DEBUG
/// @internal
static inline
bool list_node_sanity_check(const list_node_t *self)
{
	if (self == NULL)
	{
		return false;
	}

	if (self->prev == self || self->next == self)
	{
		return false;
	}

	if (self->prev == NULL && self->next == NULL)
	{
		return true;
	}

	if ((self->prev != NULL && self->next != NULL) && (self->prev->next == self && self->next->prev == self))
	{
		return true;
	}

	return false;
}
#endif

static inline
bool list_node_is_linked(const list_node_t *self)
{
	list_assert(list_node_sanity_check(self));

	return self->prev != NULL;
}

static inline
void list_node_unlink(list_node_t *self)
{
	list_assert(list_node_sanity_check(self));

	if (list_node_is_linked(self))
	{
		self->prev->next = self->next;
		self->next->prev = self->prev;
		list_node_init(self);
	}
}

static inline
list_node_t *list_node_next(list_node_t *self)
{
	list_assert(list_node_sanity_check(self));

	return self->next;
}

static inline
const list_node_t *list_node_next_const(const list_node_t *self)
{
	list_assert(list_node_sanity_check(self));

	return self->next;
}

static inline
list_node_t *list_node_prev(list_node_t *self)
{
	list_assert(list_node_sanity_check(self));

	return self->prev;
}

static inline
const list_node_t *list_node_prev_const(const list_node_t *self)
{
	list_assert(list_node_sanity_check(self));

	return self->prev;
}

/// @internal
static inline
const char *_list_entry_of(const list_node_t *self, const list_node_t *offset)
{
	list_assert(self != NULL);

	return (const char *)self - (size_t)offset;
}

#define list_entry_of(_self, _type, _memb)\
	((_type *)_list_entry_of(_self, &((_type *)0)->_memb))

static inline
void list_init(list_t *self)
{
	list_assert(self != NULL);

	self->prev = self->next = self;
}

#define LIST_DECLARE(_name)\
	list_t _name = {&_name, &_name}

#if LIST_DEBUG
/// @internal
static inline
bool list_sanity_check(const list_t *self)
{
	if (self == NULL)
	{
		return false;
	}

	if (self->prev == NULL || self->next == NULL)
	{
		return false;
	}

	if (self->prev == self && self->next == self)
	{
		return true;
	}

	if (self->prev == self || self->next == self)
	{
		return false;
	}

	return true;
}
#endif

static inline
bool list_is_empty(const list_t *self)
{
	list_assert(list_sanity_check(self));

	return self->prev == self;
}

static inline
void list_move(list_t *self, list_t *other)
{
	list_assert(list_sanity_check(self));
	list_assert(list_sanity_check(other));

	list_assert(list_is_empty(self));

	if (!list_is_empty(other))
	{
		*self = *other;

		self->prev->next = self;
		self->next->prev = self;

		list_init(other);
	}
}

static inline
list_node_t *list_front(list_t *self)
{
	list_assert(list_sanity_check(self));

	if (!list_is_empty(self))
	{
		return self->next;
	}
	else
	{
		return NULL;
	}
}

static inline
const list_node_t *list_front_const(const list_t *self)
{
	return list_front((list_t *)self);
}

static inline
list_node_t *list_back(list_t *self)
{
	list_assert(list_sanity_check(self));

	if (!list_is_empty(self))
	{
		return self->prev;
	}
	else
	{
		return NULL;
	}
}

static inline
const list_node_t *list_back_const(const list_t *self)
{
	return list_back((list_t *)self);
}

static inline
void list_push_front(list_t *self, list_node_t *node)
{
	list_assert(list_sanity_check(self));
	list_assert(list_node_sanity_check(node));

	list_assert(!list_node_is_linked(node));

	node->prev = self;
	node->next = self->next;
	self->next = node;
	node->next->prev = node;
}

static inline
void list_push_back(list_t *self, list_node_t *node)
{
	list_assert(list_sanity_check(self));
	list_assert(list_node_sanity_check(node));

	list_assert(!list_node_is_linked(node));

	node->next = self;
	node->prev = self->prev;
	self->prev = node;
	node->prev->next = node;
}

static inline
list_node_t *list_pop_front(list_t *self)
{
	list_assert(list_sanity_check(self));

	list_node_t *node = NULL;
	if (!list_is_empty(self))
	{
		node = list_front(self);
		list_node_unlink(node);
	}
	return node;
}

static inline
list_node_t *list_pop_back(list_t *self)
{
	list_assert(list_sanity_check(self));

	list_node_t *node = NULL;
	if (!list_is_empty(self))
	{
		node = list_back(self);
		list_node_unlink(node);
	}
	return node;
}

static inline
void list_join_front(list_t *self, list_t *other)
{
	list_assert(list_sanity_check(self));
	list_assert(list_sanity_check(other));

	if (!list_is_empty(other))
	{
		self->next->prev = other->prev;
		other->prev->next = self->next;
		other->next->prev = self;
		self->next = other->next;

		list_init(other);
	}
}

static inline
void list_join_back(list_t *self, list_t *other)
{
	list_assert(list_sanity_check(self));
	list_assert(list_sanity_check(other));

	if (!list_is_empty(other))
	{
		self->prev->next = other->next;
		other->next->prev = self->prev;
		other->prev->next = self;
		self->prev = other->prev;

		list_init(other);
	}
}

#define list_for_each(_node, _list)\
	for (const list_node_t *_node = list_front_const(_list);\
		 _node != (_list);\
		 _node = list_node_next_const(_node))

#define list_for_each_reverse(_node, _list)\
	for (const list_node_t *_node = list_back_const(_list);\
		 _node != (_list);\
		 _node = list_node_prev_const(_node))

#if defined(__cplusplus)
}
#endif

#endif // LIST_H_INCLUDED
