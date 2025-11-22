// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

int add(int a, float b);

//float multiply(int a, float b = 1.0f);

int legacy_func();

int (*noProtoPtr)() = &legacy_func;