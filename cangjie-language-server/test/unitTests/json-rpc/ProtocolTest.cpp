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

class ProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset global state before each test
        MessageHeaderEndOfLine::SetIsDeveco(false);
    }
};

// Test case for DidOpenTextDocumentParams FromJSON
TEST_F(ProtocolTest, FromJSON_DidOpenTextDocumentParams_ValidInput) {
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
    EXPECT_EQ(reply.textDocument.version, 1);
    EXPECT_EQ(reply.textDocument.text, "fn main() {}");
}

TEST_F(ProtocolTest, FromJSON_DidOpenTextDocumentParams_MissingFields) {
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


// Test case for TextDocumentPositionParams FromJSON
TEST_F(ProtocolTest, FromJSON_TextDocumentPositionParams_ValidInput) {
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
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
}

TEST_F(ProtocolTest, FromJSON_TextDocumentPositionParams_InvalidStructure) {
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

// Test case for CrossLanguageJumpParams FromJSON
TEST_F(ProtocolTest, FromJSON_CrossLanguageJumpParams_ValidInput) {
    json params = R"({
        "packageName": "com.example",
        "name": "MyClass",
        "outerName": "Outer",
        "isCombined": true
    })"_json;

    CrossLanguageJumpParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.packageName, "com.example");
    EXPECT_EQ(reply.name, "MyClass");
    EXPECT_EQ(reply.outerName, "Outer");
    EXPECT_TRUE(reply.isCombined);
}

TEST_F(ProtocolTest, FromJSON_CrossLanguageJumpParams_OptionalFieldsMissing) {
    json params = R"({
        "packageName": "com.example",
        "name": "MyClass"
    })"_json;

    CrossLanguageJumpParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.packageName, "com.example");
    EXPECT_EQ(reply.name, "MyClass");
    EXPECT_EQ(reply.outerName, "");  // Default value
    EXPECT_FALSE(reply.isCombined);  // Default value
}

// Test case for OverrideMethodsParams FromJSON
TEST_F(ProtocolTest, FromJSON_OverrideMethodsParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        },
        "isExtend": true
    })"_json;

    OverrideMethodsParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
    EXPECT_TRUE(reply.isExtend);
}

// Test case for ExportsNameParams FromJSON
TEST_F(ProtocolTest, FromJSON_ExportsNameParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        },
        "packageName": "com.example"
    })"_json;

    ExportsNameParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
    EXPECT_EQ(reply.packageName, "com.example");
}

TEST_F(ProtocolTest, FromJSON_SignatureHelpContext_InvalidTriggerKind) {
    json params = R"({
        "triggerKind": -1
    })"_json;

    SignatureHelpContext reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

// Test case for SignatureHelpParams FromJSON
TEST_F(ProtocolTest, FromJSON_SignatureHelpParams_ValidInput) {
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
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
}

// Test case for InitializeParams FromJSON
TEST_F(ProtocolTest, FromJSON_InitializeParams_ValidInput) {
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
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.documentHighlightClientCapabilities);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.typeHierarchyCapabilities);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.diagnosticVersionSupport);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.hoverClientCapabilities);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.documentLinkClientCapabilities);
}

// Test case for DidCloseTextDocumentParams FromJSON
TEST_F(ProtocolTest, FromJSON_DidCloseTextDocumentParams_ValidInput) {
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

// Test case for TrackCompletionParams FromJSON
TEST_F(ProtocolTest, FromJSON_TrackCompletionParams_ValidInput) {
    json params = R"({
        "label": "myFunction"
    })"_json;

    TrackCompletionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.label, "myFunction");
}

TEST_F(ProtocolTest, FromJSON_TrackCompletionParams_MissingLabel) {
    json params = R"({
        "otherField": "value"
    })"_json;

    TrackCompletionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

// Test case for CompletionContext FromJSON
TEST_F(ProtocolTest, FromJSON_CompletionContext_ValidInput) {
    json params = R"({
        "triggerKind": 2,
        "triggerCharacter": "."
    })"_json;

    CompletionContext reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(static_cast<int>(reply.triggerKind), 2);
    EXPECT_EQ(reply.triggerCharacter, ".");
}

// Test case for CompletionParams FromJSON
TEST_F(ProtocolTest, FromJSON_CompletionParams_ValidInput) {
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
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
    EXPECT_EQ(static_cast<int>(reply.context.triggerKind), 1);
}

// Test case for SemanticTokensParams FromJSON
TEST_F(ProtocolTest, FromJSON_SemanticTokensParams_ValidInput) {
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

// Test case for DidChangeTextDocumentParams FromJSON
TEST_F(ProtocolTest, FromJSON_DidChangeTextDocumentParams_ValidInput) {
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
    EXPECT_EQ(reply.textDocument.version, 2);
    ASSERT_EQ(reply.contentChanges.size(), 1u);
    EXPECT_EQ(reply.contentChanges[0].text, "updated text");
    EXPECT_EQ(reply.contentChanges[0].range->start.line, 0);
    EXPECT_EQ(reply.contentChanges[0].range->start.column, 0);
    EXPECT_EQ(reply.contentChanges[0].range->end.line, 0);
    EXPECT_EQ(reply.contentChanges[0].range->end.column, 10);
    EXPECT_EQ(reply.contentChanges[0].rangeLength, 10);
}

// Test case for RenameParams FromJSON
TEST_F(ProtocolTest, FromJSON_RenameParams_ValidInput) {
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
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
    EXPECT_EQ(reply.newName, "newVarName");
}

// Test case for TextDocumentIdentifier FromJSON
TEST_F(ProtocolTest, FromJSON_TextDocumentIdentifier_ValidInput) {
    json params = R"({
        "uri": "file:///test.cj"
    })"_json;

    TextDocumentIdentifier reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.uri.file, "file:///test.cj");
}

// Test case for TextDocumentParams FromJSON
TEST_F(ProtocolTest, FromJSON_TextDocumentParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        }
    })"_json;

    TextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Test case for TypeHierarchyItem FromJSON
TEST_F(ProtocolTest, FromJSON_TypeHierarchyItem_ValidInput) {
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
    EXPECT_EQ(static_cast<int>(reply.kind), 5);
    EXPECT_EQ(reply.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.range.start.line, 0);
    EXPECT_EQ(reply.range.start.column, 0);
    EXPECT_EQ(reply.range.end.line, 10);
    EXPECT_EQ(reply.range.end.column, 20);
    EXPECT_EQ(reply.selectionRange.start.line, 2);
    EXPECT_EQ(reply.selectionRange.start.column, 5);
    EXPECT_EQ(reply.selectionRange.end.line, 2);
    EXPECT_EQ(reply.selectionRange.end.column, 15);
    EXPECT_TRUE(reply.isKernel);
    EXPECT_FALSE(reply.isChildOrSuper);
    EXPECT_EQ(reply.symbolId, 12345ull);
}

// Test case for CallHierarchyItem FromJSON
TEST_F(ProtocolTest, FromJSON_CallHierarchyItem_ValidInput) {
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
    EXPECT_EQ(static_cast<int>(reply.kind), 6);
    EXPECT_EQ(reply.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.range.start.line, 5);
    EXPECT_EQ(reply.range.start.column, 10);
    EXPECT_EQ(reply.range.end.line, 7);
    EXPECT_EQ(reply.range.end.column, 20);
    EXPECT_EQ(reply.selectionRange.start.line, 6);
    EXPECT_EQ(reply.selectionRange.start.column, 15);
    EXPECT_EQ(reply.selectionRange.end.line, 6);
    EXPECT_EQ(reply.selectionRange.end.column, 25);
    EXPECT_EQ(reply.detail, "This is a method");
    EXPECT_FALSE(reply.isKernel);
    EXPECT_EQ(reply.symbolId, 67890ull);
}

// Test case for DocumentLinkParams FromJSON
TEST_F(ProtocolTest, FromJSON_DocumentLinkParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        }
    })"_json;

    DocumentLinkParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Test case for DidChangeWatchedFilesParam FromJSON
