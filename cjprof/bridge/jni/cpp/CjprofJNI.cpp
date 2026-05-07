// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CjprofJNI.h"
#include <stdexcept>
#include <unordered_map>

// global cache: Java class & MethodID
static jclass g_listClass = nullptr;
static jmethodID g_listAddMethod = nullptr;
static jclass g_arrayListClass = nullptr;
static jmethodID g_arrayListConstructor = nullptr;

static jclass g_heapSnapshotClass = nullptr;
static jmethodID g_heapSnapshotConstructor = nullptr;

static jclass g_instanceNodeClass = nullptr;
static jmethodID g_instanceNodeConstructor = nullptr;

static jclass g_instanceDiffNodeClass = nullptr;
static jmethodID g_instanceDiffNodeConstructor = nullptr;

static jclass g_constructorNodeClass = nullptr;
static jmethodID g_constructorNodeConstructor = nullptr;

static jclass g_constructorDiffNodeClass = nullptr;
static jmethodID g_constructorDiffNodeConstructor = nullptr;

static jclass g_threadInfoClass = nullptr;
static jmethodID g_threadInfoConstructor = nullptr;

static jclass g_frameClass = nullptr;
static jmethodID g_frameConstructor = nullptr;

static jclass g_integerClass = nullptr;
static jmethodID g_integerConstructor = nullptr;

static jfieldID fid_constructorNode_className = nullptr;
static jfieldID fid_constructorNode_totalSize = nullptr;
static jfieldID fid_constructorNode_id = nullptr;
static jfieldID fid_constructorNode_nodeIndex = nullptr;
static jfieldID fid_constructorNode_childrenCount = nullptr;
static jfieldID fid_constructorNode_distance = nullptr;
static jfieldID fid_constructorNode_shallowSize = nullptr;
static jfieldID fid_constructorNode_retainedSize = nullptr;
static jfieldID fid_constructorNode_shallowSizePercent = nullptr;
static jfieldID fid_constructorNode_retainedSizePercent = nullptr;
static jfieldID fid_constructorNode_totalInstanceCountPercent = nullptr;
static jfieldID fid_constructorNode_startPosition = nullptr;
static jfieldID fid_constructorNode_endPosition = nullptr;
static jfieldID fid_constructorNode_children = nullptr;

static jfieldID fid_instanceNode_className = nullptr;
static jfieldID fid_instanceNode_distance = nullptr;
static jfieldID fid_instanceNode_retainedSize = nullptr;
static jfieldID fid_instanceNode_shallowSize = nullptr;
static jfieldID fid_instanceNode_shallowSizePercent = nullptr;
static jfieldID fid_instanceNode_retainedSizePercent = nullptr;
static jfieldID fid_instanceNode_totalSize = nullptr;
static jfieldID fid_instanceNode_id = nullptr;
static jfieldID fid_instanceNode_nodeIndex = nullptr;
static jfieldID fid_instanceNode_type = nullptr;
static jfieldID fid_instanceNode_rootType = nullptr;
static jfieldID fid_instanceNode_childrenCount = nullptr;
static jfieldID fid_instanceNode_retainerCount = nullptr;
static jfieldID fid_instanceNode_startPosition = nullptr;
static jfieldID fid_instanceNode_endPosition = nullptr;
static jfieldID fid_instanceNode_arrayLength = nullptr;
static jfieldID fid_instanceNode_children = nullptr;
static jfieldID fid_instanceNode_retainerNodes = nullptr;

static jfieldID fid_constructorDiffNode_addedCount = nullptr;
static jfieldID fid_constructorDiffNode_removedCount = nullptr;
static jfieldID fid_constructorDiffNode_countDelta = nullptr;
static jfieldID fid_constructorDiffNode_addedSize = nullptr;
static jfieldID fid_constructorDiffNode_removedSize = nullptr;
static jfieldID fid_constructorDiffNode_sizeDelta = nullptr;
static jfieldID fid_constructorDiffNode_baseTotalSize = nullptr;
static jfieldID fid_constructorDiffNode_targetTotalSize = nullptr;

static jfieldID fid_instanceDiffNode_addedCount = nullptr;
static jfieldID fid_instanceDiffNode_removedCount = nullptr;
static jfieldID fid_instanceDiffNode_countDelta = nullptr;
static jfieldID fid_instanceDiffNode_addedSize = nullptr;
static jfieldID fid_instanceDiffNode_removedSize = nullptr;
static jfieldID fid_instanceDiffNode_sizeDelta = nullptr;
static jfieldID fid_instanceDiffNode_added = nullptr;

static jfieldID fid_frame_funcName = nullptr;
static jfieldID fid_frame_fileName = nullptr;
static jfieldID fid_frame_line = nullptr;
static jfieldID fid_frame_id = nullptr;
static jfieldID fid_frame_locals = nullptr;

static jfieldID fid_threadInfo_name = nullptr;
static jfieldID fid_threadInfo_id = nullptr;
static jfieldID fid_threadInfo_frames = nullptr;

static std::unordered_map<std::string, jstring> g_string_cache;

static void clearStringCache(JNIEnv* env) {
    for (auto& pair : g_string_cache) {
        env->DeleteGlobalRef(pair.second);
    }
    g_string_cache.clear();
}

static jstring getCachedString(JNIEnv* env, const std::string& str) {
    auto it = g_string_cache.find(str);
    if (it != g_string_cache.end()) {
        return it->second;
    }
    jstring jstr = env->NewStringUTF(str.c_str());
    jstring globalStr = (jstring)env->NewGlobalRef(jstr);
    env->DeleteLocalRef(jstr);
    g_string_cache[str] = globalStr;
    return globalStr;
}

