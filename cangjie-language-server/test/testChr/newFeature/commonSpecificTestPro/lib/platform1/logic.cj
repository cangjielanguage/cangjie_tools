// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

features {os.platform1}

package ohos_app_cangjie_entry

public specific func logicTest(): String {
    let nativeResult: CString = unsafe { FfiLogicTest() }
    let result = nativeResult.toString()
    unsafe { FfiFreeString(nativeResult) }
    result
}

foreign func FfiLogicTest(): CString
foreign func FfiFreeString(str: CString): Unit
