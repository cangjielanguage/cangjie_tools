// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func localFunc() {
    let s = 1;

    func ab() {
        return "aaa"
    }

    func ad() {
        let a = 1
        let b = 1
        if (a == 1) {
            if (b == 1) {
                12
            } else {
                13
            }
            "aaa"
        } else if (a == 2){
            let s = 1
            "bbb"
        } else if (a == 3){
            let r = 1
            "ccc"
        } else {
            "ddd"
        }
    }

    func test3() {
        var ss = 1
        if (ss == 0) {
            return "0"
        }
        var ss1 = 2

        if (ss == 1) {
            if (ss1 == 2) {
                var kk1 = 1
                "1"
            } else {
                var kk2 = 1
                "2"
            }
        } else {
            "2"
        }
    }

    return "aaa"
}
