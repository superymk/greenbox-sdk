/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 * 
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 * 
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 * 
 * Authors: Miao Yu
 *          Virgil Gligor
 */

#include <sa_headers.h>

List* list_create(void)
{
    List* result = (List*)os->mm_malloc_zeroed(sizeof(List));
	if(!result)
		return NULL;

	result->count = 0;
	result->in_iteration = false;
	result->first = NULL;
	result->last = NULL;

	return result;
}

void list_destroy(List *list)
{
	ListNode *iter, *cur, *next;

	if(!list)
		return;

	// Do nothing if <list> is in the middle of iteration
	if(list->in_iteration)
	{
		PRINT("[LIST-ERR] <list_destroy>: List cannot be destroyed during its iteration\n");
		return;
	}


	iter = list_first(list);

	while((iter != NULL))
	{
		cur = iter;
		next = iter->next;

		if(cur)
		{
			cur->prev = NULL;
			cur->next = NULL;
			os->mm_free(cur);
		}
		iter = next;
	}

	os->mm_free(list);
}

void list_clear(List *list)
{
	if(!list)
		return;

	// Do nothing if <list> is in the middle of iteration
	if(list->in_iteration)
	{
		PRINT("[LIST-ERR] <vkstl_list_clear>: List cannot be modified during its iteration\n");
		return;
	}

	{
		LIST_FOREACH(list, first, next, cur) 
		{
			os->mm_free(cur->value);
			cur->value = NULL;
		}
		END_LIST_FOREACH(list);
	}
}

void list_debug(List *list)
{
	int count = 0;
	if(!list)
		return;

	PRINT("Total count:%d\n", list->count);
	{
	    LIST_FOREACH(list, first, next, cur) 
		{
			count++;
	        PRINT("[%d] (cur:0x%lX, value:0x%lX)<-->", count, (vaddr_t)cur, (vaddr_t)cur->value);
	    }
		END_LIST_FOREACH(list);
	}
	PRINT("\n");
}


void list_clear_destroy(List *list)
{
    list_clear(list);
    list_destroy(list);
}

ListNode* list_push(List *list, void *value)
{
    ListNode* node = NULL;
	
	if(!list)
		return NULL;

	// Do nothing if <list> is in the middle of iteration
	if(list->in_iteration)
	{
		PRINT("[LIST-ERR] <list_push>: List cannot be modified during its iteration\n");
		return NULL;
	}
	
	node = os->mm_malloc_zeroed(sizeof(ListNode));
	if(!node)
		return NULL;

	node->next = NULL;
    node->value = value;

    if(list->last == NULL) {
        list->first = node;
        list->last = node;
    } else {
        list->last->next = node;
        node->prev = list->last;
        list->last = node;
    }

    list->count++;

    return node;
}

void* list_pop(List *list)
{
	ListNode *node;
	
	if(!list)
		return NULL;

	// Do nothing if <list> is in the middle of iteration
	if(list->in_iteration)
	{
		PRINT("[LIST-ERR] <list_pop>: List cannot be modified during its iteration\n");
		return NULL;
	}
	
    node = list->last;
    return node != NULL ? list_remove(list, node) : NULL;
}

void* list_dequeue(List *list)
{
	ListNode *node;
	
	if(!list)
		return NULL;

	// Do nothing if <list> is in the middle of iteration
	if(list->in_iteration)
	{
		PRINT("[LIST-ERR] <list_dequeue>: List cannot be modified during its iteration\n");
		return NULL;
	}
	
    node = list->first;
    return node != NULL ? list_remove(list, node) : NULL;
}


ListNode* list_last(List *list)
{
	ListNode *node;
	
	if(!list)
		return NULL;
	
    node = list->last;
    return node;
}

ListNode* list_first(List *list)
{
	ListNode *node;
	
	if(!list)
		return NULL;
	
    node = list->first;
    return node;
}

void* list_remove(List *list, ListNode *node)
{
    void *result = NULL;

	if(!list || !node)
		return NULL;

	// Do nothing if <list> is in the middle of iteration
	if(list->in_iteration)
	{
		PRINT("[LIST-ERR] <list_remove>: List cannot be modified during its iteration\n");
		return NULL;
	}

    if(node == list->first && node == list->last) {
        list->first = NULL;
        list->last = NULL;
    } else if(node == list->first) {
        list->first = node->next;
        //check(list->first != NULL, "Invalid list, somehow got a first that is NULL.");
        list->first->prev = NULL;
    } else if (node == list->last) {
        list->last = node->prev;
        //check(list->last != NULL, "Invalid list, somehow got a next that is NULL.");
        list->last->next = NULL;
    } else {
        ListNode *after = node->next;
        ListNode *before = node->prev;
        after->prev = before;
        before->next = after;
    }

    list->count--;
    result = node->value;
    os->mm_free(node);

    return result;
}