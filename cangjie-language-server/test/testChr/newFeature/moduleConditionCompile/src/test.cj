// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created on 2025/6/3
 */
package ohos_app_cangjie_entry

@When[feature == "tiger"]
func fee() {
    println("feature tiger")
}

@When[feature == "lion"]
func fee() {
    println("feature lion")
}
main() {
    fee()
}
