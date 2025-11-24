// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.convert.*
import std.collection.*

// Class of a single version
public class SingleVersion <: Comparable<SingleVersion> & ToString {
    public SingleVersion(
        public var majorVersion: Int64,
        public var minorVersion: Int64,
        public var patchVersion: Int64
    ) {}

    public init() {
        this.majorVersion = 0
        this.minorVersion = 0
        this.patchVersion = 0
    }

    public init(other: SingleVersion) {
        this.majorVersion = other.majorVersion
        this.minorVersion = other.minorVersion
        this.patchVersion = other.patchVersion
    }

    public init(version: String) {
        let verArr = version.split(".")
        this.majorVersion = Int64.parse(verArr[0])
        this.minorVersion = Int64.parse(verArr[1])
        this.patchVersion = Int64.parse(verArr[2])
    }

    public func toString(): String {
        return "${this.majorVersion}.${this.minorVersion}.${this.patchVersion}"
    }

    public static func minVersion(): SingleVersion {
        return SingleVersion()
    }

    public static func maxVersion(): SingleVersion {
        return SingleVersion(Int64.Max, Int64.Max, Int64.Max)
    }

    public func compare(that: SingleVersion): Ordering {
        if (this == that) {
            return Ordering.EQ
        } else if (this > that) {
            return Ordering.GT
        } else {
            return Ordering.LT
        }
    }

    public operator func <(rhs: SingleVersion): Bool {
        return (this.majorVersion < rhs.majorVersion) ||
            ((this.majorVersion == rhs.majorVersion) && (this.minorVersion < rhs.minorVersion)) ||
            ((this.majorVersion == rhs.majorVersion) && (this.minorVersion == rhs.minorVersion) && (this.patchVersion < rhs.patchVersion))
    }

    public operator func <=(rhs: SingleVersion): Bool {
        return (this < rhs) || (this == rhs)
    }

    public operator func ==(rhs: SingleVersion): Bool {
        return (this.majorVersion == rhs.majorVersion) &&
            (this.minorVersion == rhs.minorVersion) &&
            (this.patchVersion == rhs.patchVersion)
    }

    public operator func >(rhs: SingleVersion): Bool {
        return !(this <= rhs)
    }

    public operator func >=(rhs: SingleVersion): Bool {
        return !(this < rhs)
    }

    public func upgrade(): Unit {
        if (this.patchVersion < Int64.Max) {
            this.patchVersion += 1
        } else if (this.minorVersion < Int64.Max) {
            this.patchVersion = 0
            this.minorVersion += 1
        } else if (this.majorVersion < Int64.Max) {
            this.patchVersion = 0
            this.minorVersion = 0
            this.majorVersion += 1
        }
    }

    public func downgrade(): Unit {
        if (this.patchVersion > 0) {
            this.patchVersion -= 1
        } else if (this.minorVersion > 0) {
            this.patchVersion = Int64.Max
            this.minorVersion -= 1
        } else if (this.majorVersion > 0) {
            this.patchVersion = Int64.Max
            this.minorVersion = Int64.Max
            this.majorVersion -= 1
        }
    }
}

// Range of require
// Always an closed interval
class RequireRange {
    public RequireRange(
        public let floor: SingleVersion,
        public let ceil: SingleVersion
    ) {}

    public init(other: RequireRange) {
        this.floor = SingleVersion(other.floor)
        this.ceil = SingleVersion(other.ceil)
    }

    public func isMatchWith(versionStr: String): Bool {
        return this.isMatchWith(SingleVersion(versionStr))
    }

    public func isMatchWith(version: SingleVersion): Bool {
        return (this.floor <= version) && (this.ceil >= version)
    }
}

/*
 * Class of a version require
 * A SingleVersion sv matches a RequireVersion rv when sv matches any requirement of rv
 *
 * Maybe in these formats:
 * 1. a single version, e.g., '1.2.3';
 * 2. a version range, where '[]' denotes a closed interval and '()' denotes an open interval, e.g., '[1.2.3, 2.3.4)';
 *   - version boundary may be empty, in which case you should use '()', e.g., [1.2.3,);
 * 3. any combination of the above fields, separated by ',', e.g., '1.2.3, [2.3.4, 3.4.5), (4.5.6,)'
 */
public class RequireVersion {
    var name: String = ""
    let ranges: ArrayList<RequireRange> = ArrayList<RequireRange>()

    public init() {}

    public init(name: String, versionStr: String) {
        // delete all whitespace
        // version is checked by regex
        let rawStr = String.join(versionStr.split(" "))

        // split into single version or version range
        let brokenRequireArr = rawStr.split(",")
        var index = 0
        while (index < brokenRequireArr.size) {
            if (!brokenRequireArr[index].startsWith("[") && !brokenRequireArr[index].startsWith("(")) {
                // a single version, always valid
                // create a RequireRange whose floor and ceil is the same
                ranges.add(RequireRange(SingleVersion(brokenRequireArr[index]), SingleVersion(brokenRequireArr[index])))
                index += 1
                continue
            }
            // a version range, broken by ','
            // create a RequireRange and make it an closed interval
            let floorSign = brokenRequireArr[index][..1]
            let floorStr = brokenRequireArr[index].removePrefix(floorSign)
            let floor = if (floorStr.isEmpty()) { SingleVersion.minVersion() } else { SingleVersion(floorStr) }
            if (!floorStr.isEmpty() && (floorSign == "(")) {
                floor.upgrade()
            }

            let ceilSign = brokenRequireArr[index + 1][brokenRequireArr[index + 1].size - 1..]
            let ceilStr = brokenRequireArr[index + 1].removeSuffix(ceilSign)
            let ceil = if (ceilStr.isEmpty()) { SingleVersion.maxVersion() } else { SingleVersion(ceilStr) }
            if (!ceilStr.isEmpty() && (ceilSign == ")")) {
                ceil.downgrade()
            }

            // check if range is valid
            if (floor <= ceil) {
                ranges.add(RequireRange(floor, ceil))
            }

            index += 2
        }

        // if all ranges are invalid, throw exception
        if (ranges.isEmpty()) {
            throw NoneValueException("None of version in '${versionStr}' is valid")
        }

        this.name = name
    }

    public init(other: RequireVersion) {
        this.name = other.name
        for (range in other.ranges) {
            this.ranges.add(RequireRange(range))
        }
    }

    public func isSingleVersion(): Bool {
        return (this.ranges.size == 1) && (this.ranges[0].floor == this.ranges[0].ceil)
    }

    public func isMatchWith(versionStr: String): Bool {
        return this.isMatchWith(SingleVersion(versionStr))
    }

    public func isMatchWith(version: SingleVersion): Bool {
        for (reqRange in this.ranges) {
            if (reqRange.isMatchWith(version)) {
                return true
            }
        }
        return false
    }
}
