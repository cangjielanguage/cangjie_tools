// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testpkg

public func pkg_test01(): int32 {
    let a = one
    let b = two
    var c:int32 = pkg.aa
    var p:pkg = pkg(1)
    var q:pkg = pkg(2)
    let res = pkg.add3(1,2)
    var cc:int32 = p.aaa
    var dd:int32 = testpkgFunc()
    if res != 3 {
        return p.add(1,2)
    }
    return p.add(1,2)
}
