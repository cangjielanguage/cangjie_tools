// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package currentPackage;
interface I1 { }
interface I2 <: I1 { }
class C <: I2 { }
extend Bool <: I2 { }
var i:C = C()
var c:Rune= 'a'
