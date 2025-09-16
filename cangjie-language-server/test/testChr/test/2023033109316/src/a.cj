// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

public open class C3 {
    static let counter: Int64 = 3
    var name: String = "mmm"
    var annotation: String = "nnn"
    protected C3(name: String, annotation: String) {
        this.name = name
    }
    init() {
        this.annotation
    }
}

public class C4 <: C3 {
    C4(
        name: String,
        annotation!: String = "nnn",
        var width!: Int64 = 1,
        var length!: Int64,
        private var height!: Int64 = 3
    ) {
        super(name, annotation)
        var a = super.
    }
}
