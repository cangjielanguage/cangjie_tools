# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

include_directories(${CMAKE_SOURCE_DIR}/src)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
aux_source_directory(./ ALL_SRCS)
aux_source_directory(./dataflow_rule_G_CHK_01 G_CHK_01)
aux_source_directory(./dataflow_rule_G_SER_03 G_SER_03)
aux_source_directory(./dataflow_rule_G_FIO_01 G_FIO_01)
aux_source_directory(./dataflow_rule_P_02 P_02)
aux_source_directory(./dataflow_rule_G_OTH_01 G_OTH_01)
aux_source_directory(./dataflow_rule_G_VAR_01 G_VAR_01)
aux_source_directory(./dataflow_rule_G_CHK_03 G_CHK_03)
add_library(dataflowrule STATIC ${ALL_SRCS} ${G_CHK_01} ${G_SER_03} ${G_FIO_01} ${P_02} ${G_OTH_01} ${G_VAR_01} ${G_CHK_03})
if(COVERAGE_FLAG)
    target_compile_options(dataflowrule PUBLIC "-fno-exceptions")
endif()
