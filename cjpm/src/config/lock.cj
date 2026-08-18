// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*
import std.fs.*
import std.io.*
import stdx.serialization.serialization.*
import cjpm.toml.*
import cjpm.toml.Decoder as DecoderToml
import cjpm.toml.Encoder as EncoderToml

let MODULE_LOCK_VERSION: UInt64 = 0
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
        res.add(field<UInt64>(VERSION, this.version))
        res.add(field<HashMap<String, DepInfo>>("requires", this.requires))
        if (!this.scripts.isEmpty()) {
            res.add(field<HashMap<String, DepInfo>>("scripts", this.scripts))
        }
        return res
    }

    public static func deserialize(dm: DataModel): ModuleLock {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        let version = UInt64.deserialize(dms.get(VERSION));
        if (version != MODULE_LOCK_VERSION) {
            throw Exception("cjpm.lock file has incompatible version")
        }
        var result = ModuleLock(HashMap<String, DepInfo>.deserialize(dms.get("requires")))
        result.version = version
        result.scripts = HashMap<String, DepInfo>.deserialize(
            (dms.get("scripts") as DataModelStruct) ?? DataModelStruct())
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
func decodeCanonicalLock(file: File): ModuleLock {
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
        res = decodeCanonicalLock(file)
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
        var f = File(lockFilePath, Write)
        let enc = EncoderToml(f)
        enc.encode(lockFile.serialize())
        f.close()
    } catch (e: Exception) {
        eprintln("Error: generate the '${LOCK_FILE_NAME}' file failed")
        return false
    }

    if (!modifyFilePermission(lockFilePath)) {
        return false
    }

    return true
}
