// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*
import std.convert.*
import std.fs.*
@When[os == "Linux" || os == "macOS"]
import std.io.*
import std.time.*
import std.deriving.Derive
import stdx.serialization.serialization.*
import stdx.encoding.json.*
@When[os == "Windows" || os == "macOS"]
import std.runtime.*

const TEST_COMPILATION_OPTIONS = "build"
const CUSTOMIZED_OPTION = "customized-option"
const ENV_CONFIG = "env"
const ENV_VALUE = "value"
const ENV_SPLICE_TYPE = "splice-type"
const K_BYTE: Float64 = 1024.0
let PAGE_SIZE: Float64 = getOSPageSize() // unit: Byte
public const COMPILE_PERFORMANCE_DIR = "performance_analysis"

public class Profile <: Serializable<Profile> {
    public var build: BuildProfile = BuildProfile()
    public var test: TestProfile = TestProfile(JsonObject())
    public var bench: TestProfile = TestProfile(JsonObject())
    public var run: RunProfile = RunProfile()
    public var customizedOption: HashMap<String, String> = HashMap<String, String>()
    public var isExperimental = false

    // Serialize of toml is useless.
    public func serialize(): DataModel {
        return DataModelStruct()
    }

    public static func deserialize(dm: DataModel): Profile {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = Profile()
        result.build = BuildProfile.deserialize((dms.get("build") as DataModelStruct) ?? DataModelStruct())
        result.test = TestProfile.deserialize((dms.get("test") as DataModelStruct) ?? DataModelStruct())
        result.bench = TestProfile.deserialize((dms.get("bench") as DataModelStruct) ?? DataModelStruct())
        result.run = RunProfile.deserialize((dms.get("run") as DataModelStruct) ?? DataModelStruct())
        result.customizedOption = HashMap<String, String>.deserialize(
            (dms.get(CUSTOMIZED_OPTION) as DataModelStruct) ?? DataModelStruct())
        result.isExperimental = match (dms.get("experimental") as DataModelBool) {
            case Some(val) => val.getValue()
            case None => false
        }
        return result
    }
}

public class BuildProfile <: Serializable<BuildProfile> {
    public var ltoValue: String = ""
    public var incremental: Bool = false
    public var analysisCompilePerformance: Bool = false
    public var combined: HashMap<String, OutputType> = HashMap<String, OutputType>()
    public var cjcJobs: ?Int64 = None
    public var heuristic: Bool = false
    public var compilePipelineParallel: Bool = false

    // Serialize of toml is useless.
    public func serialize(): DataModel {
        return DataModelStruct()
    }

    public static func deserialize(dm: DataModel): BuildProfile {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = BuildProfile()
        result.cjcJobs = Option<Int64>.deserialize(dms.get("cjc-jobs"))
        result.heuristic = Bool.deserialize((dms.get("enable-heuristic-parallelism") as DataModelBool) ?? DataModelBool(false))
        result.ltoValue = String.deserialize((dms.get("lto") as DataModelString) ?? DataModelString(""))
        result.incremental = Bool.deserialize((dms.get("incremental") as DataModelBool) ?? DataModelBool(false))
        result.analysisCompilePerformance = Bool.deserialize((dms.get("performance_analysis") as DataModelBool) ??
            DataModelBool(false))
        result.combined = HashMap<String, OutputType>.deserialize((dms.get("combined") as DataModelStruct) ??
            DataModelStruct())
        result.compilePipelineParallel = Bool.deserialize((dms.get("compile-pipeline-parallel") as DataModelBool) ??
            DataModelBool(false))
        return result
    }
}

public class TestProfile <: Serializable<TestProfile> {
    public let value: JsonObject

    TestProfile(json: JsonValue) {
        this.value = json.asObject()
    }

    public prop verbose: Bool {
        get() {
            compilationBoolOptionByName("verbose")
        }
    }

    public prop mock: ?MockSupport {
        get() {
            match (compilationOptionByName("mock")?.asString().getValue()) {
                case Some(mock) => MockSupport.fromString(mock)
                case None => None
            }
        }
    }

