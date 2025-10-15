// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.collection.concurrent.*
import std.sync.*
import std.fs.*
import std.regex.*
import std.time.*
import std.env.getVariables
import cjpm.config.*

class ParallelBuilder {
    var buildConfig = BuildConfig()
    var taskMap = HashMap<String, ResolveConfig>()
    let buildMutex = Mutex()
    var successPkgs = ArrayList<String>()
    let queue = LinkedBlockingQueue<(Option<String>, Option<String>, Option<String>, Option<String>, Bool)>()

    init(buildConfig: BuildConfig) {
        this.buildConfig = buildConfig
    }

    func checkDepFuture(dep: String, task: ResolveConfig, futTasks: HashMap<String, Future<Bool>>,
        sem: Semaphore): Bool {
        var fut: Future<Bool>
        synchronized(this.buildMutex) {
            if (!futTasks.contains(dep)) {
                return true
            }
            fut = futTasks[dep]
        }
        if (!fut.get()) {
            sem.release()
            this.queue.add((task.outLogFile, task.errLogFile, None, None, false))
            return false
        }
        return true
    }

    func checkNoChange(depPkg: String): Bool {
        if (buildConfig.rebuildMacros.contains(depPkg)) {
            return false
        }
        let rootPkgName: String = getRootPkgName(depPkg)
        let cjoFile = Path(this.buildConfig.globalConfig.targetDir).join(rootPkgName).join("${depPkg}.cjo").toString()
        if (!fileExists(cjoFile) || !buildConfig.cjoTimeMap.contains(depPkg) || buildConfig.cjoTimeMap[depPkg] !=
            FileInfo(cjoFile).lastModificationTime.toUnixTimeStamp().toSeconds()) {
            return false
        }
        return true
    }

    func build(): Bool {
        let sem = Semaphore(maxParallelSize)
        let futTasks = HashMap<String, Future<Bool>>()

        let logFuture = spawn {
            =>
            var count: Int64 = this.taskMap.size
            var logFlag: Bool = true
            while (count > 0) {
                let (outLogFile, errLogFile, nativeOutLog, nativeErrLog, isNewLog) = this.queue.remove()
                count--
                if (!isNewLog) {
                    continue
                }
                if (!printAllLogs(outLogFile, errLogFile, nativeOutLog, nativeErrLog)) {
                    logFlag = false
                }
            }
            return logFlag
        }

        // Guaranteed valid order follows 'buildConfig.packageList.all'
        for (pkgName in this.buildConfig.packageList.all) {
            if (!this.taskMap.contains(pkgName)) {
                continue
            }

            sem.acquire()
            let task = this.taskMap[pkgName]
            let future = spawn {
                =>
                for (dep in task.requirePkgs) {
                    if (!this.checkDepFuture(dep, task, futTasks, sem)) {
                        return false
                    }
                }

                if (let Some(superPkgCfg) <- task.superPkgCfg) {
                    for (dep in superPkgCfg.subPkgSet) {
                        if (!this.checkDepFuture(dep, task, futTasks, sem)) {
                            return false
                        }
                    }
                }

                if (task.checkDepCjo) {
                    var nochange: Bool = true
                    for (depPkg in task.requirePkgs) {
                        if (!checkNoChange(depPkg)) {
                            nochange = false
                            break
                        }
                    }
                    let subPkgSet = task.superPkgCfg?.subPkgSet ?? HashSet<String>()
                    for (subPkg in subPkgSet) {
                        if (!checkNoChange(subPkg)) {
                            nochange = false
                            break
                        }
                    }
                    if (nochange) {
                        sem.release()
                        this.queue.add((None, None, None, None, false))
                        return true
                    }
                }

                if (!deleteLog(task)) {
                    sem.release()
                    this.queue.add((task.outLogFile, task.errLogFile, None, None, false))
                    return false
                }

                var flag: Bool = true
                if (task.targetPlatform && !runTask(task, this.buildConfig.globalConfig.ldPath)) {
                    flag = false
                }
                if (task.nativePlatform && !runTask(task, this.buildConfig.globalConfig.ldPath,
                    isNativeForCross: true)) {
                    flag = false
                }
                sem.release()
                if (task.targetPlatform && task.nativePlatform) {
                    this.queue.add((task.outLogFile, task.errLogFile, task.nativeOutLog, task.nativeErrLog, true))
                } else if (task.targetPlatform) {
                    this.queue.add((task.outLogFile, task.errLogFile, None, None, true))
                } else if (task.nativePlatform) {
                    this.queue.add((None, None, task.nativeOutLog, task.nativeErrLog, true))
                }
                return flag
            }

            synchronized(this.buildMutex) {
                futTasks.add(pkgName, future)
            }
        }

        var isBuilt: Bool = true
        for ((pkgName, task) in futTasks) {
            if (!task.get()) {
                isBuilt = false
            } else {
                successPkgs.add(pkgName)
            }
        }

        if (!logFuture.get()) {
            isBuilt = false
        }

        return isBuilt
    }

