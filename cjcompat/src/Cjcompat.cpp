// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/Cjcompat.h"

using namespace Cangjie;

namespace CjComplienceChecker {

int CJcompat(const CjComplienceChecker::Options& options)
{
    bool enableAPI = options.enableAPI;
    bool enableABI = options.enableABI;

    auto cjoPathOld = options.cjoPathOld;
    auto cjoModulesPath = options.cjoModulesPath;

    CjoLoader cjoLoader1{cjoPathOld, cjoModulesPath, options.cangjiePaths, options.importPaths};
    if (!cjoLoader1.Load()) {
        Errorln("Cannot load file ", cjoPathOld);
        Infoln("Check if the .cjo file of the package and all packages it depends on exists in "
            "CANGJIE_PATH or CANGJIE_HOME, or use '--import-path' to specify the .cjo file path.");
        return ERR;
    }

    auto cjoPathNew = options.cjoPathNew;
    CjoLoader cjoLoader2{cjoPathNew, cjoModulesPath, options.cangjiePaths, options.importPaths};
    if (!cjoLoader2.Load()) {
        Errorln("Cannot load file ", cjoPathNew);
        Infoln("Check if the .cjo file of the package and all packages it depends on exists in "
            "CANGJIE_PATH or CANGJIE_HOME, or use '--import-path' to specify the .cjo file path.");
        return ERR;
    }

    auto pkg1 = cjoLoader1.GetPackage();
    auto pkg2 = cjoLoader2.GetPackage();
    if (pkg1 == nullptr || pkg2 == nullptr) {
        Errorln("illegal package");
        return ERR;
    }

    Diff diff(*pkg1, *pkg2, cjoLoader1.GetTypeManager(), cjoLoader2.GetTypeManager());
    diff.CalculateDiff();
#ifndef NDEBUG
    if (options.printDebugInfo) {
        diff.Print();
    }
#endif

    Dsl dsl{cjoLoader1.GetTypeManager(), cjoLoader2.GetTypeManager(), enableAPI, enableABI};
    AllCheckers checkers;
    Logger logger{enableAPI, enableABI};
    checkers.RunAll(dsl, diff, logger);

    logger.Print(cjoLoader1, cjoLoader2);

    return OK;
}
} // namespace CjComplienceChecker
