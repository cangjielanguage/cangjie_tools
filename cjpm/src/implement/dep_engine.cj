// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.sort.*
import std.sync.*
import std.fs.*
import std.time.*
import stdx.serialization.serialization.*
import stdx.encoding.json.*
import cjpm.config.*

// Record modules finally chosen
public class FinalDepModule {
    public var name: String = ""                // module name
    public var sourceInfo: DepInfo = DepInfo()  // depInfo when imported
    public var sourcePath: String = ""          // path of source code
    public var tomlInfo: TomlInfo = TomlInfo()  // standardized toml data
    public var rootModulePath: String = ""      // path of root module for this analysis
}

/*
 * Engine for dependency-analysis
 *
 * Steps:
 * 1. Collect repo-deps after DepModel.depCheck (which only deal with local-deps and git-deps);
 * 2. Initialize dep-records for all collected repo-deps and make sure there is no confliction;
 * 3. Do BFS to fill dep-version map.
 */
public class DepAnalysisEngine {
    // sign of engine
    public var isTest: Bool = false
    public var isDebug: Bool = false
    public var rootModulePath: String = ""
    public let target: String = if (crossCompileTarget.isEmpty()) { targetConfigName } else { crossCompileTarget }

    // records of chosen dep-modules for each module name
    public let finalDepMap: HashMap<String, FinalDepModule> = HashMap<String, FinalDepModule>()

    // central-repo dependencies collected by DepModel.depCheck
    // records repo-deps imported by cjpm.lock
    public let lockRepoDepMap: HashMap<String, String> = HashMap<String, String>()
    // records repo-deps imported by local/git deps
    public let repoDepList: ArrayList<(String, DepInfo)> = ArrayList<(String, DepInfo)>()
    // records of replace modules
    public var replaceMap: HashMap<String, DepInfo> = HashMap<String, DepInfo>()
    public let replacedModuleSet: HashSet<String> = HashSet<String>()

    // data used for dep-analysis
    // current state
    public var currentState: DepAnalysisState = DepAnalysisState()
    // history states
    public let historyStates: ArrayStack<DepAnalysisState> = ArrayStack<DepAnalysisState>()
    // confliction counter: conflictedModule -> times
    public let conflictCounter: HashMap<String, Int64> = HashMap<String, Int64>()
    // record of broken index: name-version -> requireModule
    public let brokenRecord: HashMap<String, String> = HashMap<String, String>()

    // records for information from central repository
    // depot for central repository
    public var depot: Depot = Depot()
    // records of index for each version of each module
    public let indexMap: HashMap<String, HashMap<String, ArtifactIndex>> = HashMap<String, HashMap<String, ArtifactIndex>>()

    // record repo-dep from cjpm.lock
    public func recordLock(locked: HashMap<String, DepInfo>): Unit {
        for ((name, depInfo) in locked) {
            if (let Some(version) <- depInfo.version) {
                this.lockRepoDepMap.add(name, version)
            }
        }
        locked.remove(all: this.lockRepoDepMap.keys())
    }

