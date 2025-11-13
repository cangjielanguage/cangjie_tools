// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*


/*interface AutoGenType0 {
    }*/

public open class AutoGenType0 {
    
    protected AutoGenType0() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0 {
        let obj = input.asObject()
        AutoGenType0(
        
        )
    }
    
}

/*class ExportKeyword DeclareKeyword UserInfoManager {
    userPreferences?: { [key: String]: String | number | boolean };
    }*/

public class UserInfoManager {
    
    protected UserInfoManager(var arkts_object: JSObject) { }
    
    
    public mut prop userPreferences: ?AutoGenType0 {
        get() {
            checkThreadAndCall < ?AutoGenType0 >(getMainContext()) {
                ctx: JSContext => Option < AutoGenType0 >.fromJSValue(ctx, arkts_object["userPreferences"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["userPreferences"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
     static func fromJSValue(context: JSContext, input: JSValue): UserInfoManager {
        UserInfoManager(input.asObject())
    }
}

