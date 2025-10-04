// copy from http://c.learncodethehardway.org/book/ex32.html

#ifndef VK_STL_DOUBLE_LLIST_H
#define VK_STL_DOUBLE_LLIST_H

#ifndef __ASSEMBLY__

struct ListNode;

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    void *value;
} ListNode;

typedef struct List {
    int         count;
    bool 		in_iteration;
    ListNode*   first;
    ListNode*   last;
} List;

extern List* list_create(void);
extern void list_destroy(List *list);
extern void list_clear(List *list);
extern void list_clear_destroy(List *list);

#define List_count(A) ((A)->count)
#define List_first(A) ((A)->first != NULL ? (A)->first->value : NULL)
#define List_last(A) ((A)->last != NULL ? (A)->last->value : NULL)
#define list_enqueue(l, v) list_push(l, v)
#define is_list_empty(l) (l->count == 0)

extern ListNode* list_push(List *list, void *value);
extern void* list_pop(List *list);
extern void* list_dequeue(List *list);

extern ListNode* list_last(List *list);
extern ListNode* list_first(List *list);


extern void list_unshift(List *list, void *value);
extern void* list_shift(List *list);

extern void* list_remove(List *list, ListNode *node);
extern void list_debug(List *list);

#define LIST_FOREACH(L, S, M, V) ListNode *_node = NULL;\
    ListNode *V = NULL;\
    L->in_iteration = true; \
    for(V = _node = L->S; _node != NULL; V = _node = _node->M)

#define END_LIST_FOREACH(L) L->in_iteration = false;

#endif // __ASSEMBLY__
#endif // VK_STL_DOUBLE_LLIST_H