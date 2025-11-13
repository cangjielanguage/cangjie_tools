// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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
/*import { a } from '@umeng/common';*/

/*FIXME: Import details need to be verified and rewritten by user.*/
/*import buffer from '@ohos.buffer';*/

/*FIXME: Import details need to be verified and rewritten by user.*/
/*import { e } from "../g/h";*/

/*FIXME: Import details need to be verified and rewritten by user.*/
/*import { MyStringEnum, MyNumericEnum } from './exportAlias';*/

/*FIXME: Import details need to be verified and rewritten by user.*/
/*import * as Inheritances from './inheritances';*/

/*FIXME: Import details need to be verified and rewritten by user.*/
/*import { ExportedInterface } from './exportAlias';*/


/*

public const value1 = 0/* FIXME: Initialization is required */
*/
/*

public const value2 = 0/* FIXME: Initialization is required */
*/

/***********METHOD***********/
/**
	 * @brief createSub(): Inheritances.SubClass
	 */
public func createSub(): JSValue/* FIXME: `Inheritances.SubClass` */ {
    hmsGlobalApiCall < JSValue/* FIXME: `Inheritances.SubClass` */ >( "my_module_imports", "createSub", emptyArg) {
        ctx, info => info
    }
}


/***********OBJECT***********/

/*interface ExportedInterface {
    myOption?: String;
    }*/

public open class ExportedInterface {
    
    protected ExportedInterface(private var myOption!: Option<String> = None) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        if(let Some(v) <- myOption) {
            obj["myOption"] = v.toJSValue(context)
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): ExportedInterface {
        let obj = input.asObject()
        ExportedInterface(
        myOption: Option < String >.fromJSValue(context, obj["myOption"])
        )
    }
    
}

