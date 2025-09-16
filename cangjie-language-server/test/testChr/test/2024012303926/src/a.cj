// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

public class subCla_1<: ClassViaInherit {
    private var subVar1:Int32 = 1263
    public init(){
        super()
    }

}
public open  class ClassViaInherit{
    public var width: Int32 = 10
    public let length: Int32 = 20

    public init(){}
    public ClassViaInherit (par_1: Int32, par_2!: Int32=-32) {
        this.width = par_1
    }
}
