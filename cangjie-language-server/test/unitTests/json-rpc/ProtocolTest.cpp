// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "Protocol.h"

using json = nlohmann::json;
using namespace ark;

// --- Constants for Testing ---
static constexpr int VERSION_INITIAL = 1;
static constexpr int VERSION_UPDATED = 2;

// Coordinate and Range Constants
static constexpr int LINE_ZERO = 0;
static constexpr int COLUMN_ZERO = 0;
static constexpr int LINE_TWO = 2;
static constexpr int COLUMN_FIVE = 5;
static constexpr int LINE_FIVE = 5;
static constexpr int COLUMN_TEN = 10;
static constexpr int LINE_SIX = 6;
static constexpr int COLUMN_FIFTEEN = 15;
static constexpr int COLUMN_TWENTY = 20;
static constexpr int COLUMN_TWENTY_FIVE = 25;
static constexpr int LINE_SEVEN = 7;
static constexpr int LINE_TEN = 10;
static constexpr int RANGE_LENGTH_TEN = 10;

// Enumeration and ID Constants
static constexpr int TRIGGER_KIND_INVOKED = 1;
static constexpr int TRIGGER_KIND_CHARACTER = 2;
static constexpr int TRIGGER_KIND_INVALID = -1;
static constexpr int SYMBOL_KIND_CLASS = 5;
static constexpr int SYMBOL_KIND_METHOD = 6;
static constexpr int WATCHED_FILE_CHANGE_CREATED = 1;
static constexpr int DIAGNOSTIC_SEVERITY_ERROR = 1;
static constexpr uint64_t TEST_SYMBOL_ID_TH = 12345ull;
static constexpr uint64_t TEST_SYMBOL_ID_CH = 67890ull;

class ProtocolTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Reset global state before each test
        MessageHeaderEndOfLine::SetIsDeveco(false);
    }
};

// --- FromJSON Tests ---

TEST_F(ProtocolTest, FromJSON_DidOpenTextDocumentParams_ValidInput)
{
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj",
            "languageId": "Cangjie",
            "version": 1,
            "text": "fn main() {}"
        }
    })"_json;

    DidOpenTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.textDocument.languageId, "Cangjie");
    EXPECT_EQ(reply.textDocument.version, VERSION_INITIAL);
    EXPECT_EQ(reply.textDocument.text, "fn main() {}");
}

TEST_F(ProtocolTest, FromJSON_DidOpenTextDocumentParams_MissingFields)
{
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj",
            "languageId": "Cangjie"
        }
    })"_json;

    DidOpenTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

TEST_F(ProtocolTest, FromJSON_TextDocumentPositionParams_ValidInput)
{
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        }
    })"_json;

    TextDocumentPositionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, LINE_TEN);
    EXPECT_EQ(reply.position.column, COLUMN_FIVE);
}

TEST_F(ProtocolTest, FromJSON_TextDocumentPositionParams_InvalidStructure)
{
    json params = R"({
        "textDocument": "invalid",
        "position": {
            "line": 10,
            "character": 5
        }
    })"_json;

    TextDocumentPositionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

TEST_F(ProtocolTest, FromJSON_SignatureHelpContext_InvalidTriggerKind)
{
    json params = R"({
        "triggerKind": -1
    })"_json;

    SignatureHelpContext reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

TEST_F(ProtocolTest, FromJSON_SignatureHelpParams_ValidInput)
{
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        },
        "context": {
            "triggerKind": 1
        }
    })"_json;

    SignatureHelpParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, LINE_TEN);
    EXPECT_EQ(reply.position.column, COLUMN_FIVE);
}

TEST_F(ProtocolTest, FromJSON_InitializeParams_ValidInput)
{
    json params = R"({
        "rootUri": "file:///workspace",
        "capabilities": {
            "textDocument": {
                "documentHighlight": {},
                "typeHierarchy": {},
                "publishDiagnostics": {
                    "versionSupport": true
                },
                "hover": {},
                "documentLink": {}
            }
        },
        "initializationOptions": {
            "cangjieRootUri": "file:///custom_root"
        }
    })"_json;

    InitializeParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.rootUri.file, "file:///custom_root");
    EXPECT_TRUE(MessageHeaderEndOfLine::GetIsDeveco());
}

