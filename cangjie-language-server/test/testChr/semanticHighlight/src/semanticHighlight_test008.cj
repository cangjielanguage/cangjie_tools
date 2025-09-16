// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/*
/*struct A{ */
/*    let a : int32 */
/*    operator func + (inputA:A,inputB:A):A{ */
/*        var output = A(inputA.a + inputB.a) */
/*        return output */
/*    } */
/*} */
*/

/**
func main():int32{
    var a = A(1)
    var b = A(-1)
    return (a+b).a
}
*/

/*
struct A{
    let a : int32
    operator func + (inputA:A,inputB:A):A{
        var output = A(inputA.a + inputB.a)
        return output
    }
}
*/

//func main():int32{
//    var a = A(1)
//    var b = A(-1)
//    return (a+b).a
//}
