// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

//简单元组类型
public type BasicTuple = (Float64, String, Bool)

//元组类型的可选类型
public type OptionalElementTuple = (Float64, Option<String>, Option<Bool>)

//异构元组类型
public type HeterogeneousTuple = (String, Float64, Bool, JSValue/* FIXME: `Date` */)

//元组的剩余元素
public type RestTuple = (String, JSValue/* FIXME: `UnsupportedType[RestType]` */)

//包含固定长度的元组
public type FixedLengthTuple = (Float64, String, Bool)

//元组与数组的结合
public type TupleArray = Array<(Float64, String)>

//多维元组
public type TwoDimensionalTuple = ((Float64, String), (Bool, JSValue/* FIXME: `Date` */))

//元组与联合类型结合
public type UnionTuple = (JSValue/* FIXME: `string | number` */, Bool, JSValue/* FIXME: `Date` */)

//包含中文字符串的union类型
public enum another {
    | A
    | 中文
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case A => context.string("A").toJSValue()
            case 中文 => context.string("中文").toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): another {
        
        throw Exception("unsupport type!")
    }
}

//包含中文字符和非法标识符串的union类型
public enum another2 {
    | A
    | 中文_
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case A => context.string("A").toJSValue()
            case 中文_ => context.string("中文&").toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): another2 {
        
        throw Exception("unsupport type!")
    }
}

//包含非法标识符字符串的union类型
public enum another3 {
    | DO
    | WHILE
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case DO => context.string("do").toJSValue()
            case WHILE => context.string("while").toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): another3 {
        
        throw Exception("unsupport type!")
    }
}

//包含非法标识符字符串的union类型
public enum NetworkType {
    | WIFI
    | LITERAL_2G
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case WIFI => context.string("WIFI").toJSValue()
            case LITERAL_2G => context.string("2G").toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): NetworkType {
        
        throw Exception("unsupport type!")
    }
}


