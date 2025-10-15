// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

declare enum Colors {
    Red = 'RED',
    Green = 'GREEN',
    Blue = 'BLUE'
  }
 
 
export function as1(callback:AsyncCallback<void>):void;
export function as2(callback:AsyncCallback<string>):void;
export function as3(callback:AsyncCallback<Colors>):void;
export function as4(callback:AsyncCallback<Array<number>>):void;
export function as5(callback:AsyncCallback<BigInt>):void;
export function as6(callback:AsyncCallback<Uint8Array>):void;
 
declare class ASC {
    constructor(
        param1:string,
        param2:{a:number,b:string},
        param3:number,
        funcParam:()=>string,
        callback?:AsyncCallback<string>
    );
    static AS1(callback:AsyncCallback<void>):void;
}
 
declare class ForParamToJs {
    constructor(
        param1:common.Context,
        param2:Colors,
        param3:BigInt,
        param4:Uint8Array,
        param5:number[],
        param6:Array<number>,
        callback:AsyncCallback<string>,
        paramEnum:Array<Colors>,
        param7?:Array<number>,
        param8?:Colors,
        param9?:Array<UInt8>
    )
}