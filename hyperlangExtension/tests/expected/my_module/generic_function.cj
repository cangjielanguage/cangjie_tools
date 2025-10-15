// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
/***********METHOD***********/
/**
	 * @brief testGeneric(t: T): T
	 */
public func testGeneric < T >(t: T): T {
    hmsGlobalApiCall < T >( "my_module_genericFunction", "testGeneric", { ctx =>[t.toJSValue(ctx)] }) {
        ctx, info => T.fromJSValue(ctx, info)
    }
}

/**
	 * @brief testMultiGenericT(t: T, m: M): T
	 */
public func testMultiGenericT < T, M >(t: T, m: M): T {
    hmsGlobalApiCall < T >( "my_module_genericFunction", "testMultiGenericT", { ctx =>[t.toJSValue(ctx), m.toJSValue(ctx)] }) {
        ctx, info => T.fromJSValue(ctx, info)
    }
}

/**
	 * @brief testMultiGenericM(t: T, m: M): M
	 */
public func testMultiGenericM < T, M >(t: T, m: M): M {
    hmsGlobalApiCall < M >( "my_module_genericFunction", "testMultiGenericM", { ctx =>[t.toJSValue(ctx), m.toJSValue(ctx)] }) {
        ctx, info => M.fromJSValue(ctx, info)
    }
}

/**
	 * @brief testNoGeneric(t: T): T
	 */
public func testNoGeneric(t: T): T {
    hmsGlobalApiCall < T >( "my_module_genericFunction", "testNoGeneric", { ctx =>[t.toJSValue(ctx)] }) {
        ctx, info => T.fromJSValue(ctx, info)
    }
}

/**
	 * @brief init(type: string, data: any): void
	 */
public func _init(`type`: String, data: Any): Unit {
    hmsGlobalApiCall < Unit >( "my_module_genericFunction", "init", { ctx =>[`type`.toJSValue(ctx), data.toJSValue(ctx)] })
}

/**
	 * @brief getObject(extra: number, info: string): void
	 */
public func _getObject(extra: Float64, info: String): Unit {
    hmsGlobalApiCall < Unit >( "my_module_genericFunction", "getObject", { ctx =>[extra.toJSValue(ctx), info.toJSValue(ctx)] })
}


