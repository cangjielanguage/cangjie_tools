// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func TextInput(placeholder!: String = "", text!: String = "", controller!: String = "") {

}

func a() {
    TextInput(placeholder: "", text: ##"
          -\n${a}多行原始字符串字面量third ~！@#￥%……&*（）()+{}：“？》《·^*-"##
        )
}
