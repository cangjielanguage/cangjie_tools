// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package com.cjprof.jni;

import com.cjprof.jni.model.*;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Cjprof JNI Demo
 *
 */
public class CjprofDemo {

    public static void main(String[] args) {
        try {
            System.out.println("=== initialize ===");
            Cjprof.initialize();
            System.out.println("JNI initialize success");
            System.out.println("\n=== parsing ===");
            List<String> heapFiles = Arrays.asList(
                "/root/demo1.data",
                "/root/demo2.data"
            );

            if (!heapFiles.isEmpty() && !heapFiles.get(0).isEmpty()) {
                boolean parseResult = Cjprof.parseHeapSnapshotFiles(heapFiles);
                System.out.println("parse result: " + parseResult);
            } else {
                System.out.println("skip parsing");
            }

            System.out.println("\n=== queryAllHeapSnapshot ===");
            List<HeapSnapshot> snapshots = Cjprof.queryAllHeapSnapshot();
            System.out.println("snapshots size: " + snapshots.size());
            for (HeapSnapshot snapshot : snapshots) {
                System.out.println("  " + snapshot);
            }

            if (!snapshots.isEmpty()) {
                long snapshotId = snapshots.get(0).getId();
                List<ConstructorNode> constructorNodes = Cjprof.getConstructorNodesBySnapshotID(snapshotId);
                System.out.println("constructorNodes size: " + constructorNodes.size());
                for (ConstructorNode c: constructorNodes) {
                    System.out.println(Cjprof.expandConstructorNode(snapshotId, c.getId(), 0, 10000));
                }

                Set<Byte> rootTypes = new HashSet<>(Arrays.asList((byte) 1, (byte) 2, (byte) 3));
                List<ConstructorNode> rootNodes = Cjprof.getRootNodesBySnapshotID(snapshotId, rootTypes);
                System.out.println("rootNodes size: " + rootNodes.size());

                List<ThreadInfo> threadInfos = Cjprof.getThreadInfos(snapshotId);
                System.out.println("threadInfos size: " + threadInfos.size());
                for (ThreadInfo info : threadInfos) {
                    System.out.println("  thread: " + info.getName() + " (id=" + info.getId() + ")");
                    System.out.println("    frames size: " + info.getFrames().size());
                }
            }

            if (snapshots.size() >= 2) {
                long baseId = snapshots.get(0).getId();
                long targetId = snapshots.get(1).getId();

                List<ConstructorDiffNode> diffNodes = Cjprof.querySnapshotComparison(baseId, targetId);
                System.out.println("diffNodes size: " + diffNodes.size());
                for (ConstructorDiffNode diffNode : diffNodes) {
                    if (diffNode.getClassName().equals("demo::AAA")) {
                        ConstructorDiffNode node = Cjprof.expandConstructorDiffNode(baseId, targetId, diffNode.getId(), 0, 100000);
                        System.out.println(node);
                    }
                }

                Set<Byte> rootTypes = new HashSet<>(Arrays.asList((byte) 1, (byte) 2, (byte) 3));
                List<ConstructorDiffNode> rootDiffNodes = Cjprof.getRootDiffNodesBySnapshotID(
                    baseId, targetId, rootTypes);
                System.out.println("rootDiffNodes size: " + rootDiffNodes.size());
            }

            System.out.println("\n=== Demo compeleted ===");

        } catch (CjprofException e) {
            System.err.println("CjprofException: " + e.getMessage());
            e.printStackTrace();
        } catch (Exception e) {
            System.err.println("Exception: " + e.getMessage());
            e.printStackTrace();
        }
    }

    private static void printConstructorNode(ConstructorNode node, int indent) {
        String prefix = "";
        for (int i = 0; i< indent; i++) {
            prefix += "  ";
        }
        System.out.println(prefix + "ConstructorNode{");
        System.out.println(prefix + "  id=" + node.getId() + ",");
        System.out.println(prefix + "  className='" + node.getClassName() + "',");
        System.out.println(prefix + "  totalSize=" + node.getTotalSize() + ",");
        System.out.println(prefix + "  instanceCount=" + node.getChildrenCount() + ",");
        System.out.println(prefix + "  distance=" + node.getDistance() + ",");
        System.out.println(prefix + "  shallowSize=" + node.getShallowSize() + ",");
        System.out.println(prefix + "  retainedSize=" + node.getRetainedSize() + ",");
        System.out.println(prefix + "}");
    }

    private static void printInstanceNode(InstanceNode node, int indent) {
        String prefix = "";
        for (int i = 0; i< indent; i++) {
            prefix += "  ";
        }
        System.out.println(prefix + "InstanceNode{");
        System.out.println(prefix + "  id=" + node.getId() + ",");
        System.out.println(prefix + "  className='" + node.getClassName() + "',");
        System.out.println(prefix + "  type='" + node.getType() + "',");
        System.out.println(prefix + "  shallowSize=" + node.getShallowSize() + ",");
        System.out.println(prefix + "  retainedSize=" + node.getRetainedSize() + ",");
        System.out.println(prefix + "  distance=" + node.getDistance() + ",");
        System.out.println(prefix + "}");
    }
}
