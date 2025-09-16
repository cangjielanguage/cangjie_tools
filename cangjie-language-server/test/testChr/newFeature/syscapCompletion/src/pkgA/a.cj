// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.pkgA

import default.level.*

@!APILevel[9, syscap: "SystemCapability.ArkUI.xxx"]
public func f12a(): Unit {

}

@!APILevel[9, syscap: "SystemCapability.ArkUI.UiAppearance"]
public func f12b(): Unit {

}

@!APILevel[11]
public func f12c(): Unit {

}

@!APILevel[9, syscap: "SystemCapability.AI.InsightIntent"]
public func f12d(): Unit {

}

public func f12f(): Unit {

}

func use() {
    // expect completion without f12a()
    f12
}