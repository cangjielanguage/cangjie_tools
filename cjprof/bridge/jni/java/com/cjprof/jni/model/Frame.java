// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package com.cjprof.jni.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;

import java.util.ArrayList;
import java.util.List;

@AllArgsConstructor
@Builder
@Data
public class Frame {
    private String funcName;

    private String fileName;

    private int line;

    private List<InstanceNode> locals;

    private long id;

    public Frame() {
        this.locals = new ArrayList<>();
    }
}