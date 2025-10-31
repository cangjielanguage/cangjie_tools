// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
//构造函数
/*class DeclareKeyword Greeter {
    constructor(greeting : string): void
    }*/

public class Greeter {
    
    protected Greeter(var arkts_object: JSObject) {}
    /**
	 * @brief constructor(greeting : string): void
	 */
    public init(greeting : String) {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_valueOfModulename", "Greeter").toJSValue().asClass(__ctx)
            clazz.new(greeting.toJSValue(__ctx)).asObject()
        }
    }
    public init() {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_valueOfModulename", "Greeter").toJSValue().asClass(__ctx)
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

