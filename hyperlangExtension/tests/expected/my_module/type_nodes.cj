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

public enum AutoGenType0 < U > where U <: JSInteropType<U> {
    | NUMBER(Float64)
    | U0(U)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case NUMBER(x) => context.number(x).toJSValue()
            case U0(x) => x.toJSValue(context)
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0 < U > {
        
        if(input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        } 
        throw Exception("unsupport type!")
    }
}

public enum AutoGenType1 < U > where U <: JSInteropType<U> {
    | U0(U)
    | NUMBER(Float64)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case U0(x) => x.toJSValue(context)
            case NUMBER(x) => context.number(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType1 < U > {
        
        if(input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        } 
        throw Exception("unsupport type!")
    }
}


/*interface AutoGenType2 {
    x: number;
    y: U;
    }*/

public open class AutoGenType2 {
    
    protected AutoGenType2(public var x: Float64,
    public var y: U) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["x"] = x.toJSValue(context)
        obj["y"] = y.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType2 {
        let obj = input.asObject()
        AutoGenType2(
        Float64.fromJSValue(context, obj["x"]),
        U.fromJSValue(context, obj["y"])
        )
    }
    
}

/*interface AutoGenType3 {
    }*/

public open class AutoGenType3 {
    
    protected AutoGenType3() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType3 {
        let obj = input.asObject()
        AutoGenType3(
        
        )
    }
    
}

/*interface AutoGenType4 {
    }*/

public open class AutoGenType4 {
    
    protected AutoGenType4() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType4 {
        let obj = input.asObject()
        AutoGenType4(
        
        )
    }
    
}

/*interface AutoGenType5 {
    x: { value: number; unit: string } | number;
    y: { value: number; unit: string } | number;
    }*/

public open class AutoGenType5 {
    
    protected AutoGenType5(public var x: JSValue/* FIXME: `{ value: number; unit: string } | number` */,
    public var y: JSValue/* FIXME: `{ value: number; unit: string } | number` */) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["x"] = x
        obj["y"] = y
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType5 {
        let obj = input.asObject()
        AutoGenType5(
        obj["x"],
        obj["y"]
        )
    }
    
}

/*interface AutoGenType6 {
    }*/

public open class AutoGenType6 {
    
    protected AutoGenType6() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType6 {
        let obj = input.asObject()
        AutoGenType6(
        
        )
    }
    
}

/*interface AutoGenType7 {
    }*/

public open class AutoGenType7 {
    
    protected AutoGenType7() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType7 {
        let obj = input.asObject()
        AutoGenType7(
        
        )
    }
    
}

/*interface AutoGenType8 {
    }*/

public open class AutoGenType8 {
    
    protected AutoGenType8() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType8 {
        let obj = input.asObject()
        AutoGenType8(
        
        )
    }
    
}
/***********OBJECT***********/
// These interfaces are used for testing type syntax of the typescript compiler.
// Properties are named after the syntax kinds as in tsc.
/*interface BasicTypes {
    numberKeyword: number;
    stringKeyword: String;
    booleanKeyword: boolean;
    bigintKeyword: bigint;
    objectKeyword: object;
    symbolKeyword: symbol;
    voidKeyword: void;
    undefinedKeyword: undefined;
    anyKeyword: any;
    unknownKeyword: unknown;
    neverKeyword: never;
    }*/

public open class BasicTypes {
    
