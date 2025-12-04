// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*
import std.regex.*
import std.fs.*
import stdx.serialization.serialization.*
import stdx.crypto.digest.*
import stdx.encoding.hex.*

// version of metadata and index
public let META_DATA_VERSION_BYTE: Byte = 1
public let INDEX_VERSION_BYTE: Byte = 1
public let TARBALL_VERSION_BYTE: Byte = 1

// Fields in metadata
const META_ORGANIZATION = "organization"
const META_NAME = "name"
const META_VERSION = "version"
const META_DESCRIPTION = "description"
const META_ARTIFACT_TYPE = "artifact-type"
const META_EXECUTABLE = "executable"
const META_AUTHORS = "authors"
const META_REPOSITORY = "repository"
const META_HOMEPAGE = "homepage"
const META_DOCUMENTATION = "documentation"
const META_TAG = "tag"
const META_CATEGORY = "category"
const META_LICENSE = "license"
const META_INDEX = "index"
const META_META_VERSION = "meta-version"

// Fields in index
const INDEX_DEPENDENCIES = "dependencies"
const INDEX_TEST_DEPENDENCIES = "test-dependencies"
const INDEX_SCRIPT_DEPENDENCIES = "script-dependencies"
const INDEX_DEP_NAME = "name"
const INDEX_DEP_REQUIRE = "require"
const INDEX_DEP_TARGET = "target"
const INDEX_DEP_TYPE = "type"
const INDEX_OUTPUT_TYPE = "output-type"
const INDEX_SHA_256_SUM = "sha256sum"
const INDEX_YANKED = "yanked"
const INDEX_CJC_VERSION = "cjc-version"
const INDEX_INDEX_VERSION = "index-version"

// max size of tag and category
const MAX_SIZE_OF_TAG = 5

// Regex used for checking url
const WINDOWS_LOCAL_FILE_REGEX = "^[A-Za-z]:.+$"
const URL_WITH_SPACE = "^.*\\s+.*$"

// Default organization name
public const DEFAULT_ORGANIZATION: String = "default"

// Black-list of module/organization name while bundling
let BLACK_NAME_LIST = HashSet<String>([
    "break", "bool", "case", "catch", "class", "const", "continue", "else", "enum", "extend", "for", "from", "func",
    "false", "finally", "foreign", "float", "float16", "float32", "float64", "handle", "init", "inout", "import",
    "interface", "int", "int8", "int16", "int32", "int64", "intnative", "let", "mut", "main", "macro", "match",
    "nothing", "operator", "perform", "prop", "package", "quote", "resume", "return", "rune", "spawn", "super",
    "static", "struct", "synchronized", "try", "this", "true", "type", "throw", "this", "unsafe", "unit", "uint",
    "uint8", "uint16", "uint32", "uint64", "uintnative", "var", "where", "while", "abstract", "get", "internal",
    "late", "open", "override", "private", "protected", "public", "redef", "required", "sealed", "set", "throwing",
    "with"
])

// List of category
let CATEGORY_SET = HashSet<String>([
    "network", "database driver", "data encapsulation and transfer", "data analysis", "database framework",
    "object storage", "distributed", "task scheduling", "security", "utility", "logging", "algorithm",
    "audio and video", "character encoding", "image processing", "developer tools", "animation", "infrastructure",
    "geographic information", "ui", "scientific computing", "programming framework", "data monitoring",
    "circuit breaker and downgrading", "message queue"
])

// Dependencies in index
public class IndexDep <: Serializable<IndexDep> {
    public IndexDep(
        public let name: String,
        public let require: String,
        public let target: ?String,
        public let depType: ?String,
        public let outputType: OutputType
    ) {}

    public init() {
        this.name = ""
        this.require = ""
        this.target = None
        this.depType = None
        this.outputType = OutputType.Unknown("")
    }

    public func serialize(): DataModel {
        DataModelStruct()
            .add(field<String>(INDEX_DEP_NAME, this.name))
            .add(field<String>(INDEX_DEP_REQUIRE, this.require))
            .add(field<?String>(INDEX_DEP_TARGET, this.target))
            .add(field<?String>(INDEX_DEP_TYPE, this.depType))
            .add(field<?String>(INDEX_OUTPUT_TYPE, if (this.outputType.isValid) { this.outputType.toString() } else { None }))
    }

    public static func deserialize(dm: DataModel): IndexDep {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("Bad artifact index")
        }

