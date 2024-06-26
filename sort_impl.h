#include "queue.h"
struct list_head;

typedef int (*list_cmp_func_t)(void *,
                               const struct list_head *,
                               const struct list_head *);

void timsort(void *priv, struct list_head *head, list_cmp_func_t cmp);
extern void list_sort(void *priv,
                      struct list_head *head,
                      bool descend,
                      list_cmp_func_t cmp);
extern void my_sort(void *priv,
                    struct list_head *head,
                    bool descend,
                    list_cmp_func_t cmp);