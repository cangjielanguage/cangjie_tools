// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

public class BB{
    var c1 = 0
    public mut prop method: String {
        get() {
            return "getProperty"
        }
        set(v) {
             v;
        }
    }
    func ab(){}
}
public class DD{}
public func mockIto(service: String, method: String, handler: (BB) -> DD): Unit {}

func y1(){
    mockIto(
        "ddd",
        "ddfgggg",
        {
            req =>
            let a1 = req.
            return DD()
        }
    )
}
