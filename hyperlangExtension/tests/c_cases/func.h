// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

int add(int a, float b);

//float multiply(int a, float b = 1.0f);

int legacy_func();

int (*noProtoPtr)() = &legacy_func;

void cfoo1(int *a);

void cfoo2(int a[3]);

int main(int argc, char *argv[]);

typedef void (*callback)(int);

void set_callback(callback cb);

int add2(int a, int b, int (*func)(int, int));

typedef struct {
    long long x;
    long long y;
    long long z;
} Point3D;

Point3D addPoint(Point3D p1, Point3D p2);
