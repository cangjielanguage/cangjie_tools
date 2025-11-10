// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJ_HEAD_PARAM_H
#define CJ_HEAD_PARAM_H
#include <string>
namespace CJHead {
struct CJHeadParam {
    std::string input_dir_;

    std::string output_dir_;

    std::string import_path_;

    std::string exclude_config_;

    std::string macro_path_;

    bool package_mode_;
};
}  // namespace CJHead

#endif