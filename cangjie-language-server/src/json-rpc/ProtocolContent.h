// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_PROTOCOL_CONTENT_H
#define LSPSERVER_PROTOCOL_CONTENT_H

namespace ark {

enum class SignatureHelpTriggerKind {
    END = 4
};

// Sync document changes strategy for language server
enum class TextDocumentSyncKind {
    // Documents not be synced at any time.
    SK_NONE = 0,

    // Smart sync, Documents are synced using the full content on open.
    // After only incremental updates to the document.
    SK_INCREMENTAL = 2,
};

enum class DocumentHighlightKind {
    TEXT = 1,
};

enum class FileChangeType {
    // The instruction file was created.
    CREATED = 1,
    // The instruction file was changed.
    CHANGED = 2,
    // The instruction file was deleted.
    DELETED = 3,
};

enum class ErrorCode {
    PARSE_ERROR = -32700,
    INVALID_REQUEST = -32600,
    METHOD_NOT_FOUND = -32601,
    SERVER_NOT_INITIALIZED = -32002,
    UNKNOWN_ERROR_CODE = -32001,
    // Customized error code. (>= -31999 or <= -32900)
    INVALID_RENAME_FOR_MACRO_CALL_FILE = -31999
};

enum class SemanticTokenTypes {
    COMMENT_T = 0,
    KEYWORD_T = 1,
    NUMBER_T = 3,
    OPERATOR_T = 5,
    NAMESPACE_T = 6,
    TYPE_T = 7,
    STRUCT_T = 8,
    CLASS_T = 9,
    INTERFACE_T = 10,
    ENUM_T = 11,
    TYPE_PARAMETER_T = 12,
    FUNCTION_T = 13,
    PROPERTY_T = 15,
    MACRO_T = 16,
    VARIABLE_T = 17,
    LABEL_T = 19
};

enum class HighlightKind {
    FILE_H = 1,
    MODULE_H = 2,
    NAMESPACE_H = 3,
    PACKAGE_H = 4,
    CLASS_H = 5,
    METHOD_H = 6,
    PROPERTY_H = 7,
    FIELD_H = 8,
    CONSTRUCTOR_H = 9,
    ENUM_H = 10,
    INTERFACE_H = 11,
    FUNCTION_H = 12,
    VARIABLE_H = 13,
    CONSTANT_H = 14,
    NUMBER_H = 16,
    BOOLEAN_H = 17,
    ARRAY_H = 18,
    OBJECT_H = 19,
    KEY_H = 20,
    MISSING_H = 21,
    ENUMMEMBER_H = 22,
    STRUCT_H = 23,
    EVENT_H = 24,
    OPERATOR_H = 25,
    TYPEPARAMETER_H = 26,
    COMMENT_H = 27,
    RECORD_H = 28,
    TRAIT_H = 29,
    INACTIVECODE_H
};
} // namespace ark
#endif // LSPSERVER_PROTOCOL_CONTENT_H