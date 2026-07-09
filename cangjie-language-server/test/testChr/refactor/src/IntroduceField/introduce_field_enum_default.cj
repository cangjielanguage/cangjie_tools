// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

enum FieldColor {
    Red | Blue
}

type FieldColorAlias = FieldColor

class EnumDefaultInitializerOwner {
    func paint(color: FieldColorAlias): FieldColorAlias {
        return color
    }
}
