// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.util

import stdx.serialization.serialization.*

public interface DataModelStructExtends {
    func accessDeep(args: Array<String>): DataModel
}

extend DataModelStruct <: DataModelStructExtends {
    public func accessDeep(args: Array<String>): DataModel {
        var curr: DataModelStruct = this
        let iterator = args.iterator()
        var currArg = iterator.next() ?? return curr

        while (let Some(newArg) <- iterator.next()) {
            curr = curr.get(currArg) as DataModelStruct ?? return DataModelNull()
            currArg = newArg
        }

        curr.get(currArg)
    }
}
