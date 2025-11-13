// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/*

public const aesEncText = 0/* FIXME: Initialization is required */
*/

public const c1 = 1.2

public const c2 = 1

public const c3 = 0x2

public const c4 = "str"

public const c5 = "123"
/*

public const c6 = 0/* FIXME: Initialization is required */
*/
/*

public const c7 = 0/* FIXME: Initialization is required */
*/

public const c8 = "1"

public const c9 = c1

public const j = "0.3.6"

public const m = "release"

public const DEBUG = false

public const o = "default"


/*class ExportKeyword DefaultKeyword BuildProfile {
    StaticKeywordReadonlyKeywordj: string = "0.3.6";
    StaticKeywordReadonlyKeywordm: string = "release";
    StaticKeywordReadonlyKeywordDEBUG: boolean = false;
    StaticKeywordReadonlyKeywordo: string = "default";
    }*/

public class BuildProfile {
    
    protected BuildProfile(var arkts_object: JSObject) { }
    
    
    public static prop j: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, getClassConstructorObj("my_module_variable", "BuildProfile")["j"])
            }
        }
    }
    
    public static prop m: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, getClassConstructorObj("my_module_variable", "BuildProfile")["m"])
            }
        }
    }
    
    public static prop DEBUG: Bool {
        get() {
            checkThreadAndCall < Bool >(getMainContext()) {
                ctx: JSContext => Bool.fromJSValue(ctx, getClassConstructorObj("my_module_variable", "BuildProfile")["DEBUG"])
            }
        }
    }
    
    public static prop o: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, getClassConstructorObj("my_module_variable", "BuildProfile")["o"])
            }
        }
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
     static func fromJSValue(context: JSContext, input: JSValue): BuildProfile {
        BuildProfile(input.asObject())
    }
}

