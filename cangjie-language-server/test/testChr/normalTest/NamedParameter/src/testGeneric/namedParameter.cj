// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testGeneric
class ParamType{}

interface TypeInterface<typename> {}

class TypeClass<typename> <: TypeInterface<typename>{}

func nameParam<typename>(param1!:ParamType, param2!:TypeClass<ParamType>, param3!:TypeClass<typename>) {}

func TestNameParam() {
    var p1:ParamType = ParamType()
    var p2:TypeClass<ParamType> = TypeClass<ParamType>()
    var p3:TypeClass<String> = TypeClass<String>()

    nameParam<String>(param1: ParamType(), param2:TypeClass<ParamType>(), param3: TypeClass<String>())
    nameParam<String>(param1:p1, param2:p2, param3:p3)
}


