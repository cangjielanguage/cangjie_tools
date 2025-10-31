// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

public class CommandFactory {
    public func createCommandHandler(args: Array<String>): Handle {
        if (args.size == 0) {
            return CpmCommand(args)
        }

        let cmd = args[0]
        let handle: Handle = match (cmd) {
            case "build" => BuildCommand(args)
            case "clean" => CleanCommand(args)
            case "init" => InitCommand(args)
            case "update" => UpdateCommand(args)
            case "test" => TestCommand(args)
            case "bench" => BenchCommand(args)
            case "check" => CheckCommand(args)
            case "run" => RunCommand(args)
            case "tree" => TreeCommand(args)
            case "install" => InstallCommand(args)
            case "uninstall" => UninstallCommand(args)
            case "bundle" => BundleCommand(args)
            case "publish" => PublishCommand(args)
            case _ => CpmCommand(args)
        }
        return handle
    }
}
