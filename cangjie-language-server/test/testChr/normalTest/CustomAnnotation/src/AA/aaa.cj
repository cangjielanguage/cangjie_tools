// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package AA
from std import reflect.{TypeInfo}

@Annotation
public class AAA {
    public const init() {}
    public const init(name: String, version!: Int64 = 0) {}
}
