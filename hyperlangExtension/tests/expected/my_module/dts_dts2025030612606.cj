// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
/***********OBJECT***********/

/*interface Item {
    id: number;
    name: String;
    category: T;
    }*/

public open class Item<T> where T <: JSInteropType<T> {
    
    protected Item(public var id: Float64,
    public var name: String,
    public var category: T) {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["id"] = id.toJSValue(context)
        obj["name"] = name.toJSValue(context)
        obj["category"] = category.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): Item<T> {
        let obj = input.asObject()
        Item(
        Float64.fromJSValue(context, obj["id"]),
        String.fromJSValue(context, obj["name"]),
        T.fromJSValue(context, obj["category"])
        )
    }
    
}

/*interface Item1 {
    id: number;
    name: U;
    category: T;
    }*/

public open class Item1<T, U> where T <: JSInteropType<T>, U <: JSInteropType<U> {
    
    protected Item1(public var id: Float64,
    public var name: U,
    public var category: T) {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["id"] = id.toJSValue(context)
        obj["name"] = name.toJSValue(context)
        obj["category"] = category.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): Item1<T, U> {
        let obj = input.asObject()
        Item1(
        Float64.fromJSValue(context, obj["id"]),
        U.fromJSValue(context, obj["name"]),
        T.fromJSValue(context, obj["category"])
        )
    }
    
}

