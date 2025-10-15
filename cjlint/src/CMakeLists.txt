# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

add_subdirectory(checker)
add_subdirectory(common)
add_subdirectory(rules)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(lint SHARED Cjlint.cpp)
if(COVERAGE_FLAG)
    target_compile_options(lint PUBLIC "-fno-exceptions")
endif()
target_link_libraries(lint
        checker
        structuralrule
        dataflowrule
        commonlib
        cangjie-lsp
        ${labList}
        )
if (CMAKE_BUILD_TYPE MATCHES Release AND MINGW)
    target_link_options(lint PRIVATE -static-libgcc -static-libstdc++ LINKER:-Bstatic,--whole-archive -lwinpthread LINKER:-Bdynamic,--no-whole-archive)
endif ()
target_compile_options(lint PRIVATE -fPIC)
SET_TARGET_PROPERTIES(lint PROPERTIES OUTPUT_NAME "cjlint")
install(TARGETS lint LIBRARY DESTINATION lib RUNTIME DESTINATION lib)

add_executable(cjlint main.cpp)
target_link_libraries(cjlint lint)
install(TARGETS cjlint RUNTIME DESTINATION bin)