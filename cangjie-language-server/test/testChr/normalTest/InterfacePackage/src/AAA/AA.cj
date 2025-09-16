// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package AAA 
public interface AAA1 {
    func name1():UInt16
}

public interface AAA2<:AAA1 {
    func name21():UInt16
}
