// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*
import std.deriving.*
import stdx.log.getGlobalLogger
import stdx.serialization.serialization.*
import cjpm.util.{CliFailError, Result}

private let logger = getGlobalLogger()

// Struct which stores information about used 'cjc' binary
// 
// Use this structure instead of @When to implement platform depentent functionality
public struct CjcInfo {
    private static let versionPrefix = "Cangjie Compiler: "
    private static let targetPrefix = "Target: "

    private CjcInfo(
        public let version: String,
        public let target: Triple
    ) {}

    // specified in $PATH.
    // Parser result of 'cjc -v' to figure out:
    //    - Compiler version
    //    - Target triple information: {arch}-{vendor}-{os}-{env}
    static func findCjc(): CjcInfo {
        let (flag, output, errlog) = execWithOutput(COMPILE_TOOL, ArrayList<String>(["-v"]))
        if (!flag) {
            throw CliFailError([errlog, "Error: failed to run 'cjc -v'"])
        }
        if (!output.contains(versionPrefix)) {
            throw CliFailError(["Error: no target info in output of command 'cjc -v':", output])
        }
        if (!output.contains(targetPrefix)) {
            throw CliFailError(["Error: no target info in output of command 'cjc -v':", output])
        }

        let version = output.split(versionPrefix)[1].trimAscii().split(' ', 1, removeEmpty: true)[0]
        let targetString = output.split(targetPrefix)[1].trimAscii()
        let target = match (Triple.fromString(targetString)) {
            case Ok(triple) => triple
            case Err(errmesg) => throw CliFailError([errmesg])
        }
        CjcInfo(version, target)
    }
}

// Enum of known architectures specified in target triple
@Derive[Hashable, Equatable]
public enum Arch <: ToString {
    | X64
    | AARCH64
    | Unknown(String)

    public func toString(): String {
        match (this) {
            case X64 => "x86_64"
            case AARCH64 => "aarch64"
            case Unknown(s) => s
        }
    }
    static func fromString(arch: String): Arch {
        match {
            case arch == "x86_64" => X64
            case arch == "aarch64" => AARCH64
            case _ =>
                logger.warn("Unknown architecture type: ${arch}")
                Unknown(arch)
        }
    }
}

// Enum of known environments specified in target triple
@Derive[Hashable, Equatable]
public enum Vendor <: ToString {
    | APPLE
    | UNKNOWN

    public func toString(): String {
        match (this) {
            case APPLE => "apple"
            case UNKNOWN => "unknown"
        }
    }

    static func fromString(vendor: String): Vendor {
        match {
            case vendor == "apple" => APPLE
            case _ => UNKNOWN
        }
    }
}

// Enum of known operational systems specified in target triple
@Derive[Hashable, Equatable]
public enum OS <: ToString {
    | LINUX
    | DARWIN
    | WINDOWS
    | HM
    | Unknown(String)

    public func toString(): String {
        match (this) {
            case LINUX => "linux"
            case DARWIN => "darwin"
            case WINDOWS => "w64"
            case HM => "hm"
            case Unknown(s) => s
        }
    }

    static func fromString(os: String): OS {
        match {
            case os == "linux" => LINUX
            case os == "darwin" => DARWIN
            case os == "windows" || os == "w64" => WINDOWS
            case os == "hm" => HM
            case _ =>
                logger.warn("Unknown os type: ${os}")
                Unknown(os)
        }
    }
}

// Enum of known environments specified in target triple
@Derive[Hashable, Equatable]
public enum Env <: ToString {
    | HOS
    | GNU
    | ANDROID
    | MINGW32
    | OHOS
    | NOT_AVAILABLE
    | Unknown(String)

    public func toString(): String {
        match (this) {
            case HOS => "hos"
            case GNU => "gnu"
            case ANDROID => "android"
            case MINGW32 => "mingw32"
            case OHOS => "ohos"
            case NOT_AVAILABLE => ""
            case Unknown(s) => s
        }
    }

    static func fromString(env: String): Env {
        match {
            case env == "hos" => HOS
            case env == "gnu" => GNU
            case env == "android" => ANDROID
            case env == "mingw32" => MINGW32
            case env == "ohos" => OHOS
            case _ =>
                logger.warn("Unknown environment: ${env}")
                Unknown(env)
        }
    }
}

