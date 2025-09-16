// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func semanticHighlight_test018(): unit 
{
    let score: int32 = 90
    let PASS:int32 = 60 
    let FULL:int32 = 100 
    var scoreResult: String = match score {
        case 0 => "zero"
        case 10 | 20 | 30 | 40 | 50 => "fail"
        case $PASS => "pass"
        case 70 | 80 => "good"
        case 90 | $FULL => "excellent"
        case _ => "not a valid score" 
    }
    
    return
}
