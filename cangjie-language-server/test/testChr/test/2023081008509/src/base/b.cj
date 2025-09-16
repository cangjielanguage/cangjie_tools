// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package base

class MainThreadContext <: ThreadContext{
    public func end(): Unit {}
    public func hasEnded(): Bool { false }
    func getSchedulerHandle(): CPointer<Unit> { CPointer<Unit>() } // inner implement
}
func fun1():Unit {
    let a12121 = MainThreadContext()
    let fut:Future<Unit> = spawn (a12121) {
      println("hello world spawn9")
    }
    let a = MainThreadContext()

}