    public prop ltoValue: String {
        get() {
            match (compilationOptionByName("lto")?.asString().getValue()) {
                case Some(val) => val
                case None => ""
            }
        }
    }

    public prop compileOption: String {
        get() {
            match (compilationOptionByName(COMPILE_OPTION)?.asString().getValue()) {
                case Some(val) => val
                case None => ""
            }
        }
    }

    public prop reportFormat: ?String {
        get() {
            if (let Some(format) <- value.get("report-format")) {
                return String.deserialize(DataModel.fromJson(format))
            }
            return None
        }
    }

    public prop reportPath: ?String {
        get() {
            if (let Some(path) <- value.get("report-path")) {
                return String.deserialize(DataModel.fromJson(path))
            }
            return None
        }
    }

    public prop isProgressReportDisabled: Bool {
        get() {
            compilationBoolOptionByName("no-progress")
        }
    }

    public prop isProgressReportBrief: Bool {
        get() {
            compilationBoolOptionByName("progress-brief")
        }
    }

    public prop progressReportEntriesLimit: ?Int64 {
        get() {
            if (let Some(limit) <- value.get("progress-entries-limit")?.asInt().getValue()) {
                return limit
            }
            return None
        }
    }

    public prop envConfig: HashMap<String, EnvInfo> {
        get() {
            if (let Some(envs) <- value.get(ENV_CONFIG)) {
                return HashMap<String, EnvInfo>.deserialize(DataModel.fromJson(envs))
            }
            return HashMap<String, EnvInfo>()
        }
    }

    public func serialize(): DataModel {
        return DataModel.fromJson(value)
    }

    public static func deserialize(dm: DataModel): TestProfile {
        let fields = dm.toJson().asObject().getFields()
        return TestProfile(JsonObject(fields))
    }

    public func toStringWithoutCompilationOptions(): String {
        let fields = value.getFields()
        fields.remove(TEST_COMPILATION_OPTIONS)
        fields.remove(ENV_CONFIG)

        return JsonObject(fields).toJsonString().replace("\r", "").replace("\n", "")
    }

    private func compilationBoolOptionByName(name: String): Bool {
        if (let Some(value) <- compilationOptionByName(name)) {
            value.asBool().getValue()
        } else {
            false
        }
    }

    private func compilationOptionByName(name: String): ?JsonValue {
        if (let Some(compilationOptions) <- value.get(TEST_COMPILATION_OPTIONS)) {
            for ((k, v) in compilationOptions.asObject().getFields()) {
                if (k == name) {
                    return v
                }
            }
        }
        return None
    }

    public func isEmpty(): Bool {
        return this.value.size() == 0
    }
}

public class RunProfile <: Serializable<RunProfile> {
    public var envConfig: HashMap<String, EnvInfo> = HashMap<String, EnvInfo>()

    // Serialize of toml is useless.
    public func serialize(): DataModel {
        return DataModelStruct()
    }

    public static func deserialize(dm: DataModel): RunProfile {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = RunProfile()
        result.envConfig = HashMap<String, EnvInfo>.deserialize(
            (dms.get(ENV_CONFIG) as DataModelStruct) ?? DataModelStruct())
        return result
    }
}

@Derive[Equatable]
public enum SpliceType <: ToString {
    | ABSENT
    | REPLACE
    | PREPEND
    | APPEND
    | Unknown(String)

    static func fromString(value: String): SpliceType {
        match {
            case value == "absent" => ABSENT
            case value == "replace" || value.isEmpty() => REPLACE
            case value == "prepend" => PREPEND
            case value == "append" => APPEND
            case _ => Unknown(value)
        }
    }

    public func toString(): String {
        match (this) {
            case ABSENT => "absent"
            case REPLACE => "replace"
            case PREPEND => "prepend"
            case APPEND => "append"
            case Unknown(s) => s
        }
    }

    public prop isAbsent: Bool {
        get() {
            match (this) {
                case ABSENT | Unknown(_) => true
                case _ => false
            }
        }
    }
}

