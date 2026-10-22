// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// 本文件覆盖：普通注释（不绑定声明）、尾随 Doxygen 注释、注释组合场景。
//
// clang RawComment 语义（实证结论）：
//   - 普通 // 与 /* ... */ 不绑定任何声明，转换后被丢弃，本文件锁定该行为；
//   - 行注释内可出现 */ 与 /* 载荷（C 词法允许），块注释内只能出现 /* 载荷；
//   - 尾随 Doxygen（/**< ... */ 与 //!< ...）绑定到前一个声明。

// X1: 普通单行注释（含 */ 载荷），不绑定 → 输出中应被丢弃。
// X1 第二行普通注释，含 /* 载荷。
struct X1Node {
    // X2: struct 字段前的普通注释（含 */ 载荷），不绑定 → 丢弃。
    int value;
    // X3: 字段后的普通块注释（含 /* 载荷），不绑定 → 丢弃。
    /* X3 normal block comment with /* opener */
    int next;
};

/* X4: struct 前的普通块注释（含 /* 载荷），不绑定 → 丢弃。 */
int X4Global = 0;

// X5: 普通注释与 Doxygen 注释相邻，仅 Doxygen 绑定到下一声明。
// X5 这条普通注释（含 */ 载荷）应被丢弃。
/**
 * X5: 紧贴声明的 Doxygen 注释（载荷 /*）应绑定并输出。
 */
int X5Bound = 1;

// X6: 函数前的普通注释（含 */ 载荷），不绑定 → 丢弃。
void X6Func(void);

/// X7: 函数前的 /// Doxygen（载荷 */），绑定 → 输出。
void X7Func(void);

// X8: enum 前普通注释，不绑定 → 丢弃。
enum X8Mode {
    X8_A, /**< X9: enum 成员后的尾随 Doxygen 块注释（载荷 /*），绑定到 X8_A。 */
    X8_B, //!< X10: enum 成员后的尾随 /// 注释（含 */ 载荷），绑定到 X8_B。
    X8_C
};

// X11: struct 字段后的尾随 Doxygen 注释。
struct X11Pair {
    int a; /**< X11: 字段 a 后的尾随 Doxygen（载荷 /*）。 */
    int b; //!< X12: 字段 b 后的尾随 ///（含 */ 载荷）。
};

// X13: typedef 前普通注释（含 */），不绑定 → 丢弃。
typedef int X13Int;

/**
 * X14: typedef 前的 Doxygen 块注释（载荷 /*），绑定 → 输出。
 */
typedef double X14Real;

// X15: 声明之间的孤立注释（含 */ 与 /* 载荷），不绑定任何声明 → 丢弃。
// X15 第二行孤立注释。

// X16: 宏前的普通注释（含 */ 载荷），不绑定宏 → 丢弃。
// X16 宏走独立解析路径，宏前 Doxygen 也不绑定。
#define X16_MACRO 16

// X17: 宏前的 Doxygen 注释，同样不绑定宏 → 丢弃。
/**
 * X17: 宏前 Doxygen（载荷 /*），不绑定宏 → 丢弃。
 */
#define X17_MACRO 17

// X18: #include 前后的普通注释。
#include <stdint.h>
// X18: include 之后的普通注释，不绑定 → 丢弃。

// X19: 普通块注释紧贴 Doxygen 块注释，中间无空行。
/* X19: 普通块注释（载荷 /*），不绑定 → 丢弃。 */
/**
 * X19: 紧随其后的 Doxygen（载荷 /*），绑定下一声明。
 */
int X19Bound = 19;

// X20: 声明与注释同行（尾随普通注释，含 */ 载荷），不绑定 → 丢弃。
int X20Var = 20; // X20 trailing normal comment with */ payload

// X21: enum 成员注释错挂场景：X21_A 带 /// 注释，X21_B 无注释。
enum X21Cascade {
    /// X21: 成员 A 的 /// 注释（载荷 */）。
    X21_A,
    X21_B
};

// X22: struct 字段注释错挂场景：字段 a 带 /// 注释，字段 b 无注释。
struct X22Fields {
    /// X22: 字段 a 的 /// 注释（载荷 */）。
    int a;
    int b;
};

// X23: 多行普通块注释（含 /* 载荷），不绑定 → 丢弃。
/*
 * X23 line 1 with /* opener payload
 * X23 line 2
 */

// X24: Doxygen 与声明之间插入普通注释，实证绑定关系未被切断，Doxygen 仍绑定。
/**
 * X24: 与声明之间隔了普通注释的 Doxygen，仍绑定下一声明（实证行为）。
 */
// X24: 中间插入的普通注释（含 */ 载荷），被丢弃。
int X24Cut = 24;

// X25: Doxygen 注释 + 非 const 变量（doc 行注释与 VarRawString 块注释组合）。
/**
 * X25: 非 const 变量的 Doxygen（载荷 /*），与值块注释共存。
 */
char *X25Str = "x25 */ payload /*";
