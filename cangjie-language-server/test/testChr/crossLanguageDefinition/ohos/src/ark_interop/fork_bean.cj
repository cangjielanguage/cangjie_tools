// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos.ark_interop

public type JSLambda = (JSContext, JSCallInfo) -> JSValue

abstract sealed class JSObjectBase <: JSHeapObject {
    public operator func [](key: JSKeyable): JSValue {
        JSValue()
    }
    public operator func [](key: JSKeyable, value!: JSValue): Unit {}
}

public class JSObject <: JSObjectBase {}

public class JSString <: JSHeapObject & ToString & JSKeyable {
    public func toJSValue(_: JSContext): JSValue {
        JSValue()
    }

    public func toString(): String {
        ""
    }

    public func string(value: String): JSString {
        JSString()
    }
}

public class JSContext {
    public func function(lambda: JSLambda): JSFunction {
        JSFunction()
    }

    public func clazz(ctor: JSLambda, superClass!: ?JSClass = None): JSClass {
        JSClass()
    }

    public func undefined(): JSUndefined {
        JSUndefined()
    }

    public func string(value: String): JSString {
        JSString()
    }
}

public struct JSCallInfo {
    public operator func [](index: Int64): JSValue {
        JSValue()
    }

    public prop thisArg: JSValue {
        get() { JSValue() }
    }
}

public struct JSValue {
    public func toString(): String {
        ""
    }

    public func toString(_: JSContext): String {
        ""
    }
}

abstract sealed class JSHeapObject {
    public func toJSValue(): JSValue {
        JSValue()
    }
}

public class JSFunction <: JSHeapObject {}

public interface ToJSValue {
    func toJSValue(context: JSContext): JSValue
}

sealed interface JSKeyable <: ToString & ToJSValue {
}

extend String <: JSKeyable {
    public func toJSValue(context: JSContext): JSValue {
        JSValue()
    }
}

public class JSClass <: JSHeapObject {
    public func addMethod(key: JSKeyable, method: JSFunction): Unit {}
}

public struct JSUndefined {
    public func toJSValue(): JSValue {
        JSValue()
    }
}
