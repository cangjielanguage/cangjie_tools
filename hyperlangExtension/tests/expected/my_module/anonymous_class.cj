// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/*class ExportKeyword DeclareKeyword AnonymousClass {
    f1(p: {n: number}): {n: number}
    f2(n: number): Promise<String>
    f3(n: number): Promise<{
        msg: String;
    }>
    f4(): Promise<{
        msg: String;
        data: String;
        code?: undefined;
    } | {
        code: number;
        msg: String;
        data?: undefined;
    }>
    }*/

public class AnonymousClass {
    
    protected AnonymousClass(var arkts_object: JSObject) {}
    
    
    /**
	 * @brief f1(p: {n: number}): {n: number}
	 */
    public func f1(p: JSValue/* FIXME: `{ n: number }` */): JSValue/* FIXME: `{n: number}` */ {
        jsObjApiCall < JSValue/* FIXME: `{n: number}` */ >( arkts_object, "f1", { ctx =>[p] }) {
            ctx, info => info
        }
    }
    /**
	 * @brief f2(n: number): Promise<String>
	 */
    public func f2(callback: AsyncCallback < String >, n: Float64): Unit {
        jsObjApiCallPromise < String >( arkts_object, "f2", callback, { ctx =>[n.toJSValue(ctx)] })
    }
    /**
	 * @brief f3(n: number): Promise<{
        msg: String;
    }>
	 */
    public func f3(callback: AsyncCallback < JSValue/* FIXME: `{ msg: String; }` */ >, n: Float64): Unit {
        jsObjApiCallPromise < JSValue/* FIXME: `{ msg: String; }` */ >( arkts_object, "f3", callback, { ctx =>[n.toJSValue(ctx)] }) {
            ctx, info => info
        }
    }
    /**
	 * @brief f4(): Promise<{
        msg: String;
        data: String;
        code?: undefined;
    } | {
        code: number;
        msg: String;
        data?: undefined;
    }>
	 */
    public func f4(callback: AsyncCallback < JSValue/* FIXME: `{ msg: String; data: String; code?: undefined; } | { code: number; msg: String; data?: undefined; }` */ >): Unit {
        jsObjApiCallPromise < JSValue/* FIXME: `{ msg: String; data: String; code?: undefined; } | { code: number; msg: String; data?: undefined; }` */ >( arkts_object, "f4", callback, emptyArg) {
            ctx, info => info
        }
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): AnonymousClass {
        AnonymousClass(input.asObject())
    }
}

