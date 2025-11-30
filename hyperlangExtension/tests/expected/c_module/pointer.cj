// cjbind 0.2.7 生成，请勿编辑, DO NOT EDIT

package my_module

public const num: Int32 = 42

foreign func malloc(a: Int32): CPointer<Unit>

public type callback_func = CFunc<(arg1: Int32, arg2: Int32) -> Int32>
