
// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/Logger/Logger.h"

static constexpr const char* ENUM_CONSTRUCTOR_KIND = "enum_constructor_decl";

void Logger::LogIfFalse(RuleKind rule, std::vector<const Cangjie::AST::Node*>&& nodes, bool result)
{
    if (!result) {
        auto entry = Entry{rule, std::move(nodes)};
        entries.emplace_back(std::move(entry));
    }
}

void Logger::Print(CjoLoader& loader1, CjoLoader& loader2) const
{
    if (entries.empty()) {
        std::cout << "Compatible!" << std::endl;
        return;
    }

    std::cout << "Incompatible!" << std::endl << std::endl;
    auto oldWalkerID = loader1.GetPackage()->visitedByWalkerID;

    for (auto entry : entries) {
        auto& rule = Rules::GetRule(entry.rule);
        auto apiAbi = rule.IsAPI() && checkAPI && rule.IsABI() && checkABI ? "API/ABI"
            : rule.IsAPI() && checkAPI                                     ? "API"
                                                                           : "ABI";
        std::cout << "[" << apiAbi << "] " << rule.GetDescription() << std::endl;
        for (auto node : entry.nodes) {
            auto before = node->visitedByWalkerID == oldWalkerID ? true : false;
            auto beforeAfterStr = before ? "[BEFORE]" : "[AFTER]";
            auto& sm = before ? loader1.GetSourceManager() : loader2.GetSourceManager();
            auto& begin = node->begin;
            auto& end = node->end;
            auto& file = sm.GetSource(begin.fileID).path;
            std::string identifier;
            if (auto decl = dynamic_cast<const Cangjie::AST::Decl*>(node)) {
                auto kind = Cangjie::AST::ASTKIND_TO_STRING_MAP[decl->astKind];
                if (decl->TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR)) {
                    kind  = ENUM_CONSTRUCTOR_KIND;
                }
                identifier = kind + " " + decl->identifier.GetRawText() + ": ";
            }
            std::cout << "  " << identifier;
            std::cout << beforeAfterStr << " ";
            std::cout << file << ":" << begin.line << ":" << begin.column;
            std::cout << "-" << end.line << ":" << end.column << std::endl;
        }
    }
}

const std::vector<Logger::Entry>& Logger::GetEntries() const
{
    return entries;
}