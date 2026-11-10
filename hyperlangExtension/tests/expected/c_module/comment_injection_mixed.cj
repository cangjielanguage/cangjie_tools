package my_module

public const X16_MACRO: Int64 = 16

public const X17_MACRO: Int64 = 17

// X1: 普通单行注释（含 */ 载荷），不绑定 → 输出中应被丢弃。
// X1 第二行普通注释，含 /* 载荷。
@C
public struct X1Node {
    // X2: struct 字段前的普通注释（含 */ 载荷），不绑定 → 丢弃。
    public var value: Int32
    // X3: 字段后的普通块注释（含 /* 载荷），不绑定 → 丢弃。
    // X3 normal block comment with /* opener
    public var next: Int32
    public init(value: Int32, next: Int32) {
        this.value = value
        this.next = next
    }
}

// X4: struct 前的普通块注释（含 /* 载荷），不绑定 → 丢弃。
public const X4Global: Int32 = 0

// X5: 普通注释与 Doxygen 注释相邻，仅 Doxygen 绑定到下一声明。
// X5 这条普通注释（含 */ 载荷）应被丢弃。
// /**
// * X5: 紧贴声明的 Doxygen 注释（载荷 /*）应绑定并输出。
// */
public const X5Bound: Int32 = 1

// X6: 函数前的普通注释（含 */ 载荷），不绑定 → 丢弃。
foreign func X6Func(): Unit

// X7: 函数前的 /// Doxygen（载荷 */），绑定 → 输出。
foreign func X7Func(): Unit

// X9: enum 成员后的尾随 Doxygen 块注释（载荷 /*），绑定到 X8_A。
public const X8Mode_X8_A: X8Mode = 0

// X10: enum 成员后的尾随 /// 注释（含 */ 载荷），绑定到 X8_B。
public const X8Mode_X8_B: X8Mode = 1

public const X8Mode_X8_C: X8Mode = 2

// X8: enum 前普通注释，不绑定 → 丢弃。
public type X8Mode = UInt32

// X11: struct 字段后的尾随 Doxygen 注释。
@C
public struct X11Pair {
    // X11: 字段 a 后的尾随 Doxygen（载荷 /*）。
    public var a: Int32
    // X12: 字段 b 后的尾随 ///（含 */ 载荷）。
    public var b: Int32
    public init(a: Int32, b: Int32) {
        this.a = a
        this.b = b
    }
}

// X13: typedef 前普通注释（含 */），不绑定 → 丢弃。
public type X13Int = Int32

// X14: typedef 前的 Doxygen 块注释（载荷 /*），绑定 → 输出。
public type X14Real = Float64

// X19: 普通块注释紧贴 Doxygen 块注释，中间无空行。
// /* X19: 普通块注释（载荷 /*），不绑定 → 丢弃。 */
// /**
// * X19: 紧随其后的 Doxygen（载荷 /*），绑定下一声明。
// */
public const X19Bound: Int32 = 19

// X20 trailing normal comment with */ payload
public const X20Var: Int32 = 20

// X21: 成员 A 的 /// 注释（载荷 */）。
public const X21Cascade_X21_A: X21Cascade = 0

public const X21Cascade_X21_B: X21Cascade = 1

// X21: enum 成员注释错挂场景：X21_A 带 /// 注释，X21_B 无注释。
public type X21Cascade = UInt32

// X22: struct 字段注释错挂场景：字段 a 带 /// 注释，字段 b 无注释。
@C
public struct X22Fields {
    // X22: 字段 a 的 /// 注释（载荷 */）。
    public var a: Int32
    public var b: Int32
    public init(a: Int32, b: Int32) {
        this.a = a
        this.b = b
    }
}

// X24: Doxygen 与声明之间插入普通注释，实证绑定关系未被切断，Doxygen 仍绑定。
// /**
// * X24: 与声明之间隔了普通注释的 Doxygen，仍绑定下一声明（实证行为）。
// */
// X24: 中间插入的普通注释（含 */ 载荷），被丢弃。
public const X24Cut: Int32 = 24

// X25: Doxygen 注释 + 非 const 变量（doc 行注释与 VarRawString 块注释组合）。
// /**
// * X25: 非 const 变量的 Doxygen（载荷 /*），与值块注释共存。
// */
// String value: x25 */ payload /*
public const X25Str: VArray<UInt8, $18> = [120, 50, 53, 32, 42, 47, 32, 112, 97, 121, 108, 111, 97, 100, 32, 47, 42, 0]
