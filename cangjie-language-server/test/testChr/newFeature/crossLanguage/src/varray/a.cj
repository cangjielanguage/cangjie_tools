// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package varray

foreign func VArray1(a:VArray<VArray<VArray<CPointer<CPointer<CPointer<Int64>>>,$1>,$3>,$3>):Unit //"int64_t *(*)[][]"
foreign func VArray2(a:VArray<VArray<VArray<CPointer<VArray<VArray<CString,$3>,$3>>,$1>,$3>,$3>):Unit//"const char **(*)[][]"
foreign func VArray3(a:VArray<Int64,$1>):Unit //"int64_t (*)"
foreign func VArray4(a:VArray<CString,$1>):Unit//"const char *(*)"

foreign func CPointer1(a:CPointer<CPointer<VArray<CPointer<CPointer<CPointer<CString>>>,$3>>>,...):Unit//"const char ***"
foreign func CPointer2(a:CPointer<CPointer<CString>>,...):Unit// "const char ***" "..."
