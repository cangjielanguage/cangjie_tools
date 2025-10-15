// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjcov.core

import std.collection.*
import stdx.serialization.serialization.*

public class FileJson <: Serializable<FileJson> {
    public var filepath: String = ""
    public var totalLines: Int64 = 0
    public var hitLines: ArrayList<Int64> = ArrayList<Int64>()

    public init() {}

    public init(filepath: String, totalLines: Int64, hitLines: ArrayList<Int64>) {
        this.filepath = filepath
        this.totalLines = totalLines
        this.hitLines = hitLines
    }

    public func serialize(): DataModel {
        let dms = DataModelStruct()
        dms.add(field<String>("filepath", filepath))
        dms.add(field<Int64>("totalLines", totalLines))
        dms.add(field<ArrayList<Int64>>("hitLines", hitLines))
        return dms
    }

    public static func deserialize(dm: DataModel): FileJson {
        let dms = (dm as DataModelStruct).getOrThrow()
        var result = FileJson()
        result.filepath = String.deserialize(dms.get("filepath"))
        result.totalLines = Int64.deserialize(dms.get("totalLines"))
        result.hitLines = ArrayList<Int64>.deserialize(dms.get("hitLines"))
        return result
    }
}

public class FileListJson <: Serializable<FileListJson> {
    public var fileLists: ArrayList<FileJson> = ArrayList<FileJson>()

    public func addItem(fileJson: FileJson) {
        this.fileLists.add(fileJson)
    }

    public func serialize(): DataModel {
        let dms = DataModelStruct()
        dms.add(field<ArrayList<FileJson>>("fileLists", fileLists))
        return dms
    }

    public static func deserialize(dm: DataModel): FileListJson {
        let dms = (dm as DataModelStruct).getOrThrow()
        var result = FileListJson()
        result.fileLists = ArrayList<FileJson>.deserialize(dms.get("fileLists"))
        return result
    }
}
