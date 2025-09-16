// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package pkg1
class BB {
    var a = 1
}

class AA{
    static var a = 1
    static var b = BB()
    var a1 = 1
    var b1 = BB()
}
type SS = AA
type SS1 = SS
var bb: SS = SS()
var aa: pkg1.SS = SS()

main() {
   AA.a
   pkg1.AA.a
   let A = SS
   SS.a
   pkg1.SS.a 
}
