// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJHEAD_HEAD_FILE_VISITOR_H
#define CJHEAD_HEAD_FILE_VISITOR_H
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "CJHeadCompilerInstance.h"
#include "CJHeadUtil.h"

using namespace Cangjie;
using namespace Cangjie::AST;

namespace CJHead {
class HeadFileVisitor {
public:
    /* first is a line in output, second indicates whether it belongs to a comment */
    using HeadFileFormat = std::vector<std::pair<std::string, bool>>;

    static constexpr int VECTOR_SIZE = 1024;

    HeadFileVisitor(
        const std::unique_ptr<CJHeadCompilerInstance> &cjhead_compiler_instance, const OwnedPtr<File> &cur_file);

    void Run();

    auto GetHeaderFile() -> HeadFileFormat;

    auto GetFirstComment() -> std::string;

    auto GetImportInfo() -> std::vector<std::string>;

private:
    using VisitFunc = std::function<void(const OwnedPtr<Decl> &decl, const int &, const bool &)>;

    std::unordered_map<ASTKind, VisitFunc> visit_func_map_;

    void InitVisitFuncMap();

    /* visit comment, indeed, in here get the first comment */
    auto VisitFirstComment(const std::vector<Token> &comments) -> bool;

    /* visit package and import info */
    auto VisitPackage(std::vector<Ptr<AST::Package>> &source_package) -> bool;

    /* visit decl node */
    void VisitNode(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitStructDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitClassDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitInterfaceDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitEnumDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitEnumConstructor(EnumDecl *enum_decl);

    void VisitExtendDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitFuncDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitVarDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitPropDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitPrimaryCtorDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitMacroDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitMacroExpandDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    void VisitMacroExpandString(const OwnedPtr<Decl> &decl);

    void VisitTypealiasDecl(const OwnedPtr<Decl> &decl, const int &depth = 0, const bool &from_interface = false);

    /* visit type alias, keep those used in public api in .d */
    void VisitTypealias();

    /* whether a decl is public */
    auto IsPublic(const OwnedPtr<Decl> &decl) const -> bool;

    /* whether a decl is protected, used in sub decl */
    auto IsProtected(const OwnedPtr<Decl> &decl) const -> bool;

    /* whether a decl is private */
    auto IsPrivate(const OwnedPtr<Decl> &decl) const -> bool;

    /* get modifier string */
    auto GetModifierString(const OwnedPtr<Decl> &decl) const -> std::string;

    /* get src indentifier string */
    auto GetSrcIndentifierString(const OwnedPtr<Decl> &decl) const -> std::string;

    /* get comment attach to a OwnedPtr<decl> */
    auto GetCommentString(const OwnedPtr<Node> &node) const -> std::vector<std::string>;

    /* get comment attach to a Ptr<node> */
    auto GetCommentString(const Ptr<Node> &node) const -> std::vector<std::string>;

    /* get generic para, eg : <T,U> ----> return T,U */
    auto GetGeneriParam(const OwnedPtr<Decl> &decl) const -> std::string;

    /* get generic para constraint, eg : where */
    auto GetGenericCst(const OwnedPtr<Decl> &decl) const -> std::string;

    /* get super types */
    auto GetInheritedTypes(const OwnedPtr<Decl> &decl) const -> std::string;

    /* get annotation */
    auto GetAnnotationString(const OwnedPtr<Decl> &decl) const -> std::string;

    /* get signature, eg : func、classs、struct get rid of the content between "{}" */
    auto GetSignature(const OwnedPtr<Decl> &decl) const -> std::string;

    /* add a new line to the .d file */
    void addLineToHeader(const std::string &str, const bool &is_comment = false);

    const std::unique_ptr<CJHeadCompilerInstance> &cjhead_compiler_instance_;

    const SourceManager &source_manager_;

    const OwnedPtr<File> &cur_file_;

    /* header comment */
    std::string first_comment_;

    /* import info */
    std::vector<std::string> import_info_;

    /* output content */
    HeadFileFormat header_file_;

    /* current indent */
    size_t indent_{0};

    static constexpr int NUM_SPACES = 4;

    /* record type alias information */
    static std::unordered_map<std::string, std::string> typealias_to_content_;

    static std::unordered_map<std::string, int> typealias_to_line_;

    static std::unordered_map<std::string, bool> typealias_to_appear_;

    static std::unordered_map<std::string, std::vector<std::string>> typealias_to_comment_;
    /* --record type alias information */

    auto getIndent(size_t indet) const -> std::string;

    void incIndent();

    void decIndent();
};
}  // namespace CJHead

#endif
