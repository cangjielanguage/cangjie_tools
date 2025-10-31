// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
import std.math.numeric.*
import std.collection.{ HashMap }
/***********OBJECT***********/

/*interface I1 {
    a: number;
    c: Uint8Array;
    e: Float32Array;
    g: Record<String, Uint8Array>;
    h: MyStringEnum;
    i: bigint;
    b?: String;
    d?: ArrayBuffer;
    f?: boolean;
    }*/

public open class I1 {
    
    protected I1(public var a: Float64,
    public var c: Array<UInt8>,
    public var e: Array<Float32>,
    public var g: HashMap<String, Array<UInt8>>,
    public var h: MyStringEnum,
    public var i: BigInt,
    public var b!: Option<String> = None,
    public var d!: Option<Array<UInt8>> = None,
    public var f!: Option<Bool> = None) {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["a"] = a.toJSValue(context)
        obj["c"] = toJSArray < UInt8 >(context, c, { ctx: JSContext, val: UInt8 => val.toJSValue(ctx) })
        obj["e"] = toJSArray < Float32 >(context, e, { ctx: JSContext, val: Float32 => val.toJSValue(ctx) })
        obj["g"] = hashmap2Record<Array<UInt8>>(context, g, { context, i => i.toJSValue(context) })
        obj["h"] = h.get().toJSValue(context)
        obj["i"] = context.bigint(i).toJSValue(context)
        if(let Some(v) <- b) {
            obj["b"] = v.toJSValue(context)
        }
        if(let Some(v) <- d) {
            obj["d"] = toJSArray < UInt8 >(context, v, { ctx: JSContext, val: UInt8 => val.toJSValue(ctx) })
        }
        if(let Some(v) <- f) {
            obj["f"] = v.toJSValue(context)
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): I1 {
        let obj = input.asObject()
        I1(
        Float64.fromJSValue(context, obj["a"]),
        fromJSArray < UInt8 >(context, obj["c"], { ctx: JSContext, val: JSValue => UInt8.fromJSValue(ctx, val) }),
        fromJSArray < Float32 >(context, obj["e"], { ctx: JSContext, val: JSValue => Float32.fromJSValue(ctx, val) }),
        record2Hashmap<Array<UInt8>>(context, obj["g"], { context, i => Array<UInt8>.fromJSValue(context, i) }),
        MyStringEnum.parse(String.fromJSValue(context, obj["h"])),
        obj["i"].asBigInt(context).toBigInt(),
        b: Option < String >.fromJSValue(context, obj["b"]),
        d: fromJSArrayOption < UInt8 >(context, obj["d"], { ctx: JSContext, val: JSValue => UInt8.fromJSValue(ctx, val) }),
        f: Option < Bool >.fromJSValue(context, obj["f"])
        )
    }
    
}

/*class C1 {
    StaticKeywordsa: number = 123;
    StaticKeywordsb: String;
    StaticKeywordReadonlyKeywordsd: number = 999;
    StaticKeywordReadonlyKeywordse: String = "zzz";
    va: number = 234;
    vb: Float32Array;
    StaticKeywordsc?: String = "abc";
    vc?: MyNumericEnum;
    vd?: string = "def";
    ve?: bigint[];
    }*/

public class C1 {
    
    protected C1(var arkts_object: JSObject) {}
    
    
    public static mut prop sa: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, getClassConstructorObj("my_module_valueConversion", "C1")["sa"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_valueConversion", "C1")["sa"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public static mut prop sb: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, getClassConstructorObj("my_module_valueConversion", "C1")["sb"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_valueConversion", "C1")["sb"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public static prop sd: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, getClassConstructorObj("my_module_valueConversion", "C1")["sd"])
            }
        }
    }
    
    public static prop se: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, getClassConstructorObj("my_module_valueConversion", "C1")["se"])
            }
        }
    }
    
    public mut prop va: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, arkts_object["va"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["va"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop vb: Array<Float32> {
        get() {
            checkThreadAndCall < Array<Float32> >(getMainContext()) {
                ctx: JSContext => fromJSArray < Float32 >(ctx, arkts_object["vb"], { ctx: JSContext, val: JSValue => Float32.fromJSValue(ctx, val) })
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["vb"] = toJSArray < Float32 >(ctx, v, { ctx: JSContext, val: Float32 => val.toJSValue(ctx) })
            }
        }
        
    }
    
    public static mut prop sc: ?String {
        get() {
            checkThreadAndCall < ?String >(getMainContext()) {
                ctx: JSContext => Option < String >.fromJSValue(ctx, getClassConstructorObj("my_module_valueConversion", "C1")["sc"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_valueConversion", "C1")["sc"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop vc: ?MyNumericEnum {
        get() {
            checkThreadAndCall < ?MyNumericEnum >(getMainContext()) {
                ctx: JSContext => MyNumericEnum.tryParse(Option < Int32 >.fromJSValue(ctx, arkts_object["vc"]))
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["vc"] = v.get().toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop vd: ?String {
        get() {
            checkThreadAndCall < ?String >(getMainContext()) {
                ctx: JSContext => Option < String >.fromJSValue(ctx, arkts_object["vd"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["vd"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop ve: ?Array<BigInt> {
        get() {
            checkThreadAndCall < ?Array<BigInt> >(getMainContext()) {
                ctx: JSContext => fromJSArrayOption < BigInt >(ctx, arkts_object["ve"], { ctx: JSContext, val: JSValue => val.asBigInt(ctx).toBigInt() })
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["ve"] = toJSArray < BigInt >(ctx, v, { ctx: JSContext, val: BigInt => ctx.bigint(val).toJSValue(ctx) })
            }
        }
        
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): C1 {
        C1(input.asObject())
    }
}

