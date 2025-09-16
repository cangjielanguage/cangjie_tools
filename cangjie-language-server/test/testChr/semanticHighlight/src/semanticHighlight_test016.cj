// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class AAA {
	var a:int32 = 3
    init(a: int32) {
        this.a = a
    }
}

func semanticHighlight_test016():unit {
    var value:AAA = AAA(1)
    var AAA:int32 = 100
    var aAAA:int32 = 200
    var AAAa:int32 = 300
}
