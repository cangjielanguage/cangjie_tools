package my_module

// String value: macro */ payload /*
public const V10_MACRO_STR: VArray<UInt8, $20> = [109, 97, 99, 114, 111, 32, 42, 47, 32, 112, 97, 121, 108, 111, 97,
    100, 32, 47, 42, 0]

public const V11_MACRO_INT: Int64 = 11

// V1: 非 const char* 带初始化，字符串含 */ 与 /*，
//     原始 token 文本进入 VarRawString 块注释（注入点 2 主 PoC）。
// String value: */ INJECTED_CODE /*
public const V1EvilStr: VArray<UInt8, $20> = [42, 47, 32, 73, 78, 74, 69, 67, 84, 69, 68, 95, 67, 79, 68, 69, 32, 47,
    42, 0]

// V2: 非 const int 带初始化（无载荷基线，验证 VarRawString 正常包裹）。
public const V2Counter: Int32 = 42

// V3: 非 const 变量前的 Doxygen 注释（载荷 /*）与 VarRawString 块注释共存，
//     验证 doc 行注释与值块注释同时生成时均不逃逸。
public const V3Ratio: Float64 = 1.500000

// V4: 非 const 指针带地址初始化，raw token 含取地址运算符与变量名。
public const V4Base: Int32 = 0

// V5: 顶层 const 指针（char * const）字符串含 */ 与 /*，
//     clang 求值字符串进入 "// String value:" 行注释（注入点 1 主 PoC）。
// String value: */ INJECTED_CODE /*
public const V5ConstStr: VArray<UInt8, $20> = [42, 47, 32, 73, 78, 74, 69, 67, 84, 69, 68, 95, 67, 79, 68, 69, 32, 47,
    42, 0]

// V6: 顶层 const 指针含 \n 转义，求值后字符串内含真实换行（字节 0x0A），
//     换行会终止 Cangjie 行注释（注入点 1 换行逃逸 PoC）。
// String value: line1\nline2 */ tail
public const V6MultilineStr: VArray<UInt8, $20> = [108, 105, 110, 101, 49, 10, 108, 105, 110, 101, 50, 32, 42, 47, 32,
    116, 97, 105, 108, 0]

// V7: 顶层 const 指针同时含换行、*/ 与 /* 组合载荷。
// String value: a\nb */ c /* d
public const V7ComboStr: VArray<UInt8, $14> = [97, 10, 98, 32, 42, 47, 32, 99, 32, 47, 42, 32, 100, 0]

// V8: static 顶层 const 指针，验证存储类说明符不影响注入点 1 路径。
// String value: static */ payload /*
public const V8StaticStr: VArray<UInt8, $21> = [115, 116, 97, 116, 105, 99, 32, 42, 47, 32, 112, 97, 121, 108, 111, 97,
    100, 32, 47, 42, 0]

// V9: 底层 const 指针（const char *）字符串含载荷，
//     顶层非 const → 走注入点 2（VarRawString），锁定两条路径的分界。
// String value: under */ lying /*
public const V9UnderlyingConst: VArray<UInt8, $18> = [117, 110, 100, 101, 114, 32, 42, 47, 32, 108, 121, 105, 110, 103,
    32, 47, 42, 0]

// V13: 非 const char 数组用字符串字面量初始化，raw token 含字符串载荷。
// String value: ab*/cd
public const V13Buf: VArray<UInt8, $7> = [97, 98, 42, 47, 99, 100, 0]

// V14: 顶层 const 指针无载荷基线，验证正常字符串值注释输出。
// String value: plain value
public const V14PlainStr: VArray<UInt8, $12> = [112, 108, 97, 105, 110, 32, 118, 97, 108, 117, 101, 0]

// V15: const char 数组（const char[]）字符串初始化，
//      当前实现求值失败被整体丢弃（行为锁定：无注入面）。
// String value: arr */ x
public const V15Arr: VArray<UInt8, $9> = [97, 114, 114, 32, 42, 47, 32, 120, 0]
