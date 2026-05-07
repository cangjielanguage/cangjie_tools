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
public class InstanceNode implements BaseNode {
    private String className;

    private int distance;

    private int retainedSize;

    private int shallowSize;

    private double shallowSizePercent;

    private double retainedSizePercent;

    private int totalSize;

    private List<InstanceNode> children;

    private List<InstanceNode> retainerNodes;

    private long id;

    private int nodeIndex;

    private String type;

    private String rootType;

    private int childrenCount;

    private int retainerCount;

    private int startPosition;

    private int endPosition;

    private int arrayLength;

    public InstanceNode() {
        this.children = new ArrayList<>();
        this.retainerNodes = new ArrayList<>();
    }
}
