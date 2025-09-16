// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

public import std.time.*

let now = DateTime.now()
var result1 = "时间:${now.year}年${now.month}月"
var result2 = "time:${now.year}year${now.month}month"

/*多行注释
代码测试*/ type ty = Int64

/*中文*/ func nameParm/*中文*/(str: String, variable!: ty) {}

func main() {
/*多行注释
代码测试*/ nameParm("命名参数补全测试", vari)
    0
}

