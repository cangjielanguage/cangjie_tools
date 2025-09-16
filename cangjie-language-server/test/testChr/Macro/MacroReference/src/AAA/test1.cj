// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package AAA


from std import ast.*

func myPrint() {
    println("Content-Length:899")
    println()
    println("{\"id\":0,\"jsonrpc\":\"2.0\",\"result\":{\"capabilities\":{\"completionProvider\":{\"resolveProvider\":false,\"triggerCharacters\":[\".\"]},\"definitionProvider\":true,\"documentHighlightProvider\":true,\"documentLinkProvider\":{\"resolveProvider\":true},\"documentSymbolProvider\":true,\"hoverProvider\":true,\"referencesProvider\":true,\"semaHighlightProvider\":true,\"semanticTokensProvider\":{\"full\":{\"delta\":true},\"legend\":{\"tokenModifiers\":[\"declaration\",\"documentation\",\"static\",\"abstract\",\"deprecated\",\"async\",\"readonly\"],\"tokenTypes\":[\"comment\",\"keyword\",\"string\",\"number\",\"regexp\",\"operator\",\"namespace\",\"type\",\"struct\",\"class\",\"interface\",\"enum\",\"typeParameter\",\"function\",\"member\",\"property\",\"macro\",\"variable\",\"parameter\",\"label\"]},\"range\":false},\"signatureHelpProvider\":{\"triggerCharacters\":[\"(\",\",\"]},\"textDocumentSync\":2,\"workspaceSymbolProvider\":true},\"serverInfo\":{\"name\":\"Cangjie language server\",\"version\":\"1.0\"}}}")
    // eprint("eprint in AAA\n")
}

public macro A(input: Tokens) {
    input
}

public macro B(input: Tokens) {
    // parseDecl(input);
    quote(test1())
}

public macro Z(input: Tokens): Tokens {
    return quote(public func declInMacro() {
        println("declInMacro")
        return 0;
    })
}

public macro D(attr: Tokens, input: Tokens): Tokens {
    // myPrint()
    return quote($attr;$input)
}

public macro E(input: Tokens) {
    println("Content-Length:899")
    println()
    println("{\"id\":0,\"jsonrpc\":\"2.0\",\"result\":{\"capabilities\":{\"completionProvider\":{\"resolveProvider\":false,\"triggerCharacters\":[\".\"]},\"definitionProvider\":true,\"documentHighlightProvider\":true,\"documentLinkProvider\":{\"resolveProvider\":true},\"documentSymbolProvider\":true,\"hoverProvider\":true,\"referencesProvider\":true,\"semaHighlightProvider\":true,\"semanticTokensProvider\":{\"full\":{\"delta\":true},\"legend\":{\"tokenModifiers\":[\"declaration\",\"documentation\",\"static\",\"abstract\",\"deprecated\",\"async\",\"readonly\"],\"tokenTypes\":[\"comment\",\"keyword\",\"string\",\"number\",\"regexp\",\"operator\",\"namespace\",\"type\",\"struct\",\"class\",\"interface\",\"enum\",\"typeParameter\",\"function\",\"member\",\"property\",\"macro\",\"variable\",\"parameter\",\"label\"]},\"range\":false},\"signatureHelpProvider\":{\"triggerCharacters\":[\"(\",\",\"]},\"textDocumentSync\":2,\"workspaceSymbolProvider\":true},\"serverInfo\":{\"name\":\"Cangjie language server\",\"version\":\"1.0\"}}}")
    input
}
