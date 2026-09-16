// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/***********ENUM***********/
// evil_enum_inject() */
/* enum EvilEnum {
	 EvilMember = 0,
	 Normal = 1
	}*/
public enum EvilEnum <: ToString & Equatable < EvilEnum > {
    | EvilMember
    | Normal 

    func get(): Int32 {
        match(this) {
            case EvilMember => 0 
            case Normal => 1 
        }
    }
    
    static func parse(val: Int32): EvilEnum {
        match(val) {
            case 0 => EvilMember 
            case 1 => Normal 
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?Int32): ?EvilEnum {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        match(this) {
            case EvilMember => "EvilMember"
            case Normal => "Normal"
        }
    }
    
    public override operator func ==(that: EvilEnum): Bool {
        match((this, that)) {
            case(EvilMember, EvilMember) => true
            case(Normal, Normal) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: EvilEnum): Bool {
        !(this == that)
    }
}


/***********OBJECT***********/
// 测试用例：注释注入漏洞修复验证
// 覆盖路径：
//   - trans_object.cj generate() → escapeRawComment(arkComment)
//   - trans_object.cj genPropDeclare → escapeRawComment(arkProp.comment)
//   - trans_enum.cj generate() → escapeRawComment(comment)
// 攻击载荷：interface/enum/属性行注释中包含 */ 
// evil_interface_inject() */
/*interface ExportKeyword IObjectInjection {
    evilProperty: String;
    anotherProperty: number;
    }*/

public open class IObjectInjection {
    public var evilProperty: String
    public var anotherProperty: Float64
    protected init(
    evilProperty: String,
    anotherProperty: Float64) {
        this.evilProperty = evilProperty
        this.anotherProperty = anotherProperty
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["evilProperty"] = evilProperty.toJSValue(context)
        obj["anotherProperty"] = anotherProperty.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): IObjectInjection {
        let obj = input.asObject()
        IObjectInjection(
        String.fromJSValue(context, obj["evilProperty"]),
        Float64.fromJSValue(context, obj["anotherProperty"])
        )
    }
}


