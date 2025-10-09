// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/*class ExportKeyword UnionTypes {
    t_null: number | null;
    null_t: null | String;
    t1_t2: boolean | String;
    t_undefined: number | undefined;
    undefined: undefined | String;
    }*/

public class UnionTypes {
    
    protected UnionTypes(var arkts_object: JSObject) {}
    
    
    public mut prop t_null: Option<Float64> {
        get() {
            checkThreadAndCall < Option<Float64> >(getMainContext()) {
                ctx: JSContext => Option < Float64 >.fromJSValue(ctx, arkts_object["t_null"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["t_null"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop null_t: Option<String> {
        get() {
            checkThreadAndCall < Option<String> >(getMainContext()) {
                ctx: JSContext => Option < String >.fromJSValue(ctx, arkts_object["null_t"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["null_t"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop t1_t2: JSValue/* FIXME: `boolean | string` */ {
        get() {
            checkThreadAndCall < JSValue/* FIXME: `boolean | string` */ >(getMainContext()) {
                ctx: JSContext => arkts_object["t1_t2"]
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["t1_t2"] = v
            }
        }
        
    }
    
    public mut prop t_undefined: Option<Float64> {
        get() {
            checkThreadAndCall < Option<Float64> >(getMainContext()) {
                ctx: JSContext => Option < Float64 >.fromJSValue(ctx, arkts_object["t_undefined"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["t_undefined"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop undefined: Option<String> {
        get() {
            checkThreadAndCall < Option<String> >(getMainContext()) {
                ctx: JSContext => Option < String >.fromJSValue(ctx, arkts_object["undefined"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["undefined"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): UnionTypes {
        UnionTypes(input.asObject())
    }
}

