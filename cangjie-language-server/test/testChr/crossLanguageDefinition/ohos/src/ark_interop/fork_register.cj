// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos.ark_interop

public type ModuleRegister = (JSContext, JSObject) -> Unit

public type ClassRegister = (JSContext) -> JSClass

public type FuncRegister = (JSContext) -> JSFunction

public class JSModule {
    public static func registerModule(register: ModuleRegister): Unit {}

    public static func registerClass(name: String, register: ClassRegister): Unit {}

    public static func registerFunc(name: String, register: FuncRegister): Unit {}

    public static func registerFunc(name: String, lambda: JSLambda): Unit {}
}
