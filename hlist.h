#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#if defined(__GNUC__) || defined(__clang__)
#define __HAVE_TYPEOF 1
#endif

#define WRITE_ONCE(var, val) (*((volatile typeof(val) *) (&(var))) = (val))

#define READ_ONCE(var) (*((volatile typeof(var) *) (&(var))))

#define __aligned(x) __attribute((__aligned__(x)))

/**
 * container_of() - Calculate address of structure that contains address ptr
 * @ptr: pointer to member variable
 * @type: type of the structure containing ptr
 * @member: name of the member variable in struct @type
 *
 * Return: @type pointer of structure containing ptr
 */
#ifndef container_of
#ifdef __HAVE_TYPEOF
#define container_of(ptr, type, member)                            \
    __extension__({                                                \
        const __typeof__(((type *) 0)->member) *__pmember = (ptr); \
        (type *) ((char *) __pmember - offsetof(type, member));    \
    })
#else
#define container_of(ptr, type, member) \
    ((type *) ((char *) (ptr) -offsetof(type, member)))
#endif
#endif

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */

struct hlist_node {
    struct hlist_node *next, **pprev;
};

struct hlist_head {
    struct hlist_node *first;
};

#define HLIST_HEAD_INIT \
    {                   \
        .first = NULL   \
    }
#define HLIST_HEAD(name) struct hlist_head name = {.first = NULL}
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
    h->next = NULL;
    h->pprev = NULL;
}

/**
 * hlist_unhashed - Has node been removed from list and reinitialized?
 * @h: Node to be checked
 *
 * Not that not all removal functions will leave a node in unhashed
 * state.  For example, hlist_nulls_del_init_rcu() does leave the
 * node in unhashed state, but hlist_nulls_del() does not.
 */
static inline int hlist_unhashed(const struct hlist_node *h)
{
    return !h->pprev;
}

/**
 * hlist_unhashed_lockless - Version of hlist_unhashed for lockless use
 * @h: Node to be checked
 *
 * This variant of hlist_unhashed() must be used in lockless contexts
 * to avoid potential load-tearing.  The READ_ONCE() is paired with the
 * various WRITE_ONCE() in hlist helpers that are defined below.
 */
static inline int hlist_unhashed_lockless(const struct hlist_node *h)
{
    return !READ_ONCE(h->pprev);
}

/**
 * hlist_empty - Is the specified hlist_head structure an empty hlist?
 * @h: Structure to check.
 */
static inline int hlist_empty(const struct hlist_head *h)
{
    return !READ_ONCE(h->first);
}

static inline void __hlist_del(struct hlist_node *n)
{
    struct hlist_node *next = n->next;
    struct hlist_node **pprev = n->pprev;

    WRITE_ONCE(*pprev, next);
    if (next)
        WRITE_ONCE(next->pprev, pprev);
}

/**
 * hlist_del - Delete the specified hlist_node from its list
 * @n: Node to delete.
 *
 * Note that this function leaves the node in hashed state.  Use
 * hlist_del_init() or similar instead to unhash @n.
 */
static inline void hlist_del(struct hlist_node *n)
{
    struct hlist_node *next = n->next;
    struct hlist_node **pprev = n->pprev;

    *pprev = next;
    if (next)
        next->pprev = pprev;
}

/**
 * hlist_del_init - Delete the specified hlist_node from its list and initialize
 * @n: Node to delete.
 *
 * Note that this function leaves the node in unhashed state.
 */
static inline void hlist_del_init(struct hlist_node *n)
{
    if (!hlist_unhashed(n)) {
        __hlist_del(n);
        INIT_HLIST_NODE(n);
    }
}

/**
 * hlist_add_head - add a new entry at the beginning of the hlist
 * @n: new entry to be added
 * @h: hlist head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
    struct hlist_node *first = h->first;
    WRITE_ONCE(n->next, first);
    if (first)
        WRITE_ONCE(first->pprev, &n->next);
    WRITE_ONCE(h->first, n);
    WRITE_ONCE(n->pprev, &h->first);
}

/**
 * hlist_add_before - add a new entry before the one specified
 * @n: new entry to be added
 * @next: hlist node to add it before, which must be non-NULL
 */