@Derive[Hashable, Equatable]
public struct Triple <: ToString {
    private Triple(
        @DeriveExclude private let _stringValue: String,
        public let arch!: Arch,
        public let vendor!: Vendor = UNKNOWN,
        public let os!: OS,
        public let env!: Env
    ) {}

    private init(
        arch!: Arch,
        vendor!: Vendor = UNKNOWN,
        os!: OS,
        env!: Env
    ) {
        this.arch = arch
        this.vendor = vendor
        this.os = os
        this.env = env
        this._stringValue = toStringFromTriple(arch, vendor, os, env)
    }

    private static func toStringFromTriple(arch: Arch, vendor: Vendor, os: OS, env: Env): String {
        if (vendor == Vendor.UNKNOWN && os == OS.WINDOWS) {
            return "${arch}-${os}${if (env != Env.NOT_AVAILABLE) {"-${env}"} else {""}}"
        } else {
            return "${arch}-${vendor}-${os}${if (env != Env.NOT_AVAILABLE) {"-${env}"} else {""}}"
        }
    }

    public func toString(): String {
        _stringValue
    }

    public static func fromString(triple: String): Result<Triple, String> {
        let data = triple.split("-")
        if (data.size != 3 && data.size != 4) {
            return Err(
                "The target-triple must be of form {ARCH}-{VENDOR}-{SYSTEM}-{ENV} or {ARCH}-{SYSTEM}-{ENV}. Got: ${triple}"
            )
        }
        if (data[1] == "apple" && data[2] == "darwin") {
            return Ok(
                Triple(
                    arch: Arch.fromString(data[0]),
                    vendor: Vendor.fromString(data[1]),
                    os: OS.fromString(data[2]),
                    env: Env.NOT_AVAILABLE
                )
            )
        }
        return Ok(
            Triple(
                arch: Arch.fromString(data[0]),
                vendor: Vendor.UNKNOWN,
                os: OS.fromString(data[data.size - 2]),
                env: Env.fromString(data[data.size - 1])
            )
        )
    }
}

private let osToFeatureMap: HashMap<OS, Feature> = HashMap(
    [
        (OS.LINUX, Feature.OS_LINUX),
        (OS.DARWIN, Feature.OS_DARWIN),
        (OS.WINDOWS, Feature.OS_WINDOWS),
        (OS.HM, Feature.OS_HM)
    ]
)
private let envToFeatureMap: HashMap<Env, Feature> = HashMap(
    [
        (Env.HOS, Feature.ENV_HOS),
        (Env.GNU, Feature.ENV_GNU),
        (Env.ANDROID, Feature.ENV_ANDROID),
        (Env.MINGW32, Feature.ENV_MINGW32),
        (Env.OHOS, Feature.ENV_OHOS)
    ]
)
private let archToFeatureMap: HashMap<Arch, Feature> = HashMap(
    [(Arch.X64, Feature.ARCH_X64), (Arch.AARCH64, Feature.ARCH_AARCH64)])
private let stringToFeatureMap: HashMap<String, Feature> = HashMap(
    [
        ("feature.os.posix", OS_POSIX),
        ("feature.os.epoll", OS_EPOLL),
        ("feature.os.kqueue", OS_KQUEUE),
        ("feature.os.linux", OS_LINUX),
        ("feature.os.windows", OS_WINDOWS),
        ("feature.os.darwin", OS_DARWIN),
        ("feature.os.hm", OS_HM),
        ("feature.arch.big", ARCH_BIG),
        ("feature.arch.little", ARCH_LITTLE),
        ("feature.arch.x64", ARCH_X64),
        ("feature.arch.aarch64", ARCH_AARCH64),
        ("feature.arch.sse1", ARCH_SSE1),
        ("feature.arch.sse2", ARCH_SSE2),
        ("feature.arch.sse3", ARCH_SSE3),
        ("feature.arch.sse4.1", ARCH_SSE4_1),
        ("feature.arch.sse4.2", ARCH_SSE4_2),
        ("feature.arch.avx1", ARCH_AVX1),
        ("feature.arch.avx2", ARCH_AVX2),
        ("feature.arch.avx512", ARCH_AVX512),
        ("feature.arch.neon", ARCH_NEON),
        ("feature.env.ohos", ENV_OHOS),
        ("feature.env.gnu", ENV_GNU),
        ("feature.env.mingw32", ENV_MINGW32),
        ("feature.env.hos", ENV_HOS),
        ("feature.env.android", ENV_ANDROID)
    ]
)

