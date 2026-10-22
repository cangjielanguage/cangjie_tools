// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*
import std.fs.*
import std.io.*
import std.sort.*
import std.regex.*
import stdx.serialization.serialization.*
import cjpm.toml.*
import cjpm.toml.Decoder as DecoderToml
import cjpm.toml.Encoder as EncoderToml

let MODULE_LOCK_VERSION: UInt64 = 1
// Controls only the per-read I/O buffer and does not limit the lock file size.
const LOCK_READ_BUFFER_SIZE: Int64 = 8 * 1024

// lock file model
public class ModuleLock <: Serializable<ModuleLock> {
    public var requires: HashMap<String, DepInfo> = HashMap<String, DepInfo>()
    public var scripts: HashMap<String, DepInfo> = HashMap<String, DepInfo>()
    //Used so that we can change format of this file without breaking change
    var version: UInt64 = MODULE_LOCK_VERSION

    public init() {}

    public init(requires: HashMap<String, DepInfo>) {
        for ((name, depInfo) in requires) {
            if (depInfo.isGitDep() && depInfo.pathDepInGit) {
                continue
            }
            this.requires.add(name, depInfo)
        }
    }

    public init(requires: HashMap<String, DepInfo>, scripts: HashMap<String, DepInfo>) {
        for ((name, depInfo) in requires) {
            if (depInfo.isGitDep() && depInfo.pathDepInGit) {
                continue
            }
            this.requires.add(name, depInfo)
        }
        this.scripts = scripts
    }

    public func serialize(): DataModel {
        let res = DataModelStruct()
        res.add(field<UInt64>(VERSION, MODULE_LOCK_VERSION))
        res.add(Field("requires", serializeLockDeps(this.requires)))
        if (!this.scripts.isEmpty()) {
            res.add(Field("scripts", serializeLockDeps(this.scripts)))
        }
        return res
    }

    public static func deserialize(dm: DataModel): ModuleLock {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        let version = UInt64.deserialize(dms.get(VERSION));
        if (version > MODULE_LOCK_VERSION) {
            throw Exception("cjpm.lock file has incompatible version")
        }
        var result = ModuleLock(deserializeLockDeps(dms.get("requires"), version))
        result.version = version
        result.scripts = deserializeLockDeps(
            (dms.get("scripts") as DataModelStruct) ?? DataModelStruct(), version)
        return result
    }
}

// Reads one physical line at a time without retaining the complete lock file.
class LockLineReader {
    let file: File
    let buffer: Array<Byte> = Array<Byte>(LOCK_READ_BUFFER_SIZE, repeat: 0)
    var bufferSize: Int64 = 0
    var bufferPos: Int64 = 0
    var firstLine = true

    init(file: File) {
        this.file = file
    }

    func nextByte(): Option<Byte> {
        if (bufferPos >= bufferSize) {
            let size = file.read(buffer)
            if (size <= 0) {
                return Option.None
            }
            bufferSize = size
            bufferPos = 0
        }
        let result = buffer[bufferPos]
        bufferPos++
        return Option.Some(result)
    }

    func nextLine(): Option<String> {
        let line = ByteBuffer()
        var hasByte = false
        while (let Some(value) <- nextByte()) {
            hasByte = true
            if (value == b'\n') {
                break
            }
            line.writeByte(value)
        }
        if (!hasByte) {
            return Option.None
        }

        let bytes = line.bytes()
        var start: Int64 = 0
        var end: Int64 = bytes.size
        if (firstLine && end >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF) {
            start = 3
        }
        firstLine = false
        if (end > start && bytes[end - 1] == b'\r') {
            end--
        }
        return String.fromUtf8(bytes[start..end])
    }
}

