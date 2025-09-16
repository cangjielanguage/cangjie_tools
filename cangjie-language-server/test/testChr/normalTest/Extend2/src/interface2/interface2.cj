// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.interface2

//When the interface extension and the extended type are in different `package`, whether the interface extension is exported depends on the minimum access level of the interface type and the type used in the generic constraint (if any).
//Other `package` must import the extended type, corresponding interface, and constraint type (if any) to access the extended members contained in the corresponding interface.
