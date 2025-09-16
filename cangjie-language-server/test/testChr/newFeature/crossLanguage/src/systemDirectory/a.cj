// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package systemDirectory
struct _div_t{
  var quot:IntNative;
  var rem:IntNative;  
  _div_t(a:IntNative,b:IntNative){
    quot = a;
    rem = b;
  }
}
foreign func rand():IntNative
foreign func abs(a:IntNative):IntNative
foreign func atof(a:CString):Float64
foreign func div(a:IntNative,b:IntNative):_div_t