@Derive[Hashable, Equatable]
public enum Feature <: ToString & Serializable<Feature> {
    | OS_POSIX
    | OS_EPOLL
    | OS_KQUEUE
    | OS_WINDOWS
    | OS_LINUX
    | OS_HM
    | OS_DARWIN
    | ARCH_BIG
    | ARCH_LITTLE
    | ARCH_X64
    | ARCH_AARCH64
    | ARCH_SSE1
    | ARCH_SSE2
    | ARCH_SSE3
    | ARCH_SSE4_1
    | ARCH_SSE4_2
    | ARCH_AVX1
    | ARCH_AVX2
    | ARCH_AVX512
    | ARCH_NEON
    | ENV_OHOS
    | ENV_GNU
    | ENV_MINGW32
    | ENV_HOS
    | ENV_ANDROID
    | CJ_VERSION(String)

    public func toString(): String {
        match (this) {
            case OS_POSIX => "feature.os.posix"
            case OS_EPOLL => "feature.os.epoll"
            case OS_KQUEUE => "feature.os.kqueue"
            case OS_WINDOWS => "feature.os.windows"
            case OS_LINUX => "feature.os.linux"
            case OS_HM => "feature.os.hm"
            case OS_DARWIN => "feature.os.darwin"
            case ARCH_BIG => "feature.arch.big"
            case ARCH_LITTLE => "feature.arch.little"
            case ARCH_X64 => "feature.arch.x64"
            case ARCH_AARCH64 => "feature.arch.aarch64"
            case ARCH_SSE1 => "feature.arch.sse1"
            case ARCH_SSE2 => "feature.arch.sse2"
            case ARCH_SSE3 => "feature.arch.sse3"
            case ARCH_SSE4_1 => "feature.arch.sse4.1"
            case ARCH_SSE4_2 => "feature.arch.sse4.2"
            case ARCH_AVX1 => "feature.arch.avx1"
            case ARCH_AVX2 => "feature.arch.avx2"
            case ARCH_AVX512 => "feature.arch.avx512"
            case ARCH_NEON => "feature.arch.neon"
            case ENV_OHOS => "feature.env.ohos"
            case ENV_GNU => "feature.env.gnu"
            case ENV_MINGW32 => "feature.env.mingw32"
            case ENV_HOS => "feature.env.hos"
            case ENV_ANDROID => "feature.env.android"
            case CJ_VERSION(s) => "feature.cj.v${s}"
        }
    }

    public static func fromStringOp(s: String): ?Feature {
        if (let Some(res) <- stringToFeatureMap.get(s)) {
            return res
        }

        let res = s.split("feature.cj.v", 2)
        if (res.size == 2) {
            return CJ_VERSION(res[1])
        }
        return None
    }
    public static func fromString(s: String): Feature {
        return fromStringOp(s) ?? throw CliFailError("provided string \"${s}\" did not match with any of the features")
    }

    public func serialize(): DataModel {
        return DataModelString(this.toString())
    }

    public static func deserialize(dm: DataModel): Feature {
        if (let Some(sdm) <- (dm as DataModelString)) {
            return fromString(sdm.getValue())
        }
        throw CliFailError("tried to deserialize Feature not from the DataModelString")
    }
}

// Struct which basically a Node in feature dependence graph which features are causing enable of a starting one
struct FeatureGroup {
    FeatureGroup(
        // Feature which must be added in feature list in case of match
        public let enable: Array<Feature>,
        // If at least one of the features is enabled, then the feature above should be provided
        public let enabled_by: Array<Feature>
    ) {}
}