        IndexDep(
            String.deserialize((dms.get(INDEX_DEP_NAME) as DataModelString) ?? DataModelString("")),
            String.deserialize((dms.get(INDEX_DEP_REQUIRE) as DataModelString) ?? DataModelString("")),
            Option<String>.deserialize(dms.get(INDEX_DEP_TARGET)),
            Option<String>.deserialize(dms.get(INDEX_DEP_TYPE)),
            OutputType.fromString(String.deserialize((dms.get(INDEX_OUTPUT_TYPE) as DataModelString) ?? DataModelString("")))
        )
    }
}

// Index of artifact
public class ArtifactIndex <: Serializable<ArtifactIndex> {
    public ArtifactIndex(
        public let organization: String,
        public let name: String,
        public let version: String,
        public let dependencies: ArrayList<IndexDep>,
        public let testDependencies: ArrayList<IndexDep>,
        public let scriptDependencies: ArrayList<IndexDep>,
        public var sha256sum: String,
        public let yanked: Bool,
        public let cjcVersion: String,
        public let indexVersion: Byte
    ) {}

    public func serialize(): DataModel {
        DataModelStruct()
            .add(field<String>(META_ORGANIZATION, this.organization))
            .add(field<String>(META_NAME, this.name))
            .add(field<String>(META_VERSION, this.version))
            .add(field<ArrayList<IndexDep>>(INDEX_DEPENDENCIES, this.dependencies))
            .add(field<ArrayList<IndexDep>>(INDEX_TEST_DEPENDENCIES, this.testDependencies))
            .add(field<ArrayList<IndexDep>>(INDEX_SCRIPT_DEPENDENCIES, this.scriptDependencies))
            .add(field<String>(INDEX_SHA_256_SUM, this.sha256sum))
            .add(field<Bool>(INDEX_YANKED, this.yanked))
            .add(field<String>(INDEX_CJC_VERSION, this.cjcVersion))
            .add(field<Byte>(INDEX_INDEX_VERSION, this.indexVersion))
    }

    public static func deserialize(dm: DataModel): ArtifactIndex {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("Bad artifact index")
        }
        ArtifactIndex(
            String.deserialize((dms.get(META_ORGANIZATION) as DataModelString) ?? DataModelString("")),
            String.deserialize((dms.get(META_NAME) as DataModelString) ?? DataModelString("")),
            String.deserialize((dms.get(META_VERSION) as DataModelString) ?? DataModelString("")),
            ArrayList<IndexDep>.deserialize((dms.get(INDEX_DEPENDENCIES) as DataModelSeq) ?? DataModelSeq()),
            ArrayList<IndexDep>.deserialize((dms.get(INDEX_TEST_DEPENDENCIES) as DataModelSeq) ?? DataModelSeq()),
            ArrayList<IndexDep>.deserialize((dms.get(INDEX_SCRIPT_DEPENDENCIES) as DataModelSeq) ?? DataModelSeq()),
            String.deserialize((dms.get(INDEX_SHA_256_SUM) as DataModelString) ?? DataModelString("")),
            Bool.deserialize((dms.get(INDEX_YANKED) as DataModelBool) ?? DataModelBool(false)),
            String.deserialize((dms.get(INDEX_CJC_VERSION) as DataModelString) ?? DataModelString("")),
            Byte.deserialize((dms.get(INDEX_INDEX_VERSION) as DataModelInt) ?? DataModelInt(1))
        )
    }

    public func genFileCheckSum(filePath: String): Bool {
        try {
            let fileArr = File.readFrom(filePath)
            let sha256Instance = SHA256()
            sha256Instance.write(fileArr)
            this.sha256sum = toHexString(sha256Instance.finish())
            return true
        } catch (e: Exception) {
            eprintln(e.message)
            eprintln("Error: failed to generate sha256 check-sum for tarball '${filePath}'")
            return false
        }
    }

    // record all types of dependencies from toml
    public func addDependencies(tomlInfo: TomlInfo): Bool {
        // dependencies without target
        var flag = addOneTypeDeps(tomlInfo.dependencies, this.dependencies,
            INDEX_DEPENDENCIES, None, None)
        flag = addOneTypeDeps(tomlInfo.testDependencies, this.testDependencies,
            INDEX_TEST_DEPENDENCIES, None, None) && flag
        flag = addOneTypeDeps(tomlInfo.scriptDependencies, this.scriptDependencies,
            INDEX_SCRIPT_DEPENDENCIES, None, None) && flag

        // dependencies with target
        for ((targetName, targetConfig) in tomlInfo.targetConfigMap) {
            flag = addOneTargetDeps(targetConfig.commonTargetInfo, targetName, None) && flag
            flag = addOneTargetDeps(targetConfig.debugTargetInfo, targetName, "debug") && flag
            flag = addOneTargetDeps(targetConfig.releaseTargetInfo, targetName, "release") && flag
        }

        return flag
    }

    func addOneTargetDeps(targetInfo: TargetInfo, targetName: String, depType: ?String): Bool {
        var flag = addOneTypeDeps(targetInfo.dependencies, this.dependencies,
            INDEX_DEPENDENCIES, targetName, depType)
        flag = addOneTypeDeps(targetInfo.testDependencies, this.testDependencies,
            INDEX_TEST_DEPENDENCIES, targetName, depType) && flag
        flag = addOneTypeDeps(targetInfo.scriptDependencies, this.scriptDependencies,
            INDEX_SCRIPT_DEPENDENCIES, targetName, depType) && flag
        return flag
    }

    func addOneTypeDeps(deps: HashMap<String, DepInfo>, depList: ArrayList<IndexDep>,
        depListName: String, target: ?String, depType: ?String): Bool {
        var flag = true
        for ((name, depInfo) in deps) {
            if (!depInfo.isCenterRepoDep()) {
                eprintln("Error: dependency '${name}' from field '${getDepField(depListName, target, depType)}' " +
                    "should be in format of centray repository while bundling")
                flag = false
                continue
            }
            depList.add(IndexDep(name, depInfo.version ?? "", target, depType, depInfo.outputType ?? OutputType.Unknown("")))
        }
        return flag
    }

    func getDepField(depListName: String, target: ?String, depType: ?String): String {
        var depField: String = depListName
        if (let Some(t) <- target) {
            if (let Some(dt) <- depType) {
                depField = "${dt}.${depField}"
            }
            depField = "target.${t}.${depField}"
        }
        return depField
    }
}