    // analyse repo-deps imported by local-deps and git-deps
    // repo-deps are expected to depend on only repo-deps
    public func analyseRepoDeps(): Bool {
        if (this.repoDepList.isEmpty()) {
            return true
        }

        this.depot = Depot.new() ?? return false

        // update index for replaced modules
        for ((fullName, replaceModule) in this.replaceMap) {
            if (let Some(version) <- replaceModule.version) {
                try {
                    let _ = DepRecord(fullName, version, this.depot, this.indexMap)
                } catch (ne: NoneValueException) {
                    // cannot find index or no available version for requirement
                    eprintln("Error: ${ne.message}")
                    eprintln("Error: module '${fullName}' from central repository in field 'replace' does not exist")
                    return false
                }
            }
        }

        let conflictDepMap = HashMap<String, ArrayList<DepInfo>>()
        let conflictSet = HashSet<String>()

        // collect dep-record for repo-dep
        let depRecordMap: HashMap<String, DepRecord> = HashMap<String, DepRecord>()
        for ((name, repoDep) in this.repoDepList) {
            // replaced by local or git modules
            if (this.finalDepMap.contains(name)) {
                continue
            }

            // replaced by replace modules
            if (this.replaceMap.contains(name)) {
                this.replacedModuleSet.add(name)
                continue
            }

            // imported module is replaced
            if (!this.finalDepMap.contains(repoDep.importedModuleName) && repoDep.importedModuleName != BUILD_SCRIPT_NAME) {
                continue
            }

            let versionStr = repoDep.version ?? return false // always exist
            try {
                let depRecord = DepRecord(name, versionStr, this.depot, this.indexMap)
                if (depRecordMap.contains(name)) {
                    conflictDepMap[name].add(repoDep)
                    if (!depRecordMap[name].combineRecords(depRecord)) {
                        conflictSet.add(name)
                    }
                } else {
                    depRecordMap.add(name, depRecord)
                    conflictDepMap.add(name, ArrayList<DepInfo>())
                    conflictDepMap[name].add(repoDep)
                }
            } catch (ne: NoneValueException) {
                // cannot find index or no available version for requirement
                eprintln("Error: ${ne.message}")
                return false
            } catch (e: Exception) {
                // other exception
                eprintln(e.message)
                eprintln("Error: wrong format with dependency '${name} = \"${versionStr}\"' depended by '${repoDep.importedModuleName}'")
                return false
            }
        }

        for (conflictDep in conflictSet) {
            eprintln("Error: no available artifact of module '${conflictDep}' could match all requirements from these modules:")
            for (depInfo in conflictDepMap[conflictDep]) {
                eprintln("    - '${depInfo.importedModuleName}' from '${depInfo.importedPath}', " +
                    "which requirement for '${conflictDep}' is '${depInfo.version ?? ""}'")
            }
        }
        if (!conflictSet.isEmpty()) {
            return false
        }

        // initialize state
        this.currentState = DepAnalysisState(depRecordMap)
        this.historyStates.add(DepAnalysisState(this.currentState))
        return doStateProcessing()
    }

    /*
     * Till this step, we have already recorded all repo-deps directly imported from all local-deps and git-deps.
     * We have already collected all indexs of available versions, and select available versions due to original requirements.
     *
     * In this step, we will try to fix a version for each module, and download them if no confliction occurred.
     */
    func doStateProcessing(): Bool {
        // Step 1: decide which module will be fixed in this turn
        while (let Some(targetModuleName) <- this.currentState.unfixedQueue.remove()) {
            // Step 2: try to fix a version for current module
            let fixedVersion = this.currentState.depRecordMap[targetModuleName].getNextAvailable(this.lockRepoDepMap)
            if (fixedVersion.isEmpty()) {
                // Step 3: an unfixed module has no available version, backjump and mark last-chosen version as banned
                if (!this.backjump()) {
                    this.printErrorLog()
                    return false
                }
                continue
            }
            this.currentState.currentModule = targetModuleName
            this.currentState.fixedVersionMap[targetModuleName] = fixedVersion

            // Step 4: get requirements of this version
            let depMap = this.collectDepMap(targetModuleName, fixedVersion)

            // Step 5: add requirements into state
            var checkFlag = true
            for ((name, indexDep) in depMap) {
                // replaced by local or git modules
                if (this.finalDepMap.contains(name)) {
                    continue
                }

                // replaced by replaced modules
                if (this.replaceMap.contains(name)) {
                    this.replacedModuleSet.add(name)
                    continue
                }

                if (!this.currentState.depRecordMap.contains(name)) {
                    // Step 5.1: new module is involved
                    if (!this.pushNewDep(targetModuleName, fixedVersion, name, indexDep)) {
                        checkFlag = false
                        break
                    }
                }
                // Step 5.2: dep-record already exists
                if (!this.pushExistedDep(targetModuleName, fixedVersion, name, indexDep)) {
                    checkFlag = false
                    break
                }
            }

            // Step 6: all requirements are checked, push state forward
            if (checkFlag) {
                this.historyStates.add(DepAnalysisState(this.currentState))
            }
        }

        // analysis succeed
        return this.downloadRepoDeps()
    }

