// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ParenLambdaCompletion

class Outer {
    public init() {}
    public prop outerOnly: UInt16 { get() { 0 } }
}

class Inner {
    public init() {}
    public prop innerOnly: UInt16 { get() { 0 } }
}

func run() {
    let serverSocket = Outer()
    var callback: (Inner) -> UInt16 = { serverSocket: Inner => 0 }
    callback = ({ serverSocket: Inner => serverSocket.innerOnly })
}
