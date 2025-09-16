// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class Base15 {
	var a:int32 = 3
    init(a: int32) {
        this.a = a
    }
}

func semanticHighlight_test015():unit {
    var value:Base15 = Base15(1)
    var Base15:int32 = 100
    var BASE15:int32 = 200
    var base15:int32 = 300
}
