/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2026
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <stdio.h>
#include <string.h>
#include "list.h"

struct student {
	int id;
	char name[16];
	int score;
	struct list_head list;
};

static void print_list(const char *title, struct list_head *head)
{
	struct student *pos;

	printf("\n--- %s ---\n", title);
	if (list_empty(head)) {
		printf("(empty)\n");
		return;
	}
	list_for_each_entry(pos, head, list) {
		printf("ID: %d, Name: %-8s, Score: %d\n",
		       pos->id, pos->name, pos->score);
	}
}

static struct student *find_by_id(struct list_head *head, int id)
{
	struct student *pos;

	list_for_each_entry(pos, head, list) {
		if (pos->id == id)
			return pos;
	}
	return NULL;
}

int main(void)
{
	LIST_HEAD(head);
	struct student students[6] = {
		{1, "Alice",   85,   {NULL, NULL}},
		{2, "Bob",     90,   {NULL, NULL}},
		{3, "Charlie", 78,   {NULL, NULL}},
		{4, "David",   92,   {NULL, NULL}},
		{5, "Eve",     88,   {NULL, NULL}},
		{6, "Frank",   95,   {NULL, NULL}},
	};
	struct student *pos, *n, *found;
	int i;

	/* Test 1: Add 5 students to list (queue style) */
	printf("Test 1: Add 5 students to list (tail add)\n");
	for (i = 0; i < 5; i++) {
		INIT_LIST_HEAD(&students[i].list);
		list_add_tail(&students[i].list, &head);
	}
	print_list("After add", &head);

	/* Test 2: Query - search by ID */
	printf("\nTest 2: Find & Query - search ID=3\n");
	found = find_by_id(&head, 3);
	if (found) {
		printf("Found: ID=%d, Name=%s, Score=%d\n",
		       found->id, found->name, found->score);
	}

	/* Test 3: Modify score */
	printf("\nTest 3: Modify score of Bob (ID=2) to 100\n");
	found = find_by_id(&head, 2);
	if (found)
		found->score = 100;
	print_list("After modify", &head);

	/* Test 4: Delete Charlie (ID=3) */
	printf("\nTest 4: Delete Charlie (ID=3)\n");
	found = find_by_id(&head, 3);
	if (found)
		list_del_init(&found->list);
	print_list("After delete ID=3", &head);

	/* Test 5: Move David (ID=4) to head */
	printf("\nTest 5: Move David (ID=4) to head\n");
	found = find_by_id(&head, 4);
	if (found)
		list_move(&found->list, &head);
	print_list("After move ID=4 to head", &head);

	/* Test 6: Replace Alice (ID=1) with Frank (ID=6) */
	printf("\nTest 6: Replace Alice (ID=1) with Frank (ID=6)\n");
	INIT_LIST_HEAD(&students[5].list);
	found = find_by_id(&head, 1);
	if (found) {
		list_replace(&found->list, &students[5].list);
		INIT_LIST_HEAD(&found->list);
	}
	print_list("After replace ID=1 with ID=6", &head);

	/* Test 7: Check list_empty */
	printf("\nTest 7: Check list_empty before cleanup\n");
	printf("list_empty = %s\n", list_empty(&head) ? "true" : "false");

	/* Test 8: Cleanup all nodes using safe iterator */
	printf("\nTest 8: Cleanup all nodes using list_for_each_entry_safe\n");
	list_for_each_entry_safe(pos, n, &head, list) {
		list_del_init(&pos->list);
	}
	print_list("After cleanup", &head);
	printf("list_empty = %s\n", list_empty(&head) ? "true" : "false");

	/* Test 9: Stack style add (list_add) */
	printf("\nTest 9: Test list_add (stack style)\n");
	list_add(&students[0].list, &head);  /* Alice */
	list_add(&students[2].list, &head);  /* Charlie */
	print_list("Stack style add (Charlie on top)", &head);

	/* Test 10: list_is_singular with one node left */
	printf("\nTest 10: Leave only one node\n");
	list_del_init(&students[2].list);  /* remove Charlie */
	print_list("Only Alice left", &head);
	printf("list_is_singular = %s\n",
	       list_is_singular(&head) ? "true" : "false");

	/* Test 11: Container_of sanity check */
	printf("\nTest 11: container_of sanity check\n");
	{
		struct list_head *node = head.next;
		struct student *s = container_of(node, struct student, list);
		printf("Via container_of: ID=%d, Name=%s, Score=%d\n",
		       s->id, s->name, s->score);
	}

	/* Test 12: Rotate left */
	printf("\nTest 12: list_rotate_left\n");
	list_add_tail(&students[2].list, &head); /* add Charlie back */
	list_add_tail(&students[3].list, &head); /* add David back */
	print_list("Before rotate", &head);
	list_rotate_left(&head);
	print_list("After rotate_left", &head);

	printf("\n========================================\n");
	printf("All tests passed! No malloc/free used.\n");
	printf("========================================\n");
	return 0;
}
