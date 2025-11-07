// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.binary.*
import std.collection.*
import std.fs.*
import stdx.serialization.serialization.*
import stdx.encoding.json.*
import cjpm.config.*

public func doPublish(isVerbose: Bool): Bool {
    let tomlInfo = loadModuleFile(DIR_CURRENT) ?? return false
    if (tomlInfo.isWorkSpaceConfig) {
        eprintln("Error: cannot publish a workspace to central repository")
        return false
    }
    if (!verifyModuleConfigPre(tomlInfo, DIR_CURRENT, DIR_CURRENT)) {
        return false
    }

    // chech tarball and meta-data.json
    var needBundle = false

    let targetDir = getOriginTargetDirectory(TARGET, DIR_CURRENT, "", tomlInfo.pkg.targetDir, true) ?? return false
    let metaDataPath = Path(targetDir).join(META_DATA_FILE)
    let tarballPath = Path(targetDir).join("${tomlInfo.pkg.name}-${tomlInfo.pkg.version}.cjp")

    if (!fileExists(metaDataPath) || !fileExists(tarballPath)) {
        println("Warning: cannot find meta-data.json or ${tomlInfo.pkg.name}-${tomlInfo.pkg.version}.cjp " +
            "in target directory '${targetDir}', run 'cjpm bundle' before publish")
        needBundle = true
    } else {
        try {
            let metaData: MetaData = MetaData.deserialize(DataModel.fromJson(
                JsonValue.fromStr(String.fromUtf8(File.readFrom(metaDataPath)))))
            let expectSha256 = metaData.index.sha256sum
            if (!metaData.index.genFileCheckSum(tarballPath.toString()) || (metaData.index.sha256sum != expectSha256)) {
                println("Warning: ${tomlInfo.pkg.name}-${tomlInfo.pkg.version}.cjp " +
                    "in target directory '${targetDir}' is broken, run 'cjpm bundle' before publish")
                needBundle = true
            }
        } catch (e: Exception) {
            println("Warning: meta-data.json in target directory '${targetDir}' is broken, " +
                "run 'cjpm bundle' before publish")
            needBundle = true
        }
    }

    // rebundle
    if (needBundle && !doBundle(BundleConfig(isVerbose))) {
        return false
    }

    // prepare data
    let cjpArray: ArrayList<Byte> = ArrayList<Byte>()
    if (!addDataToCjp(cjpArray, metaDataPath, "meta data") || !addDataToCjp(cjpArray, tarballPath, "tarball")) {
        return false
    }

    // publish cjp data
    try {
        if (let Some(depot) <- Depot.new()) {
            let (res, output) = depot.publish(tomlInfo, cjpArray.toArray())
            if (res) {
                return true
            }
            if (!output.isEmpty()) {
                eprintln("Error: ${output}")
            }
        }
        return false
    } catch (e: Exception) {
        eprintln(e.message)
        return false
    }
}

func addDataToCjp(cjpArray: ArrayList<Byte>, dataPath: Path, dataType: String): Bool {
    try {
        let dataArray: Array<Byte> = File.readFrom(dataPath)
        if (dataType == "tarball") {
            cjpArray.add(TARBALL_VERSION_BYTE)
        } else {
            cjpArray.add(META_DATA_VERSION_BYTE)
        }
        cjpArray.add(all: size2Bytes(dataArray.size, dataType) ?? return false)
        cjpArray.add(all: dataArray)
        return true
    } catch (e: Exception) {
        eprintln(e.message)
        eprintln("Error: failed to make final data while processing ${dataType}")
        return false
    }
}

func size2Bytes(size: Int64, dataType: String): ?Array<Byte> {
    if (size > Int64(Int32.Max)) {
        eprintln("Error: size of ${dataType} cannot be over 4GB")
        return None
    }

    let buffer = Array<Byte>(4, repeat: 0)
    let _ = Int32(size).writeBigEndian(buffer)
    buffer.reverse()
    return buffer
}