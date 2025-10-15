// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGNAM02.h"
#include <regex>

using namespace Cangjie;
using namespace AST;
using namespace Meta;
using namespace CodeCheck;

/**
 * filename rule
 * The filename must be to use digits, lowercase letters and underscores(_),only digits and underscores are not allowed.
 * right eg:
 * my_class.cj , you_34.cj ,
 * wrong eg:
 * 1.cj , _.cj , 34_34_.cj
 */
static const std::string REGEX_RULE = "[a-z0-9_]+\\.cj";
static const std::string FORBID_REGEX_RULE = "[0-9_]+\\.cj";

static const size_t MIN_FILENAME_LEN = 3;
static const size_t PUBLIC_APPEAR_ONCE = 1;

void StructuralRuleGNAM02::CheckNameRule(File &file)
{
    int pubNum = 0;
    std::string pubName = "";
    for (auto &decl : file.decls) {
        if (decl->ty->kind != TypeKind::TYPE_CLASS && decl->ty->kind != TypeKind::TYPE_INTERFACE &&
            decl->ty->kind != TypeKind::TYPE_STRUCT) {
            continue;
        }
        for (auto mod : decl->modifiers) {
            if (mod.ToString() == "public") {
                ++pubNum;
                pubName = decl->identifier;
            }
        }
    }
    if (pubNum == PUBLIC_APPEAR_ONCE && file.fileName.length() >= MIN_FILENAME_LEN) {
        std::string exterLowName = pubName;
        (void)std::transform(exterLowName.begin(), exterLowName.end(), exterLowName.begin(), ::tolower);
        std::string fileName = file.fileName.substr(0, file.fileName.length() - 3);
        (void)fileName.erase(remove(fileName.begin(), fileName.end(), '_'), fileName.end());
        (void)exterLowName.erase(remove(exterLowName.begin(), exterLowName.end(), '_'), exterLowName.end());
        if (fileName != exterLowName) {
            CodeCheckDiagKind kind = CodeCheckDiagKind::G_NAM_02_filename_standard_information;
            Diagnose(file.filePath, 1, 1, 1, 1, kind, file.fileName, pubName);
        }
    }
}

void StructuralRuleGNAM02::FindFileName(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    match (*node)([this](const Package &package) {
        for (auto &it : package.files) {
            std::string name = it->fileName;
            if (regex_match(name, std::regex(REGEX_RULE)) && !regex_match(name, std::regex(FORBID_REGEX_RULE))) {
                CheckNameRule(*it);
            } else {
                Diagnose(
                    it->filePath, 1, 1, 1, 1, CodeCheckDiagKind::G_NAM_02_filename_lower_information, it->fileName);
            }
        }
    });
}

void StructuralRuleGNAM02::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindFileName(node);
}