private let PREDEFINED_FEATURE_GROUPS: Array<FeatureGroup> = [
    FeatureGroup(
        [OS_POSIX], // enable
        [OS_LINUX, OS_DARWIN] // enabled by
    ),
    FeatureGroup(
        [OS_EPOLL], // enable
        [OS_LINUX] // enabled by
    ),
    FeatureGroup(
        [OS_KQUEUE], // enable
        [OS_DARWIN] // enabled by
    ),
    FeatureGroup(
        [ARCH_NEON], // enable
        [ARCH_AARCH64] // enabled by
    ),

    // x86-64 is guaranteed to have SSE1, SSE2 instruction sets
    FeatureGroup(
        [ARCH_SSE1, ARCH_SSE2], // enable
        [ARCH_X64] // enabled by
    ),
    FeatureGroup(
        [ARCH_SSE1], // enable
        [ARCH_SSE2] // enabled by
    ),
    FeatureGroup(
        [ARCH_SSE1], // enable
        [ARCH_SSE2] // enabled by
    ),
    FeatureGroup(
        [ARCH_SSE2], // enable
        [ARCH_SSE3] // enabled by
    ),
    FeatureGroup(
        [ARCH_SSE3], // enable
        [ARCH_SSE4_1] // enabled by
    ),
    FeatureGroup(
        [ARCH_SSE4_1], // enable
        [ARCH_SSE4_2] // enabled by
    ),
    FeatureGroup(
        [ARCH_SSE4_2], // enable
        [ARCH_AVX1] // enabled by
    ),
    FeatureGroup(
        [ARCH_SSE4_2], // enable
        [ARCH_AVX1] // enabled by
    ),
    FeatureGroup(
        [ARCH_AVX1], // enable
        [ARCH_AVX2] // enabled by
    ),
    FeatureGroup(
        [ARCH_AVX2], // enable
        [ARCH_AVX512] // enabled by
    )
]

private class FeatureGraph {
    private let graph: HashMap<Feature, ArrayList<Feature>> = HashMap()

    FeatureGraph(groups: Iterable<FeatureGroup>) {
        groups |>
            forEach {
            it: FeatureGroup => for (i in it.enabled_by) {
                if (!this.graph.contains(i)) {
                    this.graph.add(i, ArrayList())
                }
                for (j in it.enable) {
                    this.graph.get(i)?.add(j)
                }
            }
        }
    }

    // Looks for features that might be enable from already enabled feature set
    // Updates the same ArrayList
    func findDependantFeatures(initFeatures: Array<Feature>): ArrayList<Feature> {
        let visited: HashSet<Feature> = HashSet()
        let features = ArrayList(initFeatures)

        for (s in features) {
            visited.add(s)
        }

        var i = 0
        while (i < features.size) {
            let curr = features[i]
            if (let Some(front) <- graph.get(curr)) {
                for (it in front where !visited.contains(it)) {
                    visited.add(curr)
                    features.add(it)
                }
            }
            i++
        }
        visited |> collectArrayList
    }
}

private let FEATURE_GRAPH: FeatureGraph = FeatureGraph(PREDEFINED_FEATURE_GROUPS)

public struct FeatureDeducer {
    private let _features: Array<Feature>
    private let _enabled: Bool

    public init(it: Iterable<Feature>, enabled!: Bool = true) {
        _features = it |> collectArray
        _enabled = enabled
    }

    public func deduceTargetFeatures(target: Triple): FeatureDeducer {
        if (!this._enabled) {
            return this
        }

        let features = ArrayList<Feature>()
        if (let Some(arch) <- archToFeatureMap.get(target.arch)) {
            features.add(arch)
        } else {
            println("Warn: Unknown `arch` feature specified in target is ignored: ${target}")
        }
        // No vendor specific feature yet, ignoring it
        if (let Some(os) <- osToFeatureMap.get(target.os)) {
            features.add(os)
        } else {
            println("Warn: Unknown `os` feature specified in target is ignored: ${target}")
        }
        if (target.env != NOT_AVAILABLE) {
            if (let Some(env) <- envToFeatureMap.get(target.env)) {
                features.add(env)
            } else {
                println("Warn: Unknown `env` feature specified in target is ignored: ${target}")
            }
        }
        return FeatureDeducer(this._features |> concat(features), enabled: this._enabled)
    }

    public func addFeature(feature: Feature): FeatureDeducer {
        return FeatureDeducer(this._features |> concat([feature]), enabled: this._enabled)
    }

    public func addFeature(features: Iterable<Feature>): FeatureDeducer {
        return FeatureDeducer(this._features |> concat(features), enabled: this._enabled)
    }

    public func deduceDependantFeatures(): FeatureDeducer {
        if (!this._enabled) {
            return this
        }
        return FeatureDeducer(FEATURE_GRAPH.findDependantFeatures(this._features), enabled: this._enabled)
    }

    public func collect(): ArrayList<Feature> {
        (_features |> collectHashSet) |> collectArrayList
    }
}