// Meta-data of artifact
public class MetaData <: Serializable<MetaData> {
    public MetaData(
        public let organization: String,
        public let name: String,
        public let version: String,
        public let description: String,
        public let artifactType: String,
        public let executable: Bool,
        public let authors: Array<String>,
        public let repository: String,
        public let homepage: String,
        public let documentation: String,
        public let tag: Array<String>,
        public let category: Array<String>,
        public let license: Array<String>,
        public let index: ArtifactIndex,
        public let metaVersion: Byte
    ) {}

    public func serialize(): DataModel {
        DataModelStruct()
            .add(field<String>(META_ORGANIZATION, this.organization))
            .add(field<String>(META_NAME, this.name))
            .add(field<String>(META_VERSION, this.version))
            .add(field<String>(META_DESCRIPTION, this.description))
            .add(field<String>(META_ARTIFACT_TYPE, this.artifactType))
            .add(field<Bool>(META_EXECUTABLE, this.executable))
            .add(field<Array<String>>(META_AUTHORS, this.authors))
            .add(field<String>(META_REPOSITORY, this.repository))
            .add(field<String>(META_HOMEPAGE, this.homepage))
            .add(field<String>(META_DOCUMENTATION, this.documentation))
            .add(field<Array<String>>(META_TAG, this.tag))
            .add(field<Array<String>>(META_CATEGORY, this.category))
            .add(field<Array<String>>(META_LICENSE, this.license))
            .add(field<ArtifactIndex>(META_INDEX, this.index))
            .add(field<Byte>(META_META_VERSION, this.metaVersion))
    }

