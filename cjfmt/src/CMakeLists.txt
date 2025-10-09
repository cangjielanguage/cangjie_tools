# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

add_subdirectory(Format)
add_executable(cjfmt main.cpp)
target_link_libraries(cjfmt
        cangjie
        CangjieFormat
        ${labList}
        )
if (CMAKE_BUILD_TYPE MATCHES Release AND MINGW)
    target_link_options(cjfmt PRIVATE -static-libgcc -static-libstdc++ LINKER:-Bstatic,--whole-archive -lwinpthread LINKER:-Bdynamic,--no-whole-archive)
endif ()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(cjfmt PROPERTIES LINK_FLAGS "${LINK_FLAGS} -Wl,-rpath,@loader_path/../lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(cjfmt PROPERTIES LINK_FLAGS "-Wl,-rpath,\\$ORIGIN/../lib")
else()
endif()
