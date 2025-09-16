// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testpkg01
import testpkg.*

func semanticHighlight_test002(){
    var value:pkg = pkg(1)
    var a:int32 = value.add(1,2)
    var b:int32 = pkg.add3(1,2)
    var c:int32 = pkg.aa
    var d:int32 = value.aaa
}