    // current state failed, backjump
    func backjump(): Bool {
        if (this.currentState.currentModule.isEmpty()) {
            // no state to backjump, analysis failed
            return false
        }
        let lastChosenName = this.currentState.currentModule
        let lastChosenVersion = this.currentState.fixedVersionMap[lastChosenName]
        this.historyStates.remove()
        this.currentState = DepAnalysisState(this.historyStates.remove() ?? return false) // always exist
        this.currentState.depRecordMap[lastChosenName].banned.add(lastChosenVersion)
        this.historyStates.add(DepAnalysisState(this.currentState))
        return true
    }

    // failed when try to push state, reset and record failure
    func resetState(name: String, version: String): Unit {
        this.currentState = DepAnalysisState(this.historyStates.remove() ?? return) // always exist
        this.currentState.depRecordMap[name].banned.add(version)
        this.historyStates.add(DepAnalysisState(this.currentState))
    }

    /*
     * Step 5.1: new module is involved
     *
     * 1. create a new dep-record for new module, which may fail due to broken index;
     * 2. add into unfixed list.
     */
    func pushNewDep(targetModuleName: String, fixedVersion: String, depName: String, indexDep: IndexDep): Bool {
        try {
            this.currentState.depRecordMap.add(depName, DepRecord(depName, indexDep.require, this.depot, this.indexMap))
        } catch (ne: NoneValueException) {
            // index is broken
            this.brokenRecord.add("${targetModuleName}-${fixedVersion}", depName)
            this.resetState(targetModuleName, fixedVersion)
            return false
        }
        this.currentState.unfixedQueue.add(depName)
        return true
    }

    /*
     * Step 5.2: dep-record already exists
     *
     * 1. create a new requirement and add it into dep-record;
     * 2. add unmatched versions into banned list;
     * 3. three cases to backjump:
     *     (1). all versions are banned;
     *     (2). version that already chosen is banned;
     *     (3). require is invalid (usually not happened).
     */
    func pushExistedDep(targetModuleName: String, fixedVersion: String, depName: String, indexDep: IndexDep): Bool {
        if (!this.currentState.depRecordMap[depName].addNewRequirement(indexDep.require, this.indexMap)) {
            // requirement is invalid (no satisfied version or broken)
            this.brokenRecord.add("${targetModuleName}-${fixedVersion}", depName)
            this.resetState(targetModuleName, fixedVersion)
            return false
        }
        // version already chosen is banned
        if (this.currentState.fixedVersionMap.contains(depName) &&
            this.currentState.depRecordMap[depName].banned.contains(this.currentState.fixedVersionMap[depName])) {
            counterConfliction(depName)
            this.resetState(targetModuleName, fixedVersion)
            return false
        }
        // all versions are banned
        if (this.currentState.depRecordMap[depName].banned.size == this.currentState.depRecordMap[depName].available.size) {
            counterConfliction(depName)
            this.resetState(targetModuleName, fixedVersion)
            return false
        }
        return true
    }

    // Collect all dependencies in need
    func collectDepMap(name: String, version: String): HashMap<String, IndexDep> {
        let depMap = HashMap<String, IndexDep>()
        let rawDeps = ArrayList<IndexDep>()
        let index = this.indexMap[name][version]
        rawDeps.add(all: index.dependencies)
        if (this.isTest) {
            rawDeps.add(all: index.testDependencies)
        }

        for (indexDep in rawDeps) {
            if (indexDep.name == name) {
                continue
            }

            if ((let Some(targetName) <- indexDep.target) && (targetName != this.target)) {
                continue
            }
            if ((let Some(mode) <- indexDep.depType) && ((mode == "debug") != this.isDebug)) {
                continue
            }

            if (depMap.contains(indexDep.name)) {
                // use high-priority deps to replace lower ones
                if ((indexDep.target.isSome() && depMap[indexDep.name].target.isNone()) ||
                    (indexDep.depType.isSome() && depMap[indexDep.name].depType.isNone())) {
                    depMap.add(indexDep.name, indexDep)
                }
            } else {
                depMap.add(indexDep.name, indexDep)
            }
        }

        return depMap
    }

