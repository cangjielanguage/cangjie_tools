# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.
set(CMAKE_BUILD_TYPE Release)

file (
     GLOB SRC_FILES
     HeadFileVisitor.cpp
     main.cpp
     CJHeadCompilerInstance.cpp
     FileHandler.cpp
     CJHeadCompilerInvocation.cpp
     CJHeadUtil.cpp
     )

set(CANGJIE_FORMAT ${CMAKE_CURRENT_SOURCE_DIR}/../fmt)

file (GLOB FORMAT_FILES "${CANGJIE_FORMAT}/src/Format/*.cpp"
    "${CANGJIE_FORMAT}/src/Format/DocProcessor/*.cpp"
    "${CANGJIE_FORMAT}/src/Format/NodeFomatter/*.cpp"
    "${CANGJIE_FORMAT}/src/Format/NodeFomatter/Decl/*.cpp"
    "${CANGJIE_FORMAT}/src/Format/NodeFomatter/Expr/*.cpp"
    "${CANGJIE_FORMAT}/src/Format/NodeFomatter/Node/*.cpp"
    "${CANGJIE_FORMAT}/src/Format/NodeFomatter/Pattern/*.cpp"
    "${CANGJIE_FORMAT}/src/Format/NodeFomatter/Type/*.cpp"
)

add_executable(cjhead ${SRC_FILES} ${FORMAT_FILES})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_link_libraries(cjhead ${CANGJIE_ROOT}/build/build/lib/libcangjie-lsp.a)
target_link_libraries(cjhead ${CANGJIE_ROOT}/build/build/lib/libboundscheck-static.a)
target_include_directories(cjhead PRIVATE ${CMAKE_SOURCE_DIR}/src/include)
target_include_directories(cjhead PRIVATE ${JSON_HEADERS})
target_include_directories(cjhead PRIVATE ${CANGJIE_HEADERS})
target_include_directories(cjhead PRIVATE ${HUAWEI_SECURE_C_HEADERS})

# target_link_libraries(cjhead cangjie-lsp)

if(NOT WIN32)
  target_link_libraries(cjhead pthread dl)
endif()

target_compile_features(cjhead PRIVATE cxx_std_17)
target_compile_options(cjhead PRIVATE -std=c++17)
target_link_libraries(cjhead stdc++fs)

target_include_directories(cjhead PRIVATE ${CANGJIE_FORMAT}/include/Format ${CANGJIE_FORMAT}/include/)
target_compile_options(cjhead PRIVATE -Wall -Wextra -O2)