static void throwJavaException(JNIEnv* env, CjprofError error) {
    const char* message;
    switch (error) {
        case CjprofError::FILE_NOT_FOUND:
            message = "Heap snapshot file not found";
            break;
        case CjprofError::INVALID_FORMAT:
            message = "Invalid heap snapshot format";
            break;
        case CjprofError::OUT_OF_MEMORY:
            message = "Out of memory";
            break;
        case CjprofError::INVALID_SNAPSHOT_ID:
            message = "Invalid snapshot ID";
            break;
        case CjprofError::INVALID_NODE_ID:
            message = "Invalid node ID";
            break;
        case CjprofError::NO_NEW_NODE_INDEX:
            message = "Cannot allocate new node index";
            break;
        default:
            message = "Unknown error in Cjprof library";
            break;
    }
    env->ThrowNew(env->FindClass("com/cjprof/jni/CjprofException"), message);
}

static std::unordered_map<uint64_t, uint32_t> id2index;
static uint32_t g_next_handle = 1;
static uint32_t getIndexById(JNIEnv* env, uint64_t id) {
    auto it = id2index.find(id);
    if (it != id2index.end()) {
        return it->second;
    }
    if (g_next_handle >= 2147483647) {
        throwJavaException(env, CjprofError::NO_NEW_NODE_INDEX);
        return 0;
    }
    g_next_handle++;
    id2index[id] = g_next_handle;
    return g_next_handle;
}

// ============ Helper Functions ============

static jobject createHeapSnapshot(JNIEnv* env, const Cjprof::HeapSnapshot& snapshot) {
    jlong id = static_cast<jlong>(snapshot.id);
    jlong fileSize = static_cast<jlong>(snapshot.fileSize);
    jstring filePath = getCachedString(env, snapshot.filePath.c_str());
    jobject obj = env->NewObject(g_heapSnapshotClass, g_heapSnapshotConstructor, id, fileSize, filePath);
    return obj;
}

static jobject createInstanceNode(JNIEnv* env, const Cjprof::InstanceNode& node) {
    jobject obj = env->NewObject(g_instanceNodeClass, g_instanceNodeConstructor);
    if (obj == nullptr) return nullptr;
    jstring className = getCachedString(env, node.className.c_str());
    env->SetObjectField(obj, fid_instanceNode_className, className);
    jstring type = getCachedString(env, node.type.c_str());
    env->SetObjectField(obj, fid_instanceNode_type, type);
    jstring rootType = getCachedString(env, node.rootType.c_str());
    env->SetObjectField(obj, fid_instanceNode_rootType, rootType);
    env->SetIntField(obj, fid_instanceNode_distance, node.distance);
    env->SetIntField(obj, fid_instanceNode_retainedSize, node.retainedSize);
    env->SetIntField(obj, fid_instanceNode_shallowSize, node.shallowSize);
    env->SetDoubleField(obj, fid_instanceNode_shallowSizePercent, node.shallowSizePercent);
    env->SetDoubleField(obj, fid_instanceNode_retainedSizePercent, node.retainedSizePercent);
    env->SetIntField(obj, fid_instanceNode_totalSize, node.totalSize);
    env->SetLongField(obj, fid_instanceNode_id, node.id);
    env->SetIntField(obj, fid_instanceNode_nodeIndex, getIndexById(env, node.id));
    env->SetIntField(obj, fid_instanceNode_childrenCount, node.childrenCount);
    env->SetIntField(obj, fid_instanceNode_retainerCount, node.retainerCount);
    env->SetIntField(obj, fid_instanceNode_startPosition, node.startPosition);
    env->SetIntField(obj, fid_instanceNode_endPosition, node.endPosition);
    env->SetIntField(obj, fid_instanceNode_arrayLength, node.arrayLength);
    jobject childrenList = env->NewObject(g_arrayListClass, g_arrayListConstructor, (jint)node.children.size());
    if (childrenList == nullptr) {
        env->DeleteLocalRef(obj);
        return nullptr;
    }
    for (const auto& child : node.children) {
        jobject childObj = createInstanceNode(env, child);
        env->CallBooleanMethod(childrenList, g_listAddMethod, childObj);
        env->DeleteLocalRef(childObj);
    }
    env->SetObjectField(obj, fid_instanceNode_children, childrenList);
    env->DeleteLocalRef(childrenList);
    jobject retainerList = env->NewObject(g_arrayListClass, g_arrayListConstructor, (jint)node.retainerNodes.size());
    if (retainerList == nullptr) {
        env->DeleteLocalRef(obj);
        return nullptr;
    }
    for (const auto& retainer : node.retainerNodes) {
        jobject retainerObj = createInstanceNode(env, retainer);
        env->CallBooleanMethod(retainerList, g_listAddMethod, retainerObj);
        env->DeleteLocalRef(retainerObj);
    }
    env->SetObjectField(obj, fid_instanceNode_retainerNodes, retainerList);
    env->DeleteLocalRef(retainerList);
    return obj;
}

