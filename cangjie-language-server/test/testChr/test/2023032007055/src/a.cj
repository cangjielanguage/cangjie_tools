// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class public {
    let private:Int64
    let open: Int64
    public public(abstract: Int64, protected: Int64) {
        private = abstract
        open = protected
    }
    func redef() {
        return private
    }
    static func ss() {

    }
}

func m1() {
    var public = public(1, 5)
    var a = public.
}
