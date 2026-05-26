// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Configure.cpp"
#include <gtest/gtest.h>

using namespace sqldb;

void testLogCallback(int ErrCode, std::string_view Msg) {
    (void)ErrCode;
    (void)Msg;
}

TEST(ConfigureTest, logCallbackTest001)
{
    LogCallback callback = testLogCallback;
    logCallback(reinterpret_cast<void *>(callback), SQLITE_OK, "Test message");
    EXPECT_EQ(1, 1);
}

TEST(ConfigureTest, setLogCallbackTest001)
{
    LogCallback callback = testLogCallback;
    setLogCallback(callback);
    EXPECT_EQ(1, 1);
}

TEST(ConfigureTest, setSingleThreadModeTest001)
{
    setSingleThreadMode();
    EXPECT_EQ(1, 1);
}

TEST(ConfigureTest, setMultiThreadModeTest001)
{
    setMultiThreadMode();
    EXPECT_EQ(1, 1);
}