static jobject createInstanceDiffNode(JNIEnv* env, const Cjprof::InstanceDiffNode& node) {
    jobject obj = env->NewObject(g_instanceDiffNodeClass, g_instanceDiffNodeConstructor);
    if (obj == nullptr) return nullptr;

    jstring className = getCachedString(env, node.className.c_str());
    env->SetObjectField(obj, fid_instanceNode_className, className);

    jstring type = getCachedString(env, node.type.c_str());
    env->SetObjectField(obj, fid_instanceNode_type, type);

    jstring rootType = getCachedString(env, node.rootType.c_str());
    env->SetObjectField(obj, fid_instanceNode_rootType, rootType);

    env->SetIntField(obj, fid_instanceNode_distance, node.distance);
    env->SetIntField(obj, fid_instanceNode_retainedSize, node.retainedSize);
    env->SetIntField(obj, fid_instanceNode_shallowSize, node.shallowSize);
    env->SetDoubleField(obj, fid_instanceNode_shallowSizePercent, node.shallowSizePercent);
    env->SetDoubleField(obj, fid_instanceNode_retainedSizePercent, node.retainedSizePercent);
    env->SetIntField(obj, fid_instanceNode_totalSize, node.totalSize);
    env->SetLongField(obj, fid_instanceNode_id, node.id);
    env->SetIntField(obj, fid_instanceNode_nodeIndex, getIndexById(env, node.id));
    env->SetIntField(obj, fid_instanceNode_childrenCount, node.childrenCount);
    env->SetIntField(obj, fid_instanceNode_retainerCount, node.retainerCount);
    env->SetIntField(obj, fid_instanceNode_startPosition, node.startPosition);
    env->SetIntField(obj, fid_instanceNode_endPosition, node.endPosition);
    env->SetIntField(obj, fid_instanceNode_arrayLength, node.arrayLength);

    jobject childrenList = env->NewObject(g_arrayListClass, g_arrayListConstructor, (jint)node.children.size());
    if (childrenList == nullptr) {
        env->DeleteLocalRef(obj);
        return nullptr;
    }
    for (const auto& child : node.children) {
        jobject childObj = createInstanceNode(env, child);
        env->CallBooleanMethod(childrenList, g_listAddMethod, childObj);
        env->DeleteLocalRef(childObj);
    }
    env->SetObjectField(obj, fid_instanceNode_children, childrenList);
    env->DeleteLocalRef(childrenList);

    jobject retainerList = env->NewObject(g_arrayListClass, g_arrayListConstructor, (jint)node.retainerNodes.size());
    if (retainerList == nullptr) {
        env->DeleteLocalRef(obj);
        return nullptr;
    }
    for (const auto& retainer : node.retainerNodes) {
        jobject retainerObj = createInstanceNode(env, retainer);
        env->CallBooleanMethod(retainerList, g_listAddMethod, retainerObj);
        env->DeleteLocalRef(retainerObj);
    }
    env->SetObjectField(obj, fid_instanceNode_retainerNodes, retainerList);
    env->DeleteLocalRef(retainerList);

    env->SetIntField(obj, fid_instanceDiffNode_addedCount, node.addedCount);
    env->SetIntField(obj, fid_instanceDiffNode_removedCount, node.removedCount);
    env->SetLongField(obj, fid_instanceDiffNode_countDelta, node.countDelta);
    env->SetIntField(obj, fid_instanceDiffNode_addedSize, node.addedSize);
    env->SetIntField(obj, fid_instanceDiffNode_removedSize, node.removedSize);
    env->SetLongField(obj, fid_instanceDiffNode_sizeDelta, node.sizeDelta);
    env->SetBooleanField(obj, fid_instanceDiffNode_added, node.added);

    return obj;
}

static jobject createConstructorNode(JNIEnv* env, const Cjprof::ConstructorNode& node) {
    jobject obj = env->NewObject(g_constructorNodeClass, g_constructorNodeConstructor);
    if (obj == nullptr) return nullptr;
    jstring className = getCachedString(env, node.className.c_str());
    env->SetObjectField(obj, fid_constructorNode_className, className);
    env->SetIntField(obj, fid_constructorNode_totalSize, node.totalSize);
    env->SetLongField(obj, fid_constructorNode_id, node.id);
    env->SetIntField(obj, fid_constructorNode_nodeIndex, getIndexById(env, node.id));
    env->SetIntField(obj, fid_constructorNode_childrenCount, node.childrenCount);
    env->SetIntField(obj, fid_constructorNode_distance, node.distance);
    env->SetIntField(obj, fid_constructorNode_shallowSize, node.shallowSize);
    env->SetIntField(obj, fid_constructorNode_retainedSize, node.retainedSize);
    env->SetDoubleField(obj, fid_constructorNode_shallowSizePercent, node.shallowSizePercent);
    env->SetDoubleField(obj, fid_constructorNode_retainedSizePercent, node.retainedSizePercent);
    env->SetDoubleField(obj, fid_constructorNode_totalInstanceCountPercent, node.totalInstanceCountPercent);
    env->SetIntField(obj, fid_constructorNode_startPosition, node.startPosition);
    env->SetIntField(obj, fid_constructorNode_endPosition, node.endPosition);

    jobject childrenList = env->NewObject(g_arrayListClass, g_arrayListConstructor, (jint)node.children.size());
    if (childrenList == nullptr) {
        env->DeleteLocalRef(obj);
        return nullptr;
    }
    for (const auto& child : node.children) {
        jobject childObj = createInstanceNode(env, child);
        env->CallBooleanMethod(childrenList, g_listAddMethod, childObj);
        env->DeleteLocalRef(childObj);
    }
    env->SetObjectField(obj, fid_constructorNode_children, childrenList);
    env->DeleteLocalRef(childrenList);
    return obj;
}

