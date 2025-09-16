# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

file(GLOB FORMAT_SRC "*.cpp" "NodeFormatter/Decl/*.cpp" "NodeFormatter/Node/*.cpp" "NodeFormatter/Pattern/*.cpp" "NodeFormatter/Expr/*.cpp" "NodeFormatter/Type/*.cpp"  "DocProcessor/*.cpp")
add_library(CangjieFormat OBJECT ${FORMAT_SRC} ${AUTODIFF_SRC})
if(CJFMT_CODE_COVERAGE)
    target_compile_options(CangjieFormat PUBLIC "-fno-exceptions")
endif()