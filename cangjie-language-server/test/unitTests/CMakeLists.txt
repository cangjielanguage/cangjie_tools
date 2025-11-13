# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

set(UNIT_TESTS_SRC
        json-rpc/ProtocolTest.cpp
        languageserver/capabilities/fileRefactor/FileMoveTest.cpp
        languageserver/capabilities/fileRefactor/FileRefactorTest.cpp
        languageserver/capabilities/overrideMethods/FindOverrideMethodsUtilsTest.cpp
        languageserver/capabilities/semanticHighlight/SemanticHighlightImplTest.cpp
        languageserver/common/BasicHelperTest.cpp
        languageserver/common/FindDeclUsageTest.cpp
        languageserver/common/ItemResolverUtilTest.cpp
        languageserver/common/SyscapCheckTest.cpp
        languageserver/common/UtilsTest.cpp
        languageserver/common/UtilsTest2.cpp
        languageserver/index/IndexStorageTest.cpp
        languageserver/index/MemIndexTest.cpp
        languageserver/logger/LoggerTest.cpp
        languageserver/logger/CrashReporterTest.cpp
        languageserver/CompilerCangjieProjectTest.cpp
        languageserver/ArkASTTest.cpp
        languageserver/sql/wrapper/BackupTest.cpp
        languageserver/sql/wrapper/BindTest.cpp
        languageserver/sql/wrapper/ColumnTest.cpp
        languageserver/sql/wrapper/ConfigureTest.cpp
        languageserver/sql/wrapper/ConnectionTest.cpp
        languageserver/sql/wrapper/ExceptionTest.cpp
        languageserver/sql/wrapper/FunctionTest.cpp
        languageserver/sql/wrapper/MemoryTest.cpp
        languageserver/sql/wrapper/RegexpTest.cpp
        languageserver/sql/wrapper/ResultTest.cpp
        languageserver/sql/wrapper/SQLiteAPITest.cpp
        languageserver/sql/wrapper/StatementTest.cpp
        languageserver/sql/wrapper/TransactionTest.cpp
        languageserver/sql/wrapper/ValueTest.cpp
)

include_directories(
        ../../src/json-rpc
        ../../src/languageserver/capabilities/fileRefactor
        ../../src/languageserver/capabilities/overrideMethods
        ../../src/languageserver/capabilities/semanticHighlight
        ../../src/languageserver/common
        ../../src/languageserver/index
        ../../src/languageserver/logger
        ../../src/languageserver/
        ../../src/languageserver/sql/wrapper
)

add_library(UnitTests OBJECT ${UNIT_TESTS_SRC})