public struct EnvInfo <: Serializable<EnvInfo> {
    private EnvInfo(
        public let envValue: String,
        public let spliceType: SpliceType
    ) {}

    public func serialize(): DataModel {
        return DataModelStruct()
    }

    public static func deserialize(dm: DataModel): EnvInfo {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }

        let spliceTypeString = match (dms.get(ENV_SPLICE_TYPE)) {
            case _: DataModelNull => ""
            case s: DataModelString => s.getValue()
            case _ => "invalid"
        }
        return EnvInfo(
            String.deserialize((dms.get(ENV_VALUE) as DataModelString) ?? DataModelString("")),
            SpliceType.fromString(spliceTypeString)
        )
    }
}

@When[os == "Linux" || os == "macOS"]
public func envMapToString(envMap: HashMap<String, String>): String {
    if (envMap.isEmpty()) {
        return ""
    }
    var res: String = ""
    for ((key, value) in envMap) {
        res += "${key}=${value} "
    }
    return res
}

@When[os == "Windows"]
public func envMapToString(envMap: HashMap<String, String>): String {
    if (envMap.isEmpty()) {
        return ""
    }
    var res: String = "cmd /V /C \""
    for ((key, value) in envMap) {
        res += "set ${key}=${value}&& "
    }
    return res
}

@When[os == "Linux" || os == "macOS"]
public let ENV_HEAD_PATCH: String = ""
@When[os == "Linux" || os == "macOS"]
public let ENV_PATCH: String = ""

@When[os == "Linux" || os == "macOS"]
public func getFinalEnvValue(envName: String, envValue: String, spliceType: SpliceType): String {
    match (spliceType) {
        case REPLACE => "${envValue}"
        case PREPEND => "${envValue}:$${envName}"
        case APPEND => "$${envName}:${envValue}"
        case _ => ""
    }
}

@When[os == "Windows"]
public let ENV_HEAD_PATCH: String = "cmd /V /C \""
@When[os == "Windows"]
public let ENV_PATCH: String = "\""

@When[os == "Windows"]
public func getFinalEnvValue(envName: String, envValue: String, spliceType: SpliceType): String {
    match (spliceType) {
        case REPLACE => "${envValue}"
        case PREPEND => "${envValue};%${envName}%"
        case APPEND => "%${envName}%;${envValue}"
        case _ => ""
    }
}

public func verifyCombinedField(cfg: TomlInfo, path: String): Bool {
    var flag = true
    for ((pkgName, outputType) in cfg.profile.build.combined) {
        if (pkgName != cfg.pkg.name) {
            eprintln("Error: field 'profile.build.combined' in ${Path(path).join(CONFIG_FILE_NAME).toString()} " +
            "can only used for root package '${cfg.pkg.name}', current value is '${pkgName}'")
            flag = false
            continue
        }
        if (outputType != Dynamic) {
            eprintln(
                "Error: field 'profile.build.combined.${cfg.pkg.name}' in " +
                    "${Path(path).join(CONFIG_FILE_NAME).toString()} can only configured as 'dynamic'")
            flag = false
            continue
        }
        eprintln("Warning: field 'profile.build.combined' in '${Path(path).join(CONFIG_FILE_NAME).toString()}' " +
            "is experimental, it is unstable and has some limitations, " +
            "please refer to related documentation for more details")
    }
    return flag
}

func verifyCustomizedOption(customizedOption: HashMap<String, String>, path: String): Bool {
    for ((k, v) in customizedOption) {
        if (!customizedOptionCheck(k, path)) {
            return false
        }
        let ownOptions = ["jobs", "coverage", "verbose", "incremental", "help", "lint", "mock", "bench"]
        if (ownOptions.contains(k)) {
            eprintln("Error: '${k}' is an option used by cjpm, it does not allow to be used as a customized key")
            return false
        }
        if (!safeCheck(v, path)) {
            return false
        }
        customizedOption[k] = fixCompileOption(v, path)
    }
    return true
}

