# Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

option(USE_CXX17_FEATURES "use C++17 features" OFF)

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror>)

add_subdirectory(Data)
add_subdirectory(Analyzer)

set(CJPROF_SRC_OBJECTS
        $<TARGET_OBJECTS:Data>
        $<TARGET_OBJECTS:Analyzer>)

if(CMAKE_BACKEND STREQUAL "cjnative")
    add_library(prof SHARED ${CJPROF_SRC_OBJECTS} Cjprof.cpp)
else()
    add_library(prof ${CJPROF_SRC_OBJECTS})
endif()

if (CMAKE_BUILD_TYPE MATCHES Release AND MINGW)
    target_link_options(prof PRIVATE -static-libgcc -static-libstdc++ LINKER:-Bstatic,--whole-archive -lwinpthread LINKER:-Bdynamic,--no-whole-archive)
endif ()
target_compile_options(prof PRIVATE -fPIC)
SET_TARGET_PROPERTIES(prof PROPERTIES OUTPUT_NAME "cjprof")
install(TARGETS prof LIBRARY DESTINATION lib RUNTIME DESTINATION lib)

if(USE_CXX17_FEATURES)
    add_compile_definitions(USE_CXX17_FEATURES)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(Commands)
    add_subdirectory(Recorder)
    add_subdirectory(Reporter)
    add_subdirectory(Symbol)

    add_executable(cjprof
        main.cpp
        $<TARGET_OBJECTS:Commands>
        $<TARGET_OBJECTS:Recorder>
        $<TARGET_OBJECTS:Reporter>
        $<TARGET_OBJECTS:Symbol>
    )
else()
    add_subdirectory(Commands)
    add_subdirectory(Symbol)

    add_executable(cjprof
        main.cpp
        $<TARGET_OBJECTS:Commands>
        $<TARGET_OBJECTS:Symbol>
    )
endif()

add_custom_command(TARGET cjprof POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/static
    $<TARGET_FILE_DIR:cjprof>/../config/static
)

if (NOT USE_CXX17_FEATURES)
    target_link_libraries(prof stdc++fs)
endif()
if(WIN32)
    target_link_libraries(prof ws2_32)
endif()
if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(prof pthread)
endif()

target_link_libraries(cjprof prof cangjie-demangle pthread)
install(TARGETS cjprof RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/static/ DESTINATION config/static)