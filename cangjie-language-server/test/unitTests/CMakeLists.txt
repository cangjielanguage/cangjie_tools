# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

set(UNIT_TESTS_SRC
        json-rpc/ProtocolTest.cpp
        languageserver/capabilities/semanticHighlight/SemanticHighlightImplTest.cpp
        languageserver/common/BasicHelperTest.cpp
        languageserver/common/FindDeclUsageTest.cpp
        languageserver/common/ItemResolverUtilTest.cpp
        languageserver/common/UtilsTest.cpp
        languageserver/common/UtilsTest2.cpp
        languageserver/index/MemIndexTest.cpp
        languageserver/logger/LoggerTest.cpp
        languageserver/logger/CrashReporterTest.cpp
        languageserver/ArkASTTest.cpp
)

include_directories(
        ../../src/json-rpc
        ../../src/languageserver/capabilities/semanticHighlight
        ../../src/languageserver/common
        ../../src/languageserver/index
        ../../src/languageserver/logger
        ../../src/languageserver/
)

add_library(UnitTests OBJECT ${UNIT_TESTS_SRC})