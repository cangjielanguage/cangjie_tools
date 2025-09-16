// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package pkg2

import pkg1.*

// function
func test(a: Int64, pkg2_test1_param1_noDefaultValue!: Int64, pkg2_test1_param1_hasDefaultValue!: Int64 = 1)
{}

func test(a: Int64, pkg2_test1_param2_noDefaultValue!: Int64)  {}

func test(){}

// class
class AAAA {
    init(a: Int64, pkg2_init_param1_noDefaultValue!: Int64, pkg2_init_param1_hasDefaultValue!: Int64 = 1){}
    init(a: Int64, pkg2_init_param2_noDefaultValue!: Int64){}
    func test(a: Int64, pkg2_testMem_param1_noDefaultValue!: Int64, pkg2_testMem_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg2_asFunc_param1_noDefaultValue!: Int64, pkg2_asFunc_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg2_asFunc_param2_noDefaultValue!: Int64){}
}

struct BBBB {
    init(a: Int64, pkg2_init_param1_noDefaultValue!: Int64, pkg2_init_param1_hasDefaultValue!: Int64 = 1){}
    init(a: Int64, pkg2_init_param2_noDefaultValue!: Int64){}
    func test(a: Int64, pkg2_testMem_param1_noDefaultValue!: Int64, pkg2_testMem_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg2_asFunc_param1_noDefaultValue!: Int64, pkg2_asFunc_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg2_asFunc_param2_noDefaultValue!: Int64){}
}

class BB{

}
extend BB{
    func test(a: Int64, pkg2_testEx_param1_noDefaultValue!: Int64, pkg2_testEx_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg2_asFuncEx_param2_noDefaultValue!: Int64){}
}

interface II {
    func test(a: Int64, pkg2_testInterface_param1_noDefaultValue!: Int64){}
    func asFunc(a: Int64, pkg2_asFuncInterface_param1_noDefaultValue!: Int64){}
}

class CC<:II{

}

enum EE{
    AA | BBB
    func test(a: Int64, pkg2_testEE_param1_noDefaultValue!: Int64, pkg2_testEE_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg2_asFuncE_param1_noDefaultValue!: Int64, pkg2_asFuncE_param1_hasDefaultValue!: Int64 = 1){}
}

func name(): Int64{0}

func main() {
    var aa = AAAA(1,pkg2_init_param1_noDefaultValue:1,pkg2_init_param1_hasDefaultValue:1)
    var bb1 = BBBB1(1,pkg1_init_param1_noDefaultValue:1,pkg1_init_param1_hasDefaultValue:1)
    var bb = BB()
    var cc = CC()
    test(1,pkg2_test1_param1_noDefaultValue:1,pkg2_test1_param1_hasDefaultValue:1)
    test1(1,pkg1_test1_param1_noDefaultValue:1,pkg1_test1_param1_hasDefaultValue:1)
    bb.test(1,pkg2_testEx_param1_noDefaultValue:1,pkg2_testEx_param1_hasDefaultValue:1)
    cc.test(1,pkg2_testInterface_param1_noDefaultValue:1)
    aa(1,pkg2_asFunc_param1_noDefaultValue:1,pkg2_asFunc_param1_hasDefaultValue:1)
}
