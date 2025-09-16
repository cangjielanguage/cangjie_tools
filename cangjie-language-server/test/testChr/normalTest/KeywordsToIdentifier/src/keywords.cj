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
}

main(){
    var public = 1
    var private = 2
    public = 1
    var abstract = public + private
    if (abstract == 3) {
        return abstract
    }
    return public
}