TEST_F(ProtocolTest, FromJSON_DidChangeWatchedFilesParam_ValidInput) {
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
    EXPECT_EQ(reply.changes[0].textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(static_cast<int>(reply.changes[0].type), 1);
}

// Test case for DiagnosticRelatedInformation FromJSON
TEST_F(ProtocolTest, FromJSON_DiagnosticRelatedInformation_ValidInput) {
    json param = R"({
        "message": "Defined here",
        "location": {
            "uri": "file:///definition.cj",
            "range": {
                "start": {"line": 10, "character": 5},
                "end": {"line": 10, "character": 15}
            }
        }
    })"_json;

    DiagnosticRelatedInformation info;
    bool result = FromJSON(param, info);

    EXPECT_TRUE(result);
    EXPECT_EQ(info.message, "Defined here");
    EXPECT_EQ(info.location.uri.file, "file:///definition.cj");
    EXPECT_EQ(info.location.range.start.line, 10);
    EXPECT_EQ(info.location.range.start.column, 5);
    EXPECT_EQ(info.location.range.end.line, 10);
    EXPECT_EQ(info.location.range.end.column, 15);
}

// Test case for DocumentSymbolParams FromJSON
TEST_F(ProtocolTest, FromJSON_DocumentSymbolParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        }
    })"_json;

    DocumentSymbolParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Test case for CodeActionParams FromJSON
TEST_F(ProtocolTest, FromJSON_CodeActionParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "range": {
            "start": {"line": 5, "character": 10},
            "end": {"line": 5, "character": 20}
        },
        "context": {
            "diagnostics": []
        }
    })"_json;

    CodeActionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.range.start.line, 5);
    EXPECT_EQ(reply.range.start.column, 10);
    EXPECT_EQ(reply.range.end.line, 5);
    EXPECT_EQ(reply.range.end.column, 20);
}

// Test case for TweakArgs FromJSON
TEST_F(ProtocolTest, FromJSON_TweakArgs_ValidInput) {
    json params = R"({
        "file": "file:///test.cj",
        "selection": {
            "start": {"line": 5, "character": 10},
            "end": {"line": 5, "character": 20}
        },
        "tweakID": "rename-variable",
        "extraOptions": {
            "newName": "newVar"
        }
    })"_json;

    TweakArgs reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.file.file, "file:///test.cj");
    EXPECT_EQ(reply.selection.start.line, 5);
    EXPECT_EQ(reply.selection.start.column, 10);
    EXPECT_EQ(reply.selection.end.line, 5);
    EXPECT_EQ(reply.selection.end.column, 20);
    EXPECT_EQ(reply.tweakID, "rename-variable");
    ASSERT_EQ(reply.extraOptions.size(), 1u);
    EXPECT_EQ(reply.extraOptions.at("newName"), "newVar");
}

// Test case for ExecuteCommandParams FromJSON
TEST_F(ProtocolTest, FromJSON_ExecuteCommandParams_ValidInput) {
    json params = R"({
        "command": "cjLsp.applyTweak",
        "arguments": [
            {
                "file": "file:///test.cj",
                "selection": {
                    "start": {"line": 5, "character": 10},
                    "end": {"line": 5, "character": 20}
                },
                "tweakID": "rename-variable"
            }
        ]
    })"_json;

    ExecuteCommandParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.command, "cjLsp.applyTweak");
    EXPECT_FALSE(reply.arguments.empty());
}

// Test case for FileRefactorReqParams FromJSON
TEST_F(ProtocolTest, FromJSON_FileRefactorReqParams_ValidInput) {
    json params = R"({
        "file": {
            "uri": "file:///old/path.cj"
        },
        "targetPath": {
            "uri": "file:///new/path.cj"
        },
        "selectedElement": {
            "uri": "file:///element.cj"
        }
    })"_json;

    FileRefactorReqParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.file.uri.file, "file:///old/path.cj");
    EXPECT_EQ(reply.targetPath.uri.file, "file:///new/path.cj");
    EXPECT_EQ(reply.selectedElement.uri.file, "file:///element.cj");
}

// Test cases for ToJSON functions would follow similar patterns...
// For brevity, only one example is shown below:

// Test case for BreakpointLocation ToJSON
TEST_F(ProtocolTest, ToJSON_BreakpointLocation_ValidInput) {
    BreakpointLocation params;
    params.uri = "file:///test.cj";
    params.range.start.line = 5;
    params.range.start.column = 10;
    params.range.end.line = 5;
    params.range.end.column = 20;

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["range"]["start"]["line"], 5);
    EXPECT_EQ(reply["range"]["start"]["character"], 10);
    EXPECT_EQ(reply["range"]["end"]["line"], 5);
    EXPECT_EQ(reply["range"]["end"]["character"], 20);
}

// Test case for ExecutableRange ToJSON
TEST_F(ProtocolTest, ToJSON_ExecutableRange_ValidInput) {
    ExecutableRange params;
    params.uri = "file:///test.cj";
    params.projectName = "MyProject";
    params.packageName = "com.example";
    params.className = "MyClass";
    params.functionName = "main";
    params.range.start.line = 0;
    params.range.start.column = 0;
    params.range.end.line = 10;
    params.range.end.column = 20;

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["projectName"], "MyProject");
    EXPECT_EQ(reply["packageName"], "com.example");
    EXPECT_EQ(reply["className"], "MyClass");
    EXPECT_EQ(reply["functionName"], "main");
    EXPECT_EQ(reply["range"]["start"]["line"], 0);
    EXPECT_EQ(reply["range"]["start"]["character"], 0);
    EXPECT_EQ(reply["range"]["end"]["line"], 10);
    EXPECT_EQ(reply["range"]["end"]["character"], 20);
}

// Test case for CodeLens ToJSON
TEST_F(ProtocolTest, ToJSON_CodeLens_ValidInput) {
    CodeLens params;
    params.range.start.line = 5;
    params.range.start.column = 10;
    params.range.end.line = 5;
    params.range.end.column = 20;

    params.command.title = "Run Test";
    params.command.command = "run.test";

    ExecutableRange arg;
    arg.uri = "file:///test.cj";
    arg.range.start.line = 0;
    arg.range.start.column = 0;
    arg.range.end.line = 10;
    arg.range.end.column = 20;
    params.command.arguments.insert(arg);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["range"]["start"]["line"], 5);
    EXPECT_EQ(reply["range"]["start"]["character"], 10);
    EXPECT_EQ(reply["range"]["end"]["line"], 5);
    EXPECT_EQ(reply["range"]["end"]["character"], 20);
    EXPECT_EQ(reply["command"]["title"], "Run Test");
    EXPECT_EQ(reply["command"]["command"], "run.test");
    ASSERT_EQ(reply["command"]["arguments"].size(), 1u);
    EXPECT_EQ(reply["command"]["arguments"][0]["uri"], "file:///test.cj");
}

// Test case for Command ToJSON
TEST_F(ProtocolTest, ToJSON_Command_ValidInput) {
    Command params;
    params.title = "Apply Fix";
    params.command = "apply.fix";

    ExecutableRange arg;
    arg.uri = "file:///test.cj";
    arg.tweakId = "fix-imports";
    arg.range.start.line = 0;
    arg.range.start.column = 0;
    arg.range.end.line = 10;
    arg.range.end.column = 20;
    arg.extraOptions["option1"] = "value1";
    params.arguments.insert(arg);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["title"], "Apply Fix");
    EXPECT_EQ(reply["command"], "apply.fix");
    ASSERT_EQ(reply["arguments"].size(), 1u);
    EXPECT_EQ(reply["arguments"][0]["tweakID"], "fix-imports");
    EXPECT_EQ(reply["arguments"][0]["file"], "file:///test.cj");
    EXPECT_EQ(reply["arguments"][0]["selection"]["start"]["line"], 0);
    EXPECT_EQ(reply["arguments"][0]["selection"]["start"]["character"], 0);
    EXPECT_EQ(reply["arguments"][0]["selection"]["end"]["line"], 10);
    EXPECT_EQ(reply["arguments"][0]["selection"]["end"]["character"], 20);
    EXPECT_EQ(reply["arguments"][0]["option1"], "value1");
}

