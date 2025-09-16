// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package currentPackage;
public interface hh {}
open class xzf <: hh {}
open class live <: xzf {}
abstract class People <: live {}

interface water {}
interface tea <: water {}
interface milk <: water {}
interface milk_tea <: milk & tea {}

interface Child {}
interface Teenager <: Child {}
interface Adult <: Teenager {}

class xiaoming <: People & Adult & milk_tea {}
class zzf<:water{}
extend zzf <: hh{
    func b() {}
}
extend Rune{}
func a() {
    var a: xiaoming
}

