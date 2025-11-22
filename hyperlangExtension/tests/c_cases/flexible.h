// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// 定义包含柔性数组成员的结构体
typedef struct {
int length; // 字符串长度
char data[]; // 柔性数组成员，存储字符串数据
} FlexibleString;