// Test case for TypeHierarchyItem ToJSON
TEST_F(ProtocolTest, ToJSON_TypeHierarchyItem_ValidInput) {
    TypeHierarchyItem iter;
    iter.name = "MyClass";
    iter.kind = SymbolKind::CLASS;
    iter.uri.file = "file:///test.cj";
    iter.range.start.line = 0;
    iter.range.start.column = 0;
    iter.range.end.line = 10;
    iter.range.end.column = 20;
    iter.selectionRange.start.line = 2;
    iter.selectionRange.start.column = 5;
    iter.selectionRange.end.line = 2;
    iter.selectionRange.end.column = 15;
    iter.isKernel = true;
    iter.isChildOrSuper = false;
    iter.symbolId = 12345ull;

    json replyTH;
    bool result = ToJSON(iter, replyTH);

    EXPECT_TRUE(result);
    EXPECT_EQ(replyTH["name"], "MyClass");
    EXPECT_EQ(replyTH["kind"], static_cast<int>(SymbolKind::CLASS));
    EXPECT_EQ(replyTH["uri"], "file:///test.cj");
    EXPECT_EQ(replyTH["range"]["start"]["line"], 0);
    EXPECT_EQ(replyTH["range"]["start"]["character"], 0);
    EXPECT_EQ(replyTH["range"]["end"]["line"], 10);
    EXPECT_EQ(replyTH["range"]["end"]["character"], 20);
    EXPECT_EQ(replyTH["selectionRange"]["start"]["line"], 2);
    EXPECT_EQ(replyTH["selectionRange"]["start"]["character"], 5);
    EXPECT_EQ(replyTH["selectionRange"]["end"]["line"], 2);
    EXPECT_EQ(replyTH["selectionRange"]["end"]["character"], 15);
    EXPECT_TRUE(replyTH["data"]["isKernel"]);
    EXPECT_FALSE(replyTH["data"]["isChildOrSuper"]);
    EXPECT_EQ(replyTH["data"]["symbolId"], "12345");
}

// Test case for CallHierarchyItem ToJSON
TEST_F(ProtocolTest, ToJSON_CallHierarchyItem_ValidInput) {
    CallHierarchyItem iter;
    iter.name = "myMethod";
    iter.kind = SymbolKind::FUNCTION;
    iter.uri.file = "file:///test.cj";
    iter.range.start.line = 5;
    iter.range.start.column = 10;
    iter.range.end.line = 7;
    iter.range.end.column = 20;
    iter.selectionRange.start.line = 6;
    iter.selectionRange.start.column = 15;
    iter.selectionRange.end.line = 6;
    iter.selectionRange.end.column = 25;
    iter.detail = "This is a method";
    iter.isKernel = false;
    iter.symbolId = 67890ull;

    json replyCH;
    bool result = ToJSON(iter, replyCH);

    EXPECT_TRUE(result);
    EXPECT_EQ(replyCH["name"], "myMethod");
    EXPECT_EQ(replyCH["kind"], static_cast<int>(SymbolKind::FUNCTION));
    EXPECT_EQ(replyCH["uri"], "file:///test.cj");
    EXPECT_EQ(replyCH["range"]["start"]["line"], 5);
    EXPECT_EQ(replyCH["range"]["start"]["character"], 10);
    EXPECT_EQ(replyCH["range"]["end"]["line"], 7);
    EXPECT_EQ(replyCH["range"]["end"]["character"], 20);
    EXPECT_EQ(replyCH["selectionRange"]["start"]["line"], 6);
    EXPECT_EQ(replyCH["selectionRange"]["start"]["character"], 15);
    EXPECT_EQ(replyCH["selectionRange"]["end"]["line"], 6);
    EXPECT_EQ(replyCH["selectionRange"]["end"]["character"], 25);
    EXPECT_EQ(replyCH["detail"], "This is a method");
    EXPECT_FALSE(replyCH["data"]["isKernel"]);
    EXPECT_EQ(replyCH["data"]["symbolId"], "67890");
}

// Test case for CompletionItem ToJSON
TEST_F(ProtocolTest, ToJSON_CompletionItem_ValidInput) {
    CompletionItem iter;
    iter.label = "myFunction";
    iter.kind = CompletionItemKind::CIK_FUNCTION;
    iter.detail = "A sample function";
    iter.documentation = "Does something useful";
    iter.sortText = "a";
    iter.filterText = "myFunc";
    iter.insertText = "myFunction()";
    iter.insertTextFormat = InsertTextFormat::SNIPPET;
    iter.deprecated = false;

    TextEdit edit;
    edit.range.start.line = 0;
    edit.range.start.column = 0;
    edit.range.end.line = 0;
    edit.range.end.column = 10;
    edit.newText = "replacement";
    iter.additionalTextEdits = std::vector{edit};

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["label"], "myFunction");
    EXPECT_EQ(reply["kind"], static_cast<int>(CompletionItemKind::CIK_FUNCTION));
    EXPECT_EQ(reply["detail"], "A sample function");
    EXPECT_EQ(reply["documentation"], "Does something useful");
    EXPECT_EQ(reply["sortText"], "a");
    EXPECT_EQ(reply["filterText"], "myFunc");
    EXPECT_EQ(reply["insertText"], "myFunction()");
    EXPECT_EQ(reply["insertTextFormat"], static_cast<int>(InsertTextFormat::SNIPPET));
    EXPECT_FALSE(reply["deprecated"]);
    ASSERT_TRUE(reply.contains("additionalTextEdits"));
    ASSERT_EQ(reply["additionalTextEdits"].size(), 1u);
    EXPECT_EQ(reply["additionalTextEdits"][0]["newText"], "replacement");
}

// Test case for DiagnosticRelatedInformation ToJSON
TEST_F(ProtocolTest, ToJSON_DiagnosticRelatedInformation_ValidInput) {
    DiagnosticRelatedInformation info;
    info.message = "Defined here";
    info.location.uri.file = "file:///definition.cj";
    info.location.range.start.line = 10;
    info.location.range.start.column = 5;
    info.location.range.end.line = 10;
    info.location.range.end.column = 15;

    json reply;
    bool result = ToJSON(info, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["message"], "Defined here");
    EXPECT_EQ(reply["location"]["uri"], "file:///definition.cj");
    EXPECT_EQ(reply["location"]["range"]["start"]["line"], 10);
    EXPECT_EQ(reply["location"]["range"]["start"]["character"], 5);
    EXPECT_EQ(reply["location"]["range"]["end"]["line"], 10);
    EXPECT_EQ(reply["location"]["range"]["end"]["character"], 15);
}

// Test case for PublishDiagnosticsParams ToJSON
TEST_F(ProtocolTest, ToJSON_PublishDiagnosticsParams_ValidInput) {
    PublishDiagnosticsParams params;
    params.uri.file = "file:///test.cj";
    params.version = 1;

    DiagnosticToken diag;
    diag.range.start.line = 5;
    diag.range.start.column = 10;
    diag.range.end.line = 5;
    diag.range.end.column = 20;
    diag.severity = 1;
    diag.source = "compiler";
    diag.message = "Undefined variable 'x'";
    params.diagnostics.push_back(diag);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["version"], 1);
    ASSERT_TRUE(reply.contains("diagnostics"));
    ASSERT_EQ(reply["diagnostics"].size(), 1u);
    EXPECT_EQ(reply["diagnostics"][0]["message"], "Undefined variable 'x'");
}

// Test case for WorkspaceEdit ToJSON
TEST_F(ProtocolTest, ToJSON_WorkspaceEdit_ValidInput) {
    WorkspaceEdit params;

    TextEdit edit;
    edit.range.start.line = 0;
    edit.range.start.column = 0;
    edit.range.end.line = 0;
    edit.range.end.column = 10;
    edit.newText = "new content";

    params.changes["file:///test.cj"] = std::vector{edit};

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    ASSERT_TRUE(reply.contains("changes"));
    ASSERT_TRUE(reply["changes"].contains("file:///test.cj"));
    ASSERT_EQ(reply["changes"]["file:///test.cj"].size(), 1u);
    EXPECT_EQ(reply["changes"]["file:///test.cj"][0]["newText"], "new content");
}