    func deleteLog(task: ResolveConfig): Bool {
        var delLog: Bool = true
        if (task.targetPlatform) {
            if (!deleteFile(task.outLogFile)) {
                delLog = false
            }
            if (!deleteFile(task.errLogFile)) {
                delLog = false
            }
        }

        if (task.nativePlatform) {
            if (!deleteFile(task.nativeOutLog)) {
                delLog = false
            }
            if (!deleteFile(task.nativeErrLog)) {
                delLog = false
            }
        }
        return delLog
    }

    func runTask(resolveConfig: ResolveConfig, ldPath: HashSet<String>, isNativeForCross!: Bool = false): Bool {
        let envBuilder = EnvironmentBuilder()
        envBuilder.prepend(LD_PATH, ldPath)

        let cjcCalls = ArrayList<ArrayList<String>>()
        if (resolveConfig.packagePath.isMultiplatform) {
            let (commonArgs, platformArgs) = getMultiplatformPipeline(resolveConfig, this.buildConfig, isNativeForCross)
            cjcCalls.add(commonArgs)
            cjcCalls.add(platformArgs)
        } else {
            let args = getCommandArgs(resolveConfig, this.buildConfig, isNativeForCross, false)
            args.add(all: getTypeCommand(resolveConfig, this.buildConfig, isNativeForCross))
            cjcCalls.add(args)
        }

        var execCmdFlag: Bool = true
        for (args in cjcCalls) {
            if (!execCmdFlag) {
                return false
            }
            execCmdFlag = false
            CUR_PARALLEL_SIZE.fetchAdd(1)
            if (!futJudge(args)) {
                calculateParallel(CUR_PARALLEL_SIZE.load(), args)
            }

            let env = getVariables()
            let commandStr = getCmdStr(envBuilder.asCliStrings(env), COMPILE_TOOL, args)
            if (this.buildConfig.isVerbose) {
                let verbose: String = "Compiling package `${resolveConfig.fullName}`: ${commandStr}\n"
                if (!isNativeForCross) {
                    if (!createAndWriteFile(resolveConfig.outLogFile, verbose, mode: Append)) {
                        return false
                    }
                } else {
                    if (!createAndWriteFile(resolveConfig.nativeOutLog, verbose, mode: Append)) {
                        return false
                    }
                }
            }

            if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
                addStartTime(resolveConfig.fullName, commandStr)
            }
            var outFile: File
            var errFile: File
            var outFilePath: String = String()
            var errFilePath: String = String()
            try {
                if (!isNativeForCross) {
                    outFilePath = resolveConfig.outLogFile
                    errFilePath = resolveConfig.errLogFile
                    outFile = File(resolveConfig.outLogFile, OpenMode.Append)
                    errFile = File(resolveConfig.errLogFile, OpenMode.Append)
                } else {
                    outFilePath = resolveConfig.nativeOutLog
                    errFilePath = resolveConfig.nativeErrLog
                    outFile = File(resolveConfig.nativeOutLog, OpenMode.Append)
                    errFile = File(resolveConfig.nativeErrLog, OpenMode.Append)
                }
            } catch (e: Exception) {
                eprintln(e.message)
                eprintln("Error: create '${outFilePath}' failed")
                return false
            }
            try {
                if (let Some(returnCode) <- execAndToFile(COMPILE_TOOL, args, outFile, errFile, envBuilder: envBuilder, originalEnv: env)) {
                    if (returnCode == 0) {
                        execCmdFlag = true
                        if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
                            moveCjcProfToCjpmDir(resolveConfig, buildConfig.globalConfig.compilePerformanceTargetDir)
                        }
                    } else {
                        execCmdFlag = false
                        errFile.write("Error: failed to compile package `${resolveConfig.fullName}`, return code is ${returnCode}\n".toArray())
                        if (resolveConfig.packagePath.isMultiplatform) {
                            errFile.write("${SourceLocationsPrinter(resolveConfig.packagePath, resolveConfig.features, project: resolveConfig.fullName)}".toArray())
                        }
                    }
                } else {
                    errFile.write("Error: failed to compile package `${resolveConfig.fullName}` with exception occurred\n".toArray())
                }
            } catch (e: Exception) {
                eprintln("Error: failed to write error log into ${errFilePath}: ${e.message}")
                execCmdFlag = false
            }
            CUR_PARALLEL_SIZE.fetchSub(1)
            outFile.close()
            errFile.close()

