// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import cjpm.config.*

// LTO/combine mode classification and CompileTask flag derivation.
//
// Build-level mode (resolveLtoCombineBuildMode): describes the whole build from BuildConfig.
// Task-level mode (resolveCompileTaskBuildMode): describes one CompileTask's role in linking.

// Build-level LTO/combine mode derived from BuildConfig.
public enum LtoCombineBuildMode {
    // No LTO and no combine.
    | Normal
    // profile.build.lto only; packages compile independently.
    | LtoOnly
    // profile.build.combined only; module root links sub-packages into one dylib.
    | ModuleCombine
    // combined + LTO; sub-packages emit .bc, module root performs LTO link.
    | ModuleLtoCombine
    // combine-all-deps + combined + LTO; project root links the entire dependency tree.
    | ProjectLtoCombine

    public prop usesLto: Bool {
        get() {
            match (this) {
                case LtoOnly | ModuleLtoCombine | ProjectLtoCombine => true
                case Normal | ModuleCombine => false
            }
        }
    }

    public prop usesModuleCombine: Bool {
        get() {
            match (this) {
                case ModuleCombine | ModuleLtoCombine | ProjectLtoCombine => true
                case Normal | LtoOnly => false
            }
        }
    }

    public prop usesProjectCombine: Bool {
        get() {
            match (this) {
                case ProjectLtoCombine => true
                case Normal | LtoOnly | ModuleCombine | ModuleLtoCombine => false
            }
        }
    }

    public prop usesModuleLtoCombine: Bool {
        get() {
            match (this) {
                case ModuleLtoCombine => true
                case Normal | LtoOnly | ModuleCombine | ProjectLtoCombine => false
            }
        }
    }
}

// Per CompileTask LTO/combine flags passed to the compiler and linker.
public struct CompileTaskCombineFlags {
    public CompileTaskCombineFlags(
        public let isLto!: Bool = false,
        public let isCombined!: Bool = false,
        public let isProjectCombined!: Bool = false,
        public let ltoValue!: String = "",
        public let keepPkgVisibility!: Option<Array<String>> = None
    ) { }

    public prop buildMode: LtoCombineBuildMode {
        get() { resolveCompileTaskBuildMode(this) }
    }

    public prop usesLto: Bool {
        get() { isLto }
    }

    public prop usesModuleCombine: Bool {
        get() { isCombined }
    }

    public prop usesProjectCombine: Bool {
        get() { isProjectCombined }
    }
}

// Which link-argument builder getDepLinkCommand should invoke.
public enum CompileTaskLinkStrategy {
    | ModuleCombineLink    // getBuildArgs + getForcedLibArgs (--whole-archive)
    | ProjectCombineLink   // getLtoCombineBuildArgs (project lto-combine root)
    | StandardLink         // getBuildArgs (normal exe/dylib)
    | NoLink
}

func resolveLtoCombineBuildMode(buildConfig: BuildConfig): LtoCombineBuildMode {
    if (buildConfig.globalConfig.isProjectCombine) {
        return ProjectLtoCombine
    }
    if (!buildConfig.superPkgMap.isEmpty() && buildConfig.isLto) {
        return ModuleLtoCombine
    }
    if (!buildConfig.superPkgMap.isEmpty()) {
        return ModuleCombine
    }
    if (buildConfig.isLto) {
        return LtoOnly
    }
    Normal
}

func resolveCompileTaskBuildMode(combineFlags: CompileTaskCombineFlags): LtoCombineBuildMode {
    if (combineFlags.isProjectCombined) {
        return ProjectLtoCombine
    }
    if (combineFlags.isCombined && combineFlags.isLto) {
        return ModuleLtoCombine
    }
    if (combineFlags.isCombined) {
        return ModuleCombine
    }
    if (combineFlags.isLto) {
        return LtoOnly
    }
    Normal
}

