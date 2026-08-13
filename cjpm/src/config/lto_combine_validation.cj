// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.fs.*

// LTO/combine validation at cjpm.toml load time (called from verifyModuleConfigPre).

public func isLtoConfiguredInToml(tomlInfo: TomlInfo): Bool {
    !tomlInfo.profile.build.ltoValue.isEmpty() || !tomlInfo.profile.build.lto.level.isEmpty()
}

public func reportIosCombinedRequiresLto(): Unit {
    eprintln("Error: on iOS, 'profile.build.combined' requires 'profile.build.lto' to be enabled; " +
        "module-level combine without LTO is not supported")
}

// Validate profile.build.combined field shape and emit experimental warning.
public func verifyCombinedField(cfg: TomlInfo, path: String): Bool {
    var flag = true
    let configFilePath = Path(path).join(CONFIG_FILE_NAME).toString()
    let compileTarget = getCompileTarget()
    for ((pkgName, outputType) in cfg.profile.build.combined) {
        if (pkgName != cfg.pkg.name) {
            eprintln("Error: field 'profile.build.combined' in ${configFilePath} " +
                "can only be used for root package '${cfg.pkg.name}', current value is '${pkgName}'")
            flag = false
            continue
        }
        if (isIosTarget(compileTarget)) {
            // iOS only supports module lto-combine (static root). LTO may live on the build root toml.
            if (outputType != Static) {
                eprintln("Error: field 'profile.build.combined.${cfg.pkg.name}' in " +
                    "${configFilePath} must be configured as 'static' on iOS")
                flag = false
                continue
            }
        } else if (outputType != Dynamic) {
            // Non-iOS combined modules must produce a dynamic root library.
            eprintln("Error: field 'profile.build.combined.${cfg.pkg.name}' in " +
                "${configFilePath} can only be configured as 'dynamic'")
            flag = false
            continue
        }
        eprintln("Warning: field 'profile.build.combined' in '${configFilePath}' " +
            "is experimental, it is unstable and has some limitations, " +
            "please refer to related documentation for more details")
    }
    return flag
}
