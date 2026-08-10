# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB CHECKERS
        CheckersAndRules/AddedDeletedDecls.cpp
        CheckersAndRules/FuncParamsAndReturn.cpp
        CheckersAndRules/ModifiedDecls.cpp
        CheckersAndRules/GenericParamAndConstraint.cpp
        CheckersAndRules/ModifiedMemberFunc.cpp
        CheckersAndRules/DeletedChangedInheritedTypes.cpp
        CheckersAndRules/ModifiedDeclModifier.cpp
        CheckersAndRules/ModifiedPackageDecl.cpp
        CheckersAndRules/ClassInstanceMemerFuncs.cpp
        CheckersAndRules/ModifiedMemberVar.cpp
)

file(GLOB SRC_FILES
        Option/Option.cpp
        CjoLoader/CjoLoader.cpp
        Diff/Diff.cpp
        Dsl/Dsl.cpp
        Logger/Logger.cpp
        ${CHECKERS}
        Cjcompat.cpp
        main.cpp
)

add_executable(cjcompat ${SRC_FILES})


target_link_libraries(
        cjcompat
        cangjie-lsp
        ${labList}
        )
if(NOT APPLE AND NOT WIN32)
    # Need to link <experimental/filesystem> library.
    target_link_libraries(cjcompat stdc++fs)
endif()
if (CMAKE_BUILD_TYPE MATCHES Release AND MINGW)
    target_link_options(cjcompat PRIVATE -static-libgcc -static-libstdc++ LINKER:-Bstatic,--whole-archive -lwinpthread LINKER:-Bdynamic,--no-whole-archive)
endif ()
target_compile_options(cjcompat PRIVATE -fPIC)
SET_TARGET_PROPERTIES(cjcompat PROPERTIES OUTPUT_NAME "cjcompat")

install(TARGETS cjcompat RUNTIME DESTINATION bin)