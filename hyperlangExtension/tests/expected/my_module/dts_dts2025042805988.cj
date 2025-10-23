// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/*class ExportKeyword DeclareKeyword UserInfoManager {
    userPreferences?: { [key: String]: String | number | boolean };
    }*/

public class UserInfoManager {
    
    protected UserInfoManager(var arkts_object: JSObject) {}
    
    
    public mut prop userPreferences: ?JSValue/* FIXME: `{ [string]: string | number | boolean }` */ {
        get() {
            checkThreadAndCall < ?JSValue/* FIXME: `{ [string]: string | number | boolean }` */ >(getMainContext()) {
                ctx: JSContext => arkts_object["userPreferences"]
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["userPreferences"] = v.getOrDefault({ => ctx.object().toJSValue() })
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

