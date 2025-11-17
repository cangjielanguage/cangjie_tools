# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

add_compile_definitions(NOT_RELEASE_VERSION_CODE)
include_directories(../third_party/flatbuffers/include)
set(FLATBUFFERS_BINARY_DIR ../third_party/flatbuffers/build)
link_directories(${LIBRARY_OUTPUT_PATH})
link_directories(${FLATBUFFERS_BINARY_DIR})
# declare the path of JSON header file
set(JSON_INCLUDE_DIR ../third_party/json-v3.11.3/include)
include_directories(${JSON_INCLUDE_DIR})
# declare the path of Cangjie header file
set(CANGJIE_INCLUDE_DIR lib/cangjie/include)
set(CANGJIE_LIB_DIR lib/cangjie/lib/cjnative)
if (COMPILER_TYPE MATCHES "MINGW")    #Compiling Using the MinGW
    # declare the link library path of Cangjie
    message("cjnative windows")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
    if (CMAKE_BUILD_TYPE MATCHES "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2 -O2")
    endif ()

    # add Cangjie header file and library
    include_directories(${CANGJIE_INCLUDE_DIR})
    link_directories(${CANGJIE_LIB_DIR})

    add_subdirectory(languageserver)
    add_subdirectory(json-rpc)

    # Add a command for the linker to statically link libgcc and libstdc++.
    # If the command is not added, the compiled exe file cannot run on other computers.
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -lssp -Wl,--no-whole-archive -Wl,--no-insert-timestamp")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-insert-timestamp")

    add_executable(LSPServer launcher/main.cpp)
    # link library
    target_link_libraries(LSPServer
            pthread
            JsonRPC
            LSP
            ${CMAKE_DL_LIBS}
            libflatbuffers.a
            -ldbghelp
            -limagehlp
            )
    target_link_libraries(LSPServer -lcangjie-lsp)
    add_library(cangjie-lsp SHARED IMPORTED)
    set_target_properties(cangjie-lsp PROPERTIES
            IMPORTED_LOCATION ${CANGJIE_LIB_DIR}/libcangjie-lsp.dll
            )
    set_target_properties(cangjie-lsp PROPERTIES
            IMPORTED_IMPLIB ${CANGJIE_LIB_DIR}/libcangjie-lsp.dll.a
            )
    add_library(LSPServerlib launcher/main.cpp)
   target_link_libraries(LSPServerlib
            pthread
            JsonRPC
            LSP
            ${CMAKE_DL_LIBS}
            )
elseif (COMPILER_TYPE MATCHES "Ninja")    #Compiling Using the ninja
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
    if (CMAKE_BUILD_TYPE MATCHES "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2 -O2")
    endif ()
    # declare the link library path of Cangjie
    if (APPLE)
        message("APPLE  +++++++++")
    endif()
    if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "Release")
        message(STATUS "release build type selected, set safe compile flag")
        if (APPLE)
            message("APPLE CMAKE_CXX_FLAGS ++++++")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftrapv -fpie")
        elseif(CROSS_WINDOWS)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wdate-time")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftrapv")
        endif ()
    endif ()
    if (NOT APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    endif ()

    # add Cangjie header file and library
    include_directories(${CANGJIE_INCLUDE_DIR})
    link_directories(${CANGJIE_LIB_DIR})

    add_subdirectory(languageserver)
    add_subdirectory(json-rpc)
    add_executable(LSPServer launcher/main.cpp)
    # link library
    target_link_libraries(LSPServer
            pthread
            JsonRPC
            LSP
            libflatbuffers.a
            )
    if (NOT CROSS_WINDOWS)
        target_link_libraries(LSPServer ${CMAKE_DL_LIBS}
                )
    else()
        target_link_libraries(LSPServer
                -ldbghelp
                -limagehlp
                )
    endif ()
    if(APPLE)
        target_link_libraries(LSPServer libcangjie-lsp.dylib)
        add_library(cangjie-lsp SHARED IMPORTED)
        set_target_properties(cangjie-lsp PROPERTIES
                IMPORTED_LOCATION ${CANGJIE_LIB_DIR}/libcangjie-lsp.dylib
                )
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,@loader_path/../lib")
    elseif(CROSS_WINDOWS)
        target_link_libraries(LSPServer -lcangjie-lsp)
        add_library(cangjie-lsp SHARED IMPORTED)
        set_target_properties(cangjie-lsp PROPERTIES
                IMPORTED_LOCATION ${CANGJIE_LIB_DIR}/libcangjie-lsp.dll
                )
        set_target_properties(cangjie-lsp PROPERTIES
                IMPORTED_IMPLIB ${CANGJIE_LIB_DIR}/libcangjie-lsp.dll.a
                )
        target_compile_options(LSPServer PRIVATE -fno-common)
        target_link_options(LSPServer PRIVATE -Wl,--allow-multiple-definition)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-insert-timestamp")
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-insert-timestamp")
    else()
        set(LINK_FLAGS "-Wl,-z,relro,-z,now,-z,noexecstack")
        set(STRIP_FLAG "-s")
        set(SAFE_EXE_LINK_FLAG "-pie")
        set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS} ${SAFE_EXE_LINK_FLAG} ${STRIP_FLAG}")
        set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS} ${STRIP_FLAG}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,@loader_path/../lib")
        target_link_libraries(LSPServer libcangjie-lsp.so ${CMAKE_DL_LIBS})
        add_library(cangjie-lsp SHARED IMPORTED)
        set_target_properties(cangjie-lsp PROPERTIES
                IMPORTED_LOCATION ${CANGJIE_LIB_DIR}/libcangjie-lsp.so
                )
    endif()

    add_library(LSPServerlib launcher/main.cpp)
    target_link_libraries(LSPServerlib
            pthread
            JsonRPC
            LSP
            )
    if (NOT CROSS_WINDOWS)
        target_link_libraries(LSPServerlib ${CMAKE_DL_LIBS}
                )
    endif ()
else ()
    message(WARNING "Use the [cmake -DCOMPILER_TYPE=MSVC/MINGW/Ninja .] command to set the compiler type.")
endif ()