// Test case for TextDocumentEdit ToJSON
TEST_F(ProtocolTest, ToJSON_TextDocumentEdit_ValidInput) {
    TextDocumentEdit params;
    params.textDocument.uri.file = "file:///test.cj";
    params.textDocument.version = 1;

    TextEdit edit;
    edit.range.start.line = 0;
    edit.range.start.column = 0;
    edit.range.end.line = 0;
    edit.range.end.column = 10;
    edit.newText = "new content";
    params.textEdits.push_back(edit);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["textDocument"]["uri"], "file:///test.cj");
    EXPECT_EQ(reply["textDocument"]["version"], 1);
    ASSERT_EQ(reply["edits"].size(), 1u);
    EXPECT_EQ(reply["edits"][0]["newText"], "new content");
}

// Test case for DocumentSymbol ToJSON
TEST_F(ProtocolTest, ToJSON_DocumentSymbol_ValidInput) {
    DocumentSymbol item;
    item.name = "MyClass";
    item.kind = SymbolKind::CLASS;
    item.detail = "A sample class";
    item.range.start.line = 0;
    item.range.start.column = 0;
    item.range.end.line = 10;
    item.range.end.column = 20;
    item.selectionRange.start.line = 2;
    item.selectionRange.start.column = 5;
    item.selectionRange.end.line = 2;
    item.selectionRange.end.column = 15;

    DocumentSymbol child;
    child.name = "myMethod";
    child.kind = SymbolKind::FUNCTION;
    child.range.start.line = 3;
    child.range.start.column = 5;
    child.range.end.line = 5;
    child.range.end.column = 15;
    child.selectionRange.start.line = 4;
    child.selectionRange.start.column = 7;
    child.selectionRange.end.line = 4;
    child.selectionRange.end.column = 13;
    item.children.push_back(child);

    json result;
    bool success = ToJSON(item, result);

    EXPECT_TRUE(success);
    EXPECT_EQ(result["name"], "MyClass");
    EXPECT_EQ(result["kind"], static_cast<int>(SymbolKind::CLASS));
    EXPECT_EQ(result["detail"], "A sample class");
    EXPECT_EQ(result["range"]["start"]["line"], 0);
    EXPECT_EQ(result["range"]["start"]["character"], 0);
    EXPECT_EQ(result["range"]["end"]["line"], 10);
    EXPECT_EQ(result["range"]["end"]["character"], 20);
    EXPECT_EQ(result["selectionRange"]["start"]["line"], 2);
    EXPECT_EQ(result["selectionRange"]["start"]["character"], 5);
    EXPECT_EQ(result["selectionRange"]["end"]["line"], 2);
    EXPECT_EQ(result["selectionRange"]["end"]["character"], 15);
    ASSERT_TRUE(result.contains("children"));
    ASSERT_EQ(result["children"].size(), 1u);
    EXPECT_EQ(result["children"][0]["name"], "myMethod");
}

// Test case for CallHierarchyOutgoingCall ToJSON
TEST_F(ProtocolTest, ToJSON_CallHierarchyOutgoingCall_ValidInput) {
    CallHierarchyOutgoingCall iter;

    CallHierarchyItem toItem;
    toItem.name = "calledFunction";
    toItem.kind = SymbolKind::FUNCTION;
    toItem.uri.file = "file:///callee.cj";
    toItem.range.start.line = 0;
    toItem.range.start.column = 0;
    toItem.range.end.line = 5;
    toItem.range.end.column = 20;
    toItem.selectionRange.start.line = 1;
    toItem.selectionRange.start.column = 5;
    toItem.selectionRange.end.line = 1;
    toItem.selectionRange.end.column = 15;
    iter.to = toItem;

    Range fromRange;
    fromRange.start.line = 10;
    fromRange.start.column = 5;
    fromRange.end.line = 10;
    fromRange.end.column = 15;
    iter.fromRanges.push_back(fromRange);

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["to"]["name"], "calledFunction");
    ASSERT_EQ(reply["fromRanges"].size(), 1u);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["line"], 10);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["character"], 5);
    EXPECT_EQ(reply["fromRanges"][0]["end"]["line"], 10);
    EXPECT_EQ(reply["fromRanges"][0]["end"]["character"], 15);
}

// Test case for CallHierarchyIncomingCall ToJSON
TEST_F(ProtocolTest, ToJSON_CallHierarchyIncomingCall_ValidInput) {
    CallHierarchyIncomingCall iter;

    CallHierarchyItem fromItem;
    fromItem.name = "callingFunction";
    fromItem.kind = SymbolKind::FUNCTION;
    fromItem.uri.file = "file:///caller.cj";
    fromItem.range.start.line = 0;
    fromItem.range.start.column = 0;
    fromItem.range.end.line = 5;
    fromItem.range.end.column = 20;
    fromItem.selectionRange.start.line = 1;
    fromItem.selectionRange.start.column = 5;
    fromItem.selectionRange.end.line = 1;
    fromItem.selectionRange.end.column = 15;
    iter.from = fromItem;

    Range fromRange;
    fromRange.start.fileID = 1;
    fromRange.start.line = 10;
    fromRange.start.column = 5;
    fromRange.end.fileID = 1;
    fromRange.end.line = 10;
    fromRange.end.column = 15;
    iter.fromRanges.push_back(fromRange);

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["from"]["name"], "callingFunction");
    ASSERT_EQ(reply["fromRanges"].size(), 1u);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["fileID"], 1);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["line"], 10);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["character"], 5);
    EXPECT_EQ(reply["fromRanges"][0]["end"]["fileID"], 1);
    EXPECT_EQ(reply["fromRanges"][0]["end"]["line"], 10);
    EXPECT_EQ(reply["fromRanges"][0]["end"]["character"], 15);
}

// Test case for CodeAction ToJSON
TEST_F(ProtocolTest, ToJSON_CodeAction_ValidInput) {
    CodeAction params;
    params.title = "Fix import";
    params.kind = CodeAction::QUICKFIX_KIND;

    DiagnosticToken diag;
    diag.range.start.line = 5;
    diag.range.start.column = 10;
    diag.range.end.line = 5;
    diag.range.end.column = 20;
    diag.severity = 1;
    diag.source = "compiler";
    diag.message = "Import missing";
    params.diagnostics = std::vector{diag};

    WorkspaceEdit edit;
    TextEdit textEdit;
    textEdit.range.start.line = 0;
    textEdit.range.start.column = 0;
    textEdit.range.end.line = 0;
    textEdit.range.end.column = 0;
    textEdit.newText = "import com.example;\n";
    edit.changes["file:///test.cj"] = std::vector{textEdit};
    params.edit = edit;

    Command command;
    command.title = "Run formatter";
    command.command = "format.code";
    params.command = command;

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["title"], "Fix import");
    EXPECT_EQ(reply["kind"], CodeAction::QUICKFIX_KIND);
    ASSERT_TRUE(reply.contains("diagnostics"));
    ASSERT_EQ(reply["diagnostics"].size(), 1u);
    EXPECT_EQ(reply["diagnostics"][0]["message"], "Import missing");
    ASSERT_TRUE(reply.contains("edit"));
    ASSERT_TRUE(reply["edit"]["changes"].contains("file:///test.cj"));
    EXPECT_EQ(reply["edit"]["changes"]["file:///test.cj"][0]["newText"], "import com.example;\n");
    ASSERT_TRUE(reply.contains("command"));
    EXPECT_EQ(reply["command"]["title"], "Run formatter");
    EXPECT_EQ(reply["command"]["command"], "format.code");
}

// Test case for ApplyWorkspaceEditParams ToJSON
TEST_F(ProtocolTest, ToJSON_ApplyWorkspaceEditParams_ValidInput) {
    ApplyWorkspaceEditParams params;

    TextEdit edit;
    edit.range.start.line = 0;
    edit.range.start.column = 0;
    edit.range.end.line = 0;
    edit.range.end.column = 10;
    edit.newText = "new content";
    params.edit.changes["file:///test.cj"] = std::vector{edit};

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    ASSERT_TRUE(reply.contains("edit"));
    ASSERT_TRUE(reply["edit"]["changes"].contains("file:///test.cj"));
    EXPECT_EQ(reply["edit"]["changes"]["file:///test.cj"][0]["newText"], "new content");
}

