// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


package pkg1

// function
public func test1(a: Int64, pkg1_test1_param1_noDefaultValue!: Int64, pkg1_test1_param1_hasDefaultValue!: Int64 = 1)
{}

public func test1(a: Int64, pkg1_test1_param2_noDefaultValue!: Int64)  {}

public func test1(){}

// class
public class AAAA1 {
    init(a: Int64, pkg1_init_param1_noDefaultValue!: Int64, pkg1_init_param1_hasDefaultValue!: Int64 = 1){}
    init(a: Int64, pkg1_init_param2_noDefaultValue!: Int64){}
    func test(a: Int64, pkg1_testMem_param1_noDefaultValue!: Int64, pkg1_testMem_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg1_asFunc_param1_noDefaultValue!: Int64, pkg1_asFunc_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg1_asFunc_param1_noDefaultValue!: Int64){}
}

public struct BBBB1 {
    init(a: Int64, pkg1_init_param1_noDefaultValue!: Int64, pkg1_init_param1_hasDefaultValue!: Int64 = 1){}
    init(a: Int64, pkg1_init_param2_noDefaultValue!: Int64){}
    func test(a: Int64, pkg1_testMem_param1_noDefaultValue!: Int64, pkg1_testMem_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg1_asFunc_param1_noDefaultValue!: Int64, pkg1_asFunc_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg1_asFunc_param2_noDefaultValue!: Int64){}
}

public class BB1{

}
extend BB1{
    func test(a: Int64, pkg1_testEx_param1_noDefaultValue!: Int64, pkg1_testEx_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg1_asFuncEx_param2_noDefaultValue!: Int64){}
}

public interface II1 {
    func test(a: Int64, pkg1_testInterface_param1_noDefaultValue!: Int64){}
    func asFunc(a: Int64, pkg1_asFuncInterface_param1_noDefaultValue!: Int64){}
}

public class CC1<:II1{

}

public enum EE1{
    AA | BBB
    func test(a: Int64, pkg1_testEE_param1_noDefaultValue!: Int64, pkg1_testEE_param1_hasDefaultValue!: Int64 = 1){}
    func asFunc(a: Int64, pkg1_asFuncE_param1_noDefaultValue!: Int64, pkg1_asFuncE_param1_hasDefaultValue!: Int64 = 1){}
}
