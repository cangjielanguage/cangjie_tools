// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import ea.a.*;
public interface abac0{}
public class typeAlias01 <: abac0{}
public interface typeAlias02 <:abac0{}
public enum typeAlias03 <:abac0{
    red|blue
}
public struct typeAlias04 <:abac0{}

type Alias1 = typeAlias1
type Alias2 = typeAlias2
type Alias3 = typeAlias3
type Alias4 = typeAlias4
type Alias01 = typeAlias01
type Alias02 = typeAlias02
type Alias03 = typeAlias03
type Alias04 = typeAlias04
type excep = Exception
type I63 = Int64
main(): Int64 {
    print("hello world\n")
    return 0
}
