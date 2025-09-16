// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.



class A1 {}
struct B1 {}
enum C1 { D }
func main() {
    var i_ANY: Any = A1() // ok
    i_ANY = B1() // ok
    i_ANY = C1.D // ok
    i_ANY = (1, 2) // ok
    i_ANY = { => 123 } // ok
return 0
}
class A2 <: Any {} // ok
class B3 {} // Implicit implement Any
extend B3 <: Any {} // error