static inline void hlist_add_before(struct hlist_node *n,
                                    struct hlist_node *next)
{
    WRITE_ONCE(n->pprev, next->pprev);
    WRITE_ONCE(n->next, next);
    WRITE_ONCE(next->pprev, &n->next);
    WRITE_ONCE(*(n->pprev), n);
}

/**
 * hlist_add_behind - add a new entry after the one specified
 * @n: new entry to be added
 * @prev: hlist node to add it after, which must be non-NULL
 */
static inline void hlist_add_behind(struct hlist_node *n,
                                    struct hlist_node *prev)
{
    WRITE_ONCE(n->next, prev->next);
    WRITE_ONCE(prev->next, n);
    WRITE_ONCE(n->pprev, &prev->next);

    if (n->next)
        WRITE_ONCE(n->next->pprev, &n->next);
}

/**
 * hlist_add_fake - create a fake hlist consisting of a single headless node
 * @n: Node to make a fake list out of
 *
 * This makes @n appear to be its own predecessor on a headless hlist.
 * The point of this is to allow things like hlist_del() to work correctly
 * in cases where there is no list.
 */
static inline void hlist_add_fake(struct hlist_node *n)
{
    n->pprev = &n->next;
}

/**
 * hlist_move_list - Move an hlist
 * @old: hlist_head for old list.
 * @new: hlist_head for new list.
 *
 * Move a list from one list head to another. Fixup the pprev
 * reference of the first entry if it exists.
 */
static inline void hlist_move_list(struct hlist_head *old,
                                   struct hlist_head *new)
{
    new->first = old->first;
    if (new->first)
        new->first->pprev = &new->first;
    old->first = NULL;
}

/**
 * hlist_splice_init() - move all entries from one list to another
 * @from: hlist_head from which entries will be moved
 * @last: last entry on the @from list
 * @to:   hlist_head to which entries will be moved
 *
 * @to can be empty, @from must contain at least @last.
 */
static inline void hlist_splice_init(struct hlist_head *from,
                                     struct hlist_node *last,
                                     struct hlist_head *to)
{
    if (to->first)
        to->first->pprev = &last->next;
    last->next = to->first;
    to->first = from->first;
    from->first->pprev = &to->first;
    from->first = NULL;
}

#define hlist_entry(ptr, type, member) container_of(ptr, type, member)

#define hlist_for_each(pos, head) \
    for (pos = (head)->first; pos; pos = pos->next)

#define hlist_for_each_safe(pos, n, head)        \
    for (pos = (head)->first; pos && ({          \
                                  n = pos->next; \
                                  1;             \
                              });                \
         pos = n)

#ifdef __HAVE_TYPEOF
#define hlist_entry_safe(ptr, type, member)                  \
    ({                                                       \
        typeof(ptr) ____ptr = (ptr);                         \
        ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
    })
#else
#define hlist_entry_safe(ptr, type, member) \
    (ptr) ? hlist_entry(ptr, type, member) : NULL
#endif

/**
 * hlist_for_each_entry	- iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#ifdef __HAVE_TYPEOF
#define hlist_for_each_entry(pos, head, member)                              \
    for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member); pos; \
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))
#else
#define hlist_for_each_entry(pos, head, member, type)              \
    for (pos = hlist_entry_safe((head)->first, type, member); pos; \
         pos = hlist_entry_safe((pos)->member.next, type, member))
#endif

/**
 * hlist_for_each_entry_continue - iterate over a hlist continuing after current
 * point
 * @pos:	the type * to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_continue(pos, member)                           \
    for (pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member); \
         pos;                                                                \
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_from - iterate over a hlist continuing from current
 * point
 * @pos:	the type * to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_from(pos, member) \
    for (; pos;                                \
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_safe - iterate over list of given type safe against
 * removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		a &struct hlist_node to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_safe(pos, n, head, member)               \
    for (pos = hlist_entry_safe((head)->first, typeof(*pos), member); \
         pos && ({                                                    \
             n = pos->member.next;                                    \
             1;                                                       \
         });                                                          \
         pos = hlist_entry_safe(n, typeof(*pos), member))

#ifdef __cplusplus
}
#endif