// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

public import ohos.business_exception.BusinessException
public import ohos.callback_invoke.allbackObject
public import kit.PerformanceAnalysisKit.Hilog

protected type AsyncCallback<T> = (?BusinessException, ?T) -> Unit
