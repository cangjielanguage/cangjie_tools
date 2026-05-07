// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE__CJPROF_JNI_H
#define CANGJIE__CJPROF_JNI_H

#include <jni.h>
#include <string>
#include <vector>
#include <set>
#include "Cjprof.h"

#ifdef __cplusplus
extern "C" {
#endif

enum class CjprofError {
    SUCCESS = 0,
    FILE_NOT_FOUND = 1,
    INVALID_FORMAT = 2,
    OUT_OF_MEMORY = 3,
    INVALID_SNAPSHOT_ID = 4,
    INVALID_NODE_ID = 5,
    NO_NEW_NODE_INDEX = 6,
    UNKNOWN_ERROR = 99
};

JNIEXPORT void JNICALL Java_com_cjprof_jni_Cjprof_initialize(JNIEnv*, jclass);

JNIEXPORT jboolean JNICALL Java_com_cjprof_jni_Cjprof_parseHeapSnapshotFiles(JNIEnv*, jclass, jobject);
JNIEXPORT void JNICALL Java_com_cjprof_jni_Cjprof_cleanHeapSnapshotFiles(JNIEnv*, jclass, jobject);
JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_queryAllHeapSnapshot(JNIEnv*, jclass);
JNIEXPORT jlong JNICALL Java_com_cjprof_jni_Cjprof_getSnapshotIDByFilePath(JNIEnv*, jclass, jstring);

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_getConstructorNodesBySnapshotID(JNIEnv*, jclass, jlong);

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_getRootNodesBySnapshotID(JNIEnv*, jclass, jlong, jobject);
JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_getRootDiffNodesBySnapshotID(JNIEnv*, jclass, jlong, jlong, jobject);

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandConstructorNode(JNIEnv*, jclass, jlong, jlong, jint, jint);
JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandConstructorDiffNode(JNIEnv*, jclass, jlong, jlong, jlong, jint, jint);
JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandInstanceNode(JNIEnv*, jclass, jlong, jlong, jint, jint);
JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandInstanceDiffNode(JNIEnv*, jclass, jlong, jlong, jlong, jint, jint);
JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandDetailNode(JNIEnv*, jclass, jlong, jlong, jboolean, jint, jint);
JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandDetailDiffNode(JNIEnv*, jclass, jlong, jlong, jlong, jboolean, jint, jint);

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_querySnapshotComparison(JNIEnv*, jclass, jlong, jlong);

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_getNodeRootpaths(JNIEnv*, jclass, jlong, jlong, jint);

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_getThreadInfos(JNIEnv*, jclass, jlong);

JNIEXPORT jint JNICALL Java_com_cjprof_jni_Cjprof_querySnapshotCountOfResults(JNIEnv*, jclass, jstring, jboolean, jlong);
JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_querySnapshotNodeByIndex(JNIEnv*, jclass, jstring, jboolean, jlong, jint, jint);
JNIEXPORT jint JNICALL Java_com_cjprof_jni_Cjprof_queryComparisonCountOfResults(JNIEnv*, jclass, jstring, jboolean, jlong, jlong);
JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_queryComparisonNodeByIndex(JNIEnv*, jclass, jstring, jboolean, jlong, jlong, jint, jint);

#ifdef __cplusplus
}
#endif

#endif