            if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
                addEndTime(resolveConfig.fullName)
            }
        }
        return execCmdFlag
    }
}

func moveCjcProfToCjpmDir(resolveConfig: ResolveConfig, profcompilePerformancePath: String): Unit {
    let profSuffixArray: Array<String> = [".mem.prof", ".time.prof", ".info.prof"]
    for (profSuffix in profSuffixArray) {
        let pkgNameProfPath = Path(resolveConfig.targetPath).join(resolveConfig.fullName + profSuffix)
        if (exists(pkgNameProfPath)) {
            let profPath = Path(profcompilePerformancePath).join(resolveConfig.fullName + profSuffix)
            copy(pkgNameProfPath, to: profPath, overwrite: true)
            remove(pkgNameProfPath)
        } else {
            eprintln("Error: cannot find ${pkgNameProfPath.toString()} file")
        }
    }
    return
}

func addStartTime(fullName: String, cmdArgs: String): Unit {
    synchronized(CACHE_MUTEX) {
        SHOW_CACHE.add(PkgInfo(fullName, "B", DateTime.now().toUnixTimeStamp().toMilliseconds()))
        COMMAND_CACHE.add(CommandInfo(fullName, cmdArgs))
    }
    return
}

func addEndTime(fullName: String): Unit {
    synchronized(CACHE_MUTEX) {
        SHOW_CACHE.add(PkgInfo(fullName, "E", DateTime.now().toUnixTimeStamp().toMilliseconds()))
    }
    return
}

func parallelBuild(res: ModuleResolve, buildConfig: BuildConfig): Bool {
    let parallel = ParallelBuilder(buildConfig)
    res.forEachPkg(
        {
            resolve: ResolveItem =>
            if (buildConfig.isIncremental && !buildConfig.rebuildList.contains(resolve.fullName)) {
                return true
            }

            if (!resolve.hasProdFiles) { // No files to compile, avoid useless cjc invocations
                buildConfig.rebuildList.remove(resolve.fullName)
                return true
            }

            var finalPath = getFinalPath(buildConfig.globalConfig.targetDir, buildConfig.globalConfig.nativeDir,
                resolve.outputType, true, resolve.rootPkgName, buildConfig.requiredForTests)
            var finalLogPath = getLogPath(buildConfig.globalConfig.targetDir, buildConfig.globalConfig.nativeDir, true,
                resolve.rootPkgName)
            if (!createDirectory(finalPath) || !createDirectory(finalLogPath)) {
                return false
            }

            if (resolve.packagePath.isMultiplatform) {
                let finalCommonPath = Path(finalPath).join("common").toString()
                if (!createDirectory(finalCommonPath)) {
                    return false
                }
            }

            var resolveConfig = ResolveConfig()
            resolveConfig.targetPath = finalPath
            resolveConfig.packagePath = resolve.packagePath
            resolveConfig.fullName = resolve.fullName
            resolveConfig.rootPkgName = resolve.rootPkgName
            resolveConfig.outputType = resolve.outputType
            resolveConfig.compileOption = resolve.compileOption
            resolveConfig.linkOption = resolve.linkOption
            resolveConfig.nativeCompileOption = resolve.nativeCompileOption
            resolveConfig.nativeLinkOption = resolve.nativeLinkOption
            resolveConfig.customizedOption = resolve.customizedOption
            resolveConfig.requirePkgs = resolve.requires
            resolveConfig.targetPlatform = resolve.targetPlatform
            resolveConfig.nativePlatform = resolve.nativePlatform
            resolveConfig.superPkgCfg = resolve.superPkgCfg
            resolveConfig.hasTestFiles = resolve.hasTestFiles
            if (buildConfig.isIncremental && resolveConfig.outputType == Static &&
                buildConfig.indirectRebuilds.contains(resolve.fullName)) {
                resolveConfig.checkDepCjo = true
            }
            resolveConfig.outLogFile = Path(finalLogPath).join("${resolveConfig.fullName}.outlog").toString()
            resolveConfig.errLogFile = Path(finalLogPath).join("${resolveConfig.fullName}.errlog").toString()

            if (resolve.nativePlatform) {
                resolveConfig.nativePath = getFinalPath(buildConfig.globalConfig.targetDir,
                    buildConfig.globalConfig.nativeDir, resolve.outputType, false, resolve.rootPkgName,
                    buildConfig.requiredForTests)
                let nativeLogPath = getLogPath(buildConfig.globalConfig.targetDir, buildConfig.globalConfig.nativeDir,
                    false, resolve.rootPkgName)
                resolveConfig.nativeOutLog = Path(nativeLogPath).join("${resolveConfig.fullName}.outlog").toString()
                resolveConfig.nativeErrLog = Path(nativeLogPath).join("${resolveConfig.fullName}.errlog").toString()
                if (!createDirectory(resolveConfig.nativePath) || !createDirectory(nativeLogPath)) {
                    return false
                }
            }

            parallel.taskMap.add(resolve.fullName, resolveConfig)
            return true
        }
    )

    if (!parallel.taskMap.isEmpty()) {
        buildConfig.isRebuild = true
    }

    let parallelResult: Bool = parallel.build()

    for (r in res.resolves) {
        if (parallel.successPkgs.contains(r.fullName)) {
            r.buildFlag = true
        }
    }

    return parallelResult
}

