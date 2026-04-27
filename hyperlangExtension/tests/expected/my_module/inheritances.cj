// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
/***********IMPORT***********/
/*FIXME: Import details need to be verified and rewritten by user.*/
/*import {p29} from "./superInterface"*/



/*interface AutoGenType0Ininheritances {
    }*/

public open class AutoGenType0Ininheritances {
    protected init() {
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0Ininheritances {
        let obj = input.asObject()
        AutoGenType0Ininheritances(
        
        )
    }
}


/*interface AutoGenType1Ininheritances {
    }*/

public open class AutoGenType1Ininheritances {
    protected init() {
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType1Ininheritances {
        let obj = input.asObject()
        AutoGenType1Ininheritances(
        
        )
    }
}


/*interface AutoGenType2Ininheritances {
    }*/

public open class AutoGenType2Ininheritances {
    protected init() {
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType2Ininheritances {
        let obj = input.asObject()
        AutoGenType2Ininheritances(
        
        )
    }
}

/***********OBJECT***********/

/*interface A {
    p: number;
    }*/

public open class A {
    public var p: Float64
    protected init(p: Float64) {
        this.p = p
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["p"] = p.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): A {
        let obj = input.asObject()
        A(
        Float64.fromJSValue(context, obj["p"])
        )
    }
}


/*interface B {
    p1: number;
    }*/

public open class B <: A {
    public var p1: Float64
    protected init(p: Float64,
    p1: Float64) {
        super( p )
        this.p1 = p1
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["p"] = p.toJSValue(context)
        obj["p1"] = p1.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): B {
        let obj = input.asObject()
        B(
        Float64.fromJSValue(context, obj["p"]),
        Float64.fromJSValue(context, obj["p1"])
        )
    }
}


/*interface ExportKeyword BuglyLogAdapter {
    }*/

public open class BuglyLogAdapter <: p29 {
    protected BuglyLogAdapter(arkts_object: JSObject) { super(arkts_object) }
    
    
    
    public open func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public static func fromJSValue(_: JSContext, input: JSValue): BuglyLogAdapter {
        BuglyLogAdapter(input.asObject())
    }
}

/*interface C {
    f(): void
    }*/

public open class C {
    protected C(var arkts_object: JSObject) { }
    
    
    /**
	 * @brief f(): void
	 */
    public func f(): Unit {
        jsObjApiCall < Unit >( arkts_object, "f", EMPTY_ARG)
    }
    
    public open func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public static func fromJSValue(_: JSContext, input: JSValue): C {
        C(input.asObject())
    }
}

/*interface D {
    }*/

public open class D <: C {
    protected D(arkts_object: JSObject) { super(arkts_object) }
    
    
    
    public open func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public static func fromJSValue(_: JSContext, input: JSValue): D {
        D(input.asObject())
    }
}

/*interface E {
    }*/

public open class E <: A {
    protected init(p: Float64) {
        super( p )
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["p"] = p.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): E {
        let obj = input.asObject()
        E(
        Float64.fromJSValue(context, obj["p"])
        )
    }
}


/*interface F {
    g(): void
    }*/

public open class F <: C {
    protected F(arkts_object: JSObject) { super(arkts_object) }
    
    
    /**
	 * @brief g(): void
	 */
    public func g(): Unit {
        jsObjApiCall < Unit >( arkts_object, "g", EMPTY_ARG)
    }
    