public func verifyEnvConfig(envConfig: HashMap<String, EnvInfo>, cmd: String, configFilePath: String): Bool {
    for ((envName, envInfo) in envConfig) {
        if (!envNameCheck(envName, cmd, configFilePath)) {
            return false
        }
        if (let Unknown(_) <- envInfo.spliceType) {
            eprintln("Error: splice type of environment variable '${envName}' in profile '${cmd}' " +
                "in ${configFilePath} can be only '${SpliceType.ABSENT}', '${SpliceType.REPLACE}', " +
                "'${SpliceType.PREPEND}' or '${SpliceType.APPEND}'")
            return false
        }
    }
    return true
}

@When[os == "macOS"]
foreign func getpagesize(): Int64

@When[os == "macOS"]
func getOSPageSize(): Float64 {
    unsafe {
        return Float64(getpagesize())
    }
}

@When[os != "macOS"]
func getOSPageSize(): Float64 {
    return 4.0 * K_BYTE
}

// Mirror of macOS proc_taskinfo for proc_pidinfo FFI calls.
@When[os == "macOS"]
@C
struct ProcTaskInfo {
    // Virtual and resident memory sizes in bytes.
    var pti_virtual_size: UInt64 = 0       
    var pti_resident_size: UInt64 = 0      
    // Accumulated user/system CPU time.
    var pti_total_user: UInt64 = 0
    var pti_total_system: UInt64 = 0
    var pti_threads_user: UInt64 = 0
    var pti_threads_system: UInt64 = 0
    // Scheduling and runtime activity counters.
    var pti_policy: Int32 = 0
    var pti_faults: Int32 = 0
    var pti_pageins: Int32 = 0
    var pti_cow_faults: Int32 = 0
    var pti_messages_sent: Int32 = 0
    var pti_messages_received: Int32 = 0
    var pti_syscalls_mach: Int32 = 0
    var pti_syscalls_unix: Int32 = 0
    var pti_csw: Int32 = 0
    var pti_threadnum: Int32 = 0
    var pti_numrunning: Int32 = 0
    var pti_priority: Int32 = 0
}

// Lists process IDs and returns the number of bytes written.
@When[os == "macOS"]
foreign func proc_listpids(ptype: UInt32, ptypeinfo: UInt32, buffer: CPointer<UInt8>, buffersize: Int32): Int32

// Writes the process name for a PID and returns the name length.
@When[os == "macOS"]
foreign func proc_name(pid: Int32, buffer: CPointer<UInt8>, buffersize: UInt32): Int32

// Fills ProcTaskInfo for a PID; return value is bytes written.
@When[os == "macOS"]
foreign func proc_pidinfo(pid: Int32, flavor: Int32, arg: UInt64, buffer: CPointer<ProcTaskInfo>, buffersize: Int32): Int32

// Allocates a C buffer for libproc calls.
@When[os == "macOS"]
foreign func malloc(size: UIntNative): CPointer<UInt8>

// Frees a buffer allocated by malloc.
@When[os == "macOS"]
foreign func free(ptr: CPointer<UInt8>): Unit

// Converts a null-terminated C byte buffer to a String.
@When[os == "macOS"]
func convertCPointer2String(buffer: CPointer<UInt8>, maxSize: Int64): String {
    unsafe {
        var len = 0
        // Scan until NUL or maxSize.
        while (len < maxSize && buffer.read(len) != 0) {
            len++
        }
        // Empty C string.
        if (len == 0) {
            return ""
        }
        var bytes = Array<UInt8>(len, repeat: 0)
        // Copy bytes into a managed array.
        for (i in 0..len) {
            bytes[i] = buffer.read(i)
        }
        return String.fromUtf8(bytes)
    }
}

