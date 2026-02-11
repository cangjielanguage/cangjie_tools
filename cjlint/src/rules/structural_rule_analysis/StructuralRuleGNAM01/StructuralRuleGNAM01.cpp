// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGNAM01.h"

namespace {
std::string RemoveBackticks(const std::string& name)
{
    const size_t lenOfBackticks = 2;
    if (name.size() > lenOfBackticks && name.front() == '`' && name.back() == '`') {
        return name.substr(1, name.size() - lenOfBackticks);
    }
    return name;
}

std::string GetFullPackageName(const Cangjie::AST::PackageSpec& pkg)
{
    if (pkg.prefixPaths.empty()) {
        return pkg.packageName;
    }
    auto prefix = Cangjie::Utils::JoinStrings(pkg.prefixPaths, ".") + ".";
    if (pkg.hasDoubleColon) {
        size_t firstDotPos = prefix.find('.');
        if (firstDotPos != std::string::npos) {
            prefix.replace(firstDotPos, 1, "::");
        }
    }
    auto res = prefix  + pkg.packageName;
    return res;
}
} // namespace

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace AST;
using namespace Meta;

const std::string REGEX = "^_?[a-z][a-z0-9_]*(\\._?[a-z][a-z0-9_]*)*$";

/*
 * This method is used to check whether the package name complies with the regular expression.
 */
void StructuralRuleGNAM01::FileDeclHandler(const File &file)
{
    if (!file.package) {
        return;
    }
    const auto& package = *file.package;
    if (package.packageName.Val() == "<invalid identifier>") {
        return;
    }
    auto fullPkgName = GetFullPackageName(package);
    std::regex reg = std::regex(REGEX);
    if (!(std::regex_match(package.packageName.Val(), reg))) {
        Diagnose(package.packageName.Begin(), package.packageName.Begin(),
            CodeCheckDiagKind::G_NAM_01_Package_Information, fullPkgName);
    }
    // Root packages can have any valid package name.
    if (package.prefixPaths.empty() || (package.hasDoubleColon && package.prefixPaths.size() == 1)) {
        return;
    }

    // Verify that package name hierarchy matches the directory structure
    auto pkgNameWithoutOrg = package.hasDoubleColon ? fullPkgName.substr(fullPkgName.find("::") + 2) : fullPkgName;
    auto pkgsSegs = FileUtil::SplitStr(pkgNameWithoutOrg, '.');
    auto filePath = FileUtil::GetDirPath(file.filePath);
    auto pathSegs = FileUtil::SplitStr(filePath, PATH_SEPARATOR.at(0));
    auto pkgMatchPath = std::search(pathSegs.rbegin(), pathSegs.rend(), pkgsSegs.rbegin(), pkgsSegs.rend());
    if (pkgMatchPath == pathSegs.rend()) {
        Diagnose(package.packageName.Begin(), package.packageName.End(),
            CodeCheckDiagKind::G_NAM_01_Package_name_should_match_path, fullPkgName);
        return;
    }
}

/*
 * Obtain the file node and obtain the package name.
 */
void StructuralRuleGNAM01::FilePackageCheckingFunction(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const File &file) {
                FileDeclHandler(file);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGNAM01::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FilePackageCheckingFunction(node);
}
} // namespace Cangjie::CodeCheck
