// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import std.sync.*
 
class BoundedQueue {
    // Create a Mutex, two Conditions.
    public let mtx : MultiConditionMonitor = MultiConditionMonitor()
    public let notFull : ConditionID = mtx.newCondition()
    public let notEmpty : ConditionID = mtx.newCondition()
    public var aaa = mtx.newCondition()
    public var count : Int64 // Object count in buffer.
    public var head : Int64 // Write index.
    public var tail : Int64 // Read index.
    public init() {
        count = 0
        head = 0
        tail = 0
    }
    public func put(x : Object) {
        synchronized(mtx) {
            while (count == 100) {
                mtx.wait(notFull)
            }
            head++
            if (head == 100) {
                head = 0
            }
            count++
        }
        mtx.notify(notEmpty)
    }
    public func get() {
        synchronized(mtx) {
            while (count == 0) {
                mtx.wait(notEmpty)
            }
            tail++
            if (tail == 100) {
            }
            count--
        }
        mtx.notify(notFull)
    }
    public func test() {
        synchronized(mtx1){} // error
        syn
    }
}