TEST_F(ProtocolTest, FromJSON_DidCloseTextDocumentParams_ValidInput)
{
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        }
    })"_json;

    DidCloseTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

TEST_F(ProtocolTest, FromJSON_TrackCompletionParams_ValidInput)
{
    json params = R"({
        "label": "myFunction"
    })"_json;

    TrackCompletionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.label, "myFunction");
}

TEST_F(ProtocolTest, FromJSON_TrackCompletionParams_MissingLabel)
{
    json params = R"({
        "otherField": "value"
    })"_json;

    TrackCompletionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

TEST_F(ProtocolTest, FromJSON_CompletionContext_ValidInput)
{
    json params = R"({
        "triggerKind": 2,
        "triggerCharacter": "."
    })"_json;

    CompletionContext reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(static_cast<int>(reply.triggerKind), TRIGGER_KIND_CHARACTER);
    EXPECT_EQ(reply.triggerCharacter, ".");
}

TEST_F(ProtocolTest, FromJSON_CompletionParams_ValidInput)
{
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        },
        "context": {
            "triggerKind": 1
        }
    })"_json;

    CompletionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, LINE_TEN);
    EXPECT_EQ(reply.position.column, COLUMN_FIVE);
    EXPECT_EQ(static_cast<int>(reply.context.triggerKind), TRIGGER_KIND_INVOKED);
}

TEST_F(ProtocolTest, FromJSON_SemanticTokensParams_ValidInput)
{
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        }
    })"_json;

    SemanticTokensParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

TEST_F(ProtocolTest, FromJSON_DidChangeTextDocumentParams_ValidInput)
{
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj",
            "version": 2
        },
        "contentChanges": [
            {
                "text": "updated text",
                "range": {
                    "start": {"line": 0, "character": 0},
                    "end": {"line": 0, "character": 10}
                },
                "rangeLength": 10
            }
        ]
    })"_json;

    DidChangeTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.textDocument.version, VERSION_UPDATED);
    ASSERT_EQ(reply.contentChanges.size(), 1u);
    EXPECT_EQ(reply.contentChanges[0].text, "updated text");
    EXPECT_EQ(reply.contentChanges[0].range->start.line, LINE_ZERO);
    EXPECT_EQ(reply.contentChanges[0].range->start.column, COLUMN_ZERO);
    EXPECT_EQ(reply.contentChanges[0].range->end.line, LINE_ZERO);
    EXPECT_EQ(reply.contentChanges[0].range->end.column, COLUMN_TEN);
    EXPECT_EQ(reply.contentChanges[0].rangeLength, RANGE_LENGTH_TEN);
}

TEST_F(ProtocolTest, FromJSON_RenameParams_ValidInput)
{
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        },
        "newName": "newVarName"
    })"_json;

    RenameParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, LINE_TEN);
    EXPECT_EQ(reply.position.column, COLUMN_FIVE);
    EXPECT_EQ(reply.newName, "newVarName");
}

TEST_F(ProtocolTest, FromJSON_TextDocumentIdentifier_ValidInput)
{
    json params = R"({
        "uri": "file:///test.cj"
    })"_json;

    TextDocumentIdentifier reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.uri.file, "file:///test.cj");
}

TEST_F(ProtocolTest, FromJSON_TypeHierarchyItem_ValidInput)
{
    json params = R"({
        "item": {
            "name": "MyClass",
            "kind": 5,
            "uri": "file:///test.cj",
            "range": {
                "start": {"line": 0, "character": 0},
                "end": {"line": 10, "character": 20}
            },
            "selectionRange": {
                "start": {"line": 2, "character": 5},
                "end": {"line": 2, "character": 15}
            },
            "data": {
                "isKernel": true,
                "isChildOrSuper": false,
                "symbolId": "12345"
            }
        }
    })"_json;

    TypeHierarchyItem reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.name, "MyClass");
    EXPECT_EQ(static_cast<int>(reply.kind), SYMBOL_KIND_CLASS);
    EXPECT_EQ(reply.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.range.start.line, LINE_ZERO);
    EXPECT_EQ(reply.range.start.column, COLUMN_ZERO);
    EXPECT_EQ(reply.range.end.line, LINE_TEN);
    EXPECT_EQ(reply.range.end.column, COLUMN_TWENTY);
    EXPECT_EQ(reply.selectionRange.start.line, LINE_TWO);
    EXPECT_EQ(reply.selectionRange.start.column, COLUMN_FIVE);
    EXPECT_TRUE(reply.isKernel);
    EXPECT_EQ(reply.symbolId, TEST_SYMBOL_ID_TH);
}