    public open func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public static func fromJSValue(_: JSContext, input: JSValue): F {
        F(input.asObject())
    }
}

/*interface ExportKeyword INamespace {
    options?: { [k: String]: any };
    nested?: { [k: String]: AnyNestedObject };
    }*/

public open class INamespace {
    public var options: Option<AutoGenType0Ininheritances> = None
    public var nested: Option<AutoGenType1Ininheritances> = None
    protected init(
    options!: Option<AutoGenType0Ininheritances> = None,
    nested!: Option<AutoGenType1Ininheritances> = None) {
        this.options = options
        this.nested = nested
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        if (let Some(v) <- options) {
            obj["options"] = v.toJSValue(context)
        }
        if (let Some(v) <- nested) {
            obj["nested"] = v.toJSValue(context)
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): INamespace {
        let obj = input.asObject()
        INamespace(
        options:
        if (obj["options"].isNull()) {
            None < AutoGenType0Ininheritances >
        } else {
            AutoGenType0Ininheritances.fromJSValue(context, obj["options"])
        },
        nested:
        if (obj["nested"].isNull()) {
            None < AutoGenType1Ininheritances >
        } else {
            AutoGenType1Ininheritances.fromJSValue(context, obj["nested"])
        }
        )
    }
}


/*interface ExportKeyword IService {
    methods: { [k: String]: IMethod };
    }*/

public open class IService <: INamespace {
    public var methods: AutoGenType2Ininheritances
    protected init(methods: AutoGenType2Ininheritances,
    options!: Option<AutoGenType0Ininheritances> = None,
    nested!: Option<AutoGenType1Ininheritances> = None) {
        super( options:  options , nested:  nested )
        this.methods = methods
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["methods"] = methods.toJSValue(context)
        if (let Some(v) <- options) {
            obj["options"] = v.toJSValue(context)
        }
        if (let Some(v) <- nested) {
            obj["nested"] = v.toJSValue(context)
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): IService {
        let obj = input.asObject()
        IService(
        AutoGenType2Ininheritances.fromJSValue(context, obj["methods"]),
        options:
        if (obj["options"].isNull()) {
            None < AutoGenType0Ininheritances >
        } else {
            AutoGenType0Ininheritances.fromJSValue(context, obj["options"])
        },
        nested:
        if (obj["nested"].isNull()) {
            None < AutoGenType1Ininheritances >
        } else {
            AutoGenType1Ininheritances.fromJSValue(context, obj["nested"])
        }
        )
    }
}


/*interface ExportKeyword DeclareKeyword SuperInterface {
    p: number;
    }*/

public open class SuperInterface {
    public var p: Float64
    protected init(p: Float64) {
        this.p = p
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["p"] = p.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): SuperInterface {
        let obj = input.asObject()
        SuperInterface(
        Float64.fromJSValue(context, obj["p"])
        )
    }
}


/*class ExportKeyword DeclareKeyword SubClass {
    p: number;
    }*/

public class SubClass <: SuperClass {
    protected SubClass(arkts_object: JSObject) { super(arkts_object) }
    
    
    
    public func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public static func fromJSValue(_: JSContext, input: JSValue): SubClass {
        SubClass(input.asObject())
    }
}

/*class ExportKeyword DeclareKeyword SubClass1 {
    p: number;
    }*/

public class SubClass1 <: SuperClass1 {
    protected SubClass1(arkts_object: JSObject) { super(arkts_object) }
    
    
    
    public func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public static func fromJSValue(_: JSContext, input: JSValue): SubClass1 {
        SubClass1(input.asObject())
    }
}

/*class ExportKeyword DeclareKeyword SubInterface {
    p: number;
    p1: number;
    }*/

public class SubInterface <: SuperInterface {
    protected SubInterface(var arkts_object: JSObject) {
        super(Float64.fromJSValue(getMainContext(), arkts_object["p"]))
    }
    
    
    public mut prop p1: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, arkts_object["p1"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["p1"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    public func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public static func fromJSValue(_: JSContext, input: JSValue): SubInterface {
        SubInterface(input.asObject())
    }
}

/*class ExportKeyword DeclareKeyword FirstContextualKeyword SuperClass {
    p: number;
    }*/

public open class SuperClass {
    protected SuperClass(var arkts_object: JSObject) { }
    
    
    public mut prop p: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, arkts_object["p"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["p"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    public open func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public static func fromJSValue(_: JSContext, input: JSValue): SuperClass {
        SuperClass(input.asObject())
    }
}

/*class ExportKeyword DeclareKeyword SuperClass1 {
    p: number;
    }*/

public open class SuperClass1 {
    protected SuperClass1(var arkts_object: JSObject) { }
    
    
    public mut prop p: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, arkts_object["p"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["p"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    public open func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public static func fromJSValue(_: JSContext, input: JSValue): SuperClass1 {
        SuperClass1(input.asObject())
    }
}