static jobject createConstructorDiffNode(JNIEnv* env, const Cjprof::ConstructorDiffNode& node) {
    jobject obj = env->NewObject(g_constructorDiffNodeClass, g_constructorDiffNodeConstructor);
    if (obj == nullptr) return nullptr;

    jstring className = getCachedString(env, node.className.c_str());
    env->SetObjectField(obj, fid_constructorNode_className, className);
    env->SetIntField(obj, fid_constructorNode_totalSize, node.totalSize);
    env->SetLongField(obj, fid_constructorNode_id, node.id);
    env->SetIntField(obj, fid_constructorNode_nodeIndex, getIndexById(env, node.id));
    env->SetIntField(obj, fid_constructorNode_childrenCount, node.childrenCount);
    env->SetIntField(obj, fid_constructorNode_distance, node.distance);
    env->SetIntField(obj, fid_constructorNode_shallowSize, node.shallowSize);
    env->SetIntField(obj, fid_constructorNode_retainedSize, node.retainedSize);
    env->SetDoubleField(obj, fid_constructorNode_shallowSizePercent, node.shallowSizePercent);
    env->SetDoubleField(obj, fid_constructorNode_retainedSizePercent, node.retainedSizePercent);
    env->SetDoubleField(obj, fid_constructorNode_totalInstanceCountPercent, node.totalInstanceCountPercent);
    env->SetIntField(obj, fid_constructorNode_startPosition, node.startPosition);
    env->SetIntField(obj, fid_constructorNode_endPosition, node.endPosition);

    env->SetIntField(obj, fid_constructorDiffNode_addedCount, node.addedCount);
    env->SetIntField(obj, fid_constructorDiffNode_removedCount, node.removedCount);
    env->SetLongField(obj, fid_constructorDiffNode_countDelta, node.countDelta);
    env->SetIntField(obj, fid_constructorDiffNode_addedSize, node.addedSize);
    env->SetIntField(obj, fid_constructorDiffNode_removedSize, node.removedSize);
    env->SetLongField(obj, fid_constructorDiffNode_sizeDelta, node.sizeDelta);
    env->SetIntField(obj, fid_constructorDiffNode_baseTotalSize, node.baseTotalSize);
    env->SetIntField(obj, fid_constructorDiffNode_targetTotalSize, node.targetTotalSize);

    jobject childrenList = env->NewObject(g_arrayListClass, g_arrayListConstructor, (jint)node.children.size());
    if (childrenList == nullptr) {
        env->DeleteLocalRef(obj);
        return nullptr;
    }
    for (const auto& child : node.children) {
        jobject childObj = createInstanceNode(env, child);
        env->CallBooleanMethod(childrenList, g_listAddMethod, childObj);
        env->DeleteLocalRef(childObj);
    }
    env->SetObjectField(obj, fid_constructorNode_children, childrenList);
    env->DeleteLocalRef(childrenList);
    return obj;
}

static jobject createFrame(JNIEnv* env, const Cjprof::ThreadInfo::Frame& frame) {
    jobject obj = env->NewObject(g_frameClass, g_frameConstructor);
    if (obj == nullptr) return nullptr;

    jstring funcName = getCachedString(env, frame.funcName.c_str());
    env->SetObjectField(obj, fid_frame_funcName, funcName);

    jstring fileName = getCachedString(env, frame.fileName.c_str());
    env->SetObjectField(obj, fid_frame_fileName, fileName);

    env->SetIntField(obj, fid_frame_line, frame.line);
    env->SetLongField(obj, fid_frame_id, frame.id);
    jobject localsList = env->NewObject(g_arrayListClass, g_arrayListConstructor, (jint)frame.locals.size());
    if (localsList == nullptr) {
        env->DeleteLocalRef(obj);
        return nullptr;
    }
    for (const auto& local : frame.locals) {
        jobject localObj = createInstanceNode(env, local);
        env->CallBooleanMethod(localsList, g_listAddMethod, localObj);
        env->DeleteLocalRef(localObj);
    }

    env->SetObjectField(obj, fid_frame_locals, localsList);
    env->DeleteLocalRef(localsList);

    return obj;
}

static jobject createThreadInfo(JNIEnv* env, const Cjprof::ThreadInfo& info) {
    jobject obj = env->NewObject(g_threadInfoClass, g_threadInfoConstructor);
    if (obj == nullptr) return nullptr;

    jstring name = getCachedString(env, info.name.c_str());
    env->SetObjectField(obj, fid_threadInfo_name, name);
    env->SetLongField(obj, fid_threadInfo_id, info.id);
    jobject framesList = env->NewObject(g_arrayListClass, g_arrayListConstructor, (jint)info.frames.size());
    if (framesList == nullptr) {
        env->DeleteLocalRef(obj);
        return nullptr;
    }
    for (const auto& frame : info.frames) {
        jobject frameObj = createFrame(env, frame);
        env->CallBooleanMethod(framesList, g_listAddMethod, frameObj);
        env->DeleteLocalRef(frameObj);
    }

    env->SetObjectField(obj, fid_threadInfo_frames, framesList);
    env->DeleteLocalRef(framesList);
    return obj;
}

// ============ Helper: Java List to C++ Vector ============

static std::vector<std::string> javaListToStringVector(JNIEnv* env, jobject javaList) {
    std::vector<std::string> result;
    jclass listClass = env->GetObjectClass(javaList);
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jint size = env->CallIntMethod(javaList, sizeMethod);
    for (jint i = 0; i < size; i++) {
        jstring jstr = (jstring) env->CallObjectMethod(javaList, getMethod, i);
        const char* str = env->GetStringUTFChars(jstr, nullptr);
        result.push_back(str);
        env->ReleaseStringUTFChars(jstr, str);
        env->DeleteLocalRef(jstr);
    }
    return result;
}

static std::vector<uint64_t> javaListToUInt64Vector(JNIEnv* env, jobject javaList) {
    std::vector<uint64_t> result;
    jclass listClass = env->GetObjectClass(javaList);
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jclass longClass = env->FindClass("java/lang/Long");
    jmethodID longValueMethod = env->GetMethodID(longClass, "longValue", "()J");
    jint size = env->CallIntMethod(javaList, sizeMethod);
    for (jint i = 0; i < size; i++) {
        jobject jLongObj = env->CallObjectMethod(javaList, getMethod, i);
        jlong value = env->CallLongMethod(jLongObj, longValueMethod);
        result.push_back(static_cast<uint64_t>(value));
        env->DeleteLocalRef(jLongObj);
    }
    return result;
}

