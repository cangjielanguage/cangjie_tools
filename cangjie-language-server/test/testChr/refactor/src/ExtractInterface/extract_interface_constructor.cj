// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created on 2025/7/16
 */
package ohos_app_cangjie_entry.ExtractInterface

public class Product {
    public func printName(): String {
        return "person"
    }
    public func classOnly(): String {
        return "class only"
    }
}
class ProductUser {
    var cached: Product = Product()
    func consume(person: Product): String {
        return person.printName()
    }
    func build(): Product {
        return Product()
    }
    func keepConcrete(person: Product): String {
        return person.classOnly()
    }
}
