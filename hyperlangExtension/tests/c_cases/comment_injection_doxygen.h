// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// 本文件覆盖：Doxygen 风格注释绑定到 C 头文件中各类声明位置时的注释注入场景。
//
// clang RawComment 语义（实证结论）：
//   - 仅 Doxygen 风格注释会绑定到紧随其后的声明；
//   - 块注释内部不可能出现字面闭合定界符（C 词法保证），故块注释载体载荷为 /* 与 *\/；
//   - 行注释（///）内部可以出现 */ 和 /*，是行注释载体的主要载荷来源。

/**
 * D1: struct 前的 Doxygen 块注释，载荷 /* 会开启 Cangjie 嵌套块注释。
 */
struct D1Point {
    /// D2: struct 字段前的 /// 行注释，载荷 */ 会提前闭合块注释。
    int x;
    /**
     * D3: struct 字段前的多行 Doxygen 块注释，载荷 /* 与转义形式 *\/。
     */
    int y;
    /**
     * D4: 匿名结构体成员前的 Doxygen 块注释，载荷 /*。
     */
    struct {
        /// D5: 匿名结构体内部字段前的 /// 注释，载荷 */。
        int inner;
    };
};

/**
 * D6: 嵌套 struct 定义前的 Doxygen 块注释（作为字段类型使用），载荷 /*。
 */
struct D6Inner {
    int a;
};

/**
 * D7: 包含嵌套 struct 字段的外层 struct，载荷 /*。
 */
struct D7Outer {
    /// D8: 嵌套 struct 字段前的 /// 注释，载荷 */。
    struct D6Inner inner;
};

/**
 * D9: 多行 Doxygen 块注释在 function 前。
 * 第二行载荷 /*。
 * 第三行载荷 *\/（转义形态）。
 * 第四行为空白行，验证空行裁剪。
 *
 * @param a 参数 a 说明，含载荷 /*
 * @return 返回值说明，含载荷 *\/
 */
int d9Add(int a, int b);

/// D10: function 前的单行 /// 注释，载荷 */。
/// D10 第二行 /// 注释，载荷 /*。多行 /// 会合并为一条 RawComment。
void d10Run(void);

/// D11: 声明（非定义）函数前的 /// 注释，载荷 */。
int d11Decl(int x);

/**
 * D12: typedef 前的 Doxygen 块注释，载荷 /*。
 */
typedef struct D1Point D1PointAlias;

/// D13: typedef 基本类型前的 /// 注释，载荷 */。
typedef unsigned long D13Size;

/**
 * D14: enum 前的 Doxygen 块注释，载荷 /*。
 */
enum D14Color {
    /// D15: enum 成员前的 /// 行注释，载荷 */。
    D15_RED,
    /**
     * D16: enum 成员前的 Doxygen 块注释，载荷 /*。
     */
    D16_GREEN = 5,
    /// D17: 有显式值的 enum 成员前的 /// 注释，载荷 */。
    D17_BLUE = 10,
    D18_PLAIN
};

/**
 * D19: const 全局变量前的 Doxygen 块注释，载荷 /*。
 */
const int D19_MAX = 100;

/// D20: const 变量前的 /// 注释，载荷 */。
const double D20_PI = 3.14;

/**
 * D21: 非 const 全局变量前的 Doxygen 块注释，载荷 /*。
 *      非 const 变量走 VarRawString 路径，声明原文进入块注释。
 */
int D21Counter = 0;

/// D22: 函数指针 typedef 前的 /// 注释，载荷 */。
typedef int (*D22Callback)(int a, int b);

/**
 * D23: 返回 struct 的函数前的 Doxygen 块注释，载荷 /*。
 */
struct D1Point d23MakePoint(int x, int y);

/// D24: void 返回函数前的 /// 注释，载荷 */。
void d24Noop(void);

/**
 * D25: 多参数函数前的 Doxygen 块注释，载荷 /*，
 *      验证注释与 @brief 签名注解共存。
 */
int d25Sum(int a, int b, int c);
