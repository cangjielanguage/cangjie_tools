// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func Test_String_Single_Line(){
    let s1 = "" // empty string
    let s2: String = "Hello Cangjie Lang" // define string s2
    var s3 = "\"Hello Cangjie Lang\"" // define string s3 containing character "
    var s4: String = "Hello Cangjie Lang\n" // define string s4 containing character \n
	let s5 = "hello\" // illegal string with character \
	let s6 = "Hello "Cangjie Lang" // illegal string with character "
}

func Test_String_Completion(){
    var str = ""
    var res: ComparisionResult = str.compareTo("a")
}

func Test_String_Other_Feature(){
    var str = ""
    var str2 = "str2"
    var res = str == str2
}

func Test_String_Interpolation(){
    let obj = "apples"
    let count = 10
    let inter =  "there are${count * count}"
    //let interps1 = "There are ${count * count} $(obj)."
    // The result of "interps1" is: There are 100 apples.

    let interps1 = "There are ${count * 10} $(obj)."

    let d1 = "The $ sign."
    // The result of "d1" is: The $ sign.
    let d2 = "The \$(v)."
    // The result of "d2" is: The $(v).
}


func Test_String_Multi_Line(){
    let s1 = """
    """ // empty multi-line string
    let errorStr = """abc // Error: there must be a line terminator after the
    begining double quotations.
    """
    /*
    The result of s2 is:
    This
    is a multi-line string
    */
    let s2 = """
        This
    is a multi-line string"""
    /*
    The result of s3 is:
    This
    is a multi-line string
    */
    let s3 = """
        This
    is a multi-line string"""
    /*
    The result of s4 is:
    This
    is a
    multi-line string
    */
    let s4 = """
    This
        is a
    multi-line string
    """

    /*
    The result of s5 is:
    This is a
    multi-line string
    */
    let s5 = """
    This is a\n
    multi-line string
    """
}

func Test_String_Multi_Line_2(){
    let empty1 = #""# // empty multi-line raw string
    let empty2 = ##""## // empty multi-line raw string
    /*
    The result of s2 is:
    This
    is a multi-line string
    */
    let s2 = ##"
    This
    is a multi-line raw string"##
    /*
    The result of s3 is:
    This is a\n
    multi-line string
    */
    let s3 = #"This is a\n
    multi-line string"#
    /*
    The result of s4 is:
    This is a "#
    */
    let s4 = ##" This is a "#
    "##
    // Error: the begining and ending numbers of `#` are not matched.
    let errorStr1 = ##" error string "#
}


