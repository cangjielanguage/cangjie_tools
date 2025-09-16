// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package b.d

import b.b.*
import b.a.*
import b.c.*
/*测试中文*/public open class smallStudent <: student {
    public static var smallStudentname: String = "aaaa";
    public static func ssmallStudentage() {
        studentage()
    }
    public smallStudent() {}
}
/*测试中文*/public class test <: ArithmeticException{}
func test1(a: smallStudent) {
    //class
    var b: smallStudent = smallStudent()
    smallStudent.smallStudentname
    //enum
    var c: eat = eat.blue
    //struct
    var f: work = work()
    work.workage()
    work.workname
}
/*测试中文*/public interface testextend {}
// extend smallStudent <: testextend {}
/*测试中文*/extend work <: testextend {}
/*测试中文*/extend eat <: testextend {}
/*测试中文*/extend xzf <: testextend{}
/*测试中文*/public open class afda{}
