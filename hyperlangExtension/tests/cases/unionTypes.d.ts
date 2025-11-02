// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
export class UnionTypes {
  // Properties with union types
  t_null: number | null;
  null_t: null | string;
  t1_t2: boolean | string;
  t_undefined: number | undefined;
  undefined: undefined | string;
  t3: number | string | boolean;
  t4: number | string | boolean | null | undefined;
  t5: number | string | boolean | bigint| void;

  // Method returning a union type
  getValueOrError(): number | string;
  
  // Method with union type parameter
  setValue(value: string): number | boolean;
  
  // // Method with union return and union parameter
  // processData(input: boolean | number): string | number;
  
  // // Static method returning union type
  // static createOrNull(shouldCreate: boolean): UnionTypes | null;
}

declare function testUnion(s: string ): boolean | string;
declare function testUnion1(s: string | number): boolean | string;

// // Function returning union type
// export function parseInput(input: string): number | boolean | null;

// // Function with multiple union types
// export function transform(value: string | number, mode: "strict" | "loose"): boolean | string;