    protected BasicTypes(private var numberKeyword: Float64,
    private var stringKeyword: String,
    private var booleanKeyword: Bool,
    private var bigintKeyword: BigInt,
    private var objectKeyword: JSValue/* FIXME: `object` */,
    private var symbolKeyword: JSValue/* FIXME: `symbol` */,
    private var voidKeyword: Unit,
    private var undefinedKeyword: JSValue/* FIXME: `undefined` */,
    private var anyKeyword: JSValue/* FIXME: `any` */,
    private var unknownKeyword: JSValue/* FIXME: `unknown` */,
    private var neverKeyword: JSValue/* FIXME: `never` */) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["numberKeyword"] = numberKeyword.toJSValue(context)
        obj["stringKeyword"] = stringKeyword.toJSValue(context)
        obj["booleanKeyword"] = booleanKeyword.toJSValue(context)
        obj["bigintKeyword"] = context.bigint(bigintKeyword).toJSValue(context)
        obj["objectKeyword"] = objectKeyword
        obj["symbolKeyword"] = symbolKeyword
        obj["voidKeyword"] = voidKeyword.toJSValue(context)
        obj["undefinedKeyword"] = undefinedKeyword
        obj["anyKeyword"] = anyKeyword
        obj["unknownKeyword"] = unknownKeyword
        obj["neverKeyword"] = neverKeyword
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): BasicTypes {
        let obj = input.asObject()
        BasicTypes(
        Float64.fromJSValue(context, obj["numberKeyword"]),
        String.fromJSValue(context, obj["stringKeyword"]),
        Bool.fromJSValue(context, obj["booleanKeyword"]),
        obj["bigintKeyword"].asBigInt(context).toBigInt(),
        obj["objectKeyword"],
        obj["symbolKeyword"],
        Unit.fromJSValue(context, obj["voidKeyword"]),
        obj["undefinedKeyword"],
        obj["anyKeyword"],
        obj["unknownKeyword"],
        obj["neverKeyword"]
        )
    }
    
}

/*interface FunctionTypes {
    functionType1: () => void;
    functionType10: <V>(arg: V) => void;
    functionType11: <V, W extends String>(arg: V) => W;
    functionType12: <V = object, W extends String = '123' | '345'>(arg: V) => W;
    functionType20: (...a: number[]) => void;
    functionType30: (a?: number) => void;
    }*/

public open class FunctionTypes {
    
    protected FunctionTypes(private var functionType1: () -> Unit,
    private var functionType10: (arg: JSValue/* FIXME: `V` */) -> Unit/* FIXME: `<V>(arg: V) => void` */,
    private var functionType11: (arg: JSValue/* FIXME: `V` */) -> W/* FIXME: `<V, W extends string>(arg: V) => W` */,
    private var functionType12: (arg: JSValue/* FIXME: `V` */) -> W/* FIXME: `<V, W extends string>(arg: V) => W` */,
    private var functionType20: (a: Array<Float64>) -> Unit,
    private var functionType30: (a: ?Float64) -> Unit) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["functionType1"] = context.function({ ctx, _ =>
            functionType1()
            ctx.undefined().toJSValue()
        }).toJSValue()
        obj["functionType10"] = context.function({ ctx, info =>
            let p0 = info[0]
            functionType10(p0)
            ctx.undefined().toJSValue()
        }).toJSValue()
        obj["functionType11"] = context.function({ ctx, info =>
            let p0 = info[0]
            let ret = functionType11(p0)
            ret.toJSValue(ctx)
        }).toJSValue()
        obj["functionType12"] = context.function({ ctx, info =>
            let p0 = info[0]
            let ret = functionType12(p0)
            ret.toJSValue(ctx)
        }).toJSValue()
        obj["functionType20"] = context.function({ ctx, info =>
            let p0 = fromJSArray < Float64 >(ctx, info[0])
            functionType20(p0)
            ctx.undefined().toJSValue()
        }).toJSValue()
        obj["functionType30"] = context.function({ ctx, info =>
            let p0 = Float64.fromJSValue(ctx, info[0])
            functionType30(p0)
            ctx.undefined().toJSValue()
        }).toJSValue()
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): FunctionTypes {
        let obj = input.asObject()
        FunctionTypes(
        { =>
            checkThreadAndCall < Unit >(context, { _ =>
                obj["functionType1"].asFunction().call()
            })
        },
        { arg: JSValue/* FIXME: `V` */ =>
            checkThreadAndCall < Unit >(context, { ctx =>
                let arg0 = arg
                obj["functionType10"].asFunction().call([arg0])
            })
        },
        { arg: JSValue/* FIXME: `V` */ =>
            checkThreadAndCall < W >(context, { ctx =>
                let arg0 = arg
                let ret = obj["functionType11"].asFunction().call([arg0])
                W.fromJSValue(ctx, ret)
            })
        },
        { arg: JSValue/* FIXME: `V` */ =>
            checkThreadAndCall < W >(context, { ctx =>
                let arg0 = arg
                let ret = obj["functionType12"].asFunction().call([arg0])
                W.fromJSValue(ctx, ret)
            })
        },
        { a: Array<Float64> =>
            checkThreadAndCall < Unit >(context, { ctx =>
                let arg0 = toJSArray < Float64 >(ctx, a)
                obj["functionType20"].asFunction().call([arg0])
            })
        },
        { a:?Float64 =>
            checkThreadAndCall < Unit >(context, { ctx =>
                let arg0 = a.toJSValue(ctx)
                obj["functionType30"].asFunction().call([arg0])
            })
        }
        )
    }
    
}

