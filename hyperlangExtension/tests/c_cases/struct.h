// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// 外层结构体包含匿名结构体
struct Point {
    // 匿名结构体：无名称，直接包含 x、y 成员
    int a;
    int b;
    struct {
        int x;
        int y;
    };
    int z;  // 外层结构体自身的成员
};

struct person {
    int age;
};

typedef struct {
    long long x;
    long long y;
    long long z;
} Point3D;

Point3D p;

struct people {
    struct person a;
    struct person b;
    int height[2];
    int total;
};

struct S {
    int a[2];
    int b[0];
};

struct Data
{
    int a;
    int b;
};

struct Data getData(int a, int b);

typedef struct {
    int i8;
    unsigned int ui8;
} teststruct;

teststruct struct1 = {1, 1};


