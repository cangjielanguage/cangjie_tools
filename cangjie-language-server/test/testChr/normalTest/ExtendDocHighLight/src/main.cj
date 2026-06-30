// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ExtendDocHighLight

import ExtendDocHighLight.A.I12_1

class BC1 {}


extend BC1 <: I12_1{
    public prop ii2:Int64{
        get(){1}
    }
}

func test28(){
    var a = BC1().ii2
}