/*interface ComplexTypes {
    typeReference10: U;
    typeReference20: ComplexTypes<T, U>;
    typeReference21: Promise<T>;
    typeReference30: Pick<ComplexTypes<T, U>, 'literalType1'>;
    literalType1: 123;
    literalType2: "abc";
    literalType3: null;
    arrayType1: number[];
    arrayType2: U[];
    unionType: number | U;
    intersectionType: object & Record<U, T>;
    parenthesizedType1: (any);
    parenthesizedType2: (U | number)[];
    tupleType: [number, number, String];
    typeLiteral10: { x: number; y: U; };
    typeLiteral20: { [p: number]: String; [p: symbol]: T };
    typeLiteral30: { (): void; (number): String };
    typeOperator1: keyof ComplexTypes<T, U>;
    typeOperator10: readonly number[];
    typeQuery: typeof setTimeout;
    mappedType10: { [k in keyof ComplexTypes<T, U>]: number };
    mappedType20: { [k in keyof Promise<U>]: number };
    }*/

public open class ComplexTypes<T, U> where T <: JSInteropType<T>, U <: JSInteropType<U> {
    
    protected ComplexTypes(private var typeReference10: U,
    private var typeReference20: ComplexTypes<T, U>,
    private var typeReference21: JSValue/* FIXME: `Promise<T>` */,
    private var typeReference30: Pick<ComplexTypes<T, U>, JSValue/* FIXME: `"literalType1"` */>,
    private var literalType1: JSValue/* FIXME: `123` */,
    private var literalType2: JSValue/* FIXME: `"abc"` */,
    private var literalType3: JSValue/* FIXME: `null` */,
    private var arrayType1: Array<Float64>,
    private var arrayType2: Array<U>,
    private var unionType: AutoGenType0< U >,
    private var intersectionType: JSValue/* FIXME: `object & HashMap<U, T>` */,
    private var parenthesizedType1: JSValue/* FIXME: `any` */,
    private var parenthesizedType2: Array<AutoGenType1< U >>,
    private var tupleType: (Float64, Float64, String),
    private var typeLiteral10: AutoGenType2,
    private var typeLiteral20: AutoGenType3,
    private var typeLiteral30: AutoGenType4,
    private var typeOperator1: JSValue/* FIXME: `UnsupportedType[TypeOperator]` */,
    private var typeOperator10: JSValue/* FIXME: `UnsupportedType[TypeOperator]` */,
    private var typeQuery: JSValue/* FIXME: `UnsupportedType[TypeQuery]` */,
    private var mappedType10: JSValue/* FIXME: `UnsupportedType[MappedType]` */,
    private var mappedType20: JSValue/* FIXME: `UnsupportedType[MappedType]` */) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["typeReference10"] = typeReference10.toJSValue(context)
        obj["typeReference20"] = typeReference20.toJSValue(context)
        obj["typeReference21"] = typeReference21
        obj["typeReference30"] = typeReference30.toJSValue(context)
        obj["literalType1"] = literalType1
        obj["literalType2"] = literalType2
        obj["literalType3"] = literalType3
        obj["arrayType1"] = toJSArray < Float64 >(context, arrayType1)
        obj["arrayType2"] = toJSArray < U >(context, arrayType2, { ctx: JSContext, val: U => val.toJSValue(ctx) })
        obj["unionType"] = unionType.toJSValue(context)
        obj["intersectionType"] = intersectionType
        obj["parenthesizedType1"] = parenthesizedType1
        obj["parenthesizedType2"] = toJSArray < AutoGenType1< U > >(context, parenthesizedType2, { ctx: JSContext, val: AutoGenType1< U > => val.toJSValue(ctx) })
        obj["tupleType"] = tupleType.toJSValue(context)
        obj["typeLiteral10"] = typeLiteral10.toJSValue(context)
        obj["typeLiteral20"] = typeLiteral20.toJSValue(context)
        obj["typeLiteral30"] = typeLiteral30.toJSValue(context)
        obj["typeOperator1"] = typeOperator1
        obj["typeOperator10"] = typeOperator10
        obj["typeQuery"] = typeQuery
        obj["mappedType10"] = mappedType10
        obj["mappedType20"] = mappedType20
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): ComplexTypes<T, U> {
        let obj = input.asObject()
        ComplexTypes(
        U.fromJSValue(context, obj["typeReference10"]),
        ComplexTypes<T, U>.fromJSValue(context, obj["typeReference20"]),
        obj["typeReference21"],
        Pick<ComplexTypes<T, U>, JSValue/* FIXME: `"literalType1"` */>.fromJSValue(context, obj["typeReference30"]),
        obj["literalType1"],
        obj["literalType2"],
        obj["literalType3"],
        fromJSArray < Float64 >(context, obj["arrayType1"]),
        fromJSArray < U >(context, obj["arrayType2"], { ctx: JSContext, val: JSValue => U.fromJSValue(ctx, val) }),
        AutoGenType0< U >.fromJSValue(context, obj["unionType"]),
        obj["intersectionType"],
        obj["parenthesizedType1"],
        fromJSArray < AutoGenType1< U > >(context, obj["parenthesizedType2"], { ctx: JSContext, val: JSValue => AutoGenType1< U >.fromJSValue(ctx, val) }),
        (Float64.fromJSValue(context, obj["tupleType"].asObject()[0]), Float64.fromJSValue(context, obj["tupleType"].asObject()[1]), String.fromJSValue(context, obj["tupleType"].asObject()[2])),
        AutoGenType2.fromJSValue(context, obj["typeLiteral10"]),
        AutoGenType3.fromJSValue(context, obj["typeLiteral20"]),
        AutoGenType4.fromJSValue(context, obj["typeLiteral30"]),
        obj["typeOperator1"],
        obj["typeOperator10"],
        obj["typeQuery"],
        obj["mappedType10"],
        obj["mappedType20"]
        )
    }
    
}

