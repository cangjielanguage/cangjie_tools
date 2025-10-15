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
import tpc.AbilityKit.common
import std.collection.{ HashMap }

public type Callback = (arg: T) -> Unit/* FIXME: `<T>(arg: T) => void` */


/***********ENUM***********/

/* enum EMNum {
	 E1 = 111,
	 E2 = 222,
	 E3 = 333
	}*/
public enum EMNum <: ToString & Equatable < EMNum > {
    | E1
    | E2
    | E3 

    func get(): Int32 {
        match(this) {
            case E1 => 111 
            case E2 => 222 
            case E3 => 333 
        }
    }
    
    static func parse(val: Int32): EMNum {
        match(val) {
            case 111 => E1 
            case 222 => E2 
            case 333 => E3 
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?Int32): ?EMNum {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        match(this) {
            case E1 => "E1"
            case E2 => "E2"
            case E3 => "E3"
        }
    }
    
    public override operator func ==(that: EMNum): Bool {
        match((this, that)) {
            case(E1, E1) => true
            case(E2, E2) => true
            case(E3, E3) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: EMNum): Bool {
        !(this == that)
    }
}


/* enum EMStr {
	 EA = aaa,
	 EB = bbb,
	 EC = ccc
	}*/
public enum EMStr <: ToString & Equatable < EMStr > {
    | EA
    | EB
    | EC 

    func get(): String {
        match(this) {
            case EA => "aaa" 
            case EB => "bbb" 
            case EC => "ccc" 
        }
    }
    
    static func parse(val: String): EMStr {
        match(val) {
            case "aaa" => EA 
            case "bbb" => EB 
            case "ccc" => EC 
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?String): ?EMStr {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        get()
    }
    
    public override operator func ==(that: EMStr): Bool {
        match((this, that)) {
            case(EA, EA) => true
            case(EB, EB) => true
            case(EC, EC) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: EMStr): Bool {
        !(this == that)
    }
}


/***********OBJECT***********/

/*interface IM {
    f1(): void
    f2(s: string): number
    f3(r: Record<string, string>): EMNum
    f4(e: EMStr): (s: String) => void
    f5(cb: (obj: CM, n: number) => string): void
    f6(evt: EMNum, cb: () => Record<number, Uint8Array>): boolean
    f7(p: bigint, cb: (p: bigint) => void): bigint
    }*/

public open class IM {
    
    protected IM(public var arkts_object: JSObject) {}
    
    
    /**
	 * @brief f1(): void
	 */
    public func f1(): Unit {
        jsObjApiCall < Unit >( arkts_object, "f1", emptyArg)
    }
    /**
	 * @brief f2(s: string): number
	 */
    public func f2(s: String): Float64 {
        jsObjApiCall < Float64 >( arkts_object, "f2", { ctx =>[s.toJSValue(ctx)] })
    }
    /**
	 * @brief f3(r: Record<string, string>): EMNum
	 */
    public func f3(r: HashMap<String, String>): EMNum {
        jsObjApiCall < EMNum >( arkts_object, "f3", { ctx =>[hashmap2Record<String>(ctx, r, { context, i => i.toJSValue(context) })] }) {
            ctx, info => EMNum.parse(Int32.fromJSValue(ctx, info))
        }
    }
    /**
	 * @brief f4(e: EMStr): (s: String) => void
	 */
    public func f4(e: EMStr): (String)->Unit {
        jsObjApiCall < (String)->Unit >( arkts_object, "f4", { ctx =>[e.get().toJSValue(ctx)] }) {
            ctx, info => (String)->Unit.fromJSValue(ctx, info)
        }
    }
    /**
	 * @brief f5(cb: (obj: CM, n: number) => string): void
	 */
    public func f5(cb: (obj: CM, n: Float64) -> String): Unit {
        jsObjApiCall < Unit >( arkts_object, "f5", { ctx =>[ctx.function({ ctx, info =>
                let p0 = CM.fromJSValue(ctx, info[0])
                let p1 = Float64.fromJSValue(ctx, info[1])
                let ret = cb(p0, p1)
                ret.toJSValue(ctx)
            }).toJSValue()] })
    }
    /**
	 * @brief f6(evt: EMNum, cb: () => Record<number, Uint8Array>): boolean
	 */
    public func f6(evt: EMNum, cb: () -> HashMap<Float64, Array<UInt8>>): Bool {
        jsObjApiCall < Bool >( arkts_object, "f6", { ctx =>[evt.get().toJSValue(ctx), ctx.function({ ctx, _ =>
                let ret = cb()
                hashmap2Record<Array<UInt8>>(ctx, ret, { context, i => i.toJSValue(context) })
            }).toJSValue()] })
    }
    /**
	 * @brief f7(p: bigint, cb: (p: bigint) => void): bigint
	 */
    public func f7(p: BigInt, cb: (p: BigInt) -> Unit): BigInt {
        jsObjApiCall < BigInt >( arkts_object, "f7", { ctx =>[ctx.bigint(p).toJSValue(ctx), ctx.function({ ctx, info =>
                let p0 = info[0].asBigInt(ctx).toBigInt()
                cb(p0)
                ctx.undefined().toJSValue()
            }).toJSValue()] }) {
            ctx, info => BigInt.fromJSValue(ctx, info)
        }
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): IM {
        IM(input.asObject())
    }
}
// 类实现接口
/*interface Drivable {
    start(): void
    stop(): void
    }*/

public open class Drivable {
    
    protected Drivable(public var arkts_object: JSObject) {}
    
    
    /**
	 * @brief start(): void
	 */
    public func start(): Unit {
        jsObjApiCall < Unit >( arkts_object, "start", emptyArg)
    }
    /**
	 * @brief stop(): void
	 */
    public func stop(): Unit {
        jsObjApiCall < Unit >( arkts_object, "stop", emptyArg)
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): Drivable {
        Drivable(input.asObject())
    }
}

/*class DeclareKeyword CM {
    StaticKeywordsa: String;
    ma: String;
    mb: EMStr;
    mc: (s: String, n: EMNum) => void;
    f00(x: (obj: IM, cb: (msg: EMStr) => void) => void): void
    f01(x: (obj: IM, cb: Callback<EMStr>) => void): void
    f1(): void
    f2(s: string): number
    f3(r: Record<string, string>): EMNum
    f4(e: EMStr): (s: String) => void
    f5(cb: (s: string, n: number) => string): void
    f6(evt: EMNum, cb: () => Record<number, Uint8Array>): boolean
    f7(p: bigint, cb: (p: bigint) => void): bigint
    f8(result: string | Promise<string>): boolean
    }*/

public class CM {
    
    protected CM(var arkts_object: JSObject) {}
    
    // 静态成员
    public static mut prop sa: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, getClassConstructorObj("my_module_methods", "CM")["sa"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_methods", "CM")["sa"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop ma: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, arkts_object["ma"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["ma"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop mb: EMStr {
        get() {
            checkThreadAndCall < EMStr >(getMainContext()) {
                ctx: JSContext => EMStr.parse(String.fromJSValue(ctx, arkts_object["mb"]))
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["mb"] = v.get().toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop mc: (s: String, n: EMNum) -> Unit {
        get() {
            checkThreadAndCall < (s: String, n: EMNum) -> Unit >(getMainContext()) {
                ctx: JSContext => { s: String, n: EMNum =>
                    checkThreadAndCall < Unit >(ctx, { ctx =>
                        let arg0 = s.toJSValue(ctx)
                        let arg1 = n.get().toJSValue(ctx)
                        arkts_object["mc"].asFunction().call([arg0, arg1])
                    })
                }
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["mc"] = ctx.function({ ctx, info =>
                    let p0 = String.fromJSValue(ctx, info[0])
                    let p1 = EMNum.parse(Int32.fromJSValue(ctx, info[1]))
                    v(p0, p1)
                    ctx.undefined().toJSValue()
                }).toJSValue()
            }
        }
        
    }
    
    /**
	 * @brief f00(x: (obj: IM, cb: (msg: EMStr) => void) => void): void
	 */
    public static func f00(x: (obj: IM, cb: (msg: EMStr) -> Unit) -> Unit): Unit {
        jsObjApiCall < Unit >(getClassConstructorObj("my_module_methods", "CM"),  "f00", { ctx =>[ctx.function({ ctx, info =>
                let p0 = IM.fromJSValue(ctx, info[0])
                let p1 = { msg: EMStr =>
                    checkThreadAndCall < Unit >(ctx, { ctx =>
                        let arg0 = msg.get().toJSValue(ctx)
                        info[1].asFunction().call([arg0])
                    })
                }
                x(p0, p1)
                ctx.undefined().toJSValue()
            }).toJSValue()] })
    }
    /**
	 * @brief f01(x: (obj: IM, cb: Callback<EMStr>) => void): void
	 */
    public static func f01(x: (obj: IM, cb: Callback<EMStr>) -> Unit): Unit {
        jsObjApiCall < Unit >(getClassConstructorObj("my_module_methods", "CM"),  "f01", { ctx =>[ctx.function({ ctx, info =>
                let p0 = IM.fromJSValue(ctx, info[0])
                let p1 = Callback<EMStr>.fromJSValue(ctx, info[1])
                x(p0, p1)
                ctx.undefined().toJSValue()
            }).toJSValue()] })
    }
    /**
	 * @brief f1(): void
	 */
    public func f1(): Unit {
        jsObjApiCall < Unit >( arkts_object, "f1", emptyArg)
    }
    /**
	 * @brief f2(s: string): number
	 */
    public func f2(s: String): Float64 {
        jsObjApiCall < Float64 >( arkts_object, "f2", { ctx =>[s.toJSValue(ctx)] })
    }
    /**
	 * @brief f3(r: Record<string, string>): EMNum
	 */
    public func f3(r: HashMap<String, String>): EMNum {
        jsObjApiCall < EMNum >( arkts_object, "f3", { ctx =>[hashmap2Record<String>(ctx, r, { context, i => i.toJSValue(context) })] }) {
            ctx, info => EMNum.parse(Int32.fromJSValue(ctx, info))
        }
    }
    /**
	 * @brief f4(e: EMStr): (s: String) => void
	 */
    public func f4(e: EMStr): (String)->Unit {
        jsObjApiCall < (String)->Unit >( arkts_object, "f4", { ctx =>[e.get().toJSValue(ctx)] }) {
            ctx, info => (String)->Unit.fromJSValue(ctx, info)
        }
    }
    /**
	 * @brief f5(cb: (s: string, n: number) => string): void
	 */
    public func f5(cb: (s: String, n: Float64) -> String): Unit {
        jsObjApiCall < Unit >( arkts_object, "f5", { ctx =>[ctx.function({ ctx, info =>
                let p0 = String.fromJSValue(ctx, info[0])
                let p1 = Float64.fromJSValue(ctx, info[1])
                let ret = cb(p0, p1)
                ret.toJSValue(ctx)
            }).toJSValue()] })
    }
    /**
	 * @brief f6(evt: EMNum, cb: () => Record<number, Uint8Array>): boolean
	 */
    public func f6(evt: EMNum, cb: () -> HashMap<Float64, Array<UInt8>>): Bool {
        jsObjApiCall < Bool >( arkts_object, "f6", { ctx =>[evt.get().toJSValue(ctx), ctx.function({ ctx, _ =>
                let ret = cb()
                hashmap2Record<Array<UInt8>>(ctx, ret, { context, i => i.toJSValue(context) })
            }).toJSValue()] })
    }
    /**
	 * @brief f7(p: bigint, cb: (p: bigint) => void): bigint
	 */
    public func f7(p: BigInt, cb: (p: BigInt) -> Unit): BigInt {
        jsObjApiCall < BigInt >( arkts_object, "f7", { ctx =>[ctx.bigint(p).toJSValue(ctx), ctx.function({ ctx, info =>
                let p0 = info[0].asBigInt(ctx).toBigInt()
                cb(p0)
                ctx.undefined().toJSValue()
            }).toJSValue()] }) {
            ctx, info => BigInt.fromJSValue(ctx, info)
        }
    }
    /**
	 * @brief f8(result: string | Promise<string>): boolean
	 */
    public func f8(callback: AsyncCallback < Bool >, result: string|Promise<String>): Unit {
        jsObjApiCallPromise < Bool >( arkts_object, "f8", callback, { ctx =>[result.toJSValue(ctx)] })
    }
    /**
	 * @brief f8(result: string | Promise<string>): boolean
	 */
    public func f8(callback: AsyncCallback < Bool >, result: string|Promise<String>): Unit {
        jsObjApiCallPromise < Bool >( arkts_object, "f8", callback, { ctx =>[result] })
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): CM {
        CM(input.asObject())
    }
}

/*class DeclareKeyword TE {
    constructor(callback:AsyncCallback<string>, param1?:Array<UInt8>, param2?:Array<string>): void
    f1(): void
    f1(result: string | Promise<string>): Promise<String>
    }*/

public class TE {
    
    protected TE(var arkts_object: JSObject) {}
    /**
  * @brief constructor(callback:AsyncCallback<string>, param1?:Array<UInt8>, param2?:Array<string>): void
  */
    public init(callback: AsyncCallback<String>, param1!:?Array<UInt8> = None, param2!:?Array<String> = None) {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_methods", "TE").asClass(__ctx)
            clazz.new(jsCallback,(param1?.toJSValue(__ctx)) ?? __ctx.undefined().toJSValue(), toJSArray < String >(__ctx, param2, { __ctx, item => item.toJSValue(ctx) })).asObject()
        }
    }
    public init() {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_methods", "TE").asClass(__ctx)
            clazz.new().asObject()
        }
    }
    
    /**
	 * @brief f1(): void
	 */
    public func f1(): Unit {
        jsObjApiCall < Unit >( arkts_object, "f1", emptyArg)
    }
    /**
	 * @brief f1(result: string | Promise<string>): Promise<String>
	 */
    public func f1(callback: AsyncCallback < String >, result: string|Promise<String>): Unit {
        jsObjApiCallPromise < String >( arkts_object, "f1", callback, { ctx =>[result.toJSValue(ctx)] })
    }
    /**
	 * @brief f1(result: string | Promise<string>): Promise<String>
	 */
    public func f1(callback: AsyncCallback < String >, result: string|Promise<String>): Unit {
        jsObjApiCallPromise < String >( arkts_object, "f1", callback, { ctx =>[result] })
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): TE {
        TE(input.asObject())
    }
}
// 构造函数
/*class DeclareKeyword Greeter {
    constructor(greeting: string): void
    }*/

public class Greeter {
    
    protected Greeter(var arkts_object: JSObject) {}
    /**
	 * @brief constructor(greeting: string): void
	 */
    public init(greeting: String) {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_methods", "Greeter").asClass(__ctx)
            clazz.new(greeting.toJSValue(__ctx)).asObject()
        }
    }
    public init() {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_methods", "Greeter").asClass(__ctx)
            clazz.new().asObject()
        }
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): Greeter {
        Greeter(input.asObject())
    }
}

/*class DeclareKeyword Person {
    }*/

public class Person {
    
    protected Person(var arkts_object: JSObject) {}
    
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): Person {
        Person(input.asObject())
    }
}

/*class DeclareKeyword AnimalProtect {
    ProtectedKeywordname: String;
    makeSound(): void
    }*/

public class AnimalProtect {
    
    protected AnimalProtect(var arkts_object: JSObject) {}
    
    // 受保护属性
    public mut prop name: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, arkts_object["name"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["name"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    /**
	 * @brief makeSound(): void
	 */
	// 受保护方法
    public func makeSound(): Unit {
        jsObjApiCall < Unit >( arkts_object, "makeSound", emptyArg)
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): AnimalProtect {
        AnimalProtect(input.asObject())
    }
}

/*class DeclareKeyword Car {
    ReadonlyKeywordbrand: String;
    name: String;
    }*/

public class Car {
    
    protected Car(var arkts_object: JSObject) {}
    
    // 只读属性
    public prop brand: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, arkts_object["brand"])
            }
        }
    }
    
