# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

# GTest-based unit tests for the C++ API in src/Cjprof.cpp (namespace Cjprof).
# Linked against the `prof` shared library target (libcjprof.so) built by src/.
# No JVM/JNI runtime is needed — these tests exercise the C++ layer directly.

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

set(CJPROF_TEST_SRCS
    test_cjprof_smoke.cpp
    test_cjprof_expand.cpp
    test_cjprof_diff.cpp
)

add_executable(cjprof_gtest ${CJPROF_TEST_SRCS})
target_link_libraries(cjprof_gtest PRIVATE
    prof
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

# Bake the prof (libcjprof.so) build dir into the test binary's RPATH so the
# binary can be run by gtest_discover_tests at build time without needing
# LD_LIBRARY_PATH. -Wl,-rpath, is what actually lands in the binary;
# BUILD_RPATH with a generator expression is unreliable on cmake 3.16.
target_link_options(cjprof_gtest PRIVATE
    "-Wl,-rpath,$<TARGET_FILE_DIR:prof>")

# Locate heap snapshot test data. The .data files (heap.data,
# heap_multi_ref.data) ship inside the cjprof repo under tests/, so the tests
# are self-contained and do not depend on a sibling cangjie_test checkout.
# Overridable via -DCJPROF_TESTDATA_DIR=... (e.g. when building out-of-tree).
if(NOT DEFINED CJPROF_TESTDATA_DIR)
    set(CJPROF_TESTDATA_DIR "${CMAKE_SOURCE_DIR}/tests")
endif()
target_compile_definitions(cjprof_gtest PRIVATE CJPROF_TESTDATA_DIR="${CJPROF_TESTDATA_DIR}")

# gtest_discover_tests in PRE_TEST mode defers test enumeration to ctest run
# time, so the test binary is not executed during the build. The RPATH baked
# above (target_link_options) still resolves libcjprof.so at ctest time.
include(GoogleTest)
gtest_discover_tests(cjprof_gtest
    DISCOVERY_MODE PRE_TEST
    PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:prof>:$ENV{LD_LIBRARY_PATH}")


