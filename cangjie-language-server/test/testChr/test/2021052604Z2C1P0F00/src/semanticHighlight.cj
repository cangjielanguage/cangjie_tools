// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// EXEC: %compiler %cmp_opt %f -o %output
// EXEC: %run %run_opt %output %run_args


enum Time{
    Day(Time) | Hour
}


main() {
    var t1 = Time.Hour
    var t2 = Time.Day(t1)

    match (t2) {
         case Day(x) => match (x) {
             case Day(x) => 525
             case
         }
         case Hour =>  0
    }
    return 0
}
