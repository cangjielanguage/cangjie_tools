// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

enum test2 {
    |Cvar1
    |CA2
    mut prop bbbbbbbbbbbbbbbbb: Int64 {
         get() {
             print("get")
             return 1
         }
         set(value){
            print("set")
            let Cvar1 = value
         }
    }
}