    public static func deserialize(dm: DataModel): MetaData {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("Bad meta data")
        }
        MetaData(
            String.deserialize((dms.get(META_ORGANIZATION) as DataModelString) ?? DataModelString("")),
            String.deserialize((dms.get(META_NAME) as DataModelString) ?? DataModelString("")),
            String.deserialize((dms.get(META_VERSION) as DataModelString) ?? DataModelString("")),
            String.deserialize((dms.get(META_DESCRIPTION) as DataModelString) ?? DataModelString("")),
            String.deserialize((dms.get(META_ARTIFACT_TYPE) as DataModelString) ?? DataModelString("")),
            Bool.deserialize((dms.get(META_EXECUTABLE) as DataModelBool) ?? DataModelBool(false)),
            Array<String>.deserialize((dms.get(META_AUTHORS) as DataModelSeq) ?? DataModelSeq()),
            String.deserialize((dms.get(META_REPOSITORY) as DataModelString) ?? DataModelString("")),
            String.deserialize((dms.get(META_HOMEPAGE) as DataModelString) ?? DataModelString("")),
            String.deserialize((dms.get(META_DOCUMENTATION) as DataModelString) ?? DataModelString("")),
            Array<String>.deserialize((dms.get(META_TAG) as DataModelSeq) ?? DataModelSeq()),
            Array<String>.deserialize((dms.get(META_CATEGORY) as DataModelSeq) ?? DataModelSeq()),
            Array<String>.deserialize((dms.get(META_LICENSE) as DataModelSeq) ?? DataModelSeq()),
            ArtifactIndex.deserialize((dms.get(META_INDEX) as DataModelStruct) ?? DataModelStruct()),
            Byte.deserialize((dms.get(META_META_VERSION) as DataModelInt) ?? DataModelInt(1))
        )
    }

    public func checkMetaData(tomlInfo: TomlInfo): Bool {
        var flag = if (this.organization.isEmpty()) { true } else { checkBundleName(this.organization, META_ORGANIZATION) }
        flag = checkBundleName(this.name, "module") && flag
        flag = this.checkUrl(this.repository, META_REPOSITORY) && flag
        flag = this.checkUrl(this.homepage, META_HOMEPAGE) && flag
        flag = this.checkUrl(this.documentation, META_DOCUMENTATION) && flag
        flag = this.index.addDependencies(tomlInfo) && flag
        flag = this.checkConfigSize() && flag
        flag = this.checkCategory() && flag
        return flag
    }

    func checkUrl(url: String, field: String): Bool {
        let trimmedUrl: String = url.trimAscii()
        if (!trimmedUrl.isEmpty() && (!checkLegalUrl(trimmedUrl) || !checkSafeUrl(trimmedUrl))) {
            eprintln("Error: url '${trimmedUrl}' in field 'package.${field}' of current module is illegal")
            return false
        }
        return true
    }

    func checkConfigSize(): Bool {
        var flag = true
        if (this.tag.size > MAX_SIZE_OF_TAG) {
            eprintln("Error: size of field 'tag' cannot be over ${MAX_SIZE_OF_TAG}")
            flag = false
        }
        if (this.category.size > MAX_SIZE_OF_TAG) {
            eprintln("Error: size of field 'category' cannot be over ${MAX_SIZE_OF_TAG}")
            flag = false
        }
        return flag
    }

    func checkCategory(): Bool {
        var flag = true
        for (index in 0..this.category.size) {
            let lowerCat = this.category[index].toAsciiLower()
            if (!CATEGORY_SET.contains(lowerCat)) {
                eprintln("Error: category '${this.category[index]}' is invalid, please check cjpm documentation for valid category names")
                flag = false
            } else {
                this.category[index] = this.category[index].toAsciiTitle().replace(" And ", " and ")
                if (this.category[index] == "Ui") {
                    this.category[index] = "UI"
                }
            }
        }
        return flag
    }
}

public func checkLegalUrl(url: String): Bool {
    return url.startsWith("/") || url.startsWith("\\\\") ||
        url.startsWith("file:") || url.startsWith("git:") ||
        url.startsWith("git@") || url.startsWith("http:") ||
        url.startsWith("https:") || url.startsWith("ssh:") ||
        Regex(WINDOWS_LOCAL_FILE_REGEX).matches(url)
}

public func checkSafeUrl(url: String): Bool {
    return !url.startsWith("-") && !url.contains("`") &&
        !url.contains("--upload-pack=") && !Regex(URL_WITH_SPACE).matches(url)
}

public func checkBundleName(val: String, nameType: String): Bool {
    if ((val.size < 3) || (val.size > 64)) {
        eprintln("Error: size of ${nameType} name '${val}' is wrong, which should be in [3, 64]")
        return false
    }

    let lowerOrgName = val.toAsciiLower()
    if (BLACK_NAME_LIST.contains(lowerOrgName)) {
        eprintln("Error: ${nameType} name cannot be '${val}'")
        return false
    }

    if ((nameType == META_ORGANIZATION) && (val == DEFAULT_ORGANIZATION)) {
        eprintln("Error: organization name cannot be 'default'")
        return false
    }

    return true
}