# 🔗 Linux Kernel Linked List — Userspace Port

> 🚀 零依赖、零动态内存、header-only 的 Linux 内核链表用户空间移植版

---

## 📦 这是什么？

本项目将 Linux 内核中经典的 `list.h` 链表实现提取出来，改造成**完全自包含**的头文件库：

- 🚫 不需要任何内核头文件
- 🚫 不需要 `malloc` / `free`
- ✅ 只需要标准库的 `<stddef.h>` 和 `<stdbool.h>`
- ✅ 全部操作都是 `static inline`，零函数调用开销
- 🎯 专为嵌入式 / MCU 场景设计

| 功能 | 状态 |
|------|------|
| 🔗 双向循环链表 (`struct list_head`) | ✅ 就绪 |
| 🔗 哈希链表 (`struct hlist_head` / `hlist_node`) | ✅ 就绪 |
| 🧲 `container_of` 宏 | ✅ 就绪 |
| 🛡️ 安全遍历 (`list_for_each_entry_safe`) | ✅ 就绪 |
| 🚫 零动态内存分配 | ✅ 保证 |

---

## 📁 文件结构

```text
.
├── 📄 list.h      ⭐ 主角 — 纯头文件链表实现
├── 📄 main.c      🧪 完整测试用例（无 malloc！）
├── 📄 Makefile    ⚙️  简单构建脚本
└── 📄 README.md   📖 你在看的这个
```

---

## 🔬 main.c 深度解析

`main.c` 不是玩具示例，而是一份**覆盖所有核心链表操作**的实战级测试代码，完全采用嵌入式固件风格编写。

### 🏗️ 数据结构

```c
struct student {
    int  id;            🔢 学号
    char name[16];      🏷️ 姓名
    int  score;         💯 分数
    struct list_head list;  🔗 嵌入结构体内部的链表节点
};
```

> 💡 **核心设计**：链表节点**嵌入**在业务结构体内部，而不是额外分配。这是 Linux 内核的标志性设计，配合 `container_of` 宏可以从节点指针反推业务结构体地址，彻底消灭指针二次分配。

---

### 🧱 内存模型 — 纯静态数组

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

> ⚡ **零堆内存使用**！所有节点都住在静态数组里。对 MCU 开发者来说，`malloc` 是敌人，静态分配才是朋友。

---

## 🧪 12 项测试逐条讲解

### 1️⃣ 尾部追加（队列行为）
```c
for (i = 0; i < 5; i++) {
    INIT_LIST_HEAD(&students[i].list);
    list_add_tail(&students[i].list, &head);
}
```
📌 使用 `list_add_tail` 按顺序追加 5 个学生。  
✨ 结果：`Alice → Bob → Charlie → David → Eve`

---

### 2️⃣ 按 ID 查找（查询）
```c
found = find_by_id(&head, 3);
```
🔍 通过 `list_for_each_entry` 遍历链表，返回 `id == 3` 的节点。  
🎯 重点展示 `container_of` 的威力：遍历得到的是 `struct list_head *`，宏自动换算成 `struct student *`。

---

### 3️⃣ 原地修改（改）
```c
found = find_by_id(&head, 2);
if (found) found->score = 100;
```
✏️ 直接修改查到的结构体字段。  
🔥 Bob 的分数从 90 提升到 100。

---

### 4️⃣ 删除节点（删）
```c
list_del_init(&found->list);
```
✂️ 将 Charlie（`ID=3`）从链表中摘除，并重新初始化他的节点。  
🛡️ `list_del_init` 让被删节点回到干净状态，可再次被插入。

---

### 5️⃣ 移动到头部
```c
list_move(&found->list, &head);
```
🔄 把 David（`ID=4`）从当前位置拔出，插到链表最前面。  
📍 结果：`David → Alice → Bob → Eve`

---

### 6️⃣ 替换节点
```c
list_replace(&found->list, &students[5].list);
```
♻️ 用 Frank（`ID=6`）原地替换 Alice（`ID=1`），不动前后邻居的连接。  
🎭 Frank 之前一直躺在数组里当替补，现在直接上位！

---

### 7️⃣ 空表检查
```c
list_empty(&head);   // false
```
☑️ 一键判断链表是否为空，返回布尔值。

---

### 8️⃣ 安全批量删除
```c
list_for_each_entry_safe(pos, n, &head, list) {
    list_del_init(&pos->list);
}
```
🛡️ 使用 `_safe` 版本迭代器，可以在遍历过程中**安全删除当前节点**。  
🧹 一次性清空整个链表，循环指针不会悬空。

---

### 9️⃣ 头部插入（栈行为）
```c
list_add(&students[0].list, &head);  // Alice
list_add(&students[2].list, &head);  // Charlie
```
📚 `list_add` 是头插法， behaves like **LIFO 栈**。  
📍 结果：`Charlie → Alice`（Charlie 后入，所以在栈顶）

---

### 🔟 单节点检查
```c
list_is_singular(&head);   // true
```
❓ 验证链表中是否**恰好只剩一个节点**。  
🎯 常用于驱动代码中的边界条件处理。

---

### 1️⃣1️⃣ container_of 校验
```c
struct student *s = container_of(node, struct student, list);
```
🧲 **灵魂宏！** 只给一个指向 `list` 成员的指针，就能算出父结构体 `struct student` 的地址。  
🔮 这是内核嵌入式链表最优雅的地方，也是为什么不需要额外分配内存。

---

### 1️⃣2️⃣ 左旋
```c
list_rotate_left(&head);
```
🔄 把第一个节点搬到尾部，其余节点依次前移一位。  
📍 前：`Alice → Charlie → David`  
📍 后：`Charlie → David → Alice`

---

## ⚙️ 编译与运行

```bash
# 🔨 编译
make

# 🏃 运行测试
make run

# 🧹 清理构建产物
make clean
```

> 💡 所有 `.o` 文件和可执行文件都输出到 `build/` 目录，项目根目录保持整洁。

---

## 🎯 嵌入式 / MCU 友好特性

| 痛点 | 本方案 |
|------|--------|
| 🚫 不想用 `malloc` | 纯静态数组，节点地址编译期确定 |
| 🚫 不想用 `free` | `list_del_init` 让节点回到可复用状态 |
| 🚫 怕 libc 膨胀 | 仅需 `<stddef.h>` + `<stdbool.h>` |
| ⚡ 要求确定性时序 | 全内联函数，无函数调用开销 |
| 🧠 怕指针搞崩 | 数组越界由编译器保证，链表操作只改 next/prev |

---

## 📜 许可证

**GPL-2.0** 🔒  
本项目是 Linux 内核源码的衍生作品，继承其许可证。  
详见 `list.h` 与 `main.c` 头部的 SPDX 声明。

---

> ⭐ 如果这个项目帮你少写了一个手搓链表、少踩了一个内存泄漏的坑，赏颗星星吧！