// Test case for FileRefactorRespParams ToJSON
TEST_F(ProtocolTest, ToJSON_FileRefactorRespParams_ValidInput) {
    FileRefactorRespParams item;

    FileRefactorChange change;
    change.type = FileRefactorChangeType::CHANGED;
    change.range.start.line = 0;
    change.range.start.column = 0;
    change.range.end.line = 0;
    change.range.end.column = 10;
    change.content = "new content";

    item.changes["file:///test.cj"].insert(change);

    json reply;
    bool result = ToJSON(item, reply);

    EXPECT_TRUE(result);
    ASSERT_TRUE(reply.contains("changes"));
    ASSERT_TRUE(reply["changes"].contains("file:///test.cj"));
    ASSERT_EQ(reply["changes"]["file:///test.cj"].size(), 1u);
    EXPECT_EQ(reply["changes"]["file:///test.cj"][0]["type"], static_cast<int>(FileRefactorChangeType::CHANGED));
    EXPECT_EQ(reply["changes"]["file:///test.cj"][0]["content"], "new content");
}

// Test case for DidOpenTextDocumentParams FromJSON with invalid languageId
TEST_F(ProtocolTest, FromJSON_DidOpenTextDocumentParams_InvalidLanguageId) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj",
            "languageId": "Java",
            "version": 1,
            "text": "fn main() {}"
        }
    })"_json;

    DidOpenTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

// Test case for DidOpenTextDocumentParams FromJSON with null fields
TEST_F(ProtocolTest, FromJSON_DidOpenTextDocumentParams_NullFields) {
    json params = R"({
        "textDocument": {
            "uri": null,
            "languageId": "Cangjie",
            "version": 1,
            "text": "fn main() {}"
        }
    })"_json;

    DidOpenTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

// Test case for TextDocumentPositionParams FromJSON with invalid position
TEST_F(ProtocolTest, FromJSON_TextDocumentPositionParams_InvalidPosition) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": null,
            "character": 5
        }
    })"_json;

    TextDocumentPositionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

// Test case for SignatureHelpParams FromJSON without context
TEST_F(ProtocolTest, FromJSON_SignatureHelpParams_WithoutContext) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        }
    })"_json;

    SignatureHelpParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
}

// Test case for InitializeParams FromJSON without initializationOptions
TEST_F(ProtocolTest, FromJSON_InitializeParams_WithoutInitializationOptions) {
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
        }
    })"_json;

    InitializeParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.rootUri.file, "file:///workspace");
    EXPECT_FALSE(MessageHeaderEndOfLine::GetIsDeveco());
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.documentHighlightClientCapabilities);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.typeHierarchyCapabilities);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.diagnosticVersionSupport);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.hoverClientCapabilities);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.documentLinkClientCapabilities);
}

// Test case for InitializeParams FromJSON with empty textDocument capabilities
TEST_F(ProtocolTest, FromJSON_InitializeParams_EmptyTextDocumentCapabilities) {
    json params = R"({
        "rootUri": "file:///workspace",
        "capabilities": {
            "textDocument": {}
        }
    })"_json;

    InitializeParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_FALSE(reply.capabilities.textDocumentClientCapabilities.documentHighlightClientCapabilities);
    EXPECT_FALSE(reply.capabilities.textDocumentClientCapabilities.typeHierarchyCapabilities);
    EXPECT_FALSE(reply.capabilities.textDocumentClientCapabilities.diagnosticVersionSupport);
    EXPECT_FALSE(reply.capabilities.textDocumentClientCapabilities.hoverClientCapabilities);
    EXPECT_FALSE(reply.capabilities.textDocumentClientCapabilities.documentLinkClientCapabilities);
}

// Test case for CompletionContext FromJSON with trigger character
TEST_F(ProtocolTest, FromJSON_CompletionContext_WithTriggerCharacter) {
    json params = R"({
        "triggerKind": 1,
        "triggerCharacter": "."
    })"_json;

    CompletionContext reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(static_cast<int>(reply.triggerKind), 1);
    EXPECT_EQ(reply.triggerCharacter, "");
}

// Test case for CompletionContext FromJSON without trigger character
TEST_F(ProtocolTest, FromJSON_CompletionContext_WithoutTriggerCharacter) {
    json params = R"({
        "triggerKind": 2
    })"_json;

    CompletionContext reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(static_cast<int>(reply.triggerKind), 2);
    EXPECT_TRUE(reply.triggerCharacter.empty());
}

// Test case for CompletionParams FromJSON without context
TEST_F(ProtocolTest, FromJSON_CompletionParams_WithoutContext) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        }
    })"_json;

    CompletionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
}

// Test case for DidChangeTextDocumentParams FromJSON with multiple content changes
TEST_F(ProtocolTest, FromJSON_DidChangeTextDocumentParams_MultipleChanges) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj",
            "version": 2
        },
        "contentChanges": [
            {
                "text": "first change",
                "range": {
                    "start": {"line": 0, "character": 0},
                    "end": {"line": 0, "character": 5}
                }
            },
            {
                "text": "second change"
            }
        ]
    })"_json;

    DidChangeTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.textDocument.version, 2);
    ASSERT_EQ(reply.contentChanges.size(), 2u);
    EXPECT_EQ(reply.contentChanges[0].text, "first change");
    EXPECT_TRUE(reply.contentChanges[0].range.has_value());
    EXPECT_EQ(reply.contentChanges[1].text, "second change");
    EXPECT_FALSE(reply.contentChanges[1].range.has_value());
}

// Test case for DidChangeTextDocumentParams FromJSON with invalid range
TEST_F(ProtocolTest, FromJSON_DidChangeTextDocumentParams_InvalidRange) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj",
            "version": 2
        },
        "contentChanges": [
            {
                "text": "change",
                "range": {
                    "start": {"line": null, "character": 0},
                    "end": {"line": 0, "character": 5}
                }
            }
        ]
    })"_json;

    DidChangeTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result); // Should still return true as it skips invalid changes
    EXPECT_TRUE(reply.contentChanges.empty()); // But no valid changes added
}

// Test case for TweakArgs FromJSON without extraOptions
TEST_F(ProtocolTest, FromJSON_TweakArgs_WithoutExtraOptions) {
    json params = R"({
        "file": "file:///test.cj",
        "selection": {
            "start": {"line": 5, "character": 10},
            "end": {"line": 5, "character": 20}
        },
        "tweakID": "rename-variable"
    })"_json;

    TweakArgs reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.file.file, "file:///test.cj");
    EXPECT_EQ(reply.tweakID, "rename-variable");
    EXPECT_TRUE(reply.extraOptions.empty());
}

// Test case for CompletionItem ToJSON without optional fields
TEST_F(ProtocolTest, ToJSON_CompletionItem_WithoutOptionalFields) {
    CompletionItem iter;
    iter.label = "myFunction";
    iter.kind = CompletionItemKind::CIK_FUNCTION;
    iter.detail = "A sample function";
    // Don't set optional fields like additionalTextEdits

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["label"], "myFunction");
    EXPECT_EQ(reply["kind"], static_cast<int>(CompletionItemKind::CIK_FUNCTION));
    EXPECT_EQ(reply["detail"], "A sample function");
    EXPECT_FALSE(reply.contains("additionalTextEdits")); // Should not include optional field
}