@When[os == "macOS"]
public func getCoreCompilerMemInfo(): Float64 {
    // Accumulator for total RSS in bytes.
    var totalRSSBytes: UInt64 = 0
    let targetProcesses = ["cjpm", "cjc", "llc", "opt", "ar", "ld"]
    
    // Constants from libproc system headers.
    const PROC_ALL_PIDS: UInt32 = 1
    const PROC_PIDTASKINFO: Int32 = 4
    
    unsafe {
        // Step 1: Determine the required buffer size for the PID list.
        // Passing a null pointer and size 0 returns the total bytes needed.
        let nullPtr = CPointer<UInt8>()
        let bytesNeeded = proc_listpids(PROC_ALL_PIDS, 0, nullPtr, 0)
        
        if (bytesNeeded <= 0) {
            return 0.0
        }
        
        // Step 2: Allocate memory for the PID list based on bytesNeeded. 
        let pidBuffer = malloc(UIntNative(bytesNeeded))

        // Step 3: Get PID count; each PID entry uses 4 bytes.
        let count = proc_listpids(PROC_ALL_PIDS, 0, pidBuffer, bytesNeeded) / 4 

        // Step 4: Allocate a 32-byte buffer to store process names.
        // 32 bytes is enough: macOS MAXCOMLEN is 16, so 32 provides ample headroom for any process name.
        let nameBuffer = malloc(UIntNative(32)) 
        
        // Step 5: Iterate through every PID found in the system.
        for (i in 0..count) {
            // Rebuild Int32 PID from 4 little-endian bytes.
            let offset = Int64(i * 4)
            let b0 = UInt32(pidBuffer.read(offset)) // byte 0 (LSB)
            let b1 = UInt32(pidBuffer.read(offset + 1)) // byte 1
            let b2 = UInt32(pidBuffer.read(offset + 2)) // byte 2
            let b3 = UInt32(pidBuffer.read(offset + 3)) // byte 3 (MSB)
            let pid = Int32(b0 | (b1 << 8) | (b2 << 16) | (b3 << 24))
            
            if (pid <= 0) { continue }
            
            // Step 6: Query the process name for the given PID.
            let nameLen = proc_name(pid, nameBuffer, 32)
            if (nameLen > 0) {
                // Convert the raw C-string buffer into a native String.
                let procName = convertCPointer2String(nameBuffer, 32)
                
                // Step 7: Check if the current process name matches our target list.
                var isTarget = false
                for (target in targetProcesses) {
                    if (procName == target) {
                        isTarget = true
                        break
                    }
                }
                
                // Step 8: If it matches, fetch detailed task/memory information.
                if (isTarget) {
                    var taskInfo = ProcTaskInfo()
                    let sizeOfTaskInfo: Int32 = 96 // proc_taskinfo is 96 bytes on 64-bit macOS.
                    
                    let status = proc_pidinfo(
                        pid, 
                        PROC_PIDTASKINFO, 
                        0, 
                        inout taskInfo, 
                        sizeOfTaskInfo
                    )
                    
                    if (status == sizeOfTaskInfo) {
                        totalRSSBytes += taskInfo.pti_resident_size
                    }
                }
            }
        }
        
        free(pidBuffer)
        free(nameBuffer)
    }
    
    return Float64(totalRSSBytes) / K_BYTE / K_BYTE
}

@When[os == "Windows"]
func parseTasklistMemUsageToBytes(memUsage: String): UInt64 {
    if (memUsage.isEmpty()) {
        return 0
    }
    // Extract all digits from text like "12,345 K", skipping commas and spaces.
    let cleanStr = memUsage.replace(",", "")
                           .replace(" ", "")
                           .replace("K", "")
    // Return 0 if the memory field is empty.
    if (cleanStr.isEmpty()) {
        return 0
    }
    let kiloBytes = Int64.tryParse(cleanStr) ?? 0
    // Convert KB to bytes.
    return UInt64(kiloBytes) * 1024
}

