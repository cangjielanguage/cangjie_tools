// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*


/*class ExportKeyword DeclareKeyword MessagePackage {
    getDataAsync(p: typeof MessagePackage): Promise<typeof MessagePackage>
    }*/

public class MessagePackage {
    
    protected MessagePackage(var arkts_object: JSObject) { }
    
    
    /**
	 * @brief getDataAsync(p: typeof MessagePackage): Promise<typeof MessagePackage>
	 */
    public static func getDataAsync(callback: AsyncCallback < JSValue/* FIXME: `Promise<UnsupportedType[TypeQuery]>` */ >, p: JSValue/* FIXME: `typeof MessagePackage` */): Unit {
        jsObjApiCallPromise < JSValue/* FIXME: `Promise<UnsupportedType[TypeQuery]>` */ >(getClassConstructorObj("my_module_typeofType", "MessagePackage"), "getDataAsync", callback, { ctx =>[p] }) {
            ctx, info => info
        }
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
     static func fromJSValue(context: JSContext, input: JSValue): MessagePackage {
        MessagePackage(input.asObject())
    }
}

