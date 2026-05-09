# :link: Linux Kernel Linked List — Userspace Port

> :rocket: A **zero-dependency**, **header-only** port of the legendary Linux kernel doubly-linked list for userspace / MCU projects.

---

## :package: What is this?

This project extracts the classic Linux kernel `list.h` implementation and makes it fully self-contained.  
**No kernel headers. No dynamic memory. No bullshit.** Just drop `list.h` into your project and go.

| Feature | Status |
|---------|--------|
| :white_check_mark: Doubly-linked list (`struct list_head`) | Ready |
| :white_check_mark: Hash list (`struct hlist_head` / `hlist_node`) | Ready |
| :white_check_mark: `container_of` macro | Ready |
| :white_check_mark: Zero `malloc` / `free` | Guaranteed |
| :white_check_mark: Zero external dependencies | Only `<stddef.h>` & `<stdbool.h>` |

---

## :open_file_folder: File Layout

```text
.
├── list.h      # :star: The star of the show — header-only linked list implementation
├── main.c      # :microscope: Comprehensive test suite (no malloc!)
├── Makefile    # :gear: Simple build system
└── README.md   # :memo: You are here
```

---

## :mag: Deep Dive into `main.c`

`main.c` is not just a toy example — it's a **battle-tested walkthrough** of every core list operation, written in the style of a real embedded firmware module.

### :classical_building: The Data Structure

```c
struct student {
    int  id;            // :1234: Student ID
    char name[16];      // :label: Name
    int  score;         // :100: Exam score
    struct list_head list;  // :link: The list node embedded INSIDE the struct
};
```

> :bulb: **Key concept**: The list node is **embedded** as a struct member, not pointed to from outside.  
> This is the kernel's signature design — it eliminates extra allocations and gives us `container_of`.

### :card_file_box: Memory Model — Static Array Only!

```c
struct student students[6] = {
    {1, "Alice",   85, {NULL, NULL}},
    {2, "Bob",     90, {NULL, NULL}},
    {3, "Charlie", 78, {NULL, NULL}},
    {4, "David",   92, {NULL, NULL}},
    {5, "Eve",     88, {NULL, NULL}},
    {6, "Frank",   95, {NULL, NULL}},
};
```

> :zap: **Zero heap usage.** Every node lives inside a statically-allocated array.  
> Perfect for MCUs where `malloc` is the enemy.

---

## :test_tube: The 12 Tests Explained

### :one: **Test 1 — Tail Add (Queue Behaviour)**
```c
for (i = 0; i < 5; i++) {
    INIT_LIST_HEAD(&students[i].list);
    list_add_tail(&students[i].list, &head);
}
```
:arrow_right: Uses `list_add_tail` to append elements in order.  
:sparkles: Result: `Alice -> Bob -> Charlie -> David -> Eve`

---

### :two: **Test 2 — Find & Query**
```c
found = find_by_id(&head, 3);
```
:mag: Walks the list with `list_for_each_entry` and returns the node whose `id == 3`.  
:bulb: Demonstrates how `container_of` lets us jump from `struct list_head *` back to the full `struct student *`.

---

### :three: **Test 3 — Modify In-Place**
```c
found = find_by_id(&head, 2);
if (found) found->score = 100;
```
:pencil2: Directly mutates the struct fields through the pointer returned by `find_by_id`.  
:fire: Bob's score is upgraded to **100**.

---

### :four: **Test 4 — Delete Node**
```c
list_del_init(&found->list);
```
:scissors: Cuts Charlie (`ID=3`) out of the list and re-initializes his node so it can be re-used later.  
:white_check_mark: After deletion: `Alice -> Bob -> David -> Eve`

---

### :five: **Test 5 — Move to Head**
```c
list_move(&found->list, &head);
```
:twisted_rightwards_arrows: Plucks David (`ID=4`) from wherever he is and sticks him at the **front** of the list.  
:arrow_right: Result: `David -> Alice -> Bob -> Eve`

---

### :six: **Test 6 — Replace Node**
```c
list_replace(&found->list, &students[5].list);
```
:recycle: Swaps out Alice (`ID=1`) for Frank (`ID=6`) **in-place**, without touching the surrounding links.  
:bulb: Frank was never in the list before — he was held in reserve inside the static array.

---

### :seven: **Test 7 — Empty Check**
```c
list_empty(&head);   // returns false
```
:ballot_box_with_check: Quick boolean check before cleanup.

---

### :eight: **Test 8 — Safe Bulk Delete**
```c
list_for_each_entry_safe(pos, n, &head, list) {
    list_del_init(&pos->list);
}
```
:shield: Uses the `_safe` iterator so we can delete the **current** node without corrupting the loop cursor.  
:arrow_down: Empties the entire list in one pass.

---

### :nine: **Test 9 — Stack Push (Head Add)**
```c
list_add(&students[0].list, &head);  // Alice
list_add(&students[2].list, &head);  // Charlie
```
:package: Uses `list_add` (head insertion) which acts like a **LIFO stack**.  
:arrow_right: Result: `Charlie -> Alice` (Charlie pushed last, so she's on top).

---

### :keycap_ten: **Test 10 — Singular Check**
```c
list_is_singular(&head);   // returns true
```
:question: Verifies the list contains **exactly one** node after removing Charlie.  
Useful for edge-case handling in drivers.

---

### :one::one: **Test 11 — `container_of` Sanity Check**
```c
struct student *s = container_of(node, struct student, list);
```
:alembic: The magic macro! Given only a pointer to the `list` member, it computes the address of the **parent** `struct student`.  
This is the entire reason kernel-style embedded lists are so elegant.

---

### :one::two: **Test 12 — Rotate Left**
```c
list_rotate_left(&head);
```
:arrows_counterclockwise: Moves the first element to the tail, shifting everyone forward by one.  
:arrow_right: Before: `Alice -> Charlie -> David`  
:arrow_right: After:  `Charlie -> David -> Alice`

---

## :gear: Build & Run

```bash
# :hammer: Compile
make

# :running_man: Run tests
make run

# :broom: Clean build artifacts
make clean
```

> :bulb: All object files and the binary are tucked away inside the `build/` directory — your project root stays clean.

---

## :robot: Embedded / MCU Friendly Features

| Concern | How we handle it |
|---------|------------------|
| :no_entry: No `malloc` | Static array only |
| :no_entry: No `free` | `list_del_init` puts nodes back in a re-usable state |
| :no_entry: No libc bloat | Only `<stddef.h>` and `<stdbool.h>` required |
| :zap: Deterministic memory | Every node lives at a fixed address in `.bss` or `.data` |
| :gear: Compile-time optimisations | All list operations are `static inline` — zero call overhead |

---

## :scroll: License

**GPL-2.0**  
This is a derivative of the Linux kernel source code and inherits its license.  
See `list.h` and `main.c` SPDX headers for full details.

---

> :star: If this saves you from writing yet another buggy hand-rolled linked list, consider giving it a star!
