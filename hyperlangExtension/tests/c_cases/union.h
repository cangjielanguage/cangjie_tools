// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

union X {
int a;
void* ptr;
};

union {
        struct {
            int length; /* XXX: should move outside union */
            int cproto; /* XXX: should move outside union */

        } func;

        struct {
            const char *name;
            int base;
        } alias;
        struct {
            const struct JSCFunctionListEntry *tab;
            int len;
        } prop_list;
        const char *str;
        int i32;
        int i64;
        double f64;
} u;