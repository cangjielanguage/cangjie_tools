// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package demo

enum RGBColor {
    | Red(Int16) | Green(Int16) | Blue(Int16)

    func test() {
        let c = RGBColor.Green(-100)
        let cs = match (c) {
            case Red(r) => "Red = ${r}"
            case _ => "error"
        }
        print(cs)
    }
}
