// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// Test A5: Unused private enum - should be reported as unused
package testUnusedSymbol

private enum UnusedPrivateEnum {
    | Case1
    | Case2
}
