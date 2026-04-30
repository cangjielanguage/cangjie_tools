// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
import std.collection.{ HashMap }

public enum AutoGenType0Inissue_issue2 {
    | STRING(String)
    | NUMBER(Float64)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case STRING(x) => context.string(x).toJSValue()
            case NUMBER(x) => context.number(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0Inissue_issue2 {
        
        if (input.isString()) {
            return STRING(String.fromJSValue(context, input))
        } 
        if (input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        } 
        throw Exception("unsupport type!")
    }
}


public type TB700 = Option<HashMap<String, AutoGenType0Inissue_issue2>>


