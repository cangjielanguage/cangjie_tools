// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.util

import std.collection.*
import std.deriving.*

import stdx.serialization.serialization.*

public class DAGBuilder<K, V> where K <: Hashable & Equatable<K> {
    private let graph = HashMap<K, (Array<K>, ArrayList<K>)>()
    private let roots = HashSet<K>()
    private let toposort = ArrayList<K>()
    private let data = HashMap<K, V>()

    init() { }

    public func add(id: K, incoming!: Array<K> = [], data!: ?V = None): Unit {
        assertion { !graph.contains(id) }
        assertion { graph.contains(all: incoming) }

        if (incoming.size == 0) {
            roots.add(id)
        }

        for (node in incoming) {
            let (_, out) = graph[node]
            out.add(id)
        }

        if (let Some(data) <- data) {
            this.data.add(id, data)
        }

        toposort.add(id)
        graph.add(id, (incoming, ArrayList()))
    }

    public func finish(): DAG<K, V> {
        let newGraph = graph.iterator().map { it => 
            let (id, (incoming, outcoming)) = it
            (id, Node<K>(incoming, outcoming.toArray())) 
        } |> collectHashMap
        DAG(newGraph, roots.clone(), toposort.toArray(), data)
    }
}

struct Node<K> {
    Node (
        let indeg: Array<K>,
        let outdeg: Array<K>
    ) { }

    func filterEdges(cond: (K) -> Bool): Node<K> {
        Node(
            this.indeg.filter(cond) |> collectArray,
            this.outdeg.filter(cond) |> collectArray
        )
    }
}

public class DAG<K, V> where K <: Hashable & Equatable<K> {
    DAG(
        let graph: HashMap<K, Node<K>>,
        let _roots: HashSet<K>,
        let _toposort: Array<K>,
        let _data: HashMap<K, V>
    ) { }


    public prop toposort: Array<K> {
        get() { _toposort }
    }


    public prop roots: Array<K> {
        get() { _roots.toArray() }
    }

    public prop data: HashMap<K, V> {
        get() { _data }
    }


    public static func builder(): DAGBuilder<K, V> {
        DAGBuilder()
    }

    public func filterGraph(cond: (K, V) -> Bool): DAG<K, V> {
        let _roots = _roots.iterator().filter { it => cond(it, this._data[it]) } |> collectHashSet
        let _data = _data.iterator().filter { it => cond(it[0], it[1]) } |> collectHashMap

        let graph = graph.iterator().filterMap { it: (K, Node<K>) =>
            let (id, state) = it
            if (!cond(id, this._data[id])) { 
                return None<(K, Node<K>)>
            }
            return (id, state.filterEdges { it => cond(id, this._data[id] )} )
        } |> collectHashMap

        let toposort = toposort.filter { it => cond(it, this._data[it]) } |> collectArray

        DAG<K, V>(graph, _roots, toposort, _data)
    }

    /**
     * Remove verticies of the graph while preserving connectivity of unaffected verticies
     */
    public func filterVertices(cond: (K, V) -> Bool): DAG<K, V> {
        let _roots = _roots.iterator().filter { it => cond(it, this._data[it]) } |> collectHashSet
        let _data = _data.iterator().filter { it => cond(it[0], it[1]) } |> collectHashMap
        let removed = HashSet<K>()

        for (id in this.toposort) {
            if (!cond(id, data[id])) {
                removed.add(id)
            }
        }

        let size = this.toposort.size

        let adj = graph.iterator().map { it => (it[0], it[1].outdeg |> collectHashSet) } |> collectHashMap
        for (k in removed) {
            for (i in this.toposort) {
                for (j in this.toposort) {
                    if (adj[i].contains(k) && adj[k].contains(j)) {
                        adj[i].add(j)
                    }
                }
            }
        }

        let builder = DAG<K, V>.builder()
        // where for ordering consistency
        for (j in this.toposort where !removed.contains(j)) {
            let incoming = ArrayList<K>()
            for (i in this.toposort where !removed.contains(i)) {
                if (adj[i].contains(j) && i != j) {
                    incoming.add(i)
                }
            }
            builder.add(j, incoming: incoming.toArray(), data: this.data.get(j))
        }
        builder.finish()
    }