// Derive isLto/isCombined/isProjectCombined for one product CompileTask.
// LTO follows the actual compile target; combine flags require a product source set.
func resolveCompileTaskCombineFlags(buildConfig: BuildConfig, fullName: String, product: Bool,
    hasSuperPkgCfg: Bool, compileTarget: String, isNativeForCross: Bool): CompileTaskCombineFlags {
    let rootName = buildConfig.globalConfig.rootName
    let isCombined = product && hasSuperPkgCfg && !isNativeForCross
    let isProjectCombined =
        product &&
        buildConfig.globalConfig.isProjectCombine &&
        fullName == rootName &&
        !isNativeForCross
    let isLto = buildConfig.isLto && supportLtoForTarget(compileTarget)
    CompileTaskCombineFlags(
        isLto: isLto,
        isCombined: isCombined,
        isProjectCombined: isProjectCombined,
        ltoValue: if (isLto) { buildConfig.ltoValue } else { "" },
        keepPkgVisibility: if (isLto) { buildConfig.keepPkgVisibility } else { Option<Array<String>>.None }
    )
}

func resolveCompileTaskLinkStrategy(compileTask: CompileTask): CompileTaskLinkStrategy {
    let flags = compileTask.combineFlags
    // Project lto-combine root may also have isCombined; check project combine first.
    if (flags.isProjectCombined) {
        return ProjectCombineLink
    }
    if (flags.isCombined) {
        return ModuleCombineLink
    }
    if (compileTask.outputType == Exe || compileTask.outputType == Dynamic) {
        return StandardLink
    }
    NoLink
}

// Non-LTO static, splittable, and CHIR tasks only require dependency type info (.cjo).
func compileTaskRequiresOnlyTypeInfo(combineFlags: CompileTaskCombineFlags, outputType: OutputType,
    isSplittable: Bool): Bool {
    !combineFlags.isLto && (outputType == Static || isSplittable || outputType == Chir)
}

// Pipeline parallel split-compile is incompatible with LTO.
func compileTaskIsSplittable(combineFlags: CompileTaskCombineFlags, product: Bool, isMultiplatform: Bool,
    isCompilePipelineParallel: Bool, isMacro: Bool, outputType: OutputType): Bool {
    !isMultiplatform &&
        !combineFlags.isLto &&
        product &&
        isCompilePipelineParallel &&
        !isMacro &&
        (outputType == Dynamic || outputType == Exe)
}

func isIosProjectCombineCompileTask(compileTask: CompileTask): Bool {
    isIosProjectCombineRoot(compileTask.combineFlags.isProjectCombined, getCompileTarget())
}

// iOS module lto-combine superPkg root (ModuleCombineLink, not combine-all-deps).
func isIosModuleLtoCombineRootCompileTask(compileTask: CompileTask): Bool {
    let flags = compileTask.combineFlags
    isIosTarget(getCompileTarget()) && flags.isLto && flags.isCombined && !flags.isProjectCombined
}

// iOS project or module lto-combine root performing the merge link / native staticlib output.
func isIosLtoCombineLinkTask(compileTask: CompileTask): Bool {
    let flags = compileTask.combineFlags
    if (!isIosTarget(getCompileTarget()) || !flags.isLto) {
        return false
    }
    flags.isProjectCombined || flags.isCombined
}

// Append --lto and related compiler flags for one CompileTask.
func appendLtoCompileArgs(res: ArrayList<String>, compileTask: CompileTask): Unit {
    let flags = compileTask.combineFlags
    if (!flags.isLto) {
        return
    }
    res.add("--lto=${flags.ltoValue}")
    if (isIosTarget(getCompileTarget())) {
        res.add("--experimental")
    }

    // Module iOS LTO-combine: add "--dy-std"
    if (isIosModuleLtoCombineRootCompileTask(compileTask)) {
        res.add("--dy-std")
    }

    if ((compileTask.outputType == Dynamic || isIosLtoCombineLinkTask(compileTask)) &&
        flags.keepPkgVisibility.isSome()) {
        let keepPkgVisibilityStr = String.join(flags.keepPkgVisibility.getOrThrow(), delimiter: ",")
        res.add(all: extractOptionByString("--lto-keep-pkg-visibility=\"${keepPkgVisibilityStr}\""))
    }
    if (isIosLtoCombineLinkTask(compileTask)) {
        res.add(all: getIosLtoCombineNativeStaticlibCompileArgs())
    }
}
