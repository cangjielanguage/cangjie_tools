// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
type ErrorCode = number;

enum EventType {}

enum ListenerStatusNumeric {
    on,
    off
}
 
enum ListenerStatusString {
    on = "ON",
    off = "OFF"
}

interface TestListener {
    "onStart"?: () => void;
    "onDestroy"?: () => void;
    onError?: (code: ErrorCode, msg: string) => void;
    onTouch?: () => void;
    onEvent?: (e: EventType) => void;
}

interface MyListener {
    on(key: string, cb: (r: Record<string, string>) => void);
    off(key: string, cb: (r: Record<string, string>, t:number) => void);
}
 
interface MyListener2 {
    on(key: string, cb: (r: ListenerStatusNumeric) => void);
}
 
interface MyListener3 {
    on(key: string, cb: (r: ListenerStatusString) => void);
}
 
class MyListener4 {
    static on(key: string, cb: (r: ListenerStatusString) => void);
}
 
interface MyListener5 {
    on(key: string, cb: Callback<number>)
}
 
export function on(key: string, cb: (r: Record<string, string>, option, t?:number) => void);
 
export function off(key: string, cb?: (r: Record<string, string>, t:number) => void);