static std::set<uint8_t> javaSetToUInt8Set(JNIEnv* env, jobject javaSet) {
    std::set<uint8_t> result;
    jclass setClass = env->GetObjectClass(javaSet);
    jmethodID iteratorMethod = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
    jmethodID hasNextMethod = env->GetMethodID(env->FindClass("java/util/Iterator"), "hasNext", "()Z");
    jmethodID nextMethod = env->GetMethodID(env->FindClass("java/util/Iterator"), "next", "()Ljava/lang/Object;");
    jobject iterator = env->CallObjectMethod(javaSet, iteratorMethod);
    while (env->CallBooleanMethod(iterator, hasNextMethod)) {
        jobject element = env->CallObjectMethod(iterator, nextMethod);
        jclass byteClass = env->FindClass("java/lang/Byte");
        jmethodID byteValueMethod = env->GetMethodID(byteClass, "byteValue", "()B");
        jbyte value = env->CallByteMethod(element, byteValueMethod);
        result.insert(static_cast<uint8_t>(value));
        env->DeleteLocalRef(element);
    }
    env->DeleteLocalRef(iterator);
    return result;
}

static void initConstructorNodeFields(JNIEnv* env) {
    fid_constructorNode_className = env->GetFieldID(g_constructorNodeClass, "className", "Ljava/lang/String;");
    fid_constructorNode_totalSize = env->GetFieldID(g_constructorNodeClass, "totalSize", "I");
    fid_constructorNode_id = env->GetFieldID(g_constructorNodeClass, "id", "J");
    fid_constructorNode_nodeIndex = env->GetFieldID(g_constructorNodeClass, "nodeIndex", "I");
    fid_constructorNode_childrenCount = env->GetFieldID(g_constructorNodeClass, "childrenCount", "I");
    fid_constructorNode_distance = env->GetFieldID(g_constructorNodeClass, "distance", "I");
    fid_constructorNode_shallowSize = env->GetFieldID(g_constructorNodeClass, "shallowSize", "I");
    fid_constructorNode_retainedSize = env->GetFieldID(g_constructorNodeClass, "retainedSize", "I");
    fid_constructorNode_shallowSizePercent = env->GetFieldID(g_constructorNodeClass, "shallowSizePercent", "D");
    fid_constructorNode_retainedSizePercent = env->GetFieldID(g_constructorNodeClass, "retainedSizePercent", "D");
    fid_constructorNode_totalInstanceCountPercent = env->GetFieldID(g_constructorNodeClass, "totalInstanceCountPercent", "D");
    fid_constructorNode_startPosition = env->GetFieldID(g_constructorNodeClass, "startPosition", "I");
    fid_constructorNode_endPosition = env->GetFieldID(g_constructorNodeClass, "endPosition", "I");
    fid_constructorNode_children = env->GetFieldID(g_constructorNodeClass, "children", "Ljava/util/List;");
}

static void initInstanceNodeFields(JNIEnv* env) {
    fid_instanceNode_className = env->GetFieldID(g_instanceNodeClass, "className", "Ljava/lang/String;");
    fid_instanceNode_distance = env->GetFieldID(g_instanceNodeClass, "distance", "I");
    fid_instanceNode_retainedSize = env->GetFieldID(g_instanceNodeClass, "retainedSize", "I");
    fid_instanceNode_shallowSize = env->GetFieldID(g_instanceNodeClass, "shallowSize", "I");
    fid_instanceNode_shallowSizePercent = env->GetFieldID(g_instanceNodeClass, "shallowSizePercent", "D");
    fid_instanceNode_retainedSizePercent = env->GetFieldID(g_instanceNodeClass, "retainedSizePercent", "D");
    fid_instanceNode_totalSize = env->GetFieldID(g_instanceNodeClass, "totalSize", "I");
    fid_instanceNode_id = env->GetFieldID(g_instanceNodeClass, "id", "J");
    fid_instanceNode_nodeIndex = env->GetFieldID(g_instanceNodeClass, "nodeIndex", "I");
    fid_instanceNode_type = env->GetFieldID(g_instanceNodeClass, "type", "Ljava/lang/String;");
    fid_instanceNode_rootType = env->GetFieldID(g_instanceNodeClass, "rootType", "Ljava/lang/String;");
    fid_instanceNode_childrenCount = env->GetFieldID(g_instanceNodeClass, "childrenCount", "I");
    fid_instanceNode_retainerCount = env->GetFieldID(g_instanceNodeClass, "retainerCount", "I");
    fid_instanceNode_startPosition = env->GetFieldID(g_instanceNodeClass, "startPosition", "I");
    fid_instanceNode_endPosition = env->GetFieldID(g_instanceNodeClass, "endPosition", "I");
    fid_instanceNode_arrayLength = env->GetFieldID(g_instanceNodeClass, "arrayLength", "I");
    fid_instanceNode_children = env->GetFieldID(g_instanceNodeClass, "children", "Ljava/util/List;");
    fid_instanceNode_retainerNodes = env->GetFieldID(g_instanceNodeClass, "retainerNodes", "Ljava/util/List;");
}

static void initConstructorDiffNodeFields(JNIEnv* env) {
    fid_constructorDiffNode_addedCount = env->GetFieldID(g_constructorDiffNodeClass, "addedCount", "I");
    fid_constructorDiffNode_removedCount = env->GetFieldID(g_constructorDiffNodeClass, "removedCount", "I");
    fid_constructorDiffNode_countDelta = env->GetFieldID(g_constructorDiffNodeClass, "countDelta", "J");
    fid_constructorDiffNode_addedSize = env->GetFieldID(g_constructorDiffNodeClass, "addedSize", "I");
    fid_constructorDiffNode_removedSize = env->GetFieldID(g_constructorDiffNodeClass, "removedSize", "I");
    fid_constructorDiffNode_sizeDelta = env->GetFieldID(g_constructorDiffNodeClass, "sizeDelta", "J");
    fid_constructorDiffNode_baseTotalSize = env->GetFieldID(g_constructorDiffNodeClass, "baseTotalSize", "I");
    fid_constructorDiffNode_targetTotalSize = env->GetFieldID(g_constructorDiffNodeClass, "targetTotalSize", "I");
}

