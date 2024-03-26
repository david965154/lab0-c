#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "sort_impl.h"

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head != NULL) {
        head->next = head;
        head->prev = head;
        return head;
    }
    return NULL;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (head != NULL) {
        struct list_head *node;
        struct list_head *safe;
        list_for_each_safe (node, safe, head) {
            list_del(node);
            free((list_entry(node, element_t, list)->value));
            free(list_entry(node, element_t, list));
            // free(node);
        }
        free(head);

        node = NULL;
        safe = NULL;
    }
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (head != NULL) {
        element_t *new_node = malloc(sizeof(element_t));
        if (new_node == NULL)
            return false;
        new_node->value = strdup(s);
        if (new_node->value == NULL) {
            free(new_node);
            return false;
        }
        list_add(&new_node->list, head);
        return true;
    }
    return false;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (head != NULL) {
        element_t *new_node = malloc(sizeof(element_t));
        if (!new_node)
            return false;
        new_node->value = strdup(s);
        if (new_node->value == NULL) {
            free(new_node);
            return false;
        }
        list_add_tail(&new_node->list, head);
        return true;
    }
    return false;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head != NULL && list_empty(head) == 0) {
        struct list_head *return_element = head->next;
        list_del(return_element);
        strncpy(sp, (list_entry(return_element, element_t, list)->value),
                bufsize);
        if (bufsize > 0)
            sp[bufsize - 1] = '\0';
        return list_entry(return_element, element_t, list);
    }
    return NULL;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head != NULL && list_empty(head) == 0) {
        struct list_head *return_element = head->prev;
        list_del(return_element);
        strncpy(sp, (list_entry(return_element, element_t, list)->value),
                bufsize);
        if (bufsize > 0)
            sp[bufsize - 1] = '\0';
        return list_entry(return_element, element_t, list);
    }
    return NULL;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (head != NULL && list_empty(head) == 0) {
        struct list_head *tmp = head->next;
        int i = 1;
        for (; tmp->next != head; tmp = tmp->next, i++)
            ;
        return i;
    }
    return 0;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (head == NULL || list_empty(head) != 0)
        return false;
    struct list_head *fast = head->next;
    struct list_head *slow = head->next;
    for (fast = fast->next; fast != head && fast->next != head;
         fast = fast->next->next, slow = slow->next)
        ;
    list_del(slow);
    free(list_entry(slow, element_t, list)->value);
    free(list_entry(slow, element_t, list));
    slow = NULL;
    fast = NULL;
    return true;
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (head == NULL)
        return false;
    else if (list_empty(head) != 0 || list_is_singular(head) != 0)
        return true;
    else if (list_empty(head) == 0) {
        struct list_head *node = NULL;
        struct list_head *safe = NULL;
        int size = q_size(head);
        struct list_head *struct_list = q_new();
        struct list_head *tmp;
        bool *is_single = malloc(sizeof(bool) * size);
        int i = 0;
        list_for_each_safe (node, safe, head) {
            list_del(node);
            tmp = struct_list->next;
            if (struct_list->next != struct_list) {
                for (int j = 0; j < i; j++) {
                    if (strcmp(list_entry(tmp, element_t, list)->value,
                               list_entry(node, element_t, list)->value) == 0) {
                        free(list_entry(node, element_t, list)->value);
                        free(list_entry(node, element_t, list));
                        is_single[j] = false;
                        node = NULL;
                        break;
                    }
                    tmp = tmp->next;
                }
            }
            if (node) {
                list_add_tail(node, struct_list);
                is_single[i] = true;
                ++i;
            }
        }
        for (int j = 0; j < i; j++) {
            node = struct_list->next;
            list_del(node);
            if (is_single[j]) {
                list_add_tail(node, head);
            } else {
                free((list_entry(node, element_t, list)->value));
                free(list_entry(node, element_t, list));
            }
        }
        free(struct_list);
        free(is_single);
        node = NULL;
        safe = NULL;
        struct_list = NULL;
        return true;
    }
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    return false;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (head != NULL && list_empty(head) == 0 && list_is_singular(head) == 0) {
        int size = q_size(head);
        struct list_head *cur_node = head->next;
        struct list_head *first_node = NULL;
        while (size >= 2) {
            list_del(cur_node);
            first_node = cur_node;
            cur_node = head->next;
            list_move_tail(cur_node, head);
            list_add_tail(first_node, head);
            size -= 2;
            cur_node = head->next;
        }
        if (size == 1) {
            list_move_tail(cur_node, head);
        }
    }
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (head != NULL && list_empty(head) == 0 && list_is_singular(head) == 0) {
        struct list_head *cur_node = head->next->next;
        while (cur_node != head) {
            struct list_head *move_node = cur_node;
            cur_node = cur_node->next;
            list_move(move_node, head);
        }
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (head != NULL && list_empty(head) == 0 && list_is_singular(head) == 0) {
        int size = q_size(head);
        struct list_head *cur_node;
        struct list_head *move_node;
        int i = 0;
        for (int time = 0; time < size / k; time++) {
            cur_node = head->next->next;
            while (i < k - 1) {
                move_node = cur_node;
                cur_node = cur_node->next;
                list_move(move_node, head);
                ++i;
            }
            ++i;
            while (i > 0) {
                move_node = head->next;
                list_move_tail(move_node, head);
                --i;
            }
        }
        for (int time = 0; time < size % k; time++) {
            move_node = head->next;
            list_move_tail(move_node, head);
        }
        // https://leetcode.com/problems/reverse-nodes-in-k-group/
    }
}


