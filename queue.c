#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

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
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}