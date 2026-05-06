// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef SINGLETON_H
#define SINGLETON_H

template <typename T> class Singleton {
public:
    static T &GetInstance()
    {
        static T instance;
        return instance;
    }
};

#endif // SINGLETON_H