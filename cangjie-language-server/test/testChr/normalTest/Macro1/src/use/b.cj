// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.use

// from std import ast.*
import default.define.*

import pkg.*

@X12[1](1)

func useMacro1(){

    
    @Y1

    return 0
}


func useMacro2(){

    @X1
    
    @Y1232(12)

    return 0
}

func useMacro2(){

    @Tes

    return 0
}