/* Sort elements of queue in ascending/descending order */
void my_sort(void *priv,
             struct list_head *head,
             bool descend,
             list_cmp_func_t cmp)
{
    if (head != NULL && list_empty(head) == 0 && list_is_singular(head) == 0) {
        struct list_head *merge_list_1;
        struct list_head *merge_list_2;
        struct list_head *safe_list_1;
        struct list_head *safe_list_2;
        int size = q_size(head);
        int unmerge;
        int len;
        struct list_head *tmp;
        do {
            unmerge = 1;
            len = 1;
            int list_1_stride = 0;
            int list_2_stride = 0;
            merge_list_1 = head->next;
            merge_list_2 = head->next;
            tmp = NULL;
            for (; merge_list_2 != head &&
                   cmp(priv, merge_list_2, merge_list_2->next) <= 0;) {
                merge_list_2 = merge_list_2->next;
                len++;
            }
            tmp = merge_list_2->next;
            for (; tmp != head && tmp->next != head &&
                   cmp(priv, tmp, tmp->next) <= 0;) {
                tmp = tmp->next;
                unmerge++;
            }
            merge_list_2 = merge_list_2->next;
            // merge
            while ((list_1_stride < len) && (list_2_stride < unmerge)) {
                // a<b
                if (cmp(priv, merge_list_1, merge_list_2) <= 0) {
                    safe_list_1 = merge_list_1->next;
                    list_del(merge_list_1);
                    list_add_tail(merge_list_1, head);
                    merge_list_1 = safe_list_1;
                    ++list_1_stride;
                }
                // a>b
                else if (cmp(priv, merge_list_1, merge_list_2) > 0) {
                    safe_list_2 = merge_list_2->next;
                    list_del(merge_list_2);
                    list_add_tail(merge_list_2, head);
                    merge_list_2 = safe_list_2;
                    ++list_2_stride;
                }
            }
            while ((list_1_stride < len) && (list_2_stride == unmerge)) {
                safe_list_1 = merge_list_1->next;
                list_del(merge_list_1);
                list_add_tail(merge_list_1, head);
                merge_list_1 = safe_list_1;
                ++list_1_stride;
            }
            while ((list_1_stride == len) && (list_2_stride < unmerge)) {
                safe_list_2 = merge_list_2->next;
                list_del(merge_list_2);
                list_add_tail(merge_list_2, head);
                merge_list_2 = safe_list_2;
                ++list_2_stride;
            }
        } while ((len + unmerge) != size);
        if (descend)
            q_reverse(head);
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (head != NULL && list_empty(head) == 0 && list_is_singular(head) == 0) {
        struct list_head *merge_list_1;
        struct list_head *merge_list_2;
        struct list_head *safe_list_1;
        struct list_head *safe_list_2;
        int size = q_size(head);
        int unmerge;
        int len;
        struct list_head *tmp;
        do {
            unmerge = 1;
            len = 1;
            int list_1_stride = 0;
            int list_2_stride = 0;
            merge_list_1 = head->next;
            merge_list_2 = head->next;
            tmp = NULL;
            for (;
                 merge_list_2 != head &&
                 strcmp(
                     list_entry(merge_list_2, element_t, list)->value,
                     list_entry(merge_list_2->next, element_t, list)->value) <=
                     0;) {
                merge_list_2 = merge_list_2->next;
                len++;
            }
            tmp = merge_list_2->next;
            for (;
                 tmp != head && tmp->next != head &&
                 strcmp(list_entry(tmp, element_t, list)->value,
                        list_entry(tmp->next, element_t, list)->value) <= 0;) {
                tmp = tmp->next;
                unmerge++;
            }
            merge_list_2 = merge_list_2->next;
            // merge
            while ((list_1_stride < len) && (list_2_stride < unmerge)) {
                // a<b
                if (strcmp(list_entry(merge_list_1, element_t, list)->value,
                           list_entry(merge_list_2, element_t, list)->value) <=
                    0) {
                    safe_list_1 = merge_list_1->next;
                    list_del(merge_list_1);
                    list_add_tail(merge_list_1, head);
                    merge_list_1 = safe_list_1;
                    ++list_1_stride;
                }
                // a>b
                else if (strcmp(
                             list_entry(merge_list_1, element_t, list)->value,
                             list_entry(merge_list_2, element_t, list)->value) >
                         0) {
                    safe_list_2 = merge_list_2->next;
                    list_del(merge_list_2);
                    list_add_tail(merge_list_2, head);
                    merge_list_2 = safe_list_2;
                    ++list_2_stride;
                }
            }
            while ((list_1_stride < len) && (list_2_stride == unmerge)) {
                safe_list_1 = merge_list_1->next;
                list_del(merge_list_1);
                list_add_tail(merge_list_1, head);
                merge_list_1 = safe_list_1;
                ++list_1_stride;
            }
            while ((list_1_stride == len) && (list_2_stride < unmerge)) {
                safe_list_2 = merge_list_2->next;
                list_del(merge_list_2);
                list_add_tail(merge_list_2, head);
                merge_list_2 = safe_list_2;
                ++list_2_stride;
            }
        } while ((len + unmerge) != size);
        if (descend)
            q_reverse(head);
    }
}


/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (head == NULL || list_empty(head) != 0)
        return 0;
    else if (list_is_singular(head) != 0)
        return 1;
    else {
        struct list_head *node;
        struct list_head *safe;
        struct list_head *cmp_node;
        bool smaller = false;

        list_for_each_safe (node, safe, head) {
            if (node->next != head) {
                cmp_node = node->next;
                while (cmp_node != head) {
                    if (strcmp(list_entry(cmp_node, element_t, list)->value,
                               list_entry(node, element_t, list)->value) < 0) {
                        smaller = true;
                        break;
                    }
                    cmp_node = cmp_node->next;
                }
            }
            if (smaller == true) {
                list_del(node);
                free(list_entry(node, element_t, list)->value);
                free(list_entry(node, element_t, list));
                smaller = false;
            }
        }
        return q_size(head);
    }
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (head == NULL || list_empty(head) != 0)
        return 0;
    else if (list_is_singular(head) != 0)
        return 1;
    else {
        struct list_head *node;
        struct list_head *safe;
        struct list_head *cmp_node;
        bool greater = false;

        list_for_each_safe (node, safe, head) {
            if (node->next != head) {
                cmp_node = node->next;
                while (cmp_node != head) {
                    if (strcmp(list_entry(cmp_node, element_t, list)->value,
                               list_entry(node, element_t, list)->value) > 0) {
                        greater = true;
                        break;
                    }
                    cmp_node = cmp_node->next;
                }
            }
            if (greater == true) {
                list_del(node);
                free(list_entry(node, element_t, list)->value);
                free(list_entry(node, element_t, list));
                greater = false;
            }
        }
        // https://leetcode.com/problems/remove-nodes-from-linked-list/
        return q_size(head);
    }
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (head == NULL || list_empty(head) != 0)
        return 0;
    else if (list_is_singular(head) != 0)
        return (q_size(list_entry(head->next, queue_contex_t, chain)->q));
    else {
        struct list_head *merge_list =
            list_entry(head->next, queue_contex_t, chain)->q;
        struct list_head *node;
        struct list_head *safe;
        struct list_head *src_list;
        struct list_head *merge_node;
        struct list_head *src_node;
        struct list_head *merge_safe;
        struct list_head *src_safe;
        struct list_head *tail;
        list_for_each_safe (node, safe, head) {
            if (node != head->next) {
                src_list = list_entry(node, queue_contex_t, chain)->q;
                src_node = src_list->next;
                merge_node = merge_list->next;
                tail = merge_list->prev;
                while (merge_node != merge_list && src_node != src_list) {
                    if (strcmp(list_entry(tail, element_t, list)->value,
                               list_entry(merge_list->prev, element_t, list)
                                   ->value) <= 0) {
                        tail = merge_list->prev;
                    }
                    if (tail == merge_list->prev &&
                        strcmp(list_entry(merge_list->prev, element_t, list)
                                   ->value,
                               list_entry(src_list->next, element_t, list)
                                   ->value) <= 0) {
                        list_splice_tail(src_list, merge_list);
                        INIT_LIST_HEAD(src_list);
                        break;
                    }
                    if (strcmp(list_entry(merge_node, element_t, list)->value,
                               list_entry(src_node, element_t, list)->value) <=
                        0) {
                        merge_safe = merge_node->next;
                        list_del(merge_node);
                        list_add_tail(merge_node, merge_list);
                        merge_node = merge_safe;
                    } else {
                        src_safe = src_node->next;
                        list_del(src_node);
                        list_add_tail(src_node, merge_list);
                        src_node = src_safe;
                    }
                }
                while (
                    strcmp(
                        list_entry(merge_node, element_t, list)->value,
                        list_entry(merge_node->next, element_t, list)->value) >
                    0) {
                    merge_safe = merge_node->next;
                    list_del(merge_node);
                    list_add_tail(merge_node, merge_list);
                    merge_node = merge_safe;
                }
            }
        }
        if (descend)
            q_reverse(merge_list);
        return (q_size(merge_list));
    }
    // https://leetcode.com/problems/merge-k-sorted-lists/
}

