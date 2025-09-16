// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.pkgD

import default.level.*

@!APILevel[9, syscap: "SystemCapability.ArkUI.xxx"]
public class D123Ca {

}

@!APILevel[9, syscap: "SystemCapability.ArkUI.UiAppearance"]
public class D123Cb {

}

@!APILevel[9, syscap: "SystemCapability.ArkUI.xxx"]
public enum D123Ea {
    | a
}

@!APILevel[9, syscap: "SystemCapability.ArkUI.UiAppearance"]
public enum D123Eb {
    | a
}

@!APILevel[9, syscap: "SystemCapability.ArkUI.xxx"]
public struct D123S1 {

}

@!APILevel[9, syscap: "SystemCapability.ArkUI.UiAppearance"]
public struct D123S2 {

}

@!APILevel[9, syscap: "SystemCapability.ArkUI.xxx"]
public interface D123Ia {

}

@!APILevel[9, syscap: "SystemCapability.ArkUI.UiAppearance"]
public interface D123Ib {

}

func use() {
    D123
}