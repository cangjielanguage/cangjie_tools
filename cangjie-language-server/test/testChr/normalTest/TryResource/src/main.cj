// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package TryResource

class Client <: Resource {
    public init() {}
    public func isClosed(): Bool { true }
    public func close(): Unit {}
}

class Server <: Resource {
    public init() {}
    public func isClosed(): Bool { true }
    public func close(): Unit {}
    public func accept(): Client { Client() }
}

func run() {
    try (serverSocket = Server()) {
        try (client = serverSocket.accept()) {}
    }
}
