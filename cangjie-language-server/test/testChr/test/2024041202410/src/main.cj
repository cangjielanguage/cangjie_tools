// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

public interface IData {}

public interface IQuery<D> where D <: IData {
    func getData(): D
}

public class MyData <: IData {
    public let a = 1
    public let b = 2
    public let sd: Option<SubData> = SubData()
}

public class SubData {
    public let d = 3
}

public class MyQuery <: IQuery<MyData> {
    public func getData(): MyData {
        return MyData()
    }
}

func wraper<D>(m: IQuery<D>, callback: (data: D) -> Unit) where D <: IData {
    // do something
}

func test() {
    wraper(MyQuery(), { data =>
            if (let Some(v) <- data.sd) {
                println("${v.}")
            }
            data.sd.getOrThrow().d
    })
}
