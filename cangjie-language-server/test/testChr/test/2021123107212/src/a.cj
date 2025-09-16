// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created by code_project on 2021/12/31
 */
enum TimeUnit {
    | Year(Float32)
    | Month(Float32, Float32)
    | Day(Float32, Float32, Float32)
    | FZZZZr(Float32, Float32, Float32, Float32)
}

let oneYear = Year(1.0)
let score: Int64 = 90
let FZZSS = 60
var FZZULL = 100

var scoreResult: String = match (score) {
    case 0 => "zero"
    case _ => match (score) {
                  case 0 => "zero"
                  case _ => match (score) {
                                case 0 => "zero"
                                case _ => match (score) {
                                              case 0 => "zero"
                                              case _ => match (score) {
                                                            case 0 => "zero"
                                                            case $FZZ
                                                            case _ => "not a valid score"
                                                        }
                                          }
                            }
              }
}


