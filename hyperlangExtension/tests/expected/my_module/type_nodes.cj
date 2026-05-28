// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

public enum AutoGenType0IntypeNodes < U > where U <: JSInteropType<U> {
    | NUMBER(Float64)
    | U0(U)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case NUMBER(x) => context.number(x).toJSValue()
            case U0(x) => x.toJSValue(context)
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0IntypeNodes < U > {
        
        if (input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        } 
        throw Exception("unsupport type!")
    }
}

public enum AutoGenType1IntypeNodes < U > where U <: JSInteropType<U> {
    | U0(U)
    | NUMBER(Float64)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case U0(x) => x.toJSValue(context)
            case NUMBER(x) => context.number(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType1IntypeNodes < U > {
        
        if (input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        } 
        throw Exception("unsupport type!")
    }
}


/*interface AutoGenType2IntypeNodes {
    x: number;
    y: U;
    }*/

public open class AutoGenType2IntypeNodes<U> where U <: JSInteropType<U> {
    public var x: Float64
    public var y: U
    protected init(
    x: Float64,
    y: U) {
        this.x = x
        this.y = y
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["x"] = x.toJSValue(context)
        obj["y"] = y.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType2IntypeNodes<U> {
        let obj = input.asObject()
        AutoGenType2IntypeNodes(
        Float64.fromJSValue(context, obj["x"]),
        U.fromJSValue(context, obj["y"])
        )
    }
}


/*interface AutoGenType3IntypeNodes {
    }*/

public open class AutoGenType3IntypeNodes {
    protected init() {
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType3IntypeNodes {
        let obj = input.asObject()
        AutoGenType3IntypeNodes(
        
        )
    }
}


/*interface AutoGenType4IntypeNodes {
    }*/

public open class AutoGenType4IntypeNodes {
    protected init() {
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType4IntypeNodes {
        let obj = input.asObject()
        AutoGenType4IntypeNodes(
        
        )
    }
}


/*interface AutoGenType5IntypeNodes {
    x: { value: number; unit: string } | number;
    y: { value: number; unit: string } | number;
    }*/

public open class AutoGenType5IntypeNodes {
    public var x: JSValue/* FIXME: `{ value: number; unit: string } | number` */
    public var y: JSValue/* FIXME: `{ value: number; unit: string } | number` */
    protected init(
    x: JSValue/* FIXME: `{ value: number; unit: string } | number` */,
    y: JSValue/* FIXME: `{ value: number; unit: string } | number` */) {
        this.x = x
        this.y = y
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["x"] = x
        obj["y"] = y
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType5IntypeNodes {
        let obj = input.asObject()
        AutoGenType5IntypeNodes(
        obj["x"],
        obj["y"]
        )
    }
}


/*interface AutoGenType6IntypeNodes {
    }*/

public open class AutoGenType6IntypeNodes {
    protected init() {
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType6IntypeNodes {
        let obj = input.asObject()
        AutoGenType6IntypeNodes(
        
        )
    }
}


/*interface AutoGenType7IntypeNodes {
    }*/

public open class AutoGenType7IntypeNodes {
    protected init() {
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType7IntypeNodes {
        let obj = input.asObject()
        AutoGenType7IntypeNodes(
        
        )
    }
}


/*interface AutoGenType8IntypeNodes {
    }*/

public open class AutoGenType8IntypeNodes {
    protected init() {
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType8IntypeNodes {
        let obj = input.asObject()
        AutoGenType8IntypeNodes(
        
        )
    }
}

/***********OBJECT***********/

/*interface ArrayTypesI {
    arr10: Array<Error>;
    arr20: number[];
    arr11?: Array<Error>;
    arr21?: number[];
    }*/

public open class ArrayTypesI {
    public var arr10: Array<JSValue/* FIXME: `Error` */>
    public var arr20: Array<Float64>
    public var arr11: Option<Array<JSValue/* FIXME: `Error` */>> = None
    public var arr21: Option<Array<Float64>> = None
    protected init(
    arr10: Array<JSValue/* FIXME: `Error` */>,
    arr20: Array<Float64>,
    arr11!: Option<Array<JSValue/* FIXME: `Error` */>> = None,
    arr21!: Option<Array<Float64>> = None) {
        this.arr10 = arr10
        this.arr20 = arr20
        this.arr11 = arr11
        this.arr21 = arr21
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["arr10"] = toJSArray < JSValue/* FIXME: `Error` */ >(context, arr10, { ctx: JSContext, val: JSValue/* FIXME: `Error` */ => val })
        obj["arr20"] = toJSArray < Float64 >(context, arr20)
        if (let Some(v) <- arr11) {
            obj["arr11"] = toJSArray < JSValue/* FIXME: `Error` */ >(context, v, { ctx: JSContext, val: JSValue/* FIXME: `Error` */ => val })
        }
        if (let Some(v) <- arr21) {
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
    public var numberKeyword: Float64
    public var stringKeyword: String
    public var booleanKeyword: Bool
    public var bigintKeyword: BigInt
    public var objectKeyword: JSValue/* FIXME: `object` */
    public var symbolKeyword: JSValue/* FIXME: `symbol` */
    public var voidKeyword: Unit
    public var undefinedKeyword: JSValue/* FIXME: `undefined` */
    public var anyKeyword: JSValue/* FIXME: `any` */
    public var unknownKeyword: JSValue/* FIXME: `unknown` */
    public var neverKeyword: JSValue/* FIXME: `never` */
    protected init(
    numberKeyword: Float64,
    stringKeyword: String,
    booleanKeyword: Bool,
    bigintKeyword: BigInt,
    objectKeyword: JSValue/* FIXME: `object` */,
    symbolKeyword: JSValue/* FIXME: `symbol` */,
    voidKeyword: Unit,
    undefinedKeyword: JSValue/* FIXME: `undefined` */,
    anyKeyword: JSValue/* FIXME: `any` */,
    unknownKeyword: JSValue/* FIXME: `unknown` */,
    neverKeyword: JSValue/* FIXME: `never` */) {
        this.numberKeyword = numberKeyword
        this.stringKeyword = stringKeyword
        this.booleanKeyword = booleanKeyword
        this.bigintKeyword = bigintKeyword
        this.objectKeyword = objectKeyword
        this.symbolKeyword = symbolKeyword
        this.voidKeyword = voidKeyword
        this.undefinedKeyword = undefinedKeyword
        this.anyKeyword = anyKeyword
        this.unknownKeyword = unknownKeyword
        this.neverKeyword = neverKeyword
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["numberKeyword"] = numberKeyword.toJSValue(context)
        obj["stringKeyword"] = stringKeyword.toJSValue(context)
        obj["booleanKeyword"] = booleanKeyword.toJSValue(context)
        obj["bigintKeyword"] = context.bigint(bigintKeyword).toJSValue()
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
        obj["bigintKeyword"].asBigInt().toBigInt(),
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
    public var typeReference10: U
    public var typeReference20: ComplexTypes<T, U>
    public var typeReference21: JSValue/* FIXME: `Promise<T>` */
    public var typeReference30: JSValue/* FIXME: `Pick` */
    public var literalType1: JSValue/* FIXME: `123` */
    public var literalType2: JSValue/* FIXME: `"abc"` */
    public var literalType3: JSValue/* FIXME: `null` */
    public var arrayType1: Array<Float64>
    public var arrayType2: Array<U>
    public var unionType: AutoGenType0IntypeNodes< U >
    public var intersectionType: JSValue/* FIXME: `object & HashMap<U, T>` */
    public var parenthesizedType1: JSValue/* FIXME: `any` */
    public var parenthesizedType2: Array<AutoGenType1IntypeNodes< U >>
    public var tupleType: (Float64, Float64, String)
    public var typeLiteral10: AutoGenType2IntypeNodes<U>
    public var typeLiteral20: AutoGenType3IntypeNodes
    public var typeLiteral30: AutoGenType4IntypeNodes
    public var typeOperator1: JSValue/* FIXME: `UnsupportedType[TypeOperator]` */
    public var typeOperator10: JSValue/* FIXME: `UnsupportedType[TypeOperator]` */
    public var typeQuery: JSValue/* FIXME: `UnsupportedType[TypeQuery]` */
    public var mappedType10: JSValue/* FIXME: `UnsupportedType[MappedType]` */
    public var mappedType20: JSValue/* FIXME: `UnsupportedType[MappedType]` */
    protected init(
    typeReference10: U,
    typeReference20: ComplexTypes<T, U>,
    typeReference21: JSValue/* FIXME: `Promise<T>` */,
    typeReference30: JSValue/* FIXME: `Pick` */,
    literalType1: JSValue/* FIXME: `123` */,
    literalType2: JSValue/* FIXME: `"abc"` */,
    literalType3: JSValue/* FIXME: `null` */,
    arrayType1: Array<Float64>,
    arrayType2: Array<U>,
    unionType: AutoGenType0IntypeNodes< U >,
    intersectionType: JSValue/* FIXME: `object & HashMap<U, T>` */,
    parenthesizedType1: JSValue/* FIXME: `any` */,
    parenthesizedType2: Array<AutoGenType1IntypeNodes< U >>,
    tupleType: (Float64, Float64, String),
    typeLiteral10: AutoGenType2IntypeNodes<U>,
    typeLiteral20: AutoGenType3IntypeNodes,
    typeLiteral30: AutoGenType4IntypeNodes,
    typeOperator1: JSValue/* FIXME: `UnsupportedType[TypeOperator]` */,
    typeOperator10: JSValue/* FIXME: `UnsupportedType[TypeOperator]` */,
    typeQuery: JSValue/* FIXME: `UnsupportedType[TypeQuery]` */,
    mappedType10: JSValue/* FIXME: `UnsupportedType[MappedType]` */,
    mappedType20: JSValue/* FIXME: `UnsupportedType[MappedType]` */) {
        this.typeReference10 = typeReference10
        this.typeReference20 = typeReference20
        this.typeReference21 = typeReference21
        this.typeReference30 = typeReference30
        this.literalType1 = literalType1
        this.literalType2 = literalType2
        this.literalType3 = literalType3
        this.arrayType1 = arrayType1
        this.arrayType2 = arrayType2
        this.unionType = unionType
        this.intersectionType = intersectionType
        this.parenthesizedType1 = parenthesizedType1
        this.parenthesizedType2 = parenthesizedType2
        this.tupleType = tupleType
        this.typeLiteral10 = typeLiteral10
        this.typeLiteral20 = typeLiteral20
        this.typeLiteral30 = typeLiteral30
        this.typeOperator1 = typeOperator1
        this.typeOperator10 = typeOperator10
        this.typeQuery = typeQuery
        this.mappedType10 = mappedType10
        this.mappedType20 = mappedType20
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["typeReference10"] = typeReference10.toJSValue(context)
        obj["typeReference20"] = typeReference20.toJSValue(context)
        obj["typeReference21"] = typeReference21
        obj["typeReference30"] = typeReference30
        obj["literalType1"] = literalType1
        obj["literalType2"] = literalType2
        obj["literalType3"] = literalType3
        obj["arrayType1"] = toJSArray < Float64 >(context, arrayType1)
        obj["arrayType2"] = toJSArray < U >(context, arrayType2, { ctx: JSContext, val: U => val.toJSValue(ctx) })
        obj["unionType"] = unionType.toJSValue(context)
        obj["intersectionType"] = intersectionType
        obj["parenthesizedType1"] = parenthesizedType1
        obj["parenthesizedType2"] = toJSArray < AutoGenType1IntypeNodes< U > >(context, parenthesizedType2, { ctx: JSContext, val: AutoGenType1IntypeNodes< U > => val.toJSValue(ctx) })
        obj["tupleType"] = context.array([tupleType[0].toJSValue(context), tupleType[1].toJSValue(context), tupleType[2].toJSValue(context)]).toJSValue()
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
        obj["typeReference30"],
        obj["literalType1"],
        obj["literalType2"],
        obj["literalType3"],
        fromJSArray < Float64 >(context, obj["arrayType1"]),
        fromJSArray < U >(context, obj["arrayType2"], { ctx: JSContext, val: JSValue => U.fromJSValue(ctx, val) }),
        AutoGenType0IntypeNodes< U >.fromJSValue(context, obj["unionType"]),
        obj["intersectionType"],
        obj["parenthesizedType1"],
        fromJSArray < AutoGenType1IntypeNodes< U > >(context, obj["parenthesizedType2"], { ctx: JSContext, val: JSValue => AutoGenType1IntypeNodes< U >.fromJSValue(ctx, val) }),
        (Float64.fromJSValue(context, obj["tupleType"].asArray()[0]), Float64.fromJSValue(context, obj["tupleType"].asArray()[1]), String.fromJSValue(context, obj["tupleType"].asArray()[2])),
        AutoGenType2IntypeNodes<U>.fromJSValue(context, obj["typeLiteral10"]),
        AutoGenType3IntypeNodes.fromJSValue(context, obj["typeLiteral20"]),
        AutoGenType4IntypeNodes.fromJSValue(context, obj["typeLiteral30"]),
        obj["typeOperator1"],
        obj["typeOperator10"],
        obj["typeQuery"],
        obj["mappedType10"],
        obj["mappedType20"]
        )
    }
}


/*interface FunctionTypes {
    functionType1: () => void;
    functionType10: (arg: V) => void;
    functionType11: (arg: V) => W;
    functionType12: (arg: V) => W;
    functionType20: (...a: number[]) => void;
    functionType30: (a?: number) => void;
    }*/

public open class FunctionTypes<V, W> where V <: JSInteropType<V>, W <: JSInteropType<W> {
    public var functionType1: () -> Unit
    public var functionType10: (V) -> Unit
    public var functionType11: (V) -> W
    public var functionType12: (V) -> W
    public var functionType20: (Array<Float64>) -> Unit
    public var functionType30: (?Float64) -> Unit
    protected init(
    functionType1: () -> Unit,
    functionType10: (V) -> Unit,
    functionType11: (V) -> W,
    functionType12: (V) -> W,
    functionType20: (Array<Float64>) -> Unit,
    functionType30: (?Float64) -> Unit) {
        this.functionType1 = functionType1
        this.functionType10 = functionType10
        this.functionType11 = functionType11
        this.functionType12 = functionType12
        this.functionType20 = functionType20
        this.functionType30 = functionType30
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["functionType1"] = context.function({ ctx, _ =>
            functionType1()
            ctx.undefined().toJSValue()
        }).toJSValue()
        obj["functionType10"] = context.function({ ctx, info =>
            let p0 = V.fromJSValue(ctx, info[0])
            functionType10(p0)
            ctx.undefined().toJSValue()
        }).toJSValue()
        obj["functionType11"] = context.function({ ctx, info =>
            let p0 = V.fromJSValue(ctx, info[0])
            let ret = functionType11(p0)
            ret.toJSValue(ctx)
        }).toJSValue()
        obj["functionType12"] = context.function({ ctx, info =>
            let p0 = V.fromJSValue(ctx, info[0])
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
    
    public static func fromJSValue(context: JSContext, input: JSValue): FunctionTypes<V, W> {
        let obj = input.asObject()
        FunctionTypes(
        { =>
            checkThreadAndCall < Unit >(context, { _ =>
                obj["functionType1"].asFunction().call()
            })
        },
        { arg: V =>
            checkThreadAndCall < Unit >(context, { ctx =>
                let arg0 = arg.toJSValue(ctx)
                obj["functionType10"].asFunction().call([arg0])
            })
        },
        { arg: V =>
            checkThreadAndCall < W >(context, { ctx =>
                let arg0 = arg.toJSValue(ctx)
                let ret = obj["functionType11"].asFunction().call([arg0])
                W.fromJSValue(ctx, ret)
            })
        },
        { arg: V =>
            checkThreadAndCall < W >(context, { ctx =>
                let arg0 = arg.toJSValue(ctx)
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
                let arg0 = match(a) {
                    case None => ctx.undefined().toJSValue()
                    case _ => a.getOrThrow().toJSValue(ctx)
                }
                obj["functionType30"].asFunction().call([arg0])
            })
        }
        )
    }
}


/*interface InferredTypes {
    inferred1: number & String;
    inferred10: ReturnType<typeof setTimeout>;
    }*/

public open class InferredTypes {
    public var inferred1: JSValue/* FIXME: `number & string` */
    public var inferred10: JSValue/* FIXME: `ReturnType` */
    protected init(
    inferred1: JSValue/* FIXME: `number & string` */,
    inferred10: JSValue/* FIXME: `ReturnType` */) {
        this.inferred1 = inferred1
        this.inferred10 = inferred10
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["inferred1"] = inferred1
        obj["inferred10"] = inferred10
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): InferredTypes {
        let obj = input.asObject()
        InferredTypes(
        obj["inferred1"],
        obj["inferred10"]
        )
    }
}


/*interface NestedTypes {
    nested10: {
		x: { value: number; unit: String; } | number;
		y: { value: number; unit: String; } | number;
	};
    }*/

public open class NestedTypes<T> where T <: JSInteropType<T> {
    public var nested10: AutoGenType5IntypeNodes
    protected init(nested10: AutoGenType5IntypeNodes) {
        this.nested10 = nested10
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["nested10"] = nested10.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): NestedTypes<T> {
        let obj = input.asObject()
        NestedTypes(
        AutoGenType5IntypeNodes.fromJSValue(context, obj["nested10"])
        )
    }
}


/*interface Optionals {
    optionalParam10: (a: number, b?: String) => void;
    optionalField1?: number;
    }*/

public open class Optionals {
    public var optionalParam10: (Float64, ?String) -> Unit
    public var optionalField1: Option<Float64> = None
    protected init(
    optionalParam10: (Float64, ?String) -> Unit,
    optionalField1!: Option<Float64> = None) {
        this.optionalParam10 = optionalParam10
        this.optionalField1 = optionalField1
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["optionalParam10"] = context.function({ ctx, info =>
            let p0 = Float64.fromJSValue(ctx, info[0])
            let p1 = String.fromJSValue(ctx, info[1])
            optionalParam10(p0, p1)
            ctx.undefined().toJSValue()
        }).toJSValue()
        if (let Some(v) <- optionalField1) {
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
                let arg1 = match(b) {
                    case None => ctx.undefined().toJSValue()
                    case _ => b.getOrThrow().toJSValue(ctx)
                }
                obj["optionalParam10"].asFunction().call([arg0, arg1])
            })
        },
        optionalField1:
        if (obj["optionalField1"].isNull()) {
            None < Float64 >
        } else {
            Float64.fromJSValue(context, obj["optionalField1"])
        }
        )
    }
}


/*interface UnsupportedTypes {
    unsupported10: { [123]: String; };
    unsupported20: { ['234']: number; };
    unsupported30: { [Symbol.toStringTag]: number };
    }*/

public open class UnsupportedTypes {
    public var unsupported10: AutoGenType6IntypeNodes
    public var unsupported20: AutoGenType7IntypeNodes
    public var unsupported30: AutoGenType8IntypeNodes
    protected init(
    unsupported10: AutoGenType6IntypeNodes,
    unsupported20: AutoGenType7IntypeNodes,
    unsupported30: AutoGenType8IntypeNodes) {
        this.unsupported10 = unsupported10
        this.unsupported20 = unsupported20
        this.unsupported30 = unsupported30
    }
    
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
        AutoGenType6IntypeNodes.fromJSValue(context, obj["unsupported10"]),
        AutoGenType7IntypeNodes.fromJSValue(context, obj["unsupported20"]),
        AutoGenType8IntypeNodes.fromJSValue(context, obj["unsupported30"])
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
    
    
    func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(_: JSContext, input: JSValue): ArrayTypesC {
        ArrayTypesC(input.asObject())
    }
}

/***********METHOD***********/
/**
	 * @brief returnType10(): any
	 */
public func returnType10(): JSValue/* FIXME: `any` */ {
    hmsGlobalApiCall < JSValue/* FIXME: `any` */ >( "my_module_typeNodes", "returnType10", EMPTY_ARG) { ctx, info => info }
}