/*interface NestedTypes {
    nested10: {
		x: { value: number; unit: String; } | number;
		y: { value: number; unit: String; } | number;
	};
    nested20: ComplexTypes<Record<String, (x: number) => Promise<String>>, String>;
    nested21: ComplexTypes<Record<String, <T>(x: T) => Promise<String>>, String>;
    nested22: ComplexTypes<Record<String, <T>(x?: T) => Promise<[String, String]>>, String>;
    }*/

public open class NestedTypes {
    
    protected NestedTypes(private var nested10: AutoGenType5,
    private var nested20: ComplexTypes<HashMap<String, (x: Float64) -> JSValue/* FIXME: `Promise<string>` */>, String>,
    private var nested21: ComplexTypes<HashMap<String, (x: T) -> JSValue/* FIXME: `Promise<string>` *//* FIXME: `<T>(x: T) => Promise<string>` */>, String>,
    private var nested22: ComplexTypes<HashMap<String, (x: ?T) -> JSValue/* FIXME: `Promise<(string, string)>` *//* FIXME: `<T>(x?: T) => Promise<(string, string)>` */>, String>) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["nested10"] = nested10.toJSValue(context)
        obj["nested20"] = nested20.toJSValue(context)
        obj["nested21"] = nested21.toJSValue(context)
        obj["nested22"] = nested22.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): NestedTypes {
        let obj = input.asObject()
        NestedTypes(
        AutoGenType5.fromJSValue(context, obj["nested10"]),
        ComplexTypes<HashMap<String, (x: Float64) -> JSValue/* FIXME: `Promise<string>` */>, String>.fromJSValue(context, obj["nested20"]),
        ComplexTypes<HashMap<String, (x: T) -> JSValue/* FIXME: `Promise<string>` *//* FIXME: `<T>(x: T) => Promise<string>` */>, String>.fromJSValue(context, obj["nested21"]),
        ComplexTypes<HashMap<String, (x: ?T) -> JSValue/* FIXME: `Promise<(string, string)>` *//* FIXME: `<T>(x?: T) => Promise<(string, string)>` */>, String>.fromJSValue(context, obj["nested22"])
        )
    }
    
}

