// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
 
package cjpm.util

import std.collection.ArrayList

public enum Result<T, E> {
    | Ok(T)
    | Err(E)
 
    public prop isOk: Bool {
        get() {
            match (this) {
                case Ok(_) => true
                case Err(_) => false
            }
        }
    }
 
    public prop isErr: Bool {
        get() {
            !isOk
        }
    }
 
    public prop ok: Option<T> {
        get() {
            match (this) {
                case Ok(t) => Some(t)
                case Err(_) => Option<T>.None
            }
        }
    }
 
    public prop err: Option<E> {
        get() {
            match (this) {
                case Ok(_) => Option<E>.None
                case Err(e) => Some(e)
            }
        }
    }
 
    public func map<U>(transform: (T) -> U): Result<U, E> {
        match (this) {
            case Ok(t) => Ok(transform(t))
            case Err(e) => Err(e)
        }
    }
 
    public func mapErr<F>(transform: (E) -> F): Result<T, F> {
        match (this) {
            case Ok(t) => Ok(t)
            case Err(e) => Err(transform(e))
        }
    }

    public func flatMap<U>(transform: (T) -> Result<U, E>): Result<U, E> {
        match (this) {
            case Ok(t) => transform(t)
            case Err(e) => Err(e)
        }
    }

    public func flatMapErr<F>(transform: (E) -> Result<T, F>): Result<T, F> {
        match (this) {
            case Ok(t) => Ok(t)
            case Err(e) => transform(e)
        }
    }

    public func inspect(action: (T) -> Unit): Result<T, E> {
        match (this) {
            case Ok(t) => action(t); Ok(t)
            case Err(_) => this 
        }
    }
 
    public func inspectErr(action: (E) -> Unit): Result<T, E> {
        match (this) {
            case Ok(_) => this
            case Err(e) => action(e); Err(e)
        }
    }

    public func unit(): Result<Unit, E> {
        this.map { _ => () }
    }

    public func finish(action: (T) -> Unit): Unit {
        match (this) {
            case Ok(x) => action(x)
            case Err(_) => ()
        }
    }
 
    public func finishErr(action: (E) -> Unit): Unit {
        match (this) {
            case Ok(_) => ()
            case Err(e) => action(e)
        }
    }
 
    public func getOrDefault(default: () -> T): T {
        this.ok ?? default()
    }

    public func getOrThrow(err: (E) -> Exception): T {
        match (this) {
            case Ok(t) => t
            case Err(e) => throw err(e)
        }
    }

}

extend <T, E> Result<T, E> where E <: ToString {
    public func getOrThrow(): T {
        match (this) {
            case Ok(t) => t
            case Err(e) => throw GetResultException(e.toString())
        }
    }
}

private class GetResultException <: Exception {
    public GetResultException(message: String) { 
        super(message)
    }
}

private func helperAllResult<A, B, E>(iter: Iterable<A>, transform: (A) -> Result<B, Array<E>>): Result<Array<B>, Array<E>> {
    let errors = ArrayList<E>()
    let result = ArrayList<B>()
    for (item in iter) {
        transform(item)
            .inspectErr { e => errors.add(all: e) }
            .inspect { r => result.add(r)}
    }
    if (errors.size != 0) {
        Err(errors.toArray())
    } else {
        Ok(result.toArray())
    }
}

private func helperAnyResult<A, B, E>(iter: Iterable<A>, transform: (A) -> Result<B, Array<E>>): Result<B, Array<E>> {
    let errors = ArrayList<E>()
    for (item in iter) {
        let res = transform(item)
            .inspectErr { e => errors.add(all: e) }
        res.err ?? return res
    }
    Err(errors.toArray())
}

public interface IterableResultExtend<A> {
    func allResult<B, E>(transform: (A) -> Result<B, Array<E>>): Result<Array<B>, Array<E>> 
    func anyResult<B, E>(transform: (A) -> Result<B, Array<E>>): Result<B, Array<E>>
}

extend <A> Iterator<A> <: IterableResultExtend<A> { 
    public func allResult<B, E>(transform: (A) -> Result<B, Array<E>>): Result<Array<B>, Array<E>> {
        helperAllResult(this, transform)
    }

    public func anyResult<B, E>(transform: (A) -> Result<B, Array<E>>): Result<B, Array<E>> {
        helperAnyResult(this, transform)
    }
}

extend <A> Array<A> <: IterableResultExtend<A> { 
    public func allResult<B, E>(transform: (A) -> Result<B, Array<E>>): Result<Array<B>, Array<E>> {
        helperAllResult(this, transform)
    }

    public func anyResult<B, E>(transform: (A) -> Result<B, Array<E>>): Result<B, Array<E>> {
        helperAnyResult(this, transform)
    }
}

extend <A> ArrayList<A> <: IterableResultExtend<A> { 
    public func allResult<B, E>(transform: (A) -> Result<B, Array<E>>): Result<Array<B>, Array<E>> {
        helperAllResult(this, transform)
    }

    public func anyResult<B, E>(transform: (A) -> Result<B, Array<E>>): Result<B, Array<E>> {
        helperAnyResult(this, transform)
    }
}
