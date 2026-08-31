// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry.IntroduceParameter

func tokenBoundarySample(base: Int64): Int64 {
    let baseValue = 2
    return base + baseValue
}

func callTokenBoundarySample(): Unit {
    let result = tokenBoundarySample(10)
}