/*interface Optionals {
    optionalParam10: (a: number, b?: String) => void;
    optionalField1?: number;
    }*/

public open class Optionals {
    
    protected Optionals(private var optionalParam10: (a: Float64, b: ?String) -> Unit,
    private var optionalField1!: Option<Float64> = None) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["optionalParam10"] = context.function({ ctx, info =>
            let p0 = Float64.fromJSValue(ctx, info[0])
            let p1 = String.fromJSValue(ctx, info[1])
            optionalParam10(p0, p1)
            ctx.undefined().toJSValue()
        }).toJSValue()
        if(let Some(v) <- optionalField1) {
            obj["optionalField1"] = v.toJSValue(context)
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): Optionals {
        let obj = input.asObject()
        Optionals(
        { a: Float64, b:?String =>
            checkThreadAndCall < Unit >(context, { ctx =>
                let arg0 = a.toJSValue(ctx)
                let arg1 = b.toJSValue(ctx)
                obj["optionalParam10"].asFunction().call([arg0, arg1])
            })
        },
        optionalField1: Option < Float64 >.fromJSValue(context, obj["optionalField1"])
        )
    }
    
}

/*interface InferredTypes {
    inferred1: number & String;
    inferred10: ReturnType<typeof setTimeout>;
    }*/

public open class InferredTypes {
    
    protected InferredTypes(private var inferred1: JSValue/* FIXME: `number & string` */,
    private var inferred10: ReturnType<JSValue/* FIXME: `UnsupportedType[TypeQuery]` */>) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["inferred1"] = inferred1
        obj["inferred10"] = inferred10.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): InferredTypes {
        let obj = input.asObject()
        InferredTypes(
        obj["inferred1"],
        ReturnType<JSValue/* FIXME: `UnsupportedType[TypeQuery]` */>.fromJSValue(context, obj["inferred10"])
        )
    }
    
}

/*interface UnsupportedTypes {
    unsupported10: { [123]: String; };
    unsupported20: { ['234']: number; };
    unsupported30: { [Symbol.toStringTag]: number };
    }*/

public open class UnsupportedTypes {
    
    protected UnsupportedTypes(private var unsupported10: AutoGenType6,
    private var unsupported20: AutoGenType7,
    private var unsupported30: AutoGenType8) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["unsupported10"] = unsupported10.toJSValue(context)
        obj["unsupported20"] = unsupported20.toJSValue(context)
        obj["unsupported30"] = unsupported30.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): UnsupportedTypes {
        let obj = input.asObject()
        UnsupportedTypes(
        AutoGenType6.fromJSValue(context, obj["unsupported10"]),
        AutoGenType7.fromJSValue(context, obj["unsupported20"]),
        AutoGenType8.fromJSValue(context, obj["unsupported30"])
        )
    }
    
}

/*interface ArrayTypesI {
    arr10: Array<Error>;
    arr20: number[];
    arr11?: Array<Error>;
    arr21?: number[];
    }*/

public open class ArrayTypesI {
    
