// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
// 字符串枚举
declare enum Colors {
  Red = 'RED',
  Green = 'GREEN',
  Blue = 'BLUE',
  Yellow = "'YELLOW'"
}

// 数字枚举
declare enum Status {
  Pending,    // 0
  Approved,   // 1
  Rejected,   // 2
}

// 常量枚举
// constants.d.ts
declare const enum Status1 {
  Pending = 3,
  Approved = 4,
  Rejected = 5
}

// 异构枚举
// response.d.ts
declare enum Response1 {
  No = 0,
  Yes = 'YES',
}

// @systemapi
declare enum SystemErrorCode{
  success = "0",
  invalidInput = "1",
  networkError = "2",
  internalError = "3"
}

// @deprecated
declare enum LegacyStatus{
  active = 0,
  inactive = 1,
  pending = 2
}