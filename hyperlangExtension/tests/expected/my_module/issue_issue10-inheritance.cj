// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/***********OBJECT***********/

/*interface I {
    p: number;
    p1: number;
    }*/

public open class I {
    
    protected I(public var p: Float64,
    public var p1: Float64) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["p"] = p.toJSValue(context)
        obj["p1"] = p1.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): I {
        let obj = input.asObject()
        I(
        Float64.fromJSValue(context, obj["p"]),
        Float64.fromJSValue(context, obj["p1"])
        )
    }
    
}

/*class DeclareKeyword C {
    p: number;
    p1: number;
    }*/

public class C <: I {
    
    protected C(var arkts_object: JSObject) {
        super(Float64.fromJSValue(getMainContext(), arkts_object["p"]),
        Float64.fromJSValue(getMainContext(), arkts_object["p1"])) }
    
    
    
    public func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): C {
        C(input.asObject())
    }
}