    // print the worst 3 conflictions when failed to analyse
    // and print all broken index
    func printErrorLog(): Unit {
        eprintln("Error: conflictions occurred between dependencies from central repository:")
        for ((name, repoDep) in this.repoDepList) {
            let versionStr = repoDep.version ?? continue
            eprintln("\t- '${name} = \"${versionStr}\"' depended by '${repoDep.importedModuleName}'")
        }

        let conflicts: Array<(String, Int64)> = this.conflictCounter.toArray()
        sort(conflicts, lessThan: { l: (String, Int64), r: (String, Int64) => l[1] > r[1] })
        eprintln("")
        eprintln("\tThese are modules which cause the most conflictions, please try to force them with field 'replace':")
        for (index in 0..conflicts.size) {
            if (index >= 3) {
                break
            }
            eprintln("\t\t${conflicts[index][0]}")
        }

        if (!this.brokenRecord.isEmpty()) {
            eprintln("")
            eprintln("\tThese index may be broken, please try to fix them with 'cjpm update':")
            for ((sourceName, depName) in this.brokenRecord) {
                eprintln("\t\tRequire for module '${depName}' in index of '${sourceName}'")
            }
        }
    }

    // download all deps
    func downloadRepoDeps(): Bool {
        var downloadTasks = ArrayList<Future<Bool>>()
        let defaultSize = 16 // default parallel value
        let actualSize = min(defaultSize, maxParallelSize)
        let downloadSem = Semaphore(actualSize)
        let downloadMtx: Mutex = Mutex()

        // down replaced repo-dep together
        for ((name, replaceVersion) in this.replaceMap) {
            if (let Some(version) <- replaceVersion.version) {
                this.currentState.fixedVersionMap.add(name, version)

                // index may not be updated
            }
        }

        for ((fullName, version) in this.currentState.fixedVersionMap) {
            let (org, name) = if (fullName.contains("::")) {
                (fullName.split("::")[0], fullName.split("::")[1])
            } else {
                ("", fullName)
            }
            downloadSem.acquire()
            downloadTasks.add(
                spawn {
                    =>
                    // download artifact
                    let sourcePath = Path(this.depot.localRepo).join("source").
                        join(if (org.isEmpty()) { "default" } else { org }).
                        join("${name}-${version}").toString()
                    if (!directoryExists(sourcePath)) {
                        let (flag, output) = this.depot.download(name, org, version)
                        if (!flag) {
                            eprintln("Error: ${output}")
                            return false
                        }
                    }

                    // save final module
                    let finalDep = FinalDepModule()
                    finalDep.name = fullName
                    finalDep.sourcePath = sourcePath
                    finalDep.rootModulePath = this.rootModulePath
                    finalDep.sourceInfo.version = version
                    synchronized(downloadMtx) {
                        this.finalDepMap.add(fullName, finalDep)
                    }

                    downloadSem.release()
                    return true
                }
            )
        }

        var downloadFlag = true
        for (task in downloadTasks) {
            downloadFlag &&= task.get()
        }

        return downloadFlag
    }

    func initFinalKey(name: String): Unit {
        if (!finalDepMap.contains(name)) {
            finalDepMap.add(name, FinalDepModule())
        }
    }

    public func getDepMap(): HashMap<String, DepInfo> {
        let depMap = HashMap<String, DepInfo>()
        for ((name, depModule) in finalDepMap) {
            depMap.add(name, depModule.sourceInfo)
        }
        return depMap
    }

