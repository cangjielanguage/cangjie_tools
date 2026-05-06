// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package com.cjprof.jni.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.experimental.SuperBuilder;

import java.util.ArrayList;
import java.util.List;

@SuperBuilder
@Data
@AllArgsConstructor
public class ConstructorNode implements BaseNode {
    private String className;

    private int totalSize;

    private long id;

    private int nodeIndex;

    private int childrenCount;

    private int distance;

    private int shallowSize;

    private int retainedSize;

    private double shallowSizePercent;

    private double retainedSizePercent;

    private double totalInstanceCountPercent;

    private List<InstanceNode> children;

    private int startPosition;

    private int endPosition;

    public ConstructorNode() {
        this.children = new ArrayList<>();
    }
}
