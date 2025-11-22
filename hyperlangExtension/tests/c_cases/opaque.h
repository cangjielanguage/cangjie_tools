// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef OPAQUE_DEMO_H
#define OPAQUE_DEMO_H

// 声明不透明类型（只给出类型名，不定义）
typedef struct OpaqueType OpaqueType;

// 提供操作接口
OpaqueType* create_opaque(int initial_value);
void set_value(OpaqueType* obj, int value);
int get_value(OpaqueType* obj);
void destroy_opaque(OpaqueType* obj);

#endif