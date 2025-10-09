// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.time.DateTime

public func createBuildGn(gconfig: GlobalConfig): String {
    let moduleName = gconfig.cjModuleName

    let buildGnContent = """
// Copyright (c) Huawei Technologies Co., Ltd. ${DateTime.nowUTC().year}. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import("//build/ohos.gni")
import("//third_party/cangjie/build/cjc.gni")

ohos_cj_library("${moduleName}") {
  source_dir = "src"

  cj_deps = [
    {
      label = "//third_party/cangjie/ohos_libs/arkui/cj:ohos.base"
      libname = "ohos.base"
    },
    {
      label = "//arkcompiler/cangjie/cangjie_systemapi/frameworks/hilog:ohos.hilog"
      libname = "ohos.hilog"
    },
    {
      label = "//third_party/cangjie/ohos_libs/interop:ohos.ark_interop"
      libname = "ohos.ark_interop"
    },
    {
      label = "//third_party/cangjie/ohos_libs/interop:ohos.ark_interop_helper"
      libname = "ohos.ark_interop_helper"
    },
    {
      label = "//third_party/cangjie/ohos_libs/ability:ohos.ability"
      libname = "ohos.ability"
    },
  ]

  cjc_args = [ "-O2" ]

  subsystem_name = "arkcompiler"
  part_name = "cangjie_systemapi"
  lib_name = "${moduleName
        .replace("ohos.", '')}"
}
"""
    return buildGnContent
}
