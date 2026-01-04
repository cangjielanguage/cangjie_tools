// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
// 可选属性
interface Product {
    price?: number; // 可选属性
}

//只读属性
interface Point {
  readonly x: number;
  readonly y: number;
}

// 函数类型
interface Callback1 {
    (data: string): void;
}

// 成员函数
// person.d.ts
interface Person {
    name: string;
    greet(): string;
}

// 索引签名
interface Dictionary {
    [key: string]: string;
}

// 函数重载
interface Calculator {
    add(x: number, y: number): number;
    add(x: string, y: string): string;
}

// 动态属性
interface Config {
    [key: string]: string | number;
}

// 嵌套对象
interface UserProfile {
    id: number;
    name: string;
    address: {
      city: string;
      zipCode: string;
    };
}

// 数组类型
// list.d.ts
interface List {
    items: string[];
}

interface ClockConstructor {
    new (hour: number, minute: number): string;
}

export interface IWBAPI {
    authorize(context: common.UIAbilityContext, listener: WbASListener ): void
    
    authorizeClient(context: Context, listener: WbASListener): void

    authorizeClient(context: BaseContext, listener: WbASListener): void

    authorizeClient(context: common.Context, listener: WbASListener): void
}

interface UnionInterface {
    foo(a: string|null): null | string;
    goo(b: undefined | string): string | undefined;    
}

export interface LoggerAdapter {
    setColorLevel(level: LogLevel): void;
    printDiagnoseLog(level: LogLevel, tag: string, msg: string, e: Error | null): void;
    getPubKey(): string;
    getLogPaths(): Array<string>;
}

export declare abstract class BasePlugin {
    pluginVersion: string;
    pluginId: Biz_Type;
    alias: string;
    alias_h: string;
    cfg: UMConfig;
    plugins: PluginContainer;
    constructor(d12: Biz_Type);
    getPluginId(): Biz_Type;
    install(b12: UMConfig, c12: PluginContainer): void;
    uninstall(): void;
    agree(): Promise<void>;
    write(x11: string, y11: Record<string, string | number>, z11: number, a12?: string): void;
    beforeEncode(w11: buffer.Buffer): Promise<buffer.Buffer>;
}

export interface PluginContainer {
    registryPlugin(plugin: BasePlugin, cfg: UMConfig): BasePlugin;
    getPluginById(pluginId: string): BasePlugin | undefined;
}

export declare class DefaultPluginContainer implements PluginContainer {
    readonly _pluginMap: BasePlugin[];
    constructor();
    registryPlugin(g12: BasePlugin, h12: UMConfig): BasePlugin;
    getPluginById(e12: Biz_Type): BasePlugin | undefined;
}