    public mut prop name: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, arkts_object["name"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["name"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): Car {
        Car(input.asObject())
    }
}
// 泛型成员
/*class DeclareKeyword Box {
    value: T;
    getValue(): T
    }*/

public class Box<T> where T <: JSInteropType<T> {
    
    protected Box(var arkts_object: JSObject) {}
    
    // 属性
    public mut prop value: T {
        get() {
            checkThreadAndCall < T >(getMainContext()) {
                ctx: JSContext => T.fromJSValue(ctx, arkts_object["value"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["value"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    /**
	 * @brief getValue(): T
	 */
	// 方法
    public func getValue(): T {
        jsObjApiCall < T >( arkts_object, "getValue", emptyArg) {
            ctx, info => T.fromJSValue(ctx, info)
        }
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): Box<T> {
        Box(input.asObject())
    }
}
// 抽象类
/*class DeclareKeyword FirstContextualKeyword Shape {
    getArea(): number
    }*/

public open class Shape {
    
    protected Shape(public var arkts_object: JSObject) {}
    
    
    /**
	 * @brief getArea(): number
	 */
	// 抽象方法
    public func getArea(): Float64 {
        jsObjApiCall < Float64 >( arkts_object, "getArea", emptyArg)
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): Shape {
        Shape(input.asObject())
    }
}

/*class DeclareKeyword Car1 {
    start(): void
    stop(): void
    }*/

public class Car1 <: Drivable {
    
    protected Car1(arkts_object: JSObject) { super(arkts_object) }
    
    
    /**
	 * @brief start(): void
	 */
    public func start(): Unit {
        jsObjApiCall < Unit >( arkts_object, "start", emptyArg)
    }
    /**
	 * @brief stop(): void
	 */
    public func stop(): Unit {
        jsObjApiCall < Unit >( arkts_object, "stop", emptyArg)
    }
    
    public func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): Car1 {
        Car1(input.asObject())
    }
}
// 继承类
/*class DeclareKeyword Animal {
    name: String;
    constructor(name: string): void
    move(distance: number): void
    }*/

public open class Animal {
    
    protected Animal(public var arkts_object: JSObject) {}
    /**
	 * @brief constructor(name: string): void
	 */
    public init(name: String) {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_methods", "Animal").asClass(__ctx)
            clazz.new(name.toJSValue(__ctx)).asObject()
        }
    }
    public init() {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_methods", "Animal").asClass(__ctx)
            clazz.new().asObject()
        }
    }
    
    public mut prop name: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, arkts_object["name"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["name"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    /**
	 * @brief move(distance: number): void
	 */
    public func move(distance: Float64): Unit {
        jsObjApiCall < Unit >( arkts_object, "move", { ctx =>[distance.toJSValue(ctx)] })
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): Animal {
        Animal(input.asObject())
    }
}
// Dog.d.ts
/*class DeclareKeyword Dog {
    bark(): void
    }*/

public class Dog <: Animal {
    
    protected Dog(arkts_object: JSObject) { super(arkts_object) }
    
    
    /**
	 * @brief bark(): void
	 */
    public func bark(): Unit {
        jsObjApiCall < Unit >( arkts_object, "bark", emptyArg)
    }
    
    public func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): Dog {
        Dog(input.asObject())
    }
}
// 重载方法
/*class DeclareKeyword Calculator {
    add(x: number, y: number): number
    add(x: string, y: string): String
    add(x: any, y: any): any
    }*/

public class Calculator {
    
    protected Calculator(var arkts_object: JSObject) {}
    
    
    /**
	 * @brief add(x: number, y: number): number
	 */
	// 方法重载
    public func add(x: Float64, y: Float64): Float64 {
        jsObjApiCall < Float64 >( arkts_object, "add", { ctx =>[x.toJSValue(ctx), y.toJSValue(ctx)] })
    }
    /**
	 * @brief add(x: string, y: string): String
	 */
    public func add(x: String, y: String): String {
        jsObjApiCall < String >( arkts_object, "add", { ctx =>[x.toJSValue(ctx), y.toJSValue(ctx)] })
    }
    /**
	 * @brief add(x: any, y: any): any
	 */
	// 实现
    public func add(x: Any, y: Any): Any {
        jsObjApiCall < Any >( arkts_object, "add", { ctx =>[x.toJSValue(ctx), y.toJSValue(ctx)] }) {
            ctx, info => Any.fromJSValue(ctx, info)
        }
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): Calculator {
        Calculator(input.asObject())
    }
}

/*class Decorator DeclareKeyword MyClass {
    name: String;
    constructor(name: string): void
    }*/

public class MyClass {
    
    protected MyClass(var arkts_object: JSObject) {}
    /**
	 * @brief constructor(name: string): void
	 */
    public init(name: String) {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_methods", "MyClass").asClass(__ctx)
            clazz.new(name.toJSValue(__ctx)).asObject()
        }
    }
    public init() {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_methods", "MyClass").asClass(__ctx)
            clazz.new().asObject()
        }
    }
    
    public mut prop name: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, arkts_object["name"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["name"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): MyClass {
        MyClass(input.asObject())
    }
}

/*class ExportKeyword DeclareKeyword UMConfig {
    appKey?: String;
    channel?: String;
    enableLog?: boolean;
    }*/

public class UMConfig {
    
    protected UMConfig(var arkts_object: JSObject) {}
    
    
    public mut prop appKey: ?String {
        get() {
            checkThreadAndCall < ?String >(getMainContext()) {
                ctx: JSContext => Option < String >.fromJSValue(ctx, arkts_object["appKey"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["appKey"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop channel: ?String {
        get() {
            checkThreadAndCall < ?String >(getMainContext()) {
                ctx: JSContext => Option < String >.fromJSValue(ctx, arkts_object["channel"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["channel"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop enableLog: ?Bool {
        get() {
            checkThreadAndCall < ?Bool >(getMainContext()) {
                ctx: JSContext => Option < Bool >.fromJSValue(ctx, arkts_object["enableLog"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["enableLog"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): UMConfig {
        UMConfig(input.asObject())
    }
}

/*class ExportKeyword DeclareKeyword RecordTest {
    recordTest: Record<String, String>;
    recordTest1: Record<String, String | number>;
    recordTest2: Record<String, String> | null;
    recordTest3: Record<String, String | number> | null;
    }*/

public class RecordTest {
    
    protected RecordTest(var arkts_object: JSObject) {}
    
    
    public mut prop recordTest: HashMap<String, String> {
        get() {
            checkThreadAndCall < HashMap<String, String> >(getMainContext()) {
                ctx: JSContext => record2Hashmap<String>(ctx, arkts_object["recordTest"], { context, i => String.fromJSValue(context, i) })
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["recordTest"] = hashmap2Record<String>(ctx, v, { context, i => i.toJSValue(context) })
            }
        }
        
    }
    
    public mut prop recordTest1: HashMap<String, JSValue/* FIXME: `string | number` */> {
        get() {
            checkThreadAndCall < HashMap<String, JSValue/* FIXME: `string | number` */> >(getMainContext()) {
                ctx: JSContext => record2Hashmap<JSValue/* FIXME: `string | number` */>(ctx, arkts_object["recordTest1"], { context, i => JSValue/* FIXME: `string | number` */.fromJSValue(context, i) })
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["recordTest1"] = hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, v, { context, i => i.toJSValue(context) })
            }
        }
        
    }
    
    public mut prop recordTest2: Option<HashMap<String, String>> {
        get() {
            checkThreadAndCall < Option<HashMap<String, String>> >(getMainContext()) {
                ctx: JSContext => record2Hashmap<String>(ctx, arkts_object["recordTest2"], { context, i => String.fromJSValue(context, i) })
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["recordTest2"] = hashmap2Record<String>(ctx, v, { context, i => i.toJSValue(context) })
            }
        }
        
    }
    
    public mut prop recordTest3: Option<HashMap<String, JSValue/* FIXME: `string | number` */>> {
        get() {
            checkThreadAndCall < Option<HashMap<String, JSValue/* FIXME: `string | number` */>> >(getMainContext()) {
                ctx: JSContext => record2Hashmap<JSValue/* FIXME: `string | number` */>(ctx, arkts_object["recordTest3"], { context, i => JSValue/* FIXME: `string | number` */.fromJSValue(context, i) })
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["recordTest3"] = hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, v, { context, i => i.toJSValue(context) })
            }
        }
        
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): RecordTest {
        RecordTest(input.asObject())
    }
}

/***********METHOD***********/
/**
	 * @brief test(): Promise<String>
	 */
public func test(callback: AsyncCallback < String >): Unit {
    hmsGlobalApiCallPromise < String >( "my_module_methods", "test", callback, emptyArg)
}

/**
	 * @brief onEnumCb(p1: number | boolean, p2: Record<string, number> | Uint8Array | string): String
	 */
public func onEnumCb(p1: Float64, p2: HashMap<String, Float64>): String {
    hmsGlobalApiCall < String >( "my_module_methods", "onEnumCb", { ctx =>[p1.toJSValue(ctx), hashmap2Record<Float64>(ctx, p2, { context, i => i.toJSValue(context) })] })
}

/**
	 * @brief onEnumCb(p1: number | boolean, p2: Record<string, number> | Uint8Array | string): String
	 */
public func onEnumCb(p1: Float64, p2: Array<UInt8>): String {
    hmsGlobalApiCall < String >( "my_module_methods", "onEnumCb", { ctx =>[p1.toJSValue(ctx), toJSArray < UInt8 >(ctx, p2, { ctx: JSContext, val: UInt8 => val.toJSValue(ctx) })] })
}

/**
	 * @brief onEnumCb(p1: number | boolean, p2: Record<string, number> | Uint8Array | string): String
	 */
public func onEnumCb(p1: Float64, p2: String): String {
    hmsGlobalApiCall < String >( "my_module_methods", "onEnumCb", { ctx =>[p1.toJSValue(ctx), p2.toJSValue(ctx)] })
}

/**
	 * @brief onEnumCb(p1: number | boolean, p2: Record<string, number> | Uint8Array | string): String
	 */
public func onEnumCb(p1: Bool, p2: HashMap<String, Float64>): String {
    hmsGlobalApiCall < String >( "my_module_methods", "onEnumCb", { ctx =>[p1.toJSValue(ctx), hashmap2Record<Float64>(ctx, p2, { context, i => i.toJSValue(context) })] })
}

/**
	 * @brief onEnumCb(p1: number | boolean, p2: Record<string, number> | Uint8Array | string): String
	 */
public func onEnumCb(p1: Bool, p2: Array<UInt8>): String {
    hmsGlobalApiCall < String >( "my_module_methods", "onEnumCb", { ctx =>[p1.toJSValue(ctx), toJSArray < UInt8 >(ctx, p2, { ctx: JSContext, val: UInt8 => val.toJSValue(ctx) })] })
}

/**
	 * @brief onEnumCb(p1: number | boolean, p2: Record<string, number> | Uint8Array | string): String
	 */
public func onEnumCb(p1: Bool, p2: String): String {
    hmsGlobalApiCall < String >( "my_module_methods", "onEnumCb", { ctx =>[p1.toJSValue(ctx), p2.toJSValue(ctx)] })
}

/**
	 * @brief logClass(target: any): void
	 */
	// 带装饰器的类
public func logClass(target: Any): Unit {
    hmsGlobalApiCall < Unit >( "my_module_methods", "logClass", { ctx =>[target.toJSValue(ctx)] })
}

/**
	 * @brief getFavoriteNumber(t: T): T
	 */
	// 泛型
public func getFavoriteNumber < T >(t: T): T {
    hmsGlobalApiCall < T >( "my_module_methods", "getFavoriteNumber", { ctx =>[t.toJSValue(ctx)] }) {
        ctx, info => T.fromJSValue(ctx, info)
    }
}

/**
	 * @brief test(): Promise<void>
	 */
public func test(callback: AsyncCallback < Unit >): Unit {
    hmsGlobalApiCallPromise < Unit >( "my_module_methods", "test", callback, emptyArg)
}

/**
	 * @brief test1(name:string): Promise<void>
	 */
public func test1(callback: AsyncCallback < Unit >, name: String): Unit {
    hmsGlobalApiCallPromise < Unit >( "my_module_methods", "test1", callback, { ctx =>[name.toJSValue(ctx)] })
}

/**
	 * @brief testUnion1(t: string | undefined): undefined | String
	 */
public func testUnion1(t: Option<String>): Option<String> {
    hmsGlobalApiCall < Option<String> >( "my_module_methods", "testUnion1", { ctx =>[t.toJSValue(ctx)] }) {
        ctx, info => Option<String>.fromJSValue(ctx, info)
    }
}

/**
	 * @brief testUnion2(t: null | T, s: A | B): null | String
	 */
public func testUnion2(t: Option<T>, s: A): Option<String> {
    hmsGlobalApiCall < Option<String> >( "my_module_methods", "testUnion2", { ctx =>[t.toJSValue(ctx), s.toJSValue(ctx)] }) {
        ctx, info => Option<String>.fromJSValue(ctx, info)
    }
}

/**
	 * @brief testUnion2(t: null | T, s: A | B): null | String
	 */
public func testUnion2(t: Option<T>, s: B): Option<String> {
    hmsGlobalApiCall < Option<String> >( "my_module_methods", "testUnion2", { ctx =>[t.toJSValue(ctx), s.toJSValue(ctx)] }) {
        ctx, info => Option<String>.fromJSValue(ctx, info)
    }
}

/**
	 * @brief testUnion3(t: number[] | null): null | number[]
	 */
public func testUnion3(t: Option<Array<Float64>>): Option<Array<Float64>> {
    hmsGlobalApiCall < Option<Array<Float64>> >( "my_module_methods", "testUnion3", { ctx =>[toJSArray < Float64 >(ctx, t)] }) {
        ctx, info => Option<Array<Float64>>.fromJSValue(ctx, info)
    }
}

/**
	 * @brief testUnion4(t: Record<string, Uint8Array>| null): void
	 */
public func testUnion4(t: Option<HashMap<String, Array<UInt8>>>): Unit {
    hmsGlobalApiCall < Unit >( "my_module_methods", "testUnion4", { ctx =>[hashmap2Record<Array<UInt8>>(ctx, t, { context, i => i.toJSValue(context) })] })
}

/**
	 * @brief RecordAndOptionalTest(eventID: string, params?: Record<string, string | number>): void
	 */
public func RecordAndOptionalTest(eventID: String, params!: Option < HashMap<String, JSValue/* FIXME: `string | number` */> >= None): Unit {
    hmsGlobalApiCall < Unit >( "my_module_methods", "RecordAndOptionalTest", { ctx =>[eventID.toJSValue(ctx), hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, params, { context, i => i.toJSValue(context) })] })
}

/**
	 * @brief RecordTest(eventID: string, params: Record<string, string | number>): void
	 */
public func RecordTest(eventID: String, params: HashMap<String, JSValue/* FIXME: `string | number` */>): Unit {
    hmsGlobalApiCall < Unit >( "my_module_methods", "RecordTest", { ctx =>[eventID.toJSValue(ctx), hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, params, { context, i => i.toJSValue(context) })] })
}

/**
	 * @brief RecordAndOptionalTest1(eventID: string, params?: Record<string, string | number>, params1?: Record<string, string | number>): void
	 */
public func RecordAndOptionalTest1(eventID: String, params!: Option < HashMap<String, JSValue/* FIXME: `string | number` */> >= None, params1!: Option < HashMap<String, JSValue/* FIXME: `string | number` */> >= None): Unit {
    hmsGlobalApiCall < Unit >( "my_module_methods", "RecordAndOptionalTest1", { ctx =>[eventID.toJSValue(ctx), hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, params, { context, i => i.toJSValue(context) }), hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, params1, { context, i => i.toJSValue(context) })] })
}

/**
	 * @brief RecordTest1(eventID: string, params: Record<string, string | number>, params1: Record<string, string | number>): void
	 */
public func RecordTest1(eventID: String, params: HashMap<String, JSValue/* FIXME: `string | number` */>, params1: HashMap<String, JSValue/* FIXME: `string | number` */>): Unit {
    hmsGlobalApiCall < Unit >( "my_module_methods", "RecordTest1", { ctx =>[eventID.toJSValue(ctx), hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, params, { context, i => i.toJSValue(context) }), hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, params1, { context, i => i.toJSValue(context) })] })
}

/**
	 * @brief RecordAndOptionalTest2(eventID: string, params?: Record<string, string | number>, params1?: Record<string, string | number>): void
	 */
public func RecordAndOptionalTest2(eventID: String, params!: Option < HashMap<String, JSValue/* FIXME: `string | number` */> >= None, params1!: Option < HashMap<String, JSValue/* FIXME: `string | number` */> >= None): Unit {
    hmsGlobalApiCall < Unit >( "my_module_methods", "RecordAndOptionalTest2", { ctx =>[eventID.toJSValue(ctx), hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, params, { context, i => i.toJSValue(context) }), hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, params1, { context, i => i.toJSValue(context) })] })
}

/**
	 * @brief RecordTest2(eventID: string, params: Record<string, number>, params1: Record<string, number>): void
	 */
public func RecordTest2(eventID: String, params: HashMap<String, Float64>, params1: HashMap<String, Float64>): Unit {
    hmsGlobalApiCall < Unit >( "my_module_methods", "RecordTest2", { ctx =>[eventID.toJSValue(ctx), hashmap2Record<Float64>(ctx, params, { context, i => i.toJSValue(context) }), hashmap2Record<Float64>(ctx, params1, { context, i => i.toJSValue(context) })] })
}

/**
	 * @brief RecordAndOptionalTest3(eventID: string, params: Record<string, string | number> | null, params1: Record<string, string | number>|undefined): void
	 */
public func RecordAndOptionalTest3(eventID: String, params: Option<HashMap<String, JSValue/* FIXME: `string | number` */>>, params1: Option<HashMap<String, JSValue/* FIXME: `string | number` */>>): Unit {
    hmsGlobalApiCall < Unit >( "my_module_methods", "RecordAndOptionalTest3", { ctx =>[eventID.toJSValue(ctx), hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, params, { context, i => i.toJSValue(context) }), hashmap2Record<JSValue/* FIXME: `string | number` */>(ctx, params1, { context, i => i.toJSValue(context) })] })
}


