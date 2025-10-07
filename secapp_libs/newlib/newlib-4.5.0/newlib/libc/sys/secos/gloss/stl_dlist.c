// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>
#include <sap_headers_libc.h>

List *stl_list_create()
{
    List *result = (List *)malloc(sizeof(List));
    if (!result)
        return NULL;

    result->count = 0;
    result->in_iteration = false;
    result->first = NULL;
    result->last = NULL;

    return result;
}

void stl_list_destroy(List *list)
{
    ListNode *iter, *cur, *next;

    if (!list)
        return;

    // Do nothing if <list> is in the middle of iteration
    if (list->in_iteration)
    {
        printf("[LIST-ERR] <list_destroy>: List cannot be destroyed during its iteration\n");
        return;
    }

    iter = stl_list_first(list);

    while ((iter != NULL))
    {
        cur = iter;
        next = iter->next;

        if (cur)
        {
            cur->prev = NULL;
            cur->next = NULL;
            free(cur);
        }
        iter = next;
    }

    free(list);
}

void stl_list_clear(List *list)
{
    if (!list)
        return;

    // Do nothing if <list> is in the middle of iteration
    if (list->in_iteration)
    {
        printf("[LIST-ERR] <list_clear>: List cannot be modified during its iteration\n");
        return;
    }

    LIST_FOREACH(list, first, next, cur)
    {
        free(cur->value);
        cur->value = NULL;
    }
    END_LIST_FOREACH(list);
}

void stl_list_debug(List *list)
{
    int count = 0;
    if (!list)
        return;

    printf("Total count:%d\n", list->count);
    {
        LIST_FOREACH(list, first, next, cur)
        {
            count++;
            printf("[%d] (cur:0x%lX, value:0x%lX)<-->", count, (vaddr_t)cur, (ulong_t)cur->value);
        }
        END_LIST_FOREACH(list);
    }
    printf("\n");
}

void stl_list_clear_destroy(List *list)
{
    stl_list_clear(list);
    stl_list_destroy(list);
}

ListNode *stl_list_push(List *list, void *value)
{
    ListNode *node = NULL;

    if (!list)
        return NULL;

    // Do nothing if <list> is in the middle of iteration
    if (list->in_iteration)
    {
        printf("[LIST-ERR] <list_push>: List cannot be modified during its iteration\n");
        return NULL;
    }

    node = (ListNode *)malloc(sizeof(ListNode));
    if (!node)
        return NULL;

    node->next = NULL;
    node->value = value;

    if (list->last == NULL)
    {
        list->first = node;
        list->last = node;
    }
    else
    {
        list->last->next = node;
        node->prev = list->last;
        list->last = node;
    }

    list->count++;

    // error:
    return node;
}

void *stl_list_pop(List *list)
{
    ListNode *node;

    if (!list)
        return NULL;

    // Do nothing if <list> is in the middle of iteration
    if (list->in_iteration)
    {
        printf("[LIST-ERR] <list_pop>: List cannot be modified during its iteration\n");
        return NULL;
    }

    node = list->last;
    return node != NULL ? stl_list_remove(list, node) : NULL;
}

void *stl_list_dequeue(List *list)
{
    ListNode *node;

    if (!list)
        return NULL;

    // Do nothing if <list> is in the middle of iteration
    if (list->in_iteration)
    {
        printf("[LIST-ERR] <list_dequeue>: List cannot be modified during its iteration\n");
        return NULL;
    }

    node = list->first;
    return node != NULL ? stl_list_remove(list, node) : NULL;
}

ListNode *stl_list_last(List *list)
{
    ListNode *node;

    if (!list)
        return NULL;

    node = list->last;
    return node;
}

ListNode *stl_list_first(List *list)
{
    ListNode *node;

    if (!list)
        return NULL;

    node = list->first;
    return node;
}

void *stl_list_remove(List *list, ListNode *node)
{
    void *result = NULL;

    if (!list || !node)
        return NULL;

    // Do nothing if <list> is in the middle of iteration
    if (list->in_iteration)
    {
        printf("[LIST-ERR] <list_remove>: List cannot be modified during its iteration\n");
        return NULL;
    }

    if (node == list->first && node == list->last)
    {
        list->first = NULL;
        list->last = NULL;
    }
    else if (node == list->first)
    {
        list->first = node->next;
        // check(list->first != NULL, "Invalid list, somehow got a first that is NULL.");
        list->first->prev = NULL;
    }
    else if (node == list->last)
    {
        list->last = node->prev;
        // check(list->last != NULL, "Invalid list, somehow got a next that is NULL.");
        list->last->next = NULL;
    }
    else
    {
        ListNode *after = node->next;
        ListNode *before = node->prev;
        after->prev = before;
        before->next = after;
    }

    list->count--;
    result = node->value;
    free(node);

    // error:
    return result;
}
