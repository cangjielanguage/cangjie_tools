// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CrashReporter.cpp"
#include <gtest/gtest.h>

using namespace ark;

#ifdef __linux__

#elif defined(_WIN32)
TEST(CrashReporterTest, ReportExceptionTest001)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_ACCESS_VIOLATION;

    EXPECT_EQ("Access violation", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest002)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_BREAKPOINT;

    EXPECT_EQ("Exception: Breakpoint", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest003)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_ARRAY_BOUNDS_EXCEEDED;

    EXPECT_EQ("Exception: Array index out of bounds", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest004)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_STACK_OVERFLOW;

    EXPECT_EQ("Exception: Stack overflow", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest005)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_PRIV_INSTRUCTION;

    EXPECT_EQ("Exception: General Protection Fault", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest006)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_ILLEGAL_INSTRUCTION;

    EXPECT_EQ("Exception: Illegal instruction in program", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest007)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_INT_OVERFLOW;

    EXPECT_EQ("Exception: Integer overflow", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest008)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_INT_DIVIDE_BY_ZERO;

    EXPECT_EQ("Exception: Integer division by zero", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest009)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = EXCEPTION_FLT_UNDERFLOW;

    EXPECT_EQ("Exception: Floating point value underflow", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, ReportExceptionTest010)
{
    EXCEPTION_POINTERS exceptionPointers = {nullptr, nullptr};
    exceptionPointers.ExceptionRecord = new EXCEPTION_RECORD();
    exceptionPointers.ExceptionRecord->ExceptionCode = 0x12345678;

    EXPECT_EQ("Unknown exception", ReportException(&exceptionPointers));
}

TEST(CrashReporterTest, InitStackTest001)
{
#ifdef _M_IX86

#elif _M_X64
    DWORD dwImageType = IMAGE_FILE_MACHINE_AMD64;
    STACKFRAME64 frame = {0};
    CONTEXT context = {0};
    context.Rip = 0x12345678;
    context.Rsp = 0x9ABCDEF0;

    InitStack(dwImageType, frame, context);

    EXPECT_EQ(dwImageType, IMAGE_FILE_MACHINE_AMD64);
    EXPECT_EQ(frame.AddrPC.Offset, 0x12345678);
    EXPECT_EQ(frame.AddrPC.Mode, AddrModeFlat);
    EXPECT_EQ(frame.AddrFrame.Offset, 0x9ABCDEF0);
    EXPECT_EQ(frame.AddrFrame.Mode, AddrModeFlat);
    EXPECT_EQ(frame.AddrStack.Offset, 0x9ABCDEF0);
    EXPECT_EQ(frame.AddrStack.Mode, AddrModeFlat);
#elif _M_IA64

#else
#error "Platform not supported!"
#endif
}

#endif

#ifdef __linux__
TEST(CrashReporterTest, RegisterHandlersTest001)
{
    CrashReporter::RegisterHandlers();
    EXPECT_EQ(1, 1);
}

#elif defined(_WIN32)
TEST(CrashReporterTest, VectoredExceptionHandlerTest001)
{
    EXCEPTION_RECORD exceptionRecord = {0};
    EXCEPTION_POINTERS exceptionPointers = {0};
    exceptionPointers.ExceptionRecord = &exceptionRecord;
    exceptionPointers.ContextRecord = new CONTEXT();
    exceptionPointers.ContextRecord->Rip = 0x12345678;
    exceptionPointers.ContextRecord->Rsp = 0x87654321;

    LONG result = VectoredExceptionHandler(&exceptionPointers);

    EXPECT_EQ(result, EXCEPTION_EXECUTE_HANDLER);
    delete exceptionPointers.ContextRecord;
}

TEST(CrashReporterTest, RegisterHandlersTest002)
{
    CrashReporter::RegisterHandlers();
    EXPECT_EQ(1, 1);
}


#elif defined(__APPLE__)

#endif