// Test case for DiagnosticToken ToJSON with all optional fields
TEST_F(ProtocolTest, ToJSON_DiagnosticToken_WithAllOptionalFields) {
    DiagnosticToken iter;
    iter.range.start.line = 5;
    iter.range.start.column = 10;
    iter.range.end.line = 5;
    iter.range.end.column = 20;
    iter.severity = 1;
    iter.code = 001;
    iter.source = "compiler";
    iter.message = "Undefined variable";
    iter.category = 1;
    iter.tags = {1, 2};

    DiagnosticRelatedInformation relatedInfo;
    relatedInfo.message = "Defined here";
    relatedInfo.location.uri.file = "file:///def.cj";
    relatedInfo.location.range.start.line = 10;
    relatedInfo.location.range.start.column = 5;
    relatedInfo.location.range.end.line = 10;
    relatedInfo.location.range.end.column = 15;
    iter.relatedInformation = std::vector{relatedInfo};

    CodeAction action;
    action.title = "Fix import";
    action.kind = "quickfix";
    iter.codeActions = std::vector{action};

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["range"]["start"]["line"], 5);
    EXPECT_EQ(reply["range"]["start"]["character"], 10);
    EXPECT_EQ(reply["range"]["end"]["line"], 5);
    EXPECT_EQ(reply["range"]["end"]["character"], 20);
    EXPECT_EQ(reply["severity"], 1);
    EXPECT_EQ(reply["code"], 001);
    EXPECT_EQ(reply["source"], "compiler");
    EXPECT_EQ(reply["message"], "Undefined variable");
    EXPECT_EQ(reply["category"], 1);
    ASSERT_TRUE(reply.contains("tags"));
    ASSERT_EQ(reply["tags"].size(), 2u);
    ASSERT_TRUE(reply.contains("relatedInformation"));
    ASSERT_EQ(reply["relatedInformation"].size(), 1u);
    ASSERT_TRUE(reply.contains("codeActions"));
    ASSERT_EQ(reply["codeActions"].size(), 1u);
}

// Test case for PublishDiagnosticsParams ToJSON with empty diagnostics
TEST_F(ProtocolTest, ToJSON_PublishDiagnosticsParams_EmptyDiagnostics) {
    PublishDiagnosticsParams params;
    params.uri.file = "file:///test.cj";
    params.version = 1;
    // No diagnostics added

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["version"], 1);
    ASSERT_TRUE(reply.contains("diagnostics"));
    EXPECT_TRUE(reply["diagnostics"].is_array());
    EXPECT_TRUE(reply["diagnostics"].empty());
}

// Test case for PublishDiagnosticsParams ToJSON without version
TEST_F(ProtocolTest, ToJSON_PublishDiagnosticsParams_WithoutVersion) {
    PublishDiagnosticsParams params;
    params.uri.file = "file:///test.cj";
    // No version set

    DiagnosticToken diag;
    diag.range.start.line = 5;
    diag.range.start.column = 10;
    diag.range.end.line = 5;
    diag.range.end.column = 20;
    diag.severity = 1;
    diag.message = "Error";
    params.diagnostics.push_back(diag);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_FALSE(reply.contains("version")); // Should not include version
    ASSERT_TRUE(reply.contains("diagnostics"));
    ASSERT_EQ(reply["diagnostics"].size(), 1u);
}

// Test case for WorkspaceEdit ToJSON with multiple changes
TEST_F(ProtocolTest, ToJSON_WorkspaceEdit_MultipleChanges) {
    WorkspaceEdit params;

    TextEdit edit1;
    edit1.range.start.line = 0;
    edit1.range.start.column = 0;
    edit1.range.end.line = 0;
    edit1.range.end.column = 10;
    edit1.newText = "new content 1";

    TextEdit edit2;
    edit2.range.start.line = 1;
    edit2.range.start.column = 0;
    edit2.range.end.line = 1;
    edit2.range.end.column = 5;
    edit2.newText = "new content 2";

    params.changes["file:///test1.cj"] = std::vector{edit1};
    params.changes["file:///test2.cj"] = std::vector{edit1, edit2};

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    ASSERT_TRUE(reply.contains("changes"));
    ASSERT_TRUE(reply["changes"].contains("file:///test1.cj"));
    ASSERT_TRUE(reply["changes"].contains("file:///test2.cj"));
    EXPECT_EQ(reply["changes"]["file:///test1.cj"].size(), 1u);
    EXPECT_EQ(reply["changes"]["file:///test2.cj"].size(), 2u);
}

// Test case for CodeAction ToJSON with only diagnostics
TEST_F(ProtocolTest, ToJSON_CodeAction_OnlyDiagnostics) {
    CodeAction params;
    params.title = "Fix issues";
    params.kind = "quickfix";

    DiagnosticToken diag;
    diag.range.start.line = 5;
    diag.range.start.column = 10;
    diag.range.end.line = 5;
    diag.range.end.column = 20;
    diag.severity = 1;
    diag.message = "Issue found";
    params.diagnostics = std::vector{diag};
    // No edit or command

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["title"], "Fix issues");
    EXPECT_EQ(reply["kind"], "quickfix");
    ASSERT_TRUE(reply.contains("diagnostics"));
    ASSERT_EQ(reply["diagnostics"].size(), 1u);
    EXPECT_FALSE(reply.contains("edit"));
    EXPECT_FALSE(reply.contains("command"));
}

// Test case for CodeAction ToJSON with only edit
TEST_F(ProtocolTest, ToJSON_CodeAction_OnlyEdit) {
    CodeAction params;
    params.title = "Refactor";
    params.kind = "refactor";

    WorkspaceEdit edit;
    TextEdit textEdit;
    textEdit.range.start.line = 0;
    textEdit.range.start.column = 0;
    textEdit.range.end.line = 0;
    textEdit.range.end.column = 10;
    textEdit.newText = "refactored";
    edit.changes["file:///test.cj"] = std::vector{textEdit};
    params.edit = edit;
    // No diagnostics or command

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["title"], "Refactor");
    EXPECT_EQ(reply["kind"], "refactor");
    ASSERT_TRUE(reply.contains("edit"));
    EXPECT_FALSE(reply.contains("diagnostics"));
    EXPECT_FALSE(reply.contains("command"));
}

// Test case for CodeAction ToJSON with only command
TEST_F(ProtocolTest, ToJSON_CodeAction_OnlyCommand) {
    CodeAction params;
    params.title = "Execute";
    params.kind = "info";

    Command command;
    command.title = "Run";
    command.command = "execute";
    params.command = command;
    // No diagnostics or edit

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["title"], "Execute");
    EXPECT_EQ(reply["kind"], "info");
    ASSERT_TRUE(reply.contains("command"));
    EXPECT_FALSE(reply.contains("diagnostics"));
    EXPECT_FALSE(reply.contains("edit"));
}

// Test case for DocumentSymbol ToJSON without children
TEST_F(ProtocolTest, ToJSON_DocumentSymbol_WithoutChildren) {
    DocumentSymbol item;
    item.name = "MyFunction";
    item.kind = SymbolKind::FUNCTION;
    item.detail = "A function";
    item.range.start.line = 0;
    item.range.start.column = 0;
    item.range.end.line = 5;
    item.range.end.column = 20;
    item.selectionRange.start.line = 1;
    item.selectionRange.start.column = 5;
    item.selectionRange.end.line = 1;
    item.selectionRange.end.column = 15;
    // No children

    json result;
    bool success = ToJSON(item, result);

    EXPECT_TRUE(success);
    EXPECT_EQ(result["name"], "MyFunction");
    EXPECT_EQ(result["kind"], static_cast<int>(SymbolKind::FUNCTION));
    EXPECT_EQ(result["detail"], "A function");
    EXPECT_FALSE(result.contains("children")); // Should not include children array
}

// Test case for CallHierarchyOutgoingCall ToJSON with multiple fromRanges
TEST_F(ProtocolTest, ToJSON_CallHierarchyOutgoingCall_MultipleFromRanges) {
    CallHierarchyOutgoingCall iter;

    CallHierarchyItem toItem;
    toItem.name = "callee";
    iter.to = toItem;

    Range range1;
    range1.start.line = 10;
    range1.start.column = 5;
    range1.end.line = 10;
    range1.end.column = 15;

    Range range2;
    range2.start.line = 20;
    range2.start.column = 0;
    range2.end.line = 20;
    range2.end.column = 10;

    iter.fromRanges.push_back(range1);
    iter.fromRanges.push_back(range2);

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["to"]["name"], "callee");
    ASSERT_EQ(reply["fromRanges"].size(), 2u);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["line"], 10);
    EXPECT_EQ(reply["fromRanges"][1]["start"]["line"], 20);
}