static void initInstanceDiffNodeFields(JNIEnv* env) {
    fid_instanceDiffNode_addedCount = env->GetFieldID(g_instanceDiffNodeClass, "addedCount", "I");
    fid_instanceDiffNode_removedCount = env->GetFieldID(g_instanceDiffNodeClass, "removedCount", "I");
    fid_instanceDiffNode_countDelta = env->GetFieldID(g_instanceDiffNodeClass, "countDelta", "J");
    fid_instanceDiffNode_addedSize = env->GetFieldID(g_instanceDiffNodeClass, "addedSize", "I");
    fid_instanceDiffNode_removedSize = env->GetFieldID(g_instanceDiffNodeClass, "removedSize", "I");
    fid_instanceDiffNode_sizeDelta = env->GetFieldID(g_instanceDiffNodeClass, "sizeDelta", "J");
    fid_instanceDiffNode_added = env->GetFieldID(g_instanceDiffNodeClass, "added", "I");
}

static void initFrameFields(JNIEnv* env) {
    fid_frame_funcName = env->GetFieldID(g_frameClass, "funcName", "Ljava/lang/String;");
    fid_frame_fileName = env->GetFieldID(g_frameClass, "fileName", "Ljava/lang/String;");
    fid_frame_line = env->GetFieldID(g_frameClass, "line", "I");
    fid_frame_id = env->GetFieldID(g_frameClass, "id", "J");
    fid_frame_locals = env->GetFieldID(g_frameClass, "locals", "Ljava/util/List;");
}

static void initThreadInfoFields(JNIEnv* env) {
    fid_threadInfo_name = env->GetFieldID(g_threadInfoClass, "name", "Ljava/lang/String;");
    fid_threadInfo_id = env->GetFieldID(g_threadInfoClass, "id", "J");
    fid_threadInfo_frames = env->GetFieldID(g_threadInfoClass, "frames", "Ljava/util/List;");
}

// ============ JNI Functions ============

JNIEXPORT void JNICALL Java_com_cjprof_jni_Cjprof_initialize(JNIEnv* env, jclass) {
    g_listClass = (jclass) env->NewGlobalRef(env->FindClass("java/util/List"));
    g_listAddMethod = env->GetMethodID(g_listClass, "add", "(Ljava/lang/Object;)Z");

    g_arrayListClass = (jclass) env->NewGlobalRef(env->FindClass("java/util/ArrayList"));
    g_arrayListConstructor = env->GetMethodID(g_arrayListClass, "<init>", "()V");

    g_heapSnapshotClass = (jclass) env->NewGlobalRef(env->FindClass("com/cjprof/jni/model/HeapSnapshot"));
    g_heapSnapshotConstructor = env->GetMethodID(g_heapSnapshotClass, "<init>", "(JJLjava/lang/String;)V");

    g_instanceNodeClass = (jclass) env->NewGlobalRef(env->FindClass("com/cjprof/jni/model/InstanceNode"));
    g_instanceNodeConstructor = env->GetMethodID(g_instanceNodeClass, "<init>", "()V");

    g_instanceDiffNodeClass = (jclass) env->NewGlobalRef(env->FindClass("com/cjprof/jni/model/InstanceDiffNode"));
    g_instanceDiffNodeConstructor = env->GetMethodID(g_instanceDiffNodeClass, "<init>", "()V");

    g_constructorNodeClass = (jclass) env->NewGlobalRef(env->FindClass("com/cjprof/jni/model/ConstructorNode"));
    g_constructorNodeConstructor = env->GetMethodID(g_constructorNodeClass, "<init>", "()V");

    g_constructorDiffNodeClass = (jclass) env->NewGlobalRef(env->FindClass("com/cjprof/jni/model/ConstructorDiffNode"));
    g_constructorDiffNodeConstructor = env->GetMethodID(g_constructorDiffNodeClass, "<init>", "()V");

    g_threadInfoClass = (jclass) env->NewGlobalRef(env->FindClass("com/cjprof/jni/model/ThreadInfo"));
    g_threadInfoConstructor = env->GetMethodID(g_threadInfoClass, "<init>", "()V");

    g_frameClass = (jclass) env->NewGlobalRef(env->FindClass("com/cjprof/jni/model/Frame"));
    g_frameConstructor = env->GetMethodID(g_frameClass, "<init>", "()V");

    g_integerClass = (jclass) env->NewGlobalRef(env->FindClass("java/lang/Integer"));
    g_integerConstructor = env->GetMethodID(g_integerClass, "<init>", "(I)V");

    initConstructorNodeFields(env);
    initInstanceNodeFields(env);
    initConstructorDiffNodeFields(env);
    initInstanceDiffNodeFields(env);
    initFrameFields(env);
    initThreadInfoFields(env);
}

