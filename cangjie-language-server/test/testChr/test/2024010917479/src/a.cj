// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

from std import process.*
from std import collection.*
from encoding import hex.*

func test2(){
    var str = "63616e676a6965"
    var opt1 = hex.fromHexString("63616e676a6965")
    var opt2 = encoding.hex.fromHexString(str)
}

func tt(){
    process.Process.current.setEnv("k: String", "v: String")
    std.process.Process.current.setEnv("k: String", "v: String")
    collection.ArrayList<Int64>()
}