// Test case for CallHierarchyIncomingCall ToJSON with multiple fromRanges
TEST_F(ProtocolTest, ToJSON_CallHierarchyIncomingCall_MultipleFromRanges) {
    CallHierarchyIncomingCall iter;

    CallHierarchyItem fromItem;
    fromItem.name = "caller";
    iter.from = fromItem;

    Range range1;
    range1.start.fileID = 1;
    range1.start.line = 10;
    range1.start.column = 5;
    range1.end.fileID = 1;
    range1.end.line = 10;
    range1.end.column = 15;

    Range range2;
    range2.start.fileID = 2;
    range2.start.line = 20;
    range2.start.column = 0;
    range2.end.fileID = 2;
    range2.end.line = 20;
    range2.end.column = 10;

    iter.fromRanges.push_back(range1);
    iter.fromRanges.push_back(range2);

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["from"]["name"], "caller");
    ASSERT_EQ(reply["fromRanges"].size(), 2u);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["fileID"], 1);
    EXPECT_EQ(reply["fromRanges"][1]["start"]["fileID"], 2);
}

// Test case for ApplyWorkspaceEditParams ToJSON with empty changes
TEST_F(ProtocolTest, ToJSON_ApplyWorkspaceEditParams_EmptyChanges) {
    ApplyWorkspaceEditParams params;
    // No changes

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_TRUE(reply.empty()); // Should produce empty JSON
}

// Test case for FileRefactorRespParams ToJSON with empty changes
TEST_F(ProtocolTest, ToJSON_FileRefactorRespParams_EmptyChanges) {
    FileRefactorRespParams item;
    // No changes added

    json reply;
    bool result = ToJSON(item, reply);

    EXPECT_TRUE(result);
    ASSERT_FALSE(reply.contains("changes"));
    EXPECT_TRUE(reply["changes"].empty());
}

// Test case for CodeLens ToJSON with empty arguments
TEST_F(ProtocolTest, ToJSON_CodeLens_EmptyArguments) {
    CodeLens params;
    params.range.start.line = 5;
    params.range.start.column = 10;
    params.range.end.line = 5;
    params.range.end.column = 20;

    params.command.title = "Run Test";
    params.command.command = "run.test";
    // No arguments

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["range"]["start"]["line"], 5);
    EXPECT_EQ(reply["command"]["title"], "Run Test");
    EXPECT_EQ(reply["command"]["command"], "run.test");
    ASSERT_TRUE(reply["command"].contains("arguments"));
    EXPECT_TRUE(reply["command"]["arguments"].empty());
}

// Test case for Command ToJSON with empty arguments
TEST_F(ProtocolTest, ToJSON_Command_EmptyArguments) {
    Command params;
    params.title = "Empty Command";
    params.command = "empty.command";
    // No arguments

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["title"], "Empty Command");
    EXPECT_EQ(reply["command"], "empty.command");
    ASSERT_TRUE(reply.contains("arguments"));
    EXPECT_TRUE(reply["arguments"].empty());
}

// Test case for Command ToJSON without extraOptions
TEST_F(ProtocolTest, ToJSON_Command_WithoutExtraOptions) {
    Command params;
    params.title = "Simple Command";
    params.command = "simple.command";

    ExecutableRange arg;
    arg.uri = "file:///test.cj";
    arg.tweakId = "simple-tweak";
    arg.range.start.line = 0;
    arg.range.start.column = 0;
    arg.range.end.line = 10;
    arg.range.end.column = 20;
    // No extraOptions

    params.arguments.insert(arg);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["title"], "Simple Command");
    EXPECT_EQ(reply["command"], "simple.command");
    ASSERT_EQ(reply["arguments"].size(), 1u);
    EXPECT_EQ(reply["arguments"][0]["tweakID"], "simple-tweak");
    EXPECT_FALSE(reply["arguments"][0].contains("extraOptions"));
}

// Test case for TextDocumentEdit ToJSON with empty textEdits
TEST_F(ProtocolTest, ToJSON_TextDocumentEdit_EmptyTextEdits) {
    TextDocumentEdit params;
    params.textDocument.uri.file = "file:///test.cj";
    params.textDocument.version = 1;
    // No textEdits

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["textDocument"]["uri"], "file:///test.cj");
    EXPECT_EQ(reply["textDocument"]["version"], 1);
    ASSERT_TRUE(reply.contains("edits"));
    EXPECT_TRUE(reply["edits"].empty());
}

// Test case for OverrideMethodsParams FromJSON without isExtend
TEST_F(ProtocolTest, FromJSON_OverrideMethodsParams_WithoutIsExtend) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        }
    })"_json;

    OverrideMethodsParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
    EXPECT_FALSE(reply.isExtend); // Default value
}

// Test case for SignatureHelpContext FromJSON with empty signatures
TEST_F(ProtocolTest, FromJSON_SignatureHelpContext_EmptySignatures) {
    json params = R"({
        "triggerKind": 1,
        "activeSignatureHelp": {
            "activeSignature": 0,
            "activeParameter": 1,
            "signatures": []
        }
    })"_json;

    SignatureHelpContext reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.activeSignatureHelp.activeSignature, 0);
    EXPECT_EQ(reply.activeSignatureHelp.activeParameter, 1);
    EXPECT_TRUE(reply.activeSignatureHelp.signatures.empty());
}

// Test case for InitializeParams FromJSON with empty cangjieRootUri
TEST_F(ProtocolTest, FromJSON_InitializeParams_EmptyCangjieRootUri) {
    json params = R"({
        "rootUri": "",
        "capabilities": {
            "textDocument": {}
        },
        "initializationOptions": {
            "cangjieRootUri": ""
        }
    })"_json;

    InitializeParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.rootUri.file, "");
    EXPECT_TRUE(MessageHeaderEndOfLine::GetIsDeveco()); // Should still be set to true
}

// Test case for DidChangeTextDocumentParams FromJSON with empty contentChanges
TEST_F(ProtocolTest, FromJSON_DidChangeTextDocumentParams_EmptyContentChanges) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj",
            "version": 2
        },
        "contentChanges": []
    })"_json;

    DidChangeTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result); // Should return false for empty contentChanges
}

// Test case for DidChangeTextDocumentParams FromJSON with invalid contentChanges
TEST_F(ProtocolTest, FromJSON_DidChangeTextDocumentParams_InvalidContentChanges) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj",
            "version": 2
        },
        "contentChanges": [
            {
                "text": null
            }
        ]
    })"_json;

    DidChangeTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result); // Should return false when no valid changes
}

// Test case for RenameParams FromJSON with invalid position
TEST_F(ProtocolTest, FromJSON_RenameParams_InvalidPosition) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": null,
            "character": 5
        },
        "newName": "newName"
    })"_json;

    RenameParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

// Test case for TypeHierarchyItem FromJSON with empty symbolId
TEST_F(ProtocolTest, FromJSON_TypeHierarchyItem_EmptySymbolId) {
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
                "symbolId": ""
            }
        }
    })"_json;

    TypeHierarchyItem reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.name, "MyClass");
    EXPECT_EQ(reply.symbolId, 0ull); // Should be 0 for empty symbolId
}

// Test case for CallHierarchyItem FromJSON with empty symbolId
TEST_F(ProtocolTest, FromJSON_CallHierarchyItem_EmptySymbolId) {
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
                "symbolId": ""
            }
        }
    })"_json;

    CallHierarchyItem reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.name, "myMethod");
    EXPECT_EQ(reply.symbolId, 0ull); // Should be 0 for empty symbolId
}

// Test case for DidChangeWatchedFilesParam FromJSON with empty changes
TEST_F(ProtocolTest, FromJSON_DidChangeWatchedFilesParam_EmptyChanges) {
    json params = R"({
        "changes": []
    })"_json;

    DidChangeWatchedFilesParam reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_TRUE(reply.changes.empty());
}

