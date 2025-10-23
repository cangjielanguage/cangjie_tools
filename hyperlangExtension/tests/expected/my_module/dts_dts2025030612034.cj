// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

public enum MyUnion8 {
    | _STRING_NUMBER___(JSValue/* FIXME: `(string | number)[]` */)
    | _KEY_STRING_(JSValue/* FIXME: `{ key: string }` */)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case _STRING_NUMBER___(x) => x
            case _KEY_STRING_(x) => x
        }
    }
}