JNIEXPORT jboolean JNICALL Java_com_cjprof_jni_Cjprof_parseHeapSnapshotFiles(JNIEnv* env, jclass, jobject filePathsList) {
    std::vector<std::string> filePaths = javaListToStringVector(env, filePathsList);
    return Cjprof::ParseHeapSnapshotFiles(filePaths) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL Java_com_cjprof_jni_Cjprof_cleanHeapSnapshotFiles(JNIEnv* env, jclass, jobject idsList) {
    id2index.clear();
    g_next_handle = 1;
    clearStringCache(env);
    std::vector<uint64_t> ids = javaListToUInt64Vector(env, idsList);
    Cjprof::CleanHeapSnapshotFiles(ids);
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_queryAllHeapSnapshot(JNIEnv* env, jclass) {
    std::vector<Cjprof::HeapSnapshot> snapshots = Cjprof::QueryAllHeapSnapshot();
    jobject result = env->NewObject(g_arrayListClass, g_arrayListConstructor);
    if (result == nullptr) return nullptr;
    for (const auto& snapshot : snapshots) {
        jobject obj = createHeapSnapshot(env, snapshot);
        env->CallBooleanMethod(result, g_listAddMethod, obj);
        env->DeleteLocalRef(obj);
    }
    return result;
}

JNIEXPORT jlong JNICALL Java_com_cjprof_jni_Cjprof_getSnapshotIDByFilePath(JNIEnv* env, jclass, jstring jFilePath) {
    const char* filePath = env->GetStringUTFChars(jFilePath, nullptr);
    uint64_t id = Cjprof::GetSnapshotIDByFilePath(std::string(filePath));
    env->ReleaseStringUTFChars(jFilePath, filePath);
    return static_cast<jlong>(id);
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_getConstructorNodesBySnapshotID(JNIEnv* env, jclass, jlong id) {
    std::vector<Cjprof::ConstructorNode> nodes = Cjprof::GetConstructorNodesBySnapshotID(static_cast<uint64_t>(id));
    jobject result = env->NewObject(g_arrayListClass, g_arrayListConstructor);
    if (result == nullptr) return nullptr;
    for (const auto& node : nodes) {
        jobject obj = createConstructorNode(env, node);
        env->CallBooleanMethod(result, g_listAddMethod, obj);
        env->DeleteLocalRef(obj);
    }
    return result;
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_getRootNodesBySnapshotID(JNIEnv* env, jclass, jlong id, jobject rootTypesSet) {
    std::set<uint8_t> rootTypes = javaSetToUInt8Set(env, rootTypesSet);
    std::vector<Cjprof::ConstructorNode> nodes = Cjprof::GetRootNodesBySnapshotID(static_cast<uint64_t>(id), rootTypes);
    jobject result = env->NewObject(g_arrayListClass, g_arrayListConstructor, (jint)nodes.size());
    if (result == nullptr) return nullptr;
    for (const auto& node : nodes) {
        jobject obj = createConstructorNode(env, node);
        env->CallBooleanMethod(result, g_listAddMethod, obj);
        env->DeleteLocalRef(obj);
    }
    return result;
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_getRootDiffNodesBySnapshotID(JNIEnv* env, jclass, jlong baseId, jlong targetId, jobject rootTypesSet) {
    std::set<uint8_t> rootTypes = javaSetToUInt8Set(env, rootTypesSet);
    std::vector<Cjprof::ConstructorDiffNode> nodes = Cjprof::GetRootDiffNodesBySnapshotID(
        static_cast<uint64_t>(baseId), static_cast<uint64_t>(targetId), rootTypes);
    jobject result = env->NewObject(g_arrayListClass, g_arrayListConstructor);
    if (result == nullptr) return nullptr;
    for (const auto& node : nodes) {
        jobject obj = createConstructorDiffNode(env, node);
        env->CallBooleanMethod(result, g_listAddMethod, obj);
        env->DeleteLocalRef(obj);
    }
    return result;
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandConstructorNode(JNIEnv* env, jclass, jlong snapshotId, jlong nodeId, jint startIndex, jint length) {
    Cjprof::ConstructorNode node = Cjprof::ExpandConstructorNode(
        static_cast<uint64_t>(snapshotId), static_cast<uint64_t>(nodeId),
        static_cast<uint32_t>(startIndex), static_cast<uint32_t>(length));
    return createConstructorNode(env, node);
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandConstructorDiffNode(JNIEnv* env, jclass, jlong baseSnapshotId, jlong targetSnapshotId, jlong nodeId, jint startIndex, jint length) {
    Cjprof::ConstructorDiffNode node = Cjprof::ExpandConstructorDiffNode(
        static_cast<uint64_t>(baseSnapshotId), static_cast<uint64_t>(targetSnapshotId),
        static_cast<uint64_t>(nodeId), static_cast<uint32_t>(startIndex), static_cast<uint32_t>(length));
    return createConstructorDiffNode(env, node);
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandInstanceNode(JNIEnv* env, jclass, jlong snapshotId, jlong nodeId, jint startIndex, jint length) {
    Cjprof::InstanceNode node = Cjprof::ExpandInstanceNode(
        static_cast<uint64_t>(snapshotId), static_cast<uint64_t>(nodeId),
        static_cast<uint32_t>(startIndex), static_cast<uint32_t>(length));
    return createInstanceNode(env, node);
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandInstanceDiffNode(JNIEnv* env, jclass, jlong baseSnapshotId, jlong targetSnapshotId, jlong nodeId, jint startIndex, jint length) {
    Cjprof::InstanceDiffNode node = Cjprof::ExpandInstanceDiffNode(
        static_cast<uint64_t>(baseSnapshotId), static_cast<uint64_t>(targetSnapshotId),
        static_cast<uint64_t>(nodeId), static_cast<uint32_t>(startIndex), static_cast<uint32_t>(length));
    return createInstanceDiffNode(env, node);
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandDetailNode(JNIEnv* env, jclass, jlong snapshotId, jlong nodeId, jboolean isReference, jint startIndex, jint length) {
    Cjprof::InstanceNode node = Cjprof::ExpandDetailNode(
        static_cast<uint64_t>(snapshotId), static_cast<uint64_t>(nodeId),
        static_cast<bool>(isReference), static_cast<uint32_t>(startIndex), static_cast<uint32_t>(length));
    return createInstanceNode(env, node);
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_expandDetailDiffNode(JNIEnv* env, jclass, jlong baseSnapshotId, jlong targetSnapshotId, jlong nodeId, jboolean isReference, jint startIndex, jint length) {
    Cjprof::InstanceDiffNode node = Cjprof::ExpandDetailDiffNode(
        static_cast<uint64_t>(baseSnapshotId), static_cast<uint64_t>(targetSnapshotId),
        static_cast<uint64_t>(nodeId), static_cast<bool>(isReference),
        static_cast<uint32_t>(startIndex), static_cast<uint32_t>(length));
    return createInstanceDiffNode(env, node);
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_querySnapshotComparison(JNIEnv* env, jclass, jlong baseId, jlong targetId) {
    std::vector<Cjprof::ConstructorDiffNode> nodes = Cjprof::QuerySnapshotComparison(
        static_cast<uint64_t>(baseId), static_cast<uint64_t>(targetId));
    jobject result = env->NewObject(g_arrayListClass, g_arrayListConstructor);
    if (result == nullptr) return nullptr;
    for (const auto& node : nodes) {
        jobject obj = createConstructorDiffNode(env, node);
        env->CallBooleanMethod(result, g_listAddMethod, obj);
        env->DeleteLocalRef(obj);
    }
    return result;
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_getNodeRootpaths(JNIEnv* env, jclass, jlong snapshotId, jlong nodeId, jint pathNum) {
    std::vector<std::vector<Cjprof::InstanceNode>> nodes = Cjprof::GetNodeRootpaths(
        static_cast<uint64_t>(snapshotId), static_cast<uint64_t>(nodeId), static_cast<int32_t>(pathNum));
    jobject result = env->NewObject(g_arrayListClass, g_arrayListConstructor);
    if (result == nullptr) return nullptr;
    for (const auto& path : nodes) {
        // C++ vector<InstanceNode> to Java List
        jobject pathList = env->NewObject(g_arrayListClass, g_arrayListConstructor);
        if (pathList == nullptr) return nullptr;
        for (const auto& node : path) {
            jobject obj = createInstanceNode(env, node);
            env->CallBooleanMethod(pathList, g_listAddMethod, obj);
            env->DeleteLocalRef(obj);
        }
        env->CallBooleanMethod(result, g_listAddMethod, pathList);
        env->DeleteLocalRef(pathList);
    }
    return result;
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_getThreadInfos(JNIEnv* env, jclass, jlong snapshotId) {
    std::vector<Cjprof::ThreadInfo> infos = Cjprof::GetThreadInfos(static_cast<uint64_t>(snapshotId));
    jobject result = env->NewObject(g_arrayListClass, g_arrayListConstructor);
    if (result == nullptr) return nullptr;
    for (const auto& info : infos) {
        jobject obj = createThreadInfo(env, info);
        env->CallBooleanMethod(result, g_listAddMethod, obj);
        env->DeleteLocalRef(obj);
    }
    return result;
}

JNIEXPORT jint JNICALL Java_com_cjprof_jni_Cjprof_querySnapshotCountOfResults(JNIEnv* env, jclass, jstring keyword, jboolean isIgnoreCase, jlong snapshotId) {
    const char* kw = env->GetStringUTFChars(keyword, nullptr);
    uint32_t count = Cjprof::QuerySnapshotCountOfResults(std::string(kw), static_cast<bool>(isIgnoreCase), static_cast<uint64_t>(snapshotId));
    env->ReleaseStringUTFChars(keyword, kw);
    return static_cast<jint>(count);
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_querySnapshotNodeByIndex(JNIEnv* env, jclass, jstring keyword, jboolean isIgnoreCase, jlong snapshotId, jint length, jint index) {
    const char* kw = env->GetStringUTFChars(keyword, nullptr);
    Cjprof::ConstructorNode node = Cjprof::QuerySnapshotNodeByIndex(
        std::string(kw), static_cast<bool>(isIgnoreCase),
        static_cast<uint64_t>(snapshotId), static_cast<uint32_t>(length), static_cast<uint32_t>(index));
    env->ReleaseStringUTFChars(keyword, kw);
    return createConstructorNode(env, node);
}

JNIEXPORT jint JNICALL Java_com_cjprof_jni_Cjprof_queryComparisonCountOfResults(JNIEnv* env, jclass, jstring keyword, jboolean isIgnoreCase, jlong baseId, jlong targetId) {
    const char* kw = env->GetStringUTFChars(keyword, nullptr);
    uint32_t count = Cjprof::QueryComparisonCountOfResults(
        std::string(kw), static_cast<bool>(isIgnoreCase),
        static_cast<uint64_t>(baseId), static_cast<uint64_t>(targetId));
    env->ReleaseStringUTFChars(keyword, kw);
    return static_cast<jint>(count);
}

JNIEXPORT jobject JNICALL Java_com_cjprof_jni_Cjprof_queryComparisonNodeByIndex(JNIEnv* env, jclass, jstring keyword, jboolean isIgnoreCase, jlong baseId, jlong targetId, jint length, jint index) {
    const char* kw = env->GetStringUTFChars(keyword, nullptr);
    Cjprof::ConstructorDiffNode node = Cjprof::QueryComparisonNodeByIndex(
        std::string(kw), static_cast<bool>(isIgnoreCase),
        static_cast<uint64_t>(baseId), static_cast<uint64_t>(targetId),
        static_cast<uint32_t>(length), static_cast<uint32_t>(index));
    env->ReleaseStringUTFChars(keyword, kw);
    return createConstructorDiffNode(env, node);
}
