// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package pkg1

// 补齐模块名称
from s
// 补齐该模块下的 包名
from std import m
// 支持该场景下补齐对应模块中的包名
from std import math.*,socket.*
// 补齐目录下的包名
import utils
import a.*,utils.
// 限制 不支持跨行的包导入 如果跨行说明太长 规范的角度建议 在写一个导入模块包的函数


