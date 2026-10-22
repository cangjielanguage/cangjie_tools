// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// 本文件覆盖：值注入路径（已修复的两个高危注入点）。
//
// 注入点 1（VarString，lib.cj "// String value:" 行注释）：
//   顶层 const 的字符串指针变量（char * const）经 clang 求值后写入单行注释，
//   载荷中的换行符与注释定界符会逃逸行注释。
//   注意：const char *（底层 const）顶层非 const，走注入点 2 路径。
//
// 注入点 2（VarRawString，lib.cj 块注释包裹）：
//   非 const 全局变量的原始 token 文本（含初始化器原文，即 C 字符串
//   字面量的 spelling）被包进块注释，载荷中的字面闭合符会提前终止块注释。
//
// 说明：C 块注释内不能出现字面闭合定界符，故描述性注释一律使用 //，
// 载荷只出现在字符串字面量与 // 行注释中。

// V1: 非 const char* 带初始化，字符串含 */ 与 /*，
//     原始 token 文本进入 VarRawString 块注释（注入点 2 主 PoC）。
char *V1EvilStr = "*/ INJECTED_CODE /*";

// V2: 非 const int 带初始化（无载荷基线，验证 VarRawString 正常包裹）。
int V2Counter = 42;

/**
 * V3: 非 const 变量前的 Doxygen 注释（载荷 /*）与 VarRawString 块注释共存，
 *     验证 doc 行注释与值块注释同时生成时均不逃逸。
 */
double V3Ratio = 1.5;

// V4: 非 const 指针带地址初始化，raw token 含取地址运算符与变量名。
int V4Base = 0;
int *V4Ptr = &V4Base;

// V5: 顶层 const 指针（char * const）字符串含 */ 与 /*，
//     clang 求值字符串进入 "// String value:" 行注释（注入点 1 主 PoC）。
char * const V5ConstStr = "*/ INJECTED_CODE /*";

// V6: 顶层 const 指针含 \n 转义，求值后字符串内含真实换行（字节 0x0A），
//     换行会终止 Cangjie 行注释（注入点 1 换行逃逸 PoC）。
char * const V6MultilineStr = "line1\nline2 */ tail";

// V7: 顶层 const 指针同时含换行、*/ 与 /* 组合载荷。
char * const V7ComboStr = "a\nb */ c /* d";

// V8: static 顶层 const 指针，验证存储类说明符不影响注入点 1 路径。
static char * const V8StaticStr = "static */ payload /*";

// V9: 底层 const 指针（const char *）字符串含载荷，
//     顶层非 const → 走注入点 2（VarRawString），锁定两条路径的分界。
const char *V9UnderlyingConst = "under */ lying /*";

// V10: 字符串宏含 */ 载荷。宏经 fallback TU 求值，
//      当前实现对字符串宏求值失败而被整体丢弃（行为锁定：无注入面）。
#define V10_MACRO_STR "macro */ payload /*"

// V11: 整数宏基线（无载荷），验证宏求值机制本身工作正常。
#define V11_MACRO_INT 11

// V12: 非 const 数组变量带初始化列表，整个声明 raw token 进入块注释。
int V12Arr[3] = {1, 2, 3};

// V13: 非 const char 数组用字符串字面量初始化，raw token 含字符串载荷。
char V13Buf[8] = "ab*/cd";

// V14: 顶层 const 指针无载荷基线，验证正常字符串值注释输出。
char * const V14PlainStr = "plain value";

// V15: const char 数组（const char[]）字符串初始化，
//      当前实现求值失败被整体丢弃（行为锁定：无注入面）。
const char V15Arr[] = "arr */ x";
