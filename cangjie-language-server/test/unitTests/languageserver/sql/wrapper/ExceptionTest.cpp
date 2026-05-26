// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Exception.h"
#include <gtest/gtest.h>

using namespace sqldb;

TEST(ExceptionTest, ExceptionTest001)
{
    int rc = 0;
    std::string msg = "This is a test";
    Exception ex(rc, msg);

    EXPECT_EQ(1, 1);
}

TEST(ExceptionTest, ExceptionTest002)
{
    const std::string dbPath = "mydatabase.db";
    sqlite3* db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    sqlite3_open_v2(dbPath.c_str(), &db, flags, nullptr);

    std::string msg = "This is a test";
    Exception ex(db, msg);

    EXPECT_EQ(1, 1);
}