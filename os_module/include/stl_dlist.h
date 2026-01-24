// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

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

/// @brief Create a list
/// @return New list or NULL on failure
extern List* list_create(void);

/// @brief Destroy a list and free nodes
/// @param list List to destroy
extern void list_destroy(List *list);

/// @brief Clear list values
/// @param list List to clear
extern void list_clear(List *list);

/// @brief Clear and destroy list
/// @param list List to clear and destroy
extern void list_clear_destroy(List *list);

#define List_count(A) ((A)->count)
#define List_first(A) ((A)->first != NULL ? (A)->first->value : NULL)
#define List_last(A) ((A)->last != NULL ? (A)->last->value : NULL)
#define list_enqueue(l, v) list_push(l, v)
#define is_list_empty(l) (l->count == 0)

/// @brief Push value to end of list
/// @param list List
/// @param value Value to add
/// @return New node or NULL on failure
extern ListNode* list_push(List *list, void *value);

/// @brief Pop value from end of list
/// @param list List
/// @return Value or NULL if empty
extern void* list_pop(List *list);

/// @brief Dequeue value from start of list
/// @param list List
/// @return Value or NULL if empty
extern void* list_dequeue(List *list);

/// @brief Get last node
/// @param list List
/// @return Last node or NULL
extern ListNode* list_last(List *list);

/// @brief Get first node
/// @param list List
/// @return First node or NULL
extern ListNode* list_first(List *list);

/// @brief Insert value at start of list
/// @param list List
/// @param value Value to insert
extern void list_unshift(List *list, void *value);

/// @brief Remove value from start of list
/// @param list List
/// @return Value or NULL
extern void* list_shift(List *list);

/// @brief Remove node from list
/// @param list List
/// @param node Node to remove
/// @return Value of removed node
extern void* list_remove(List *list, ListNode *node);

/// @brief Debug dump of list
/// @param list List
extern void list_debug(List *list);

#define LIST_FOREACH(L, S, M, V) ListNode *_node = NULL;\
    ListNode *V = NULL;\
    L->in_iteration = true; \
    for(V = _node = L->S; _node != NULL; V = _node = _node->M)

#define END_LIST_FOREACH(L) L->in_iteration = false;

#endif // __ASSEMBLY__
#endif // VK_STL_DOUBLE_LLIST_H