func printAllLogs(outLogFile: Option<String>, errLogFile: Option<String>, nativeOutLog: Option<String>,
    nativeErrLog: Option<String>): Bool {
    var logFlag: Bool = true
    if (let Some(outLogFile) <- outLogFile) {
        if (fileExists(outLogFile) && !loadLogAndPrint(outLogFile, false)) {
            logFlag = false
        }
    }
    if (let Some(errLogFile) <- errLogFile) {
        if (fileExists(errLogFile) && !loadLogAndPrint(errLogFile, true)) {
            logFlag = false
        }
    }
    if (let Some(nativeOutLog) <- nativeOutLog) {
        if (fileExists(nativeOutLog) && !loadLogAndPrint(nativeOutLog, false)) {
            logFlag = false
        }
    }
    if (let Some(nativeErrLog) <- nativeErrLog) {
        if (fileExists(nativeErrLog) && !loadLogAndPrint(nativeErrLog, true)) {
            logFlag = false
        }
    }
    return logFlag
}

func loadLogAndPrint(file: String, isStderr: Bool): Bool {
    var logStr: String = ""
    unsafe {
        logStr = String.fromUtf8Unchecked(File.readFrom(file))
    }

    if (isStderr) {
        eprint(logStr)
    } else {
        print(logStr)
    }
    return true
}

func getFinalPath(targetDir: String, nativeDir: String, outputType: OutputType, isTargetPlatform: Bool, name: String,
    requiredForTests: Bool): String {
    var res = ""
    if (outputType == Exe && !requiredForTests) {
        if (isTargetPlatform) {
            res = Path(targetDir).join(BIN).toString()
        } else {
            res = Path(nativeDir).join(BIN).toString()
        }
    } else {
        if (isTargetPlatform) {
            res = Path(targetDir).join(name).toString()
        } else {
            res = Path(nativeDir).join(name).toString()
        }
    }
    return res
}

func getLogPath(targetDir: String, nativeDir: String, isTargetPlatform: Bool, rootPkgName: String): String {
    var res = ""
    if (isTargetPlatform) {
        res = Path(targetDir).join(".build-logs").join(rootPkgName).toString()
    } else {
        res = Path(nativeDir).join(".build-logs").join(rootPkgName).toString()
    }
    return res
}

func calculateParallel(currentParalla: Int64, args: ArrayList<String>): Unit {
    var thisParallelSize = currentParalla
    let totalParallelSize = maxParallelSize

    if (currentParalla <= 2) {
        thisParallelSize = totalParallelSize
    } else {
        thisParallelSize = 2 * totalParallelSize / currentParalla
    }
    args.add("-j${thisParallelSize}", at: 0)
}

func futJudge(args: ArrayList<String>): Bool {
    let tmp = args.toString()
    let fut1: Future<Bool> = spawn {
        let regexParallel = Regex("-j[[:space:]]*(?:[[:digit:]]|=[[:digit:]])")
        regexParallel.matches(tmp)
    }
    let fut2: Future<Bool> = spawn {
        tmp.contains("--jobs") || args.contains("-j")
    }
    return (fut1.get() || fut2.get())
}
