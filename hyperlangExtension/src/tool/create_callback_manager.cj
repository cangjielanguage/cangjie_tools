// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.time.DateTime

public func createCallbackManager(gconfig: GlobalConfig): String {
    let moduleName = gconfig.cjModuleName

    let content = """
// Copyright (c) Huawei Technologies Co., Ltd. ${DateTime.nowUTC().year}. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
package ${moduleName}

import std.sync.ReentrantMutex
import std.collection.{HashMap, ArrayList}
import ohos.callback_invoke.CallbackObject

protected class CallbackManager<K, V> where K <: Hashable & Equatable<K> {
    let callbackMap: HashMap<K, ArrayList<(CallbackObject, V)>>
    let mutex: ReentrantMutex = ReentrantMutex()

    protected init() {
        callbackMap = HashMap<K, ArrayList<(CallbackObject, V)>>()
    }

    protected func put(key: K, item: (CallbackObject, V)): Unit {
        synchronized (mutex) {
            if (callbackMap.contains(key)) {
                if (findCallbackObject(key, item[0]).isNone()) {
                    callbackMap[key].add(item)
                }
                return
            }
            callbackMap.add(key, ArrayList<(CallbackObject, V)>())
            callbackMap[key].add(item)
        }
    }

    protected func isCallbackExist(event: K): Bool {
        synchronized (mutex) {
            if (callbackMap.contains(event) && callbackMap[event].size > 0) {
                return true
            }
            return false
        }
    }

    protected func clear(event: K): Unit {
        synchronized (mutex) {
            callbackMap[event].clear()
        }
    }

    protected func findCallbackObject(key: K, callback: CallbackObject, remove!: Bool = false): ?V {
        synchronized (mutex) {
            if (!callbackMap.contains(key)) {
                return Option<V>.None
            }
            let callbackList = callbackMap[key]
            for (idx in 0..callbackList.size) {
                if (refEq(callback, callbackList[idx][0])) {
                    let jsv = callbackList[idx][1]
                    if (remove) {
                        callbackList.remove(idx..=idx)
                    }
                    return jsv
                }
            }
            return Option<V>.None
        }
    }
}
"""
    return content
}
