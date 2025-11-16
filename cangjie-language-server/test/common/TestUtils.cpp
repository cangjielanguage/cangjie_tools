// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "TestUtils.h"
#include "SingleInstance.h"

namespace TestUtils {
using namespace Cangjie::FileUtil;

std::string ChangeMessageUrlOfString(const std::string &projectPath, const std::string &tmp,
                                        std::string &rootUri,
                                        bool &isMultiModule) {
    std::string relativePath;
    std::string uri;
    if (isMultiModule) {
        relativePath = GetRelativePathForTest(rootUri, tmp).value();
    } else {
        auto index = tmp.find("src");
        if (index == std::string::npos) {
            return "";
        }
        relativePath = tmp.substr(tmp.find("src"));
    }
    uri = ark::PathWindowsToLinux(JoinPath(projectPath, relativePath));
    auto found = uri.find(':');
    if (found != std::string::npos) {
        uri.replace(found, 1, "%3A");
    }
    if (!uri.empty() && uri.back() == '/') {
        uri.pop_back();
    }
    if (TEST_FILE_SEPERATOR == "\\") {
        uri = "file:///" + uri;
    } else {
        uri = "file://" + uri;
    }
    return uri;
}

std::string replaceCrLf(const std::string &str) {
    std::string result = str;
    size_t start_pos = 0;
    while ((start_pos = result.find(CRLF, start_pos)) != std::string::npos) {
        result.replace(start_pos, CRLF.length(), LF);
        start_pos += 1;
    }
    return result;
}

void PrintJson(const nlohmann::json &exp, const std::string &prefix) {
    std::ostringstream actOs;
    actOs << exp.dump(-1);
    std::string lines = actOs.str();
    printf("%s=%s \n", prefix.c_str(), lines.c_str());
}

std::optional<std::string> GetRelativePathForTest(const std::string &basePath, const std::string &path) {
    if (basePath == path) {
        return "";
    }
    auto found = path.find(basePath);
    if (found != std::string::npos) {
        return path.substr(found + basePath.size());
    }
    return "";
}

void ChangeMessageUrlOfTextDocument(const std::string &projectPath, nlohmann::json &root,
                                    std::string &rootUri,
                                    bool &isMultiModule) {
    if (root.empty()) {
        return;
    }
    std::string tmp = root.get<std::string>();
    if (tmp.empty()) {
        return;
    }
    std::string temp = ChangeMessageUrlOfString(projectPath, tmp, rootUri, isMultiModule);
    if (temp.empty()) {
        return;
    }
    root = temp;
}

bool SemanticTokensFormat::operator==(const SemanticTokensFormat &right) const
{
    return this->line == right.line && this->startChar == right.startChar &&
            this->length == right.length && this->tokenType == right.tokenType;
}

bool SemanticTokensFormat::operator<(const SemanticTokensFormat &right) const
{
    if (this->line < right.line) {
        return true;
    }
    if (this->line == right.line && (this->startChar < right.startChar ||
                                        this->startChar == right.startChar && this->length < right.length)) {
        return true;
    }
    return false;
}

ark::Command CreateCommandStruct(const nlohmann::json &exp) {
    ark::Command result;
    if (exp.contains("command")) {
        result.command = exp.value("command", "");
        result.title = exp.value("title", "");
        result.arguments = CreateExecutableRangeStruct(exp["arguments"][0]);
    }
    return result;
}


ark::Range CreateRangeStruct(const nlohmann::json &exp) {
    ark::Range result;
    if (exp.contains("range")) {
        result.start.column = exp["range"]["start"].value("character", -1);
        result.start.line = exp["range"]["start"].value("line", -1);
        result.end.column = exp["range"]["end"].value("character", -1);
        result.end.line = exp["range"]["end"].value("line", -1);
    }
    return result;
}

std::set<ark::ExecutableRange> CreateExecutableRangeStruct(const nlohmann::json &exp) {
    std::set<ark::ExecutableRange> result;
    ark::ExecutableRange executableRange;
    executableRange.uri = GetFileUrl(exp.value("uri", ""));
    executableRange.projectName = exp.value("projectName", "");
    executableRange.packageName = exp.value("packageName", "");
    executableRange.className = exp.value("className", "");
    executableRange.functionName = exp.value("functionName", "");
    executableRange.range = CreateRangeStruct(exp);
    result.insert(executableRange);
    return result;
}

std::string GetFileUrl(const std::string &file) {
    std::string filePath;
    auto loc = file.find("test/testChr");
    if (loc == std::string::npos) {
        if (file.find("output/bin/stdlib/cjdecl/") != std::string::npos) {
            return file;
        }
        return filePath;
    }

    std::string subUri = "/" + file.substr(loc, file.length());
    filePath = SingleInstance::GetInstance()->workPath + subUri;
    return filePath;
}

bool CompareCodeLen(std::vector<ark::CodeLens> exp, std::vector<ark::CodeLens> act,
                    std::string &reason, int i)
{
    if ((exp[i].range.start.column != act[i].range.start.column) ||
        (exp[i].range.end.column != act[i].range.end.column)) {
        reason = "the expect and actual " + std::to_string(i) + " range start or end column is different";
        return false;
    }

    if ((exp[i].range.start.line != act[i].range.start.line) ||
        (exp[i].range.end.line != act[i].range.end.line)) {
        reason = "the expect and actual " + std::to_string(i) + " range start or end line is different";
        return false;
    }

    if (exp[i].command.title != act[i].command.title) {
        reason = "the expect and actual " + std::to_string(i) + " command title is different";
        return false;
    }

    if (exp[i].command.command != act[i].command.command) {
        reason = "the expect and actual " + std::to_string(i) + " command command is different";
        return false;
    }

    if (exp[i].command.arguments.size() != act[i].command.arguments.size() ||
        exp[i].command.arguments.size() == 0 || act[i].command.arguments.size() == 0) {
        reason = "the expect and actual " + std::to_string(i) + " command arguments is different";
        return false;
    }

    if (exp[i].command.arguments.begin()->uri != act[i].command.arguments.begin()->uri) {
        reason = "the expect and actual " + std::to_string(i) + " arguments uri is different";
        return false;
    }

    if (exp[i].command.arguments.begin()->projectName != act[i].command.arguments.begin()->projectName) {
        reason = "the expect and actual " + std::to_string(i) + " arguments projectName is different";
        return false;
    }

    if (exp[i].command.arguments.begin()->packageName != act[i].command.arguments.begin()->packageName) {
        reason = "the expect and actual " + std::to_string(i) + " arguments packageName is different";
        return false;
    }

    if (exp[i].command.arguments.begin()->className != act[i].command.arguments.begin()->className) {
        reason = "the expect and actual " + std::to_string(i) + " arguments className is different";
        return false;
    }

    if (exp[i].command.arguments.begin()->functionName != act[i].command.arguments.begin()->functionName) {
        reason = "the expect and actual " + std::to_string(i) + " arguments functionName is different";
        return false;
    }
    return true;
}
} // namespace TestUtils