TEST_F(ProtocolTest, FromJSON_CallHierarchyItem_ValidInput)
{
    json params = R"({
        "item": {
            "name": "myMethod",
            "kind": 6,
            "uri": "file:///test.cj",
            "range": {
                "start": {"line": 5, "character": 10},
                "end": {"line": 7, "character": 20}
            },
            "selectionRange": {
                "start": {"line": 6, "character": 15},
                "end": {"line": 6, "character": 25}
            },
            "detail": "This is a method",
            "data": {
                "isKernel": false,
                "symbolId": "67890"
            }
        }
    })"_json;

    CallHierarchyItem reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.name, "myMethod");
    EXPECT_EQ(static_cast<int>(reply.kind), SYMBOL_KIND_METHOD);
    EXPECT_EQ(reply.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.range.start.line, LINE_FIVE);
    EXPECT_EQ(reply.range.start.column, COLUMN_TEN);
    EXPECT_EQ(reply.selectionRange.start.line, LINE_SIX);
    EXPECT_EQ(reply.symbolId, TEST_SYMBOL_ID_CH);
}

TEST_F(ProtocolTest, FromJSON_DidChangeWatchedFilesParam_ValidInput)
{
    json params = R"({
        "changes": [
            {
                "uri": "file:///test.cj",
                "type": 1
            }
        ]
    })"_json;

    DidChangeWatchedFilesParam reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    ASSERT_EQ(reply.changes.size(), 1u);
    EXPECT_EQ(static_cast<int>(reply.changes[0].type), WATCHED_FILE_CHANGE_CREATED);
}

// --- ToJSON Tests ---
TEST_F(ProtocolTest, ToJSON_BreakpointLocation_ValidInput)
{
    BreakpointLocation params;
    params.uri = "file:///test.cj";
    params.range.start.line = LINE_FIVE;
    params.range.start.column = COLUMN_TEN;
    params.range.end.line = LINE_FIVE;
    params.range.end.column = COLUMN_TWENTY;

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["range"]["start"]["line"], LINE_FIVE);
    EXPECT_EQ(reply["range"]["start"]["character"], COLUMN_TEN);
}

TEST_F(ProtocolTest, ToJSON_CodeLens_ValidInput)
{
    CodeLens params;
    params.range.start.line = LINE_FIVE;
    params.range.start.column = COLUMN_TEN;
    params.range.end.line = LINE_FIVE;
    params.range.end.column = COLUMN_TWENTY;

    params.command.title = "Run Test";
    params.command.command = "run.test";

    ExecutableRange arg;
    arg.uri = "file:///test.cj";
    arg.range.start.line = LINE_ZERO;
    arg.range.start.column = COLUMN_ZERO;
    arg.range.end.line = LINE_TEN;
    arg.range.end.column = COLUMN_TWENTY;
    params.command.arguments.insert(arg);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["range"]["start"]["line"], LINE_FIVE);
    EXPECT_EQ(reply["range"]["start"]["character"], COLUMN_TEN);
}

