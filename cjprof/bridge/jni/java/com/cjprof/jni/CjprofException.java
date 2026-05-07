// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package com.cjprof.jni;

public class CjprofException extends RuntimeException {
    public CjprofException(String message) {
        super(message);
    }

    public CjprofException(String message, Throwable cause) {
        super(message, cause);
    }
}
