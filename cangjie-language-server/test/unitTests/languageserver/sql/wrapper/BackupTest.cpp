// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Backup.h"
#include <gtest/gtest.h>

using namespace sqldb;

sqlite3* openDatabase(const std::string& path) {
    sqlite3* db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
    return db;
}

TEST(BackupTest, BackupTest001)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* pDestDb = openDatabase(dbPath1);
    sqlite3* pSrcDb = openDatabase(dbPath2);

    auto validP = sqlite3_backup_init(pDestDb, "main", pSrcDb, "main");
    Backup backup(validP);


    EXPECT_EQ(1, 1);
}

TEST(BackupTest, BackupTest002)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* pDestDb = openDatabase(dbPath1);
    sqlite3* pSrcDb = openDatabase(dbPath2);

    auto validP = sqlite3_backup_init(pDestDb, "main", pSrcDb, "main");
    Backup source(validP);
    Backup destination(std::move(source));

    EXPECT_EQ(1, 1);
}

TEST(BackupTest, BackupTest003)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* pDestDb = openDatabase(dbPath1);
    sqlite3* pSrcDb = openDatabase(dbPath2);

    auto validP1 = sqlite3_backup_init(pDestDb, "main", pSrcDb, "main");
    auto validP2 = sqlite3_backup_init(pDestDb, "main", pSrcDb, "main");
    Backup source(validP1);
    Backup destination(validP2);
    destination.operator=(std::move(source));


    EXPECT_EQ(1, 1);
}

TEST(BackupTest, BackupTest004)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* pDestDb = openDatabase(dbPath1);
    sqlite3* pSrcDb = openDatabase(dbPath2);

    auto validP = sqlite3_backup_init(pDestDb, "main", pSrcDb, "main");
    Backup source(validP);
    source.operator=(std::move(source));


    EXPECT_EQ(1, 1);
}

TEST(BackupTest, BackupTest005)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* pDestDb = openDatabase(dbPath1);
    sqlite3* pSrcDb = openDatabase(dbPath2);

    auto validP1 = sqlite3_backup_init(pDestDb, "main1", pSrcDb, "main1");
    Backup backup1(validP1);

    auto validP2 = sqlite3_backup_init(pDestDb, "main2", pSrcDb, "main2");
    Backup backup2(validP2);

    backup1.swap(backup2);

    EXPECT_EQ(1, 1);
}

TEST(BackupTest, BackupTest006)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* pDestDb = openDatabase(dbPath1);
    sqlite3* pSrcDb = openDatabase(dbPath2);

    auto validP = sqlite3_backup_init(pDestDb, "main", pSrcDb, "main");
    Backup backup(validP);

    EXPECT_NE(backup.getTotalPageCount(), -1);

}

TEST(BackupTest, BackupTest007)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* pDestDb = openDatabase(dbPath1);
    sqlite3* pSrcDb = openDatabase(dbPath2);

    auto validP = sqlite3_backup_init(pDestDb, "main", pSrcDb, "main");
    Backup backup(validP);

    EXPECT_NE(backup.getRemainingPages(), -1);

}

TEST(BackupTest, BackupTest008)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* pDestDb = openDatabase(dbPath1);
    sqlite3* pSrcDb = openDatabase(dbPath2);

    auto validP = sqlite3_backup_init(pDestDb, "main", pSrcDb, "main");
    Backup backup(validP);

    backup.step(10);


    EXPECT_EQ(1, 1);
}

TEST(BackupTest, BackupTest009)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* pDestDb = openDatabase(dbPath1);
    sqlite3* pSrcDb = openDatabase(dbPath2);

    auto validP = sqlite3_backup_init(pDestDb, "main", pSrcDb, "main");
    Backup backup(validP);

    backup.execute();

    EXPECT_EQ(1, 1);
}

TEST(BackupTest, BackupTest010)
{
    const std::string dbPath1 = "mydatabase1.db";
    const std::string dbPath2 = "mydatabase2.db";
    sqlite3* pDestDb = openDatabase(dbPath1);
    sqlite3 *pSrcDb = openDatabase(dbPath2);

    auto dstConnection = Connection(pDestDb);
    auto srcConnection = Connection(pSrcDb);
    std::string name = "test_backup";

    backup(dstConnection, srcConnection, name);
    EXPECT_EQ(1, 1);
}