    public func addDepInfo(name: String, depInfo: DepInfo): Unit {
        initFinalKey(name)
        finalDepMap[name].sourceInfo = depInfo
    }

    public func addDepPath(name: String, depPath: String): Unit {
        initFinalKey(name)
        finalDepMap[name].sourcePath = depPath
    }

    public func addDepToml(name: String, depToml: TomlInfo): Unit {
        initFinalKey(name)
        finalDepMap[name].tomlInfo = depToml
    }

    public func addDepRoot(name: String, depRoot: String): Unit {
        initFinalKey(name)
        finalDepMap[name].rootModulePath = depRoot
    }

    func counterConfliction(name: String): Unit {
        if (this.conflictCounter.contains(name)) {
            this.conflictCounter[name] += 1
        } else {
            this.conflictCounter[name] = 1
        }
    }
}

// Record of template analysis result for one module
public class DepRecord {
    public let name: String
    public let org: String
    public let fullName: String
    public let requires: ArrayList<RequireVersion> = ArrayList<RequireVersion>()
    public let available: ArrayList<String> = ArrayList<String>()
    public let banned: HashSet<String> = HashSet<String>()

    public init(other: DepRecord) {
        this.name = other.name
        this.org = other.org
        this.fullName = other.fullName
        this.available.add(all: other.available)
        this.banned.add(all: other.banned)
        for (require in other.requires) {
            this.requires.add(RequireVersion(require))
        }
    }

    // initialize with exception
    public init(name: String, require: String,
        depot: Depot, indexMap: HashMap<String, HashMap<String, ArtifactIndex>>) {
        this.fullName = name
        (this.org, this.name) = if (this.fullName.contains("::")) {
            (this.fullName.split("::")[0], this.fullName.split("::")[1])
        } else {
            ("", this.fullName)
        }
        this.requires.add(RequireVersion(this.fullName, require))

        // try to get available versions based on index
        var newDownloadFlag = false
        var indexPath = depot.getIndex(this.name, this.org)
        if (indexPath.isEmpty()) {
            // no local index, try to update
            if (!depot.updateIndex(this.name, this.org)) {
                // failed to update with error log in updateIndex
                throw NoneValueException("cannot get index for '${this.fullName}'")
            }
            indexPath = depot.getIndex(this.name, this.org)
            newDownloadFlag = true
        }

        // get available set
        let availableSet = HashSet<String>()
        getAvailableSet(indexPath, indexMap, availableSet)

        // local index cannot match, and it is too old
        if (availableSet.isEmpty() && !newDownloadFlag &&
            (DateTime.now().toUnixTimeStamp().toHours() - FileInfo(indexPath).lastModificationTime.toUnixTimeStamp().toHours() >= 24)) {
            if (!depot.updateIndex(this.name, this.org)) {
                // failed to update with error log in updateIndex
                throw NoneValueException("cannot get index for '${this.fullName}', and local index is invalid")
            }
            getAvailableSet(indexPath, indexMap, availableSet)
        }

        // no available version for first requirement
        if (availableSet.isEmpty()) {
            throw NoneValueException("no available version for module '${this.fullName}' by require '${require}'")
        }

        // sort available list
        let availableArray = availableSet.toArray()
        sort(availableArray, lessThan: { l: String, r: String => SingleVersion(l) > SingleVersion(r) })
        this.available.add(all: availableArray)
    }

    // update index map and get available set
    func getAvailableSet(indexPath: String, indexMap: HashMap<String, HashMap<String, ArtifactIndex>>, availableSet: HashSet<String>): Unit {
        // read index for each version
        var indexRawData = String.fromUtf8(File.readFrom(indexPath))
        var indexIter = indexRawData.lines()
        while (true) {
            match (indexIter.next()) {
                case Some(indexStr) =>
                    try {
                        let index: ArtifactIndex = ArtifactIndex.deserialize(DataModel.fromJson(JsonValue.fromStr(indexStr)))
                        if (index.yanked) {
                            continue
                        }
                        if (!indexMap.contains(this.fullName)) {
                            indexMap.add(this.fullName, HashMap<String, ArtifactIndex>())
                        }
                        indexMap[this.fullName].add(index.version, index)
                    } catch (e: Exception) {
                        eprintln(e.message)
                        throw NoneValueException("wrong format of index for '${this.fullName}'")
                    }
                case None => break
            }
        }

        // get available set
        for ((version, _) in indexMap[this.fullName]) {
            if (this.requires[0].isMatchWith(version)) {
                availableSet.add(version)
            }
        }
    }