// Decodes the canonical format emitted by Encoder without building a TOML tree for the whole file.
// Each dependency line is converted directly to DepInfo before its temporary TOML objects are released.
func decodeCanonicalLock(file: File, lockFilePath: String): ModuleLock {
    let reader = LockLineReader(file)
    let requires = HashMap<String, DepInfo>()
    let scripts = HashMap<String, DepInfo>()
    var section = ""
    var hasVersion = false
    var hasRequires = false
    var hasScripts = false

    while (let Some(line) <- reader.nextLine()) {
        let trimmed = line.trimAscii()
        if (trimmed.isEmpty() || trimmed.startsWith("#")) {
            continue
        }
        if (trimmed == "[requires]" || trimmed == "[scripts]") {
            if (!hasVersion) {
                throw Exception("missing 'version = 0' before '${trimmed}' in ${LOCK_FILE_NAME}")
            }
            if (trimmed == "[requires]") {
                if (hasRequires) {
                    throw Exception("duplicate '[requires]' table in ${LOCK_FILE_NAME}")
                }
                hasRequires = true
            } else {
                if (hasScripts) {
                    throw Exception("duplicate '[scripts]' table in ${LOCK_FILE_NAME}")
                }
                hasScripts = true
            }
            section = trimmed
            continue
        }

        if (trimmed.startsWith("[[")) {
            throw Exception("invalid '${LOCK_FILE_NAME}' format: found array table '${trimmed}'; " +
                "expected 'version = 0' followed by '[requires]' and optional '[scripts]' tables")
        }
        if (trimmed.startsWith("[")) {
            throw Exception("unsupported table '${trimmed}' in ${LOCK_FILE_NAME}")
        }

        if (section.isEmpty()) {
            let parts = trimmed.split("=")
            if (parts.size >= 2 && parts[0].trimAscii() == "version" &&
                parts[1].split("#")[0].trimAscii() == "1") {
                return ModuleLock.deserialize(DataModel.fromToml(DecoderToml(File.readFrom(lockFilePath)).decode()))
            }
            if (parts.size != 2 || parts[0].trimAscii() != "version" || parts[1].trimAscii() != "0") {
                throw Exception("invalid top-level field in ${LOCK_FILE_NAME}; expected 'version = 0'")
            }
            if (hasVersion) {
                throw Exception("duplicate 'version' field in ${LOCK_FILE_NAME}")
            }
            hasVersion = true
            continue
        }

        let entry = DecoderToml(line.toArray()).decode()
        var found = false
        for ((name, value) in entry.value) {
            if (found) {
                throw Exception("a lock entry must contain exactly one dependency")
            }
            let depInfo = DepInfo.deserialize(DataModel.fromToml(value))
            if (section == "[requires]") {
                if (requires.contains(name)) {
                    throw Exception("duplicate dependency '${name}' in '[requires]'")
                }
                requires.add(name, depInfo)
            } else {
                if (scripts.contains(name)) {
                    throw Exception("duplicate dependency '${name}' in '[scripts]'")
                }
                scripts.add(name, depInfo)
            }
            found = true
        }
        if (!found) {
            throw Exception("empty dependency entry in ${LOCK_FILE_NAME}")
        }
    }

    if (!hasVersion) {
        throw Exception("missing 'version = 0' in ${LOCK_FILE_NAME}")
    }
    if (!hasRequires) {
        throw Exception("missing '[requires]' table in ${LOCK_FILE_NAME}")
    }
    let result = ModuleLock()
    result.version = 0
    result.requires = requires
    result.scripts = scripts
    return result
}

public func loadLockFile(rootPath: String): (Bool, ModuleLock) {
    var res = ModuleLock()
    var lockFile: String = Path(rootPath).join(LOCK_FILE_NAME).toString()

    if (!fileExists(lockFile)) {
        // This file does not exist by default.
        return (true, res)
    }

    try (file = File(lockFile, Read)) {
        res = decodeCanonicalLock(file, lockFile)
    } catch (e: Exception) {
        eprintln("Error: ${e.message}")
        eprintln("Error: parse the '${LOCK_FILE_NAME}' file failed")
        return (false, res)
    }

    return (true, res)
}

public func creatLockFile(rootPath: String, lockFile: ModuleLock): Bool {
    var lockFilePath: String = Path(rootPath).join(LOCK_FILE_NAME).toString()
    if (directoryExists(lockFilePath)) {
        eprintln("Error: failed to create '${LOCK_FILE_NAME}', because a directory with the same name already exists")
        return false
    }

    try {
        let buffer = ByteBuffer()
        buffer.write("version = ${MODULE_LOCK_VERSION}\n".toArray())
        writeLockSection(buffer, "requires", lockFile.requires)
        if (!lockFile.scripts.isEmpty()) {
            writeLockSection(buffer, "scripts", lockFile.scripts)
        }
        // Serialize before opening the file so incomplete v0 metadata cannot truncate it.
        let bytes = buffer.bytes()
        if (!fileExists(lockFilePath) || File.readFrom(lockFilePath) != bytes) {
            File.writeTo(lockFilePath, bytes)
        }
    } catch (e: Exception) {
        eprintln("Error: ${e.message}")
        eprintln("Error: generate the '${LOCK_FILE_NAME}' file failed")
        return false
    }

    if (!modifyFilePermission(lockFilePath)) {
        return false
    }

    return true
}

// Keep lock fields separate from DepInfo.serialize(), which also writes cjpm.toml.
func serializeLockDep(dep: DepInfo): DataModelStruct {
    if (dep.version.isNone() || dep.source.isNone()) {
        throw Exception("resolve dependencies before migrating cjpm.lock to v1")
    }
    validateLockSource(dep)
    let result = DataModelStruct()
    result.add(field("version", dep.version.getOrThrow()))
    result.add(field("source", dep.source.getOrThrow()))
    if (let Some(checksum) <- dep.checksum) {
        result.add(field("checksum", checksum))
    }
    if (let Some(outputType) <- dep.outputType) {
        result.add(field(OUTPUT_TYPE, outputType))
    }
    let names = HashSet<String>(dep.dependencies).toArray()
    sort(names)
    result.add(field("dependencies", names))
    return result
}

func serializeLockDeps(deps: HashMap<String, DepInfo>): DataModelStruct {
    let result = DataModelStruct()
    let names = deps.keys().toArray()
    sort(names)
    for (name in names) {
        result.add(Field(name, serializeLockDep(deps[name])))
    }
    return result
}