TEST_F(ProtocolTest, ToJSON_TypeHierarchyItem_ValidInput)
{
    TypeHierarchyItem iter;
    iter.name = "MyClass";
    iter.kind = SymbolKind::CLASS;
    iter.uri.file = "file:///test.cj";
    iter.range.start.line = LINE_ZERO;
    iter.range.start.column = COLUMN_ZERO;
    iter.range.end.line = LINE_TEN;
    iter.range.end.column = COLUMN_TWENTY;
    iter.selectionRange.start.line = LINE_TWO;
    iter.selectionRange.start.column = COLUMN_FIVE;
    iter.selectionRange.end.line = LINE_TWO;
    iter.selectionRange.end.column = COLUMN_FIFTEEN;
    iter.isKernel = true;
    iter.isChildOrSuper = false;
    iter.symbolId = TEST_SYMBOL_ID_TH;

    json replyTH;
    bool result = ToJSON(iter, replyTH);

    EXPECT_TRUE(result);
    EXPECT_EQ(replyTH["name"], "MyClass");
    EXPECT_EQ(replyTH["kind"], static_cast<int>(SymbolKind::CLASS));
    EXPECT_EQ(replyTH["data"]["symbolId"], std::to_string(TEST_SYMBOL_ID_TH));
}

TEST_F(ProtocolTest, ToJSON_CallHierarchyItem_ValidInput)
{
    CallHierarchyItem iter;
    iter.name = "myMethod";
    iter.kind = SymbolKind::FUNCTION;
    iter.uri.file = "file:///test.cj";
    iter.range.start.line = LINE_FIVE;
    iter.range.start.column = COLUMN_TEN;
    iter.range.end.line = LINE_SEVEN;
    iter.range.end.column = COLUMN_TWENTY;
    iter.selectionRange.start.line = LINE_SIX;
    iter.selectionRange.start.column = COLUMN_FIFTEEN;
    iter.selectionRange.end.line = LINE_SIX;
    iter.selectionRange.end.column = COLUMN_TWENTY_FIVE;
    iter.detail = "This is a method";
    iter.isKernel = false;
    iter.symbolId = TEST_SYMBOL_ID_CH;

    json replyCH;
    bool result = ToJSON(iter, replyCH);

    EXPECT_TRUE(result);
    EXPECT_EQ(replyCH["data"]["symbolId"], std::to_string(TEST_SYMBOL_ID_CH));
}

TEST_F(ProtocolTest, ToJSON_CompletionItem_ValidInput)
{
    CompletionItem iter;
    iter.label = "myFunction";
    iter.kind = CompletionItemKind::CIK_FUNCTION;
    iter.insertTextFormat = InsertTextFormat::SNIPPET;
    iter.deprecated = false;

    TextEdit edit;
    edit.range.start.line = LINE_ZERO;
    edit.range.start.column = COLUMN_ZERO;
    edit.range.end.line = LINE_ZERO;
    edit.range.end.column = COLUMN_TEN;
    edit.newText = "replacement";
    iter.additionalTextEdits = std::vector{edit};

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["label"], "myFunction");
    EXPECT_EQ(reply["kind"], static_cast<int>(CompletionItemKind::CIK_FUNCTION));
}

TEST_F(ProtocolTest, ToJSON_PublishDiagnosticsParams_ValidInput)
{
    PublishDiagnosticsParams params;
    params.uri.file = "file:///test.cj";
    params.version = VERSION_INITIAL;

    DiagnosticToken diag;
    diag.range.start.line = LINE_FIVE;
    diag.range.start.column = COLUMN_TEN;
    diag.range.end.line = LINE_FIVE;
    diag.range.end.column = COLUMN_TWENTY;
    diag.severity = DIAGNOSTIC_SEVERITY_ERROR;
    diag.message = "Undefined variable 'x'";
    params.diagnostics.push_back(diag);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["version"], VERSION_INITIAL);
    EXPECT_EQ(reply["diagnostics"][0]["severity"], DIAGNOSTIC_SEVERITY_ERROR);
}

TEST_F(ProtocolTest, ToJSON_WorkspaceEdit_ValidInput)
{
    WorkspaceEdit params;

    TextEdit edit;
    edit.range.start.line = LINE_ZERO;
    edit.range.start.column = COLUMN_ZERO;
    edit.range.end.line = LINE_ZERO;
    edit.range.end.column = COLUMN_TEN;
    edit.newText = "new content";

    params.changes["file:///test.cj"] = std::vector{edit};

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    ASSERT_TRUE(reply.contains("changes"));
    EXPECT_EQ(reply["changes"]["file:///test.cj"][0]["range"]["end"]["character"], COLUMN_TEN);
}