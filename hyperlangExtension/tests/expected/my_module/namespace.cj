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

/*interface B {
    }*/

public open class B {
    
    protected B() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): B {
        let obj = input.asObject()
        B(
        
        )
    }
    
}

/***********OBJECT***********/

/*interface E {
    }*/

public open class E {
    
    protected E() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): E {
        let obj = input.asObject()
        E(
        
        )
    }
    
}

/***********OBJECT***********/

/*interface H {
    }*/

public open class H {
    
    protected H() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): H {
        let obj = input.asObject()
        H(
        
        )
    }
    
}

