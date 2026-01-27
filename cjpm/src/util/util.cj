// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.util

import std.collection.*
import std.sort.sort
import std.deriving.Derive
import stdx.serialization.serialization.*

// Checks whether full-qualified name of the package represents some subpackage
// 
// According to Cangjie Language grammar, package name cannot contain
// backtick (`) character, making parsing straightforward. Thus if we meet dot (.)
// not being enclosed in backticks - it should be subpackage.
// 
// @param fqname  CORRECT full-qualified name of package
// 
// @return true  - subpackage
//         false - root package
// 
public func isSubpackage(fqname: String): Bool {
    var quoted = false
    for (character in fqname.runes()) {
        match {
            case character == r'`' => quoted = !quoted
            case character == r'.' && !quoted => return true
            case _ => ()
        }
    }
    return false
}

// Structure which represents full-qualified package name parsed from the String literal
// 
// If struct is tried to be constructed from incorrect one (missing backticks '`' when necessary)
// Will attempt to place them in some way for package name to be valid
@Derive[Hashable, Equatable]
public struct PackageFqName <: ToString & Serializable<PackageFqName> {
    public let value: String

    public init(value: String) {
        this.value = parse(unparse(value))
    }

    private static func unparse(fqname: String): Array<String> {
        var quoted = false
        let substr = ArrayList<Rune>()
        let result = ArrayList<String>()

        for (character in fqname.runes()) {
            match {
                case character == r'`' =>
                    quoted = !quoted
                    if (!quoted) {
                        result.add(String(substr))
                        substr.clear()
                    }
                case character == r'.' && !quoted =>
                    result.add(String(substr))
                    substr.clear()
                case _ => substr.add(character)
            }
        }

        if (!substr.isEmpty() && !quoted) {
            result.add(String(substr))
            substr.clear()
        }
        return result.toArray()
    }

    private static func parse(packages: Array<String>): String {
        packages |>
            map {
            pkg: String =>
            let flag: Bool = pkg.contains("-") || pkg.contains(" ") || pkg.contains(".")
            if (flag) {
                "`${pkg}`"
            } else {
                "${pkg}"
            }
        } |> collectString(delimiter: ".")
    }

    public prop packages: Array<String> {
        get() {
            unparse(value)
        }
    }

    public func toString(): String {
        return value
    }

    public func serialize(): DataModel {
        DataModelString(value)
    }

    public static func deserialize(dm: DataModel): PackageFqName {
        return match (dm) {
            case dmstr: DataModelString => PackageFqName(dmstr.getValue())
            case _ => throw DataModelException("this data is not DataModelString")
        }
    }
}

// Support struct which helps to identify whether value of type T is a default or was modified
public struct WithDefault<T> {
    private var _value: OptionDefault<T>

    public WithDefault(default: T) {
        _value = Default(default)
    }

    public prop isSet: Bool {
        get() {
            match (_value) {
                case Setted(_) => true
                case _ => false
            }
        }
    }

    public prop isDefault: Bool {
        get() {
            match (_value) {
                case Default(_) => true
                case _ => false
            }
        }
    }

    public prop value: T {
        get() {
            match (_value) {
                case Setted(x) => x
                case Default(x) => x
            }
        }
    }

    public mut func modify(value: T) {
        _value = Setted(value)
    }
}

private enum OptionDefault<T> {
    | Setted(T)
    | Default(T)
}


public func tryResult<T>(comp: () -> T): Result<T, String> {
    try {
        return Ok(comp())
    } catch(e: Exception) {
        return Err(e.message)
    }
}

public func unorderedHash<T>(iter: Iterable<T>): Int64 where T <: Hashable {
    let hashes = iter.iterator().map { it => it.hashCode() } |> collectArray
    sort(hashes)
    var hashser = DefaultHasher()
    hashser.write(hashes.size)
    for (hash in hashes) {
        hashser.write(hash)
    }
    hashser.finish()
}
