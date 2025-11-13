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

/*interface i {
    }*/

public open class i <: e {
    
    protected i() {
        super() }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): i {
        let obj = input.asObject()
        i(
        
        )
    }
    
}

/*class ExportKeyword c {
    }*/

public class c <: e {
    
    protected c(public var arkts_object: JSObject) {
        super() }
    
    
    
    public func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public  static func fromJSValue(context: JSContext, input: JSValue): c {
        c(input.asObject())
    }
}

