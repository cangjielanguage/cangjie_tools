// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created on 2025/7/16
 */
package ohos_app_cangjie_entry.ExtractMethod

import std.collection.ArrayList
import std.collection.HashMap

func func1(a: Int64, b: Int64) {
    var c = a + 1
    var d = b + 1
    // check4
    c = c + 1
    d = d + 1
    var e = c + 1
}

func func2() {
    var aa: String
    // check1
    aa = "xxx"
    var bb = aa + "x"
    return bb
}

class Clazz2 {
    var list = ArrayList<String>()
    var var1 = 10

    func func1(aa: Int64) {
        list.add("111")
        // check2
        list.add("222")
        var cc = aa + 1
        var bb = list
            .get(1)
            .toString() + cc.toString()
    }
}

func id<T, U>(a: T, b: U): T {
    // check3
    return a
}

extend String {
    func func1(dd: Int64) {
        // check4
        var aa = HashMap<String, String>()
        aa.add("key", "value").toString()
        var bb: Int64
        bb = 10
        var cc = bb
        if (dd > 0) {
            bb = bb + 10
            cc = bb + 1
        } else {
            cc = bb + 1
            cc = cc + 1
        }
    }
}


enum Enum1 {
	Red

    static func func1() {
        // check5
        var aa: Int64 = 10
        while (aa > 0) {
        	aa--
        }
    }
}
