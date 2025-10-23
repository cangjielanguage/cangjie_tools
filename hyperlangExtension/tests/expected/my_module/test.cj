// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
import std.collection.{ HashMap }
/***********IMPORT***********/
/*FIXME: Import details need to be verified and rewritten by user.*/
/*ohos.base*/


/*class ExportKeyword DeclareKeyword PushReporter {
    getTags(t12: Callback<Record<string, any>>): void
    }*/

public class PushReporter {
    
    protected PushReporter(var arkts_object: JSObject) {}
    
    
    /**
	 * @brief getTags(t12: Callback<Record<string, any>>): void
	 */
    public static func getTags(t12: Callback<HashMap<String, Any>>): Unit {
        jsObjApiCall < Unit >(getClassConstructorObj("my_module_test", "PushReporter"),  "getTags", { ctx =>[t12.toJSValue(ctx)] })
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): PushReporter {
        PushReporter(input.asObject())
    }
}

/*class ExportKeyword DeclareKeyword MessagePackage {
    StaticKeywordidt: idt_struct;
    }*/

public class MessagePackage {
    
    protected MessagePackage(var arkts_object: JSObject) {}
    
    
    public static mut prop idt: JSValue/* FIXME: `idt_struct` */ {
        get() {
            checkThreadAndCall < JSValue/* FIXME: `idt_struct` */ >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_test", "MessagePackage")["idt"]
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_test", "MessagePackage")["idt"] = v
            }
        }
        
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): MessagePackage {
        MessagePackage(input.asObject())
    }
}

