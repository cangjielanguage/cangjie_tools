// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

public enum AutoGenType0 {
    | NUMBER(Float64)
    | STRING(String)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case NUMBER(x) => context.number(x).toJSValue()
            case STRING(x) => context.string(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0 {
        
        if(input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        }  
        if(input.isString()) {
            return STRING(String.fromJSValue(context, input))
        }
        throw Exception("unsupport type!")
    }
}

public enum AutoGenType1 {
    | BOOLEAN(Bool)
    | STRING(String)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case BOOLEAN(x) => context.boolean(x).toJSValue()
            case STRING(x) => context.string(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType1 {
        
        if (input.isBoolean()) {
            return BOOLEAN(Bool.fromJSValue(context, input))
        } 
        if(input.isString()) {
            return STRING(String.fromJSValue(context, input))
        }
        throw Exception("unsupport type!")
    }
}

public enum AutoGenType2 {
    | NUMBER(Float64)
    | _BOOLEAN_STRING___(JSValue/* FIXME: `(boolean | string)[]` */)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case NUMBER(x) => context.number(x).toJSValue()
            case _BOOLEAN_STRING___(x) => x
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType2 {
        
        if(input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        } 
        throw Exception("unsupport type!")
    }
}

public enum AutoGenType3 {
    | NUMBER(Float64)
    | BOOLEAN__(JSValue/* FIXME: `boolean[]` */)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case NUMBER(x) => context.number(x).toJSValue()
            case BOOLEAN__(x) => x
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType3 {
        
        if(input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        } 
        throw Exception("unsupport type!")
    }
}


/*interface AutoGenType4 {
    city: string;
    zipCode: string;
    }*/

public open class AutoGenType4 {
    
    protected AutoGenType4(public var city: String,
    public var zipCode: String) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["city"] = city.toJSValue(context)
        obj["zipCode"] = zipCode.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType4 {
        let obj = input.asObject()
        AutoGenType4(
        String.fromJSValue(context, obj["city"]),
        String.fromJSValue(context, obj["zipCode"])
        )
    }
    
}
/***********OBJECT***********/

/*interface UserProfile {
    array: (number | String)[];
    test: (num: number | String) => String;
    test1: number|(boolean | String)[];
    test2: number| boolean[];
    name: String;
    address: {
      city: String;
      zipCode: String;
    };
    id?: number|String;
    }*/

public open class UserProfile {
    
    protected UserProfile(private var array: Array<AutoGenType0>,
    private var test: (num: AutoGenType0) -> String,
    private var test1: AutoGenType2,
    private var test2: AutoGenType3,
    private var name: String,
    private var address: AutoGenType4,
    private var id!: Option<AutoGenType0> = None) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["array"] = toJSArray < AutoGenType0 >(context, array, { ctx: JSContext, val: AutoGenType0 => val.toJSValue(ctx) })
        obj["test"] = context.function({ ctx, info =>
            let p0 = AutoGenType0.fromJSValue(ctx, info[0])
            let ret = test(p0)
            ret.toJSValue(ctx)
        }).toJSValue()
        obj["test1"] = test1.toJSValue(context)
        obj["test2"] = test2.toJSValue(context)
        obj["name"] = name.toJSValue(context)
        obj["address"] = address.toJSValue(context)
        if(let Some(v) <- id) {
            obj["id"] = v.toJSValue(context)
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): UserProfile {
        let obj = input.asObject()
        UserProfile(
        fromJSArray < AutoGenType0 >(context, obj["array"], { ctx: JSContext, val: JSValue => AutoGenType0.fromJSValue(ctx, val) }),
        { num: AutoGenType0 =>
            checkThreadAndCall < String >(context, { ctx =>
                let arg0 = num.toJSValue(ctx)
                let ret = obj["test"].asFunction().call([arg0])
                String.fromJSValue(ctx, ret)
            })
        },
        AutoGenType2.fromJSValue(context, obj["test1"]),
        AutoGenType3.fromJSValue(context, obj["test2"]),
        String.fromJSValue(context, obj["name"]),
        AutoGenType4.fromJSValue(context, obj["address"]),
        id: Option < AutoGenType0 >.fromJSValue(context, obj["id"])
        )
    }
    
}

