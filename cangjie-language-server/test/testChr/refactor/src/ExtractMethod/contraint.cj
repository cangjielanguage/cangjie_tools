// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created on 2025/7/16
 */
package ohos_app_cangjie_entry.ExtractMethod

// check9
var global_var = 10

class Clazz1 {
    var aa = 10
    var bb = 10

    init(a1: Int64, b1: Int64) {
        // check8
        this.aa = a1
        bb = b1
    }

    func func1() {
        // check1
        var bb: Int64
        // check2
        if (global_var > 0) {
            return 0
        }
        bb = 10
        var aa = bb
        // check3
        aa = aa + 2
        bb = bb + 1
        var cc = aa + bb
        // check4
        if (aa > 0) {
            global_var = global_var + aa
        } else {
            global_var = global_var + bb
        }
        // check5
        while (aa > 0) {
            if (global_var > 10) {
                break
            }
            global_var = global_var - 1
            aa = aa - 1
        }
        // check6
        try {
            throw NegativeArraySizeException("xx")
        } catch (e: NegativeArraySizeException) {
            print("xxxxxxx")
        }
        // check7
        var x = 10
        match (x) {
            case 0 => print("x = 0")
            case 1 => print("x = 1")
            case 2 => print("x = 2")
            case _ => print("x = 0")
        }
        return bb
    }
}

