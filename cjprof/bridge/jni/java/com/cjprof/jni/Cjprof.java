// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package com.cjprof.jni;

import com.cjprof.jni.model.*;
import java.util.List;
import java.util.Set;

public class Cjprof {
    static {
        System.loadLibrary("cjprof-jni");
    }

    public static native void initialize();

    public static native boolean parseHeapSnapshotFiles(List<String> filePaths);
    public static native void cleanHeapSnapshotFiles(List<Long> ids);
    public static native List<HeapSnapshot> queryAllHeapSnapshot();
    public static native long getSnapshotIDByFilePath(String filePath);

    public static native List<ConstructorNode> getConstructorNodesBySnapshotID(long id);

    public static native List<ConstructorNode> getRootNodesBySnapshotID(long id, Set<Byte> rootTypes);
    public static native List<ConstructorDiffNode> getRootDiffNodesBySnapshotID(long baseSnapshotId, long targetSnapshotId, Set<Byte> rootTypes);

    public static native ConstructorNode expandConstructorNode(long snapshotId, long nodeId, int startIndex, int length);
    public static native ConstructorDiffNode expandConstructorDiffNode(long baseSnapshotId, long targetSnapshotId, long nodeId, int startIndex, int length);
    public static native InstanceNode expandInstanceNode(long snapshotId, long nodeId, int startIndex, int length);
    public static native InstanceDiffNode expandInstanceDiffNode(long baseSnapshotId, long targetSnapshotId, long nodeId, int startIndex, int length);
    public static native InstanceNode expandDetailNode(long snapshotId, long nodeId, boolean isReference, int startIndex, int length);
    public static native InstanceDiffNode expandDetailDiffNode(long baseSnapshotId, long targetSnapshotId, long nodeId, boolean isReference, int startIndex, int length);

    public static native List<ConstructorDiffNode> querySnapshotComparison(long baseId, long targetId);

    public static native java.util.List<java.util.List<InstanceNode>> getNodeRootpaths(long snapshotId, long nodeId, int pathNum);

    public static native List<ThreadInfo> getThreadInfos(long snapshotId);

    public static native int querySnapshotCountOfResults(String keyword, boolean isIgnoreCase, long snapshotId);
    public static native ConstructorNode querySnapshotNodeByIndex(String keyword, boolean isIgnoreCase, long snapshotId, int length, int index);
    public static native int queryComparisonCountOfResults(String keyword, boolean isIgnoreCase, long baseId, long targetId);
    public static native ConstructorDiffNode queryComparisonNodeByIndex(String keyword, boolean isIgnoreCase, long baseId, long targetId, int length, int index);
}