// Test case for CodeActionContext FromJSON with null only field
TEST_F(ProtocolTest, FromJSON_CodeActionContext_NullOnlyField) {
    json params = R"({
        "diagnostics": [],
        "only": null
    })"_json;

    CodeActionContext reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_TRUE(reply.diagnostics.empty());
    EXPECT_FALSE(reply.only.has_value()); // Should not have value for null
}

// Test case for CodeActionContext FromJSON with empty diagnostics
TEST_F(ProtocolTest, FromJSON_CodeActionContext_EmptyDiagnostics) {
    json params = R"({
        "diagnostics": []
    })"_json;

    CodeActionContext reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_TRUE(reply.diagnostics.empty());
}

// Test case for TweakArgs FromJSON with non-string extraOptions
TEST_F(ProtocolTest, FromJSON_TweakArgs_NonStringExtraOptions) {
    json params = R"({
        "file": "file:///test.cj",
        "selection": {
            "start": {"line": 5, "character": 10},
            "end": {"line": 5, "character": 20}
        },
        "tweakID": "test",
        "extraOptions": {
            "numberOption": 123,
            "boolOption": true,
            "stringOption": "value"
        }
    })"_json;

    TweakArgs reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.file.file, "file:///test.cj");
    EXPECT_EQ(reply.tweakID, "test");
    ASSERT_EQ(reply.extraOptions.size(), 1u); // Only string option should be added
    EXPECT_EQ(reply.extraOptions.at("stringOption"), "value");
}

// Test case for ExecuteCommandParams FromJSON with empty arguments
TEST_F(ProtocolTest, FromJSON_ExecuteCommandParams_EmptyArguments) {
    json params = R"({
        "command": "test.command",
        "arguments": []
    })"_json;

    ExecuteCommandParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.command, "test.command");
    EXPECT_TRUE(reply.arguments.empty());
}

// Test case for CompletionItem ToJSON with empty optional vectors
TEST_F(ProtocolTest, ToJSON_CompletionItem_EmptyOptionalVectors) {
    CompletionItem iter;
    iter.label = "test";
    iter.kind = CompletionItemKind::CIK_FUNCTION;

    // Set empty optional vectors
    iter.additionalTextEdits = std::vector<TextEdit>{};

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["label"], "test");
    EXPECT_TRUE(reply.contains("additionalTextEdits"));
}

// Test case for DiagnosticToken ToJSON with empty optional vectors
TEST_F(ProtocolTest, ToJSON_DiagnosticToken_EmptyOptionalVectors) {
    DiagnosticToken iter;
    iter.range.start.line = 5;
    iter.range.start.column = 10;
    iter.range.end.line = 5;
    iter.range.end.column = 20;
    iter.severity = 1;
    iter.message = "test";

    // Set empty optional vectors
    iter.tags = {};
    iter.relatedInformation = std::vector<DiagnosticRelatedInformation>{};
    iter.codeActions = std::vector<CodeAction>{};

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["message"], "test");
    EXPECT_FALSE(reply.contains("tags")); // Should not include empty vectors
    EXPECT_FALSE(reply.contains("relatedInformation"));
    EXPECT_TRUE(reply.contains("codeActions"));
}

// Test case for CodeAction ToJSON with empty diagnostics
TEST_F(ProtocolTest, ToJSON_CodeAction_EmptyDiagnostics) {
    CodeAction params;
    params.title = "Test Action";
    params.kind = "quickfix";

    // Set empty diagnostics
    params.diagnostics = std::vector<DiagnosticToken>{};

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["title"], "Test Action");
    EXPECT_TRUE(reply.contains("diagnostics"));
}

// Test case for MessageHeaderEndOfLine static methods
TEST_F(ProtocolTest, MessageHeaderEndOfLine_StaticMethods) {
    EXPECT_FALSE(MessageHeaderEndOfLine::GetIsDeveco());

    MessageHeaderEndOfLine::SetIsDeveco(true);
    EXPECT_TRUE(MessageHeaderEndOfLine::GetIsDeveco());

    MessageHeaderEndOfLine::SetIsDeveco(false);
    EXPECT_FALSE(MessageHeaderEndOfLine::GetIsDeveco());
}

// Test case for TextDocumentIdentifier FromJSON with null uri
TEST_F(ProtocolTest, FromJSON_TextDocumentIdentifier_NullUri) {
    json params = R"({
        "uri": null
    })"_json;

    TextDocumentIdentifier reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

// Test case for FetchTextDocument with completion capability but null completionItem
TEST_F(ProtocolTest, FetchTextDocument_NullCompletionItem) {
    json textDocument = R"({
        "completion": {
            "completionItem": null
        }
    })"_json;

    InitializeParams params;
    bool result = FetchTextDocument(textDocument, params);

    EXPECT_FALSE(result); // Should return false for null completionItem
}

// Test case for FetchTextDocument with empty capabilities
TEST_F(ProtocolTest, FetchTextDocument_EmptyCapabilities) {
    json textDocument = json::object();

    InitializeParams params;
    bool result = FetchTextDocument(textDocument, params);

    EXPECT_TRUE(result);
    // All capabilities should remain false
    EXPECT_FALSE(params.capabilities.textDocumentClientCapabilities.documentHighlightClientCapabilities);
    EXPECT_FALSE(params.capabilities.textDocumentClientCapabilities.typeHierarchyCapabilities);
    EXPECT_FALSE(params.capabilities.textDocumentClientCapabilities.diagnosticVersionSupport);
    EXPECT_FALSE(params.capabilities.textDocumentClientCapabilities.hoverClientCapabilities);
    EXPECT_FALSE(params.capabilities.textDocumentClientCapabilities.documentLinkClientCapabilities);
}

// Test case for ToJSON functions with default constructed objects
TEST_F(ProtocolTest, ToJSON_DefaultConstructedObjects) {
    // Test various ToJSON functions with default constructed objects
    BreakpointLocation breakpoint;
    json breakpointReply;
    EXPECT_TRUE(ToJSON(breakpoint, breakpointReply));

    ExecutableRange executable;
    json executableReply;
    EXPECT_TRUE(ToJSON(executable, executableReply));

    CodeLens codeLens;
    json codeLensReply;
    EXPECT_TRUE(ToJSON(codeLens, codeLensReply));

    Command command;
    json commandReply;
    EXPECT_TRUE(ToJSON(command, commandReply));

    TypeHierarchyItem typeItem;
    json typeItemReply;
    EXPECT_TRUE(ToJSON(typeItem, typeItemReply));

    CallHierarchyItem callItem;
    json callItemReply;
    EXPECT_TRUE(ToJSON(callItem, callItemReply));

    CompletionItem completionItem;
    json completionReply;
    EXPECT_TRUE(ToJSON(completionItem, completionReply));

    DiagnosticToken diagnostic;
    json diagnosticReply;
    EXPECT_TRUE(ToJSON(diagnostic, diagnosticReply));

    DocumentSymbol documentSymbol;
    json symbolReply;
    EXPECT_TRUE(ToJSON(documentSymbol, symbolReply));
}

// Test case for edge case values in numeric fields
TEST_F(ProtocolTest, FromJSON_EdgeCaseNumericValues) {
    // Test with edge case values like -1, 0, large numbers
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": -1,
            "character": 0
        }
    })"_json;

    TextDocumentPositionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.position.line, -1);
    EXPECT_EQ(reply.position.column, 0);
}

// Test case for FromJSON with special characters in strings
TEST_F(ProtocolTest, FromJSON_SpecialCharacters) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test with spaces.cj",
            "languageId": "Cangjie",
            "version": 1,
            "text": "line1\nline2\tline3\"quoted\""
        }
    })"_json;

    DidOpenTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test with spaces.cj");
    EXPECT_EQ(reply.textDocument.text, "line1\nline2\tline3\"quoted\"");
}

// Test case for FromJSON with unicode characters
TEST_F(ProtocolTest, FromJSON_UnicodeCharacters) {
    json params = R"({
        "textDocument": {
            "uri": "file:///测试.cj",
            "languageId": "Cangjie",
            "version": 1,
            "text": "中文测试 🚀"
        }
    })"_json;

    DidOpenTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///测试.cj");
    EXPECT_EQ(reply.textDocument.text, "中文测试 🚀");
}