func writeLockSection(buffer: ByteBuffer, section: String, deps: HashMap<String, DepInfo>): Unit {
    if (deps.isEmpty()) {
        buffer.write("\n[${section}]\n".toArray())
        return
    }
    let names = deps.keys().toArray()
    sort(names)
    for (name in names) {
        buffer.write("\n[${section}.${TomlString(name)}]\n".toArray())
        EncoderToml(buffer).encode(serializeLockDep(deps[name]))
    }
}

func deserializeLockDeps(dm: DataModel, version: UInt64): HashMap<String, DepInfo> {
    if (version == 0) {
        return HashMap<String, DepInfo>.deserialize(dm)
    }
    let table = (dm as DataModelStruct) ?? throw Exception("invalid cjpm.lock dependency table")
    let result = HashMap<String, DepInfo>()
    for (entry in table.getFields()) {
        let fields = (entry.getData() as DataModelStruct) ?? throw Exception("invalid v1 lock entry")
        let dep = DepInfo.deserialize(fields)
        dep.version = String.deserialize(fields.get("version"))
        dep.source = String.deserialize(fields.get("source"))
        dep.checksum = Option<String>.deserialize(fields.get("checksum"))
        dep.dependencies = Array<String>.deserialize(fields.get("dependencies"))
        validateLockSource(dep)
        result.add(entry.getName(), dep)
    }
    return result
}

func validateLockSource(dep: DepInfo): Unit {
    let source = dep.source ?? throw Exception("missing source in v1 lock entry")
    if (source.startsWith("registry+")) {
        if (source.removePrefix("registry+").isEmpty()) {
            throw Exception("empty registry URL in cjpm.lock")
        }
        let checksum = dep.checksum ?? throw Exception("missing checksum in registry lock entry")
        if (!checksum.isEmpty() && !Regex("^sha256:[0-9a-f]{64}$").matches(checksum)) {
            throw Exception("registry checksum must be sha256: followed by 64 lowercase hexadecimal digits")
        }
    } else if (source.startsWith("git+")) {
        if (dep.checksum.isSome()) {
            throw Exception("Git lock entry cannot contain checksum")
        }
        let parts = source.removePrefix("git+").split("#")
        if (parts.size != 2 || parts[0].isEmpty() || !Regex("^([0-9a-f]{40}|[0-9a-f]{64})$").matches(parts[1])) {
            throw Exception("Git source must contain a URL and full resolved commit")
        }
        let url = parts[0].split("?")
        dep.git = url[0]
        dep.commitId = parts[1]
        if (url.size == 2) {
            if (url[1].startsWith("branch=")) {
                dep.branch = decodeLockRef(url[1].removePrefix("branch="))
            } else if (url[1].startsWith("tag=")) {
                dep.tag = decodeLockRef(url[1].removePrefix("tag="))
            } else {
                throw Exception("invalid Git selector in cjpm.lock")
            }
        } else if (url.size != 1) {
            throw Exception("invalid Git selector in cjpm.lock")
        }
    } else {
        throw Exception("lock source must start with registry+ or git+")
    }
}

public func gitLockSource(dep: DepInfo): String {
    let selector = if (let Some(branch) <- dep.branch) {
        "?branch=" + encodeLockRef(branch)
    } else if (let Some(tag) <- dep.tag) {
        "?tag=" + encodeLockRef(tag)
    } else { "" }
    return "git+${dep.git.getOrThrow()}${selector}#${dep.commitId.getOrThrow()}"
}

func encodeLockRef(ref: String): String {
    let result = StringBuilder()
    let hex = "0123456789ABCDEF".toArray()
    for (value in ref.toArray()) {
        if ((value >= b'a' && value <= b'z') || (value >= b'A' && value <= b'Z') ||
            (value >= b'0' && value <= b'9') || "-._~".toArray().contains(value)) {
            result.append(String.fromUtf8([value]))
        } else {
            result.append(String.fromUtf8([b'%', hex[Int64(value / 16)], hex[Int64(value % 16)]]))
        }
    }
    return result.toString()
}

func decodeLockRef(ref: String): String {
    let bytes = ref.toArray()
    let result = ByteBuffer()
    var i: Int64 = 0
    while (i < bytes.size) {
        if (bytes[i] == b'%') {
            if (i + 2 >= bytes.size) { throw Exception("invalid Git selector in cjpm.lock") }
            let hex = "0123456789abcdef"
            let hi = hex.indexOf(ref[i + 1..i + 2].toAsciiLower()) ?? throw Exception("invalid Git selector in cjpm.lock")
            let lo = hex.indexOf(ref[i + 2..i + 3].toAsciiLower()) ?? throw Exception("invalid Git selector in cjpm.lock")
            result.writeByte(UInt8(hi * 16 + lo))
            i += 3
        } else {
            result.writeByte(bytes[i])
            i++
        }
    }
    return String.fromUtf8(result.bytes())
}