void q_shuffle(struct list_head *head)
{
    struct list_head *node;
    if (head != NULL && list_empty(head) == 0 && list_is_singular(head) == 0) {
        int size = q_size(head);
        while (size) {
            int p = rand() % size;
            list_for_each (node, head)
                if (p-- == 0)
                    break;
            list_move_tail(node, head);
            size--;
        }
    }
}



static struct list_head *merge(void *priv,
                               list_cmp_func_t cmp,
                               struct list_head *a,
                               struct list_head *b)
{
    struct list_head *head = NULL, **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(priv, a, b) <= 0) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

static void merge_final(void *priv,
                        list_cmp_func_t cmp,
                        struct list_head *head,
                        struct list_head *a,
                        struct list_head *b)
{
    struct list_head *tail = head;
    unsigned int count = 0;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(priv, a, b) <= 0) {
            tail->next = a;
            a->prev = tail;
            tail = a;
            a = a->next;
            if (!a)
                break;
        } else {
            tail->next = b;
            b->prev = tail;
            tail = b;
            b = b->next;
            if (!b) {
                b = a;
                break;
            }
        }
    }

    /* Finish linking remainder of list b on to tail */
    tail->next = b;
    do {
        if (unlikely(!++count))
            cmp(priv, b, b);
        b->prev = tail;
        tail = b;
        b = b->next;
    } while (b);

    /* And the final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}

void list_sort(void *priv,
               struct list_head *head,
               bool descend,
               list_cmp_func_t cmp)
{
    struct list_head *list = head->next, *pending = NULL;
    size_t count = 0; /* Count of pending */

    if (head == head->prev) /* Zero or one elements */
        return;
    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    do {
        size_t bits;
        struct list_head **tail = &pending;

        /* Find the least-significant clear bit in count */
        for (bits = count; bits & 1; bits >>= 1)
            tail = &(*tail)->prev;
        /* Do the indicated merge */
        if (likely(bits)) {
            struct list_head *a = *tail, *b = a->prev;

            a = merge(priv, cmp, b, a);
            /* Install the merged result in place of the inputs */
            a->prev = b->prev;
            *tail = a;
        }

        /* Move one element from input list to pending */
        list->prev = pending;
        pending = list;
        list = list->next;
        pending->next = NULL;
        count++;
    } while (list);

    /* End of input; merge together all the pending lists. */
    list = pending;
    pending = pending->prev;
    for (;;) {
        struct list_head *next = pending->prev;

        if (!next)
            break;
        list = merge(priv, cmp, pending, list);
        pending = next;
    }
    /* The final merge, rebuilding prev links */
    merge_final(priv, cmp, head, pending, list);
    if (descend)
        q_reverse(head);
}