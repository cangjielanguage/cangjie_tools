// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class A {
    public var a: Int64
    public var b: Int64
    public init(x: Int64){
        a = x
        b = x
    }
    public operator func []( index: Int64, value: Int64) {
        this.b = value
    }
    public operator func ()(a:A):A{ return a}
    public operator func !( ) {}
    public operator func -( ) {}
    public operator func **(a:A):A{ return a}
    public operator func *(a:A):A{  return a}
    public operator func /(a:A):A{  return a}
    public operator func %(a:A):A{  return a}
    public operator func +(a:A):A{  return a}
    public operator func <<(a:A){}
    public operator func >>(a:A){}
    public operator func <(a:A){}
    public operator func >(a:A){}
    public operator func <=(a:A){}
    public operator func >=(a:A){}
    public operator func ==(a:A){}
    public operator func !=(a:A){}
    public operator func &(a:A){}
    public operator func ^(a:A){}
    public operator func |(a:A){}
}

func  maiun(): Int64{
    var a = A(2) 
    a [1,1]
    a (a)
    !a
    -a
    a ** a
    a * a
    a/a
    a%a
    a+a
    a<<a
    a>>a
    a>a
    a<a
    a<=a
    a>=a
    a==a
    a!=a
    a & a
    a ^ a
    a | a
    return 1
}
