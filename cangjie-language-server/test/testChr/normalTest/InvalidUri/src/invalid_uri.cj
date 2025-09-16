// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created on 2024/9/11
 */
package ohos_app_cangjie_entry

import stdx.serialization.serialization.*
import std.ast.*
import std.collection.ArrayList

func serializeClass(cd: ClassDecl): Tokens {
    let modifiers = cd.modifiers.toTokens()
    let identifier = cd.identifier
    var superTypes = Tokens()
    var body = Tokens()
    let allFields = ArrayList<(Token, TypeNode, Bool)>() // All member variables and its' type info.

    // Add 'Serializable<T>' to its super types.
    if (cd.superTypes.size == 0) {
        superTypes = quote(Serializable<$(identifier)>)
    } else {
        superTypes = quote($(cd.superTypes) & Serializable<$(identifier)>)
    }

    return quote(
        $modifiers class $identifier <: $superTypes {
            $body
        }
    )
}