@When[os == "Windows"]
public func getCoreCompilerMemInfo(): Float64 {
    // Compiler-related processes to include in the total.
    let targets = HashSet<String>(["cjpm.exe", "cjc.exe", "llc.exe", "opt.exe", "ar.exe", "ld.exe"])
    // Arguments: /FO CSV (format as CSV), /NH (No Header).
    let args = ArrayList<String>(["/FO", "CSV", "/NH"])
    // Execute: tasklist /FO CSV /NH
    let (ok, outInfo, errInfo) = execWithOutput("tasklist", args)

    if (!ok) {
        eprintln("Error: tasklist failed: ${errInfo}")
        return 0.0
    }

    // Sum memory usage of matched processes in bytes.
    var totalRSSBytes: UInt64 = 0
    // Parse CSV lines: "image","pid","session name","session id","mem usage"
    for (line in outInfo.split("\n")) {
        let row = line.replace("\r", "")
        if (row.isEmpty()) {
            continue
        }
        // Split CSV fields by "," delimiter.
        let fields = row.split("\",\"")
        if (fields.size < 5) {
            continue
        }
        // Strip leading quote from first field and trailing quote from last field.
        let imageName = fields[0].trimStart("\"")
        let memUsage = fields[4].trimEnd("\"")
        if (targets.contains(imageName)) {
            totalRSSBytes += parseTasklistMemUsageToBytes(memUsage)
        }
    }

    // Convert bytes to MB.
    return Float64(totalRSSBytes) / K_BYTE / K_BYTE
}

/**
 * @brief Get total RSS of core compiler processes on Linux by scanning /proc/.
 * Reads /proc/{pid}/comm for process name and /proc/{pid}/statm for RSS.
 */
@When[os == "Linux"]
public func getCoreCompilerMemInfo(): Float64 {
    var totalRSSBytes: UInt64 = 0

    try {
        let entries = Directory.readFrom(Path("/proc"))
        for (entry in entries) {
            if (!entry.isDirectory()) {
                continue
            }
            let dirName = entry.path.fileName
            // Only check numeric directories (PID directories)
            if (Int64.tryParse(dirName) == None) {
                continue
            }

            try {
                // Read the process name from /proc/{pid}/comm and check if it matches a target.
                if (["cjpm", "cjc", "llc", "opt", "ar", "ld"].contains(
                    String.fromUtf8(readToEnd(File(Path("/proc/${dirName}/comm"), Read))).trimEnd("\n")
                )) {
                    // Read RSS from /proc/{pid}/statm (field index 1, unit: pages)
                    let statmPath = Path("/proc/${dirName}/statm")
                    let statmFile = File(statmPath, Read)
                    let statmBuffer = readToEnd(statmFile)
                    let statmStr = String.fromUtf8(statmBuffer)
                    // format: "size resident shared text lib data dt"
                    let statmArray = statmStr.split(" ")

                    // Check for empty or incomplete statm fields if the process terminates during reading.
                    if (statmArray.size > 1) {
                        let rssPages = UInt64.parse(statmArray[1])
                        totalRSSBytes += rssPages * UInt64(PAGE_SIZE)
                    }
                }
            } catch (e: Exception) {
                eprintln("Warning: failed to read /proc/${dirName}: ${e}")
                continue
            }
        }
    } catch (e: Exception) {
        eprintln("Warning: failed to read /proc: ${e}")
    }

    return Float64(totalRSSBytes) / K_BYTE / K_BYTE
}

// Shared path for all platforms: sample total RSS of compiler processes.
@When[os == "macOS" || os == "Windows" || os == "Linux"]
public func addMemoryProcess(): Future<Thread> {
    // Record an initial sample before starting the background loop.
    CJPM_COMPILE_INFO.add(DateTime.now().toUnixTimeStamp().toMilliseconds().toString(), getCoreCompilerMemInfo())
    let memoryProcess = spawn { =>
        while (true) {
            if (Thread.currentThread.hasPendingCancellation) {
                return Thread.currentThread
            }
            sleep(100 * Duration.millisecond)
            
            // Sample total RSS via FFI-based process scan.
            let compilerMem = getCoreCompilerMemInfo()
            var cjpmTime: String = DateTime.now().toUnixTimeStamp().toMilliseconds().toString()
            CJPM_COMPILE_INFO.add(cjpmTime, compilerMem)
        }
        return Thread.currentThread
    }
    return memoryProcess
}
