// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default

enum TimeUnit18 {
    Year | Month | Day | Hour1
}

let r1 = TimeUnit18.Year
let howManyHours: Int64 = match (r1) {
    case Year => 365 * 24
    case Month => 30 * 24
    case Day => 24
    case Hour1 => 1
}

