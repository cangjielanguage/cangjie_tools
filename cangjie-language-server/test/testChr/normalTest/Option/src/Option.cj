// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

let aaa:?String = "" //等价于 Option<String>
let v1: ?Int64 = 100 //等价于 Option<Int64>
let v2: ??Int64 = 100 //等价于 Option<Option<Int64>>

func testOptionRight() {
    let number: Int64 = match (v1) {
        case Some(num) => num // matched
        case None => 0
    }
    let number2: Int64 = match (v2) {
        case Some(num) => match (num) {
              case Some(num) => num // matched
              case None => 0
              } // matched
        case None => 0
    }
}
enum TimeUnit1 {
	 |Year|Month|Day|Hour
}
let opInt32_1 = Option<Int64>.Some(100) // The type of 'opInt32_1' is 'Option<Int32>'
let opInt32_2 = Option<Int32>.None // The type of 'opInt32_2' is 'Option<Int32>'
let opChar = Option<Rune>.Some('m') // The type of 'opChar' is 'Option<Rune>'
let opBool = Option<Bool>.None // The type of 'opBool' is 'Option<Bool>'
let opEnum = Option<TimeUnit1>.Some(TimeUnit1.Year) // The type of 'opEnum' is 'Option<TimeUnit1>'

//支持match解构
func test1(){
    let number1: Int64 = match (opInt32_1) {
        case Some(num) => num // matched
        case None => 0
    }
    let number2: Int32 = match (opInt32_2) {
        case Some(num) => num
        case None => 0 // matched
    }
    let enumValue: String = match (opEnum) {
    case Some(tu) => match (tu) {
        case Year => "Year" // matched
        case Month => "Month"
        case Day => "Day"
        case Hour => "Hour"
        }
    case None => "None"
    }
}
