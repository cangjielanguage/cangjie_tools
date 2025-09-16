// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ea.a;
public interface abac{}
public class typeAlias1 <: abac{}
public interface typeAlias2 <:abac{}
public enum typeAlias3 <:abac{
    red|blue
}
public struct typeAlias4 <:abac{}
