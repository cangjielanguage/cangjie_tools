// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.toml

import std.collection.*

public class LinkedHashMap<K, V> <: Map<K, V> where K <: Hashable & Equatable<K> {
    var table: HashMap<K, V>
    var ordering = LinkedList<K>()

    public prop size: Int64 {
        get() {
            ordering.size
        }
    }

    public init() {
        table = HashMap<K, V>()
    }

    public init(elements: Collection<(K, V)>) {
        this.table = HashMap<K, V>(elements)
        for (i in elements) {
            ordering.addLast(i[0])
        }
    }

    public func get(key: K): Option<V> {
        return table.get(key)
    }

    public func contains(key: K): Bool {
        table.contains(key)
    }

    public func contains(all!: Collection<K>): Bool {
        table.contains(all: all)
    }

    public func add(key: K, value: V): Option<V> {
        match (table.add(key, value)) {
            case Some(n) => return n
            case _ =>
                ordering.addLast(key)
                return None
        }
    }

    public func add(all!: Collection<(K, V)>): Unit {
        throw IllegalStateException("not implement")
    }

    public func remove(key: K): Option<V> {
        func compare(lkey: K): Bool {
            return key == lkey
        }
        match (table.remove(key)) {
            case Some(n) =>
                ordering.removeIf(compare)
                return n
            case _ => return None
        }
    }

    public func remove(all!: Collection<K>): Unit {
        throw IllegalStateException("not implement")
    }

    public func removeIf(_: (K, V) -> Bool): Unit {
        throw IllegalStateException("not implement")
    }

    public func clear(): Unit {
        table.clear()
        ordering.clear()
    }

    public func clone(): LinkedHashMap<K, V> {
        return LinkedHashMap<K, V>(this)
    }

    public func keys(): EquatableCollection<K> {
        LinkedHashMapKeys<K, V>(this)
    }

    public func values(): Collection<V> {
        LinkedHashMapValues<K, V>(this)
    }

    public func isEmpty(): Bool {
        table.isEmpty()
    }

    public func entryView(key: K): MapEntryView<K, V> {
        table.entryView(key)
    }

    public func iterator(): Iterator<(K, V)> {
        LinkedHashMapIterator<K, V>(ordering.iterator(), table)
    }

    public operator func [](key: K): V {
        table[key]
    }

    public operator func [](key: K, value!: V): Unit {
        table.add(key, value)
    }
}

extend<K, V> LinkedHashMap<K, V> <: ToString where V <: ToString, K <: ToString {
    public func toString(): String {
        if (size == 0) {
            return "[]"
        }
        let sb = StringBuilder("[")
        let it = iterator()
        var tmp: (K, V) = it.next().getOrThrow()
        while (let Some(next) <- it.next()) {
            //"(${k}, ${v})"
            sb.append(r'(')
            sb.append(tmp[0])
            unsafe { sb.appendFromUtf8Unchecked(", ".toArray()) }
            sb.append(tmp[1])
            unsafe { sb.appendFromUtf8Unchecked("), ".toArray()) }
            tmp = next
        }
        sb.append(r'(')
        sb.append(tmp[0])
        unsafe { sb.appendFromUtf8Unchecked(", ".toArray()) }
        sb.append(tmp[1])
        unsafe { sb.appendFromUtf8Unchecked(")]".toArray()) }
        return sb.toString()
    }
}

public class LinkedHashMapIterator<K, V> <: Iterator<(K, V)> where K <: Hashable & Equatable<K> {
    var iter: Iterator<K>
    var table: HashMap<K, V>

    init(iter: Iterator<K>, table: HashMap<K, V>) {
        this.iter = iter
        this.table = table
    }

    public func next(): ?(K, V) {
        match (iter.next()) {
            case Some(k) => (k, table[k])
            case _ => return None
        }
    }
}

class LinkedHashMapKeys<K, V> <: EquatableCollection<K> where K <: Hashable & Equatable<K> {
    let keys: Array<K>

    public prop size: Int64 {
        get() {
            return keys.size
        }
    }

    init(map: LinkedHashMap<K, V>) {
        keys = Array<K>(map.size, repeat: unsafe { zeroValue<K>() })
        for (i in 0..map.ordering.size) {
            keys[i] = map.ordering.nodeAt(i).getOrThrow().value
        }
    }

    public func isEmpty(): Bool {
        return keys.isEmpty()
    }

    public func contains(element: K): Bool {
        return keys.contains(element)
    }

    public func contains(all!: Collection<K>): Bool {
        throw IllegalStateException("not implement")
    }

    public func iterator(): Iterator<K> {
        return keys.iterator()
    }
}

class LinkedHashMapValues<K, V> <: Collection<V> where K <: Hashable & Equatable<K> {
    let values: Array<V>

    public prop size: Int64 {
        get() {
            return values.size
        }
    }

    init(map: LinkedHashMap<K, V>) {
        values = Array<V>(map.size, repeat: unsafe { zeroValue<V>() })
        for (i in 0..map.ordering.size) {
            values[i] = map.table[map.ordering.nodeAt(i).getOrThrow().value]
        }
    }

    public func isEmpty(): Bool {
        return values.isEmpty()
    }

    public func iterator(): Iterator<V> {
        return values.iterator()
    }

    public func toArray(): Array<V> {
        return values
    }
}
