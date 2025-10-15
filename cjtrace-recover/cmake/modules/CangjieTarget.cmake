# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

cmake_minimum_required(VERSION 3.10)

function(_find_or_set_program_validator candidate result_var)
    execute_process(
        COMMAND "${candidate}" --version
        RESULT_VARIABLE _vres
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if(_vres EQUAL 0)
        set(${result_var} TRUE PARENT_SCOPE)
    else()
        set(${result_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

# search for cjc, check for env and cmake definitions
find_program(cjc_path cjc
    PATHS "${CANGJIE_HOME}/bin" "$ENV{CANGJIE_HOME}/bin"
    VALIDATOR _find_or_set_program_validator REQUIRED NO_DEFAULT_PATH)

get_filename_component(cangjie_bin_path ${cjc_path} DIRECTORY)
get_filename_component(target_cangjie_home ${cangjie_bin_path} DIRECTORY)
message(STATUS "found cangjie toolchain path: ${target_cangjie_home}")

# generate cangjie triple
if(CMAKE_CROSSCOMPILING)
    set(CANGJIE_LIB_TRIPLE "${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}_cjnative")
else()
    set(CANGJIE_LIB_TRIPLE "${CMAKE_SYSTEM_NAME}_${CMAKE_HOST_SYSTEM_PROCESSOR}_cjnative")
endif()
string(TOLOWER "${CANGJIE_LIB_TRIPLE}" CANGJIE_LIB_TRIPLE)

set(CANGJIE_TOOLCHAIN_PATH ${target_cangjie_home} CACHE FILEPATH "cangjie toolchain path")
set(CANGJIE_COMPILER_PATH ${cjc_path} CACHE FILEPATH "cangjie compiler path")
set(CANGJIE_DYNAMIC_LIBRARY_PATH ${target_cangjie_home}/runtime/lib/${CANGJIE_LIB_TRIPLE} CACHE FILEPATH "cangjie dynamic library path")
set(CANGJIE_STATIC_LIBRARY_PATH ${target_cangjie_home}/lib/${CANGJIE_LIB_TRIPLE} CACHE FILEPATH "cangjie static library path")

function(add_cangjie_executable target)
    set(multi_value_args SOURCES COMPILE_OPTIONS DEPENDS)
    cmake_parse_arguments(
        CANGJIE_TARGET
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN})

    set(full_path_sources)
    foreach(arg ${CANGJIE_TARGET_SOURCES})
        find_file(source_code ${arg} PATHS ${CMAKE_CURRENT_SOURCE_DIR} REQUIRED NO_DEFAULT_PATH)
        list(APPEND full_path_sources ${source_code})
    endforeach()

    set(cangjie_compile_flags)
    # build types
    # RelWithDebInfo and MinSizeRelWithDebInfo and Debug
    if(CMAKE_BUILD_TYPE MATCHES "WithDebInfo$" OR (CMAKE_BUILD_TYPE MATCHES "^Debug$"))
        list(APPEND cangjie_compile_flags "-g")
    endif()
    # MinSizeRel and MinSizeRelWithDebInfo
    if(CMAKE_BUILD_TYPE MATCHES "^MinSize")
        list(APPEND cangjie_compile_flags "-Os")
    endif()
    # RelWithDebInfo and Release
    if(CMAKE_BUILD_TYPE MATCHES "^Rel")
        list(APPEND cangjie_compile_flags "-O2")
    endif()

    # cross compile configurations
    if(CMAKE_CROSSCOMPILING)
        if(NOT CANGJIE_TRIPLE)
            message(FATAL_ERROR "Please define CANGJIE_TRIPLE in your toolchain file")
        endif()
        list(APPEND cangjie_compile_flags "--target=${CANGJIE_TRIPLE}")
        list(APPEND cangjie_compile_flags "--sysroot=${CMAKE_SYSROOT}")
    endif()
    
    # depends
    foreach(arg ${CANGJIE_TARGET_DEPENDS})
        if (NOT TARGET ${arg})
            message(FATAL_ERROR "${arg} is not a target")
        endif()
        list(APPEND cangjie_compile_flags -L$<TARGET_FILE_DIR:${arg}>)
        list(APPEND cangjie_compile_flags -l${arg})
    endforeach()

    # move compile options to the last, avoiding depends library's deps cannot be found
    list(APPEND cangjie_compile_flags ${CANGJIE_TARGET_COMPILE_OPTIONS})

    set(output ${target}$<$<BOOL:${WIN32}>:.exe>)
    add_custom_target(cj_${target} ALL
        COMMAND ${CANGJIE_COMPILER_PATH} -o ${output} ${cangjie_compile_flags} ${full_path_sources}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${output}
        SOURCES ${full_path_sources}
    )
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${output} DESTINATION bin)
    
endfunction()