    protected ArrayTypesI(private var arr10: Array<JSValue/* FIXME: `Error` */>,
    private var arr20: Array<Float64>,
    private var arr11!: Option<Array<JSValue/* FIXME: `Error` */>> = None,
    private var arr21!: Option<Array<Float64>> = None) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["arr10"] = toJSArray < JSValue/* FIXME: `Error` */ >(context, arr10, { ctx: JSContext, val: JSValue/* FIXME: `Error` */ => val })
        obj["arr20"] = toJSArray < Float64 >(context, arr20)
        if(let Some(v) <- arr11) {
            obj["arr11"] = toJSArray < JSValue/* FIXME: `Error` */ >(context, v, { ctx: JSContext, val: JSValue/* FIXME: `Error` */ => val })
        }
        if(let Some(v) <- arr21) {
            obj["arr21"] = toJSArray < Float64 >(context, v)
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): ArrayTypesI {
        let obj = input.asObject()
        ArrayTypesI(
        fromJSArray < JSValue/* FIXME: `Error` */ >(context, obj["arr10"], { ctx: JSContext, val: JSValue => val }),
        fromJSArray < Float64 >(context, obj["arr20"]),
        arr11: fromJSArrayOption < JSValue/* FIXME: `Error` */ >(context, obj["arr11"], { ctx: JSContext, val: JSValue => val }),
        arr21: fromJSArrayOption < Float64 >(context, obj["arr21"])
        )
    }
    
}

/*class DeclareKeyword ArrayTypesC {
    arr10: Array<Error>;
    arr20: number[];
    arr11?: Array<Error>;
    arr21?: number[];
    }*/

public class ArrayTypesC {
    
    protected ArrayTypesC(var arkts_object: JSObject) { }
    
    
    public mut prop arr10: Array<JSValue/* FIXME: `Error` */> {
        get() {
            checkThreadAndCall < Array<JSValue/* FIXME: `Error` */> >(getMainContext()) {
                ctx: JSContext => fromJSArray < JSValue/* FIXME: `Error` */ >(ctx, arkts_object["arr10"], { ctx: JSContext, val: JSValue => val })
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["arr10"] = toJSArray < JSValue/* FIXME: `Error` */ >(ctx, v, { ctx: JSContext, val: JSValue/* FIXME: `Error` */ => val })
            }
        }
        
    }
    
    public mut prop arr20: Array<Float64> {
        get() {
            checkThreadAndCall < Array<Float64> >(getMainContext()) {
                ctx: JSContext => fromJSArray < Float64 >(ctx, arkts_object["arr20"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["arr20"] = toJSArray < Float64 >(ctx, v)
            }
        }
        
    }
    
    public mut prop arr11: ?Array<JSValue/* FIXME: `Error` */> {
        get() {
            checkThreadAndCall < ?Array<JSValue/* FIXME: `Error` */> >(getMainContext()) {
                ctx: JSContext => fromJSArrayOption < JSValue/* FIXME: `Error` */ >(ctx, arkts_object["arr11"], { ctx: JSContext, val: JSValue => val })
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["arr11"] = toJSArray < JSValue/* FIXME: `Error` */ >(ctx, v, { ctx: JSContext, val: JSValue/* FIXME: `Error` */ => val })
            }
        }
        
    }
    
    public mut prop arr21: ?Array<Float64> {
        get() {
            checkThreadAndCall < ?Array<Float64> >(getMainContext()) {
                ctx: JSContext => fromJSArrayOption < Float64 >(ctx, arkts_object["arr21"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["arr21"] = toJSArray < Float64 >(ctx, v)
            }
        }
        
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
     static func fromJSValue(context: JSContext, input: JSValue): ArrayTypesC {
        ArrayTypesC(input.asObject())
    }
}

/***********METHOD***********/
/**
	 * @brief returnType10(): any
	 */
public func returnType10(): JSValue/* FIXME: `any` */ {
    hmsGlobalApiCall < JSValue/* FIXME: `any` */ >( "my_module_typeNodes", "returnType10", emptyArg) {
        ctx, info => info
    }
}