    public func map<U>(transform: (K, V) -> U): DAG<K, U> {
        let _roots = _roots.clone()
        let _data = _data.iterator().map { it => (it[0], transform(it[0], it[1]))} |> collectHashMap
        DAG<K, U>(graph, _roots, toposort, _data)
    }

    public func traverse(from: K, onEntry: (K) -> Unit): Unit {
        traverse(from, { _ => true }, onEntry)
    }


    public func traverse(from: K, entryCondition: (K) -> Bool, onEntry: (K) -> Unit): Unit {
        let stack = ArrayDeque<K>()
        let visited = HashSet<K>()
        stack.addFirst(from)
        while (let Some(from) <- stack.removeFirst()) {
            visited.add(from)
            onEntry(from)
            for (item in graph[from].outdeg) {
                if (!visited.contains(item) && entryCondition(item)) {
                    stack.addLast(item)
                }
            }
        }
    }

    public func traverseReverse(from: K, onEntry: (K) -> Unit): Unit {
        traverseReverse(from, { _ => true }, onEntry)
    }


    public func traverseReverse(from: K, entryCondition: (K) -> Bool, onEntry: (K) -> Unit): Unit {
        let stack = ArrayDeque<K>()
        let visited = HashSet<K>()
        stack.addFirst(from)
        while (let Some(from) <- stack.removeFirst()) {
            visited.add(from)
            onEntry(from)
            for (item in graph[from].indeg) {
                if (!visited.contains(item) && entryCondition(item)) {
                    stack.addLast(item)
                }
            }
        }
    }
}

extend <K, V> DAG<K, V> where K <: Hashable & Equatable<K> {
    /**
     * Provides transitive closure of DAG by incoming edges starting from given
     */
    public func incomingClosure(id: K): DAG<K, V> {
        let asked = HashSet<K>()
        traverseReverse(id) { it => asked.add(it) }
        this.filterGraph { it: K, _ => asked.contains(it) }
    }
}



extend <K, V> DAG<K, V> where K <: ToString {
    public func incoming(id: K): Array<K> {
        graph[id].indeg
    }

    public func outcoming(id: K): Array<K> {
        graph[id].outdeg
    }
}


extend <V> DAG<String, V> <: Serializable<DAG<String, V>> where V <: Serializable<V> {
    public func serialize(): DataModel {
        DataModelStruct()
            .add(field("roots", roots))
            .add(field("graph", graph))
            .add(field("toposort", toposort))
            .add(field("data", data))
    }

    public static func deserialize(dm: DataModel): DAG<String, V> {
        let dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw DataModelException("this data is not DataModelStruct")
        }

        let roots = HashSet<String>.deserialize(dms.get("roots"))
        let toposort = Array<String>.deserialize(dms.get("toposort"))
        let graph = HashMap<String, Node<String>>.deserialize(dms.get("graph"))
        let data = HashMap<String, V>.deserialize(dms.get("data"))

        DAG<String, V>(graph, roots, toposort, data)
    }
}

extend <K> Node<K> <: Serializable<Node<K>> where K <: Serializable<K> {
    public func serialize(): DataModel {
        DataModelStruct()
            .add(field("indeg", indeg))
            .add(field("outdeg", outdeg))
    }

    public static func deserialize(dm: DataModel): Node<K> {
        let dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw DataModelException("this data is not DataModelStruct")
        }

        let indeg = Array<K>.deserialize(dms.get("indeg"))
        let outdeg = Array<K>.deserialize(dms.get("outdeg"))
        Node(indeg, outdeg)
    }
}
