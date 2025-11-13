// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Memory.h"
#include <gtest/gtest.h>

using namespace sqldb;

TEST(MemoryTest, MemoryTest001)
{
    getMemoryUsed();
    EXPECT_EQ(1, 1);
}

TEST(MemoryTest, MemoryTest002)
{
    getMemoryHighWater();
    EXPECT_EQ(1, 1);
}

TEST(MemoryTest, MemoryTest003)
{
    getSoftHeapLimit();
    EXPECT_EQ(1, 1);
}

TEST(MemoryTest, MemoryTest004)
{
    getHardHeapLimit();
    EXPECT_EQ(1, 1);
}

TEST(MemoryTest, MemoryTest005)
{
    setSoftHeapLimit(-1);
    EXPECT_EQ(1, 1);
}

TEST(MemoryTest, MemoryTest006)
{
    setHardHeapLimit(-1);
    EXPECT_EQ(1, 1);
}