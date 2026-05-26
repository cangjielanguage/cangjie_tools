// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created on 2025/7/17
 */
package ohos_app_cangjie_entry.IntroduceParameter

func introduceParameterSample(base: Int64) {
    var value = base + 1
}

func callIntroduceParameterSample() {
    introduceParameterSample(10)
}

func localReferenceParameter(base: Int64): Int64 {
    let delta = 2
    return base + delta
}

func twoOriginalParameters(left: Int64, right: Int64): Int64 {
    return left + right
}

func literalParameterSample(): Float64 {
    return 0.9 + 12.1
}

func localOnlyExpression(): Int64 {
    let left = 1
    let right = 2
    return left + right
}
