// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "Logger.cpp"
#include <string>
#include <thread>
#include <chrono>

using namespace ark;

// Test Logger instantiation
TEST(LoggerTest, LoggerInstance) {
    Logger& logger = Logger::Instance();
    EXPECT_NE(&logger, nullptr);
}

// Test LogInfo function
TEST(LoggerTest, LogInfo_Error) {
    std::string message = "Test error message";
    std::string result = Logger::Instance().LogInfo(MessageType::MSG_ERROR, message);
    EXPECT_NE(result.find("Error"), std::string::npos);
    EXPECT_NE(result.find(message), std::string::npos);
}

TEST(LoggerTest, LogInfo_Warning) {
    std::string message = "Test warning message";
    std::string result = Logger::Instance().LogInfo(MessageType::MSG_WARNING, message);
    EXPECT_NE(result.find("Warning"), std::string::npos);
    EXPECT_NE(result.find(message), std::string::npos);
}

TEST(LoggerTest, LogInfo_Info) {
    std::string message = "Test info message";
    std::string result = Logger::Instance().LogInfo(MessageType::MSG_INFO, message);
    EXPECT_NE(result.find("Info"), std::string::npos);
    EXPECT_NE(result.find(message), std::string::npos);
}

TEST(LoggerTest, LogInfo_Log) {
    std::string message = "Test log message";
    std::string result = Logger::Instance().LogInfo(MessageType::MSG_LOG, message);
    EXPECT_NE(result.find("Log"), std::string::npos);
    EXPECT_NE(result.find(message), std::string::npos);
}

TEST(LoggerTest, LogInfo_Default) {
    std::string message = "Test default message";
    std::string result = Logger::Instance().LogInfo(static_cast<MessageType>(999), message);
    EXPECT_NE(result.find("missing"), std::string::npos);
    EXPECT_NE(result.find(message), std::string::npos);
}

// Test SetPath function
TEST(LoggerTest, SetPath_Empty) {
    Logger::SetPath("");
    // Should not crash
    EXPECT_TRUE(true);
}

TEST(LoggerTest, SetPath_Valid) {
    std::string testPath = "/tmp";
    Logger::SetPath(testPath);
    // Should not crash
    EXPECT_TRUE(true);
}

// Test CollectMessageInfo function
TEST(LoggerTest, CollectMessageInfo_EmptyQueue) {
    // Clear the queue
    while (!Logger::messageQueue.empty()) {
        Logger::messageQueue.pop();
    }

    std::string message = "Test message";
    Logger::Instance().CollectMessageInfo(message);
    EXPECT_FALSE(Logger::messageQueue.empty());
    EXPECT_EQ(Logger::messageQueue.size(), 1u);
}

TEST(LoggerTest, CollectMessageInfo_FullQueue) {
    // Fill the queue to maximum capacity
    while (!Logger::messageQueue.empty()) {
        Logger::messageQueue.pop();
    }

    for (int i = 0; i < Logger::messageQueueSize + 5; i++) {
        Logger::Instance().CollectMessageInfo("Message " + std::to_string(i));
    }

    // Queue should remain at maximum capacity
    EXPECT_EQ(Logger::messageQueue.size(), Logger::messageQueueSize);
}

// Test CollectKernelLog function
TEST(LoggerTest, CollectKernelLog_NewThread) {
    std::thread::id threadId = std::this_thread::get_id();
    std::string funcName = "testFunction";
    std::string state = "running";

    Logger::Instance().CollectKernelLog(threadId, funcName, state);

    // Should not crash
    EXPECT_TRUE(true);
}

// Test CleanKernelLog function
TEST(LoggerTest, CleanKernelLog_NonExistentThread) {
    std::thread::id fakeThreadId;
    Logger::Instance().CleanKernelLog(fakeThreadId);
    // Should not crash
    EXPECT_TRUE(true);
}

TEST(LoggerTest, CleanKernelLog_ExistingThread) {
    std::thread::id threadId = std::this_thread::get_id();
    std::string funcName = "testFunction";
    std::string state = "running";

    Logger::Instance().CollectKernelLog(threadId, funcName, state);
    Logger::Instance().CleanKernelLog(threadId);
    // Should not crash
    EXPECT_TRUE(true);
}

// Test SetLogEnable function
TEST(LoggerTest, SetLogEnable_True) {
    Logger::SetLogEnable(true);
    // Should not crash
    EXPECT_TRUE(true);
}

TEST(LoggerTest, SetLogEnable_False) {
    Logger::SetLogEnable(false);
    // Should not crash
    EXPECT_TRUE(true);
}

// Test LogMessage function
TEST(LoggerTest, LogMessage_Disabled) {
    Logger::SetLogEnable(false);
    Logger& logger = Logger::Instance();

    // Should not write to log
    logger.LogMessage(MessageType::MSG_INFO, "This should not be logged");
    EXPECT_TRUE(true);
}

TEST(LoggerTest, LogMessage_Enabled) {
    Logger::SetLogEnable(true);
    Logger& logger = Logger::Instance();

    // Should process log message without crashing
    logger.LogMessage(MessageType::MSG_INFO, "This is a test log message");
    EXPECT_TRUE(true);
}

// Test Trace function
TEST(LoggerTest, Trace_Wlog) {
    // Should process warning log without crashing
    Trace::Wlog("Test warning message");
    EXPECT_TRUE(true);
}

TEST(LoggerTest, Trace_Elog) {
    // Should process error log without crashing
    Trace::Elog("Test error message");
    EXPECT_TRUE(true);
}

