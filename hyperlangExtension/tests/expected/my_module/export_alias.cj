// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*


public const MY_VALUE = 123

public const EXPORTED_VALUE = "abc"

/***********ENUM***********/

/* enum MyStringEnum {
	 A = aaa
	}*/
public enum MyStringEnum <: ToString & Equatable < MyStringEnum > {
    | A 

    func get(): String {
        match(this) {
            case A => "aaa" 
        }
    }
    
    static func parse(val: String): MyStringEnum {
        match(val) {
            case "aaa" => A 
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?String): ?MyStringEnum {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        get()
    }
    
    public override operator func ==(that: MyStringEnum): Bool {
        match((this, that)) {
            case(A, A) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: MyStringEnum): Bool {
        !(this == that)
    }
}


/* enum MyNumericEnum {
	 A = 111
	}*/
public enum MyNumericEnum <: ToString & Equatable < MyNumericEnum > {
    | A 

    func get(): Int32 {
        match(this) {
            case A => 111 
        }
    }
    
    static func parse(val: Int32): MyNumericEnum {
        match(val) {
            case 111 => A 
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?Int32): ?MyNumericEnum {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        match(this) {
            case A => "A"
        }
    }
    
    public override operator func ==(that: MyNumericEnum): Bool {
        match((this, that)) {
            case(A, A) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: MyNumericEnum): Bool {
        !(this == that)
    }
}


/***********OBJECT***********/

/*interface MyInterface {
    }*/

public open class MyInterface {
    
    protected MyInterface() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): MyInterface {
        let obj = input.asObject()
        MyInterface(
        
        )
    }
    
}

/*interface ExportedInterface {
    }*/

public open class ExportedInterface {
    
    protected ExportedInterface() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): ExportedInterface {
        let obj = input.asObject()
        ExportedInterface(
        
        )
    }
    
}

/*interface ExportKeyword I3 {
    }*/

public open class I3 {
    
    protected I3() { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): I3 {
        let obj = input.asObject()
        I3(
        
        )
    }
    
}

/*class MyClass {
    }*/

public class MyClass {
    
    protected MyClass(var arkts_object: JSObject) { }
    
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
     static func fromJSValue(context: JSContext, input: JSValue): MyClass {
        MyClass(input.asObject())
    }
}

/*class ExportedClass {
    }*/

public class ExportedClass {
    
    protected ExportedClass(var arkts_object: JSObject) { }
    
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
     static func fromJSValue(context: JSContext, input: JSValue): ExportedClass {
        ExportedClass(input.asObject())
    }
}

/***********METHOD***********/
/**
	 * @brief aliasedFunc(): void
	 */
public func aliasedFunc(): Unit {
    hmsGlobalApiCall < Unit >( "my_module_exportAlias", "aliasedFunc", emptyArg)
}

/**
	 * @brief anotherFunc(): void
	 */
public func anotherFunc(): Unit {
    hmsGlobalApiCall < Unit >( "my_module_exportAlias", "anotherFunc", emptyArg)
}


