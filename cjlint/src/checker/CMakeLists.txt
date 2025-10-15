# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

include_directories(${CMAKE_SOURCE_DIR}/src)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
aux_source_directory(./ ALL_SRCS)
add_library(checker STATIC ${ALL_SRCS})
if(COVERAGE_FLAG)
    target_compile_options(checker PUBLIC "-fno-exceptions")
endif()