    // combine two records
    public func combineRecords(other: DepRecord): Bool {
        this.requires.add(all: other.requires)

        let combinedAvailable = HashSet<String>()
        for (version in this.available) {
            var conflictFlag = false
            for (req in this.requires) {
                if (!req.isMatchWith(version)) {
                    conflictFlag = true
                    break
                }
            }
            if (!conflictFlag) {
                combinedAvailable.add(version)
            }
        }
        for (version in other.available) {
            var conflictFlag = false
            for (req in this.requires) {
                if (!req.isMatchWith(version)) {
                    conflictFlag = true
                    break
                }
            }
            if (!conflictFlag) {
                combinedAvailable.add(version)
            }
        }

        this.available.clear()
        let availableArray = combinedAvailable.toArray()
        sort(availableArray, lessThan: { l: String, r: String => SingleVersion(l) > SingleVersion(r) })
        this.available.add(all: availableArray)
        return !this.available.isEmpty()
    }

    // get the highest available version
    public func getNextAvailable(lockRepoDepMap: HashMap<String, String>): String {
        // if any version is locked, use it first
        if (let Some(lockedVersion) <- lockRepoDepMap.get(this.fullName) &&
            this.available.contains(lockedVersion) &&
            !this.banned.contains(lockedVersion)) {
            return lockedVersion
        }

        for (version in this.available) {
            if (!this.banned.contains(version)) {
                return version
            }
        }
        return ""
    }

    // add new requirement and update banned list
    public func addNewRequirement(require: String, indexMap: HashMap<String, HashMap<String, ArtifactIndex>>): Bool {
        try {
            this.requires.add(RequireVersion(this.fullName, require))
        } catch (ne: NoneValueException) {
            // requirement from index is invalid, maybe broken
            return false
        }

        for (version in this.available) {
            for (req in this.requires) {
                if (!req.isMatchWith(version)) {
                    this.banned.add(version)
                    break
                }
            }
        }

        // check if new requirement is invalid
        for (version in indexMap[this.fullName].keys()) {
            if (this.requires[this.requires.size - 1].isMatchWith(version)) {
                return true
            }
        }

        // no version in index matches with newest requirement
        return false
    }
}

/*
 * State for each iteration of dep-analysis
 *
 * Each state includes:
 * 1. version map for fixed module names so far;
 * 2. dep-record map for all related modules;
 * 3. unfixed modules in dep-record map.
 */
public class DepAnalysisState {
    public var currentModule: String = ""
    public let fixedVersionMap: HashMap<String, String> = HashMap<String, String>()
    public let depRecordMap: HashMap<String, DepRecord>
    public let unfixedQueue: ArrayQueue<String> = ArrayQueue<String>()

    public init() {
        this.depRecordMap = HashMap<String, DepRecord>()
    }

    public init(depRecordMap: HashMap<String, DepRecord>) {
        this.depRecordMap = depRecordMap
        for (name in this.depRecordMap.keys()) {
            unfixedQueue.add(name)
        }
    }

    public init(other: DepAnalysisState) {
        this.currentModule = other.currentModule
        this.fixedVersionMap.add(all: other.fixedVersionMap)
        this.depRecordMap = HashMap<String, DepRecord>()
        for ((name, depRecord) in other.depRecordMap) {
            this.depRecordMap.add(name, DepRecord(depRecord))
        }
        for (name in other.unfixedQueue) {
            this.unfixedQueue.add(name)
        }
    }
}