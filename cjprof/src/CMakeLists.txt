# Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

option(USE_CXX17_FEATURES "use C++17 features" OFF)

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror>)

if(USE_CXX17_FEATURES)
    add_compile_definitions(USE_CXX17_FEATURES)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(Data)
    add_subdirectory(Analyzer)
    add_subdirectory(Commands)
    add_subdirectory(Recorder)
    add_subdirectory(Reporter)
    add_subdirectory(Symbol)

    add_executable(cjprof
        main.cpp
        $<TARGET_OBJECTS:Data>
        $<TARGET_OBJECTS:Analyzer>
        $<TARGET_OBJECTS:Commands>
        $<TARGET_OBJECTS:Recorder>
        $<TARGET_OBJECTS:Reporter>
        $<TARGET_OBJECTS:Symbol>
    )
else()
    add_subdirectory(Data)
    add_subdirectory(Analyzer)
    add_subdirectory(Commands)
    add_subdirectory(Symbol)

    add_executable(cjprof
        main.cpp
        $<TARGET_OBJECTS:Data>
        $<TARGET_OBJECTS:Analyzer>
        $<TARGET_OBJECTS:Commands>
        $<TARGET_OBJECTS:Symbol>
    )
endif()

target_link_libraries(cjprof cangjie-demangle pthread)
install(TARGETS cjprof RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})