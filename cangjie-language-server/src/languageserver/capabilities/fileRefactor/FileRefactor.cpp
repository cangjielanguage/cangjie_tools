// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "FileRefactor.h"

namespace ark {
std::unordered_set<TokenKind> FileRefactor::ReExportKinds = 
    {TokenKind::INTERNAL, TokenKind::PROTECTED, TokenKind::PUBLIC};

FileRefactor::FileRefactor(FileRefactorRespParams &result): result(result)
{
    InitMatcher();
}

void FileRefactor::AddImport()
{
    if (!fileNode) {
        return;
    }
    Position lastImport = FindLastImportPos(*fileNode);
    Position insertStart = {0, lastImport.line, 1};
    Range insertRange = {insertStart, insertStart};
    insertRange = TransformFromChar2IDE(insertRange);
    std::string insertContent = CONSTANTS::IMPORT + CONSTANTS::WHITE_SPACE + newPkg
                                + CONSTANTS::DOT + sym + "\n";
    std::string uri = kind == FileRefactorKind::RefactorMoveFile ? URI::URIFromAbsolutePath(targetPath).ToString()
                                                                 : URI::URIFromAbsolutePath(file).ToString();
    result.changes[uri].insert({FileRefactorChangeType::ADD, insertRange, insertContent});
}

void FileRefactor::DeleteImport()
{
    if (!fileNode || fileNode->imports.empty()) {
        return;
    }
    std::string uri = kind == FileRefactorKind::RefactorMoveFile ? URI::URIFromAbsolutePath(targetPath).ToString()
                                                                 : URI::URIFromAbsolutePath(file).ToString();
    // collect multi import
    std::vector<Ptr<ImportSpec>> multiImports;
    for (auto &fileImport: fileNode->imports) {
        if (!fileImport || fileImport->end.IsZero()) {
            continue;
        }
        if (fileImport.get()->content.kind == ImportKind::IMPORT_MULTI) {
            multiImports.emplace_back(fileImport);
        }
    }
    //delete multi import for RefactorMoveFile
    std::vector<Ptr<ImportSpec>> deleteMultiImports;
    if (kind == FileRefactorKind::RefactorMoveFile) {
        for (auto &multiImport: multiImports) {
            std::string multiImportFullPkg = GetImportFullPkg(multiImport->content);
            if (multiImportFullPkg != refactorPkg) {
                continue;
            }
            Position end = multiImport->content.rightCurlPos;
            end.column++;
            Range deleteRange = {multiImport->begin, end};
            deleteRange = TransformFromChar2IDE(deleteRange);
            result.changes[uri].insert({FileRefactorChangeType::DELETED, deleteRange, ""});
            deleteMultiImports.push_back(multiImport);
        }
    }
    // check whether single-import is within delete multi-import
    auto IsInDeleteMultiImport = [&deleteMultiImports](const ImportContent& singleImport) {
        return std::any_of(deleteMultiImports.begin(), deleteMultiImports.end(),
            [&singleImport](const auto multiImport) {
                return multiImport->begin <= singleImport.begin && multiImport->end >= singleImport.end;
            });
    };

    auto DeleteMoveFileSingleImport = [&](ImportContent& importContent, Ptr<ImportSpec> fileImport) {
        if (importContent.kind == ImportKind::IMPORT_MULTI) {
            return;
        }
        std::string importFullPkg = GetImportFullPkg(importContent);
        if (importFullPkg != refactorPkg || IsInDeleteMultiImport(importContent)) {
            return;
        }
        Position endPos;
        Position beginPos = fileImport->begin;
        if (GetDeletePosInMultiImport(multiImports, importContent, beginPos, endPos)) {
            Range deleteRange = {beginPos, endPos};
            deleteRange = TransformFromChar2IDE(deleteRange);
            result.changes[uri].insert({FileRefactorChangeType::DELETED, deleteRange, ""});
            return;
        }
        Range deleteRange = {fileImport->begin, fileImport->end};
        deleteRange = TransformFromChar2IDE(deleteRange);
        result.changes[uri].insert({FileRefactorChangeType::DELETED, deleteRange, ""});
    };

    auto DeleteRefFileSingleImport = [&](ImportContent& importContent, Ptr<ImportSpec> fileImport) {
        if (importContent.kind == ImportKind::IMPORT_MULTI || importContent.kind == ImportKind::IMPORT_ALL) {
            return;
        }
        std::string refactorFullSym = refactorPkg + CONSTANTS::DOT + sym;
        std::string importFullSym = GetImportFullSymWithoutAlias(importContent);
        if (importFullSym != refactorFullSym) {
            return;
        }
        Position endPos;
        Position beginPos = fileImport->begin;
        if (GetDeletePosInMultiImport(multiImports, importContent, beginPos, endPos)) {
            Range deleteRange = {beginPos, endPos};
            deleteRange = TransformFromChar2IDE(deleteRange);
            result.changes[uri].insert({FileRefactorChangeType::DELETED, deleteRange, ""});
            return;
        }
        Range deleteRange = {fileImport->begin, fileImport->end};
        deleteRange = TransformFromChar2IDE(deleteRange);
        result.changes[uri].insert({FileRefactorChangeType::DELETED, deleteRange, ""});
    };

    auto DeleteReExportSingleImport = [&](ImportContent& importContent, Ptr<ImportSpec> fileImport) {
        if (importContent.kind == ImportKind::IMPORT_MULTI || importContent.kind == ImportKind::IMPORT_ALL) {
            return;
        }
        std::string reExportFullSym = refactorPkg + CONSTANTS::DOT + sym;
        std::string reExportedFullSym = reExportedPkg + CONSTANTS::DOT + sym;
        std::string importFullSym = GetImportFullSymWithoutAlias(importContent);
        if (importFullSym != reExportFullSym && importFullSym != reExportedFullSym) {
            return;
        }
        Position endPos;
        Position beginPos = fileImport->begin;
        if (GetDeletePosInMultiImport(multiImports, importContent, beginPos, endPos)) {
            Range deleteRange = {beginPos, endPos};
            deleteRange = TransformFromChar2IDE(deleteRange);
            result.changes[uri].insert({FileRefactorChangeType::DELETED, deleteRange, ""});
            return;
        }
        Range deleteRange = {fileImport->begin, fileImport->end};
        deleteRange = TransformFromChar2IDE(deleteRange);
        result.changes[uri].insert({FileRefactorChangeType::DELETED, deleteRange, ""});
    };

    // delete single import
    for (auto &fileImport: fileNode->imports) {
        if (!fileImport || fileImport->end.IsZero()) {
            continue;
        }
        auto importContent = fileImport.get()->content;
        switch (kind) {
            case FileRefactorKind::RefactorMoveFile:
                DeleteMoveFileSingleImport(importContent, fileImport);
                break;
            case FileRefactorKind::RefactorRefFile:
                DeleteRefFileSingleImport(importContent, fileImport);
                break;
            case FileRefactorKind::RefactorReExport:
                DeleteReExportSingleImport(importContent, fileImport);
                break;
            default:
                break;
        }
    }
}

void FileRefactor::ChangeImport()
{
    if (!fileNode || fileNode->imports.empty()) {
        return;
    }
    Position lastImport = FindLastImportPos(*fileNode);
    std::string uri = URI::URIFromAbsolutePath(file).ToString();
    // collect multi import
    std::vector<Ptr<ImportSpec>> multiImports;
    for (auto &fileImport: fileNode->imports) {
        if (!fileImport || fileImport->end.IsZero()) {
            continue;
        }
        if (fileImport.get()->content.kind == ImportKind::IMPORT_MULTI) {
            multiImports.emplace_back(fileImport);
        }
    }

    auto DealImportAll = [&](const ImportSpec &importSpec) {
        if (moveDir) {
            auto importContent = importSpec.content;
            Position changeBegin = importContent.prefixPoses[0];
            Position changeEnd = importContent.prefixPoses[importContent.prefixPoses.size() - 1];
            changeEnd.column += static_cast<int>(importContent.prefixPaths[importContent.prefixPoses.size() - 1].length());
            Range changeRange = {changeBegin, changeEnd};
            changeRange = TransformFromChar2IDE(changeRange);
            result.changes[uri].insert({FileRefactorChangeType::CHANGED, changeRange, newPkg});
            return;
        }
        Position lastImport = FindLastImportPos(*fileNode);
        Position insertStart = {0, lastImport.line, 1};
        Range insertRange = {insertStart, insertStart};
        insertRange = TransformFromChar2IDE(insertRange);
        std::ostringstream insertContent;
        if (importSpec.modifier) {
            insertContent << importSpec.modifier->ToString() << CONSTANTS::WHITE_SPACE;
        }
        insertContent << CONSTANTS::IMPORT << CONSTANTS::WHITE_SPACE << newPkg
                    << CONSTANTS::DOT << sym + "\n";
        result.changes[uri].insert({FileRefactorChangeType::ADD, insertRange, insertContent.str()});
    };

    // change single import
    for (auto &fileImport: fileNode->imports) {
        if (!fileImport || fileImport->end.IsZero()) {
            continue;
        }
        auto importContent = fileImport.get()->content;
        if (importContent.kind == ImportKind::IMPORT_MULTI) {
            continue;
        }
        std::string refactorFullSym = refactorPkg + CONSTANTS::DOT + sym;
        std::string importFullSym = GetImportFullSymWithoutAlias(importContent);
        std::string importFullPkg = GetImportFullPkg(importContent);
        if (importContent.kind == ImportKind::IMPORT_ALL && importFullPkg == refactorPkg) {
            DealImportAll(*fileImport);
            continue;
        }
        if (importFullSym != refactorFullSym) {
            continue;
        }
        Position endPos;
        Position beginPos = fileImport->begin;
        if (GetDeletePosInMultiImport(multiImports, importContent, beginPos, endPos)) {
            // delete within multi-import
            Range deleteRange = {beginPos, endPos};
            deleteRange = TransformFromChar2IDE(deleteRange);
            result.changes[uri].insert({FileRefactorChangeType::DELETED, deleteRange, ""});
            // add single-import
            Position insertStart = {0, lastImport.line, 1};
            Range insertRange = {insertStart, insertStart};
            insertRange = TransformFromChar2IDE(insertRange);
            std::ostringstream oss;
            if (fileImport->modifier) {
                oss << fileImport->modifier->ToString() << CONSTANTS::WHITE_SPACE;
            }
            oss << CONSTANTS::IMPORT << CONSTANTS::WHITE_SPACE
                << newPkg << CONSTANTS::DOT << sym;
            if (importContent.kind == Cangjie::AST::ImportKind::IMPORT_ALIAS) {
                oss << CONSTANTS::WHITE_SPACE << CONSTANTS::AS
                    << CONSTANTS::WHITE_SPACE << importContent.aliasName.Val();
            }
            oss << "\n";
            std::string insertContent = oss.str();
            result.changes[uri].insert({FileRefactorChangeType::ADD, insertRange, insertContent});
            continue;
        }
        if (importContent.prefixPoses.empty()) {
            continue;
        }
        Position changeBegin = importContent.prefixPoses[0];
        Position changeEnd = importContent.prefixPoses[importContent.prefixPoses.size() -1];
        changeEnd.column += static_cast<int>(importContent.prefixPaths[importContent.prefixPoses.size() - 1].length());
        Range changeRange = {changeBegin, changeEnd};
        changeRange = TransformFromChar2IDE(changeRange);
        result.changes[uri].insert({FileRefactorChangeType::CHANGED, changeRange, newPkg});
    }
}

void FileRefactor::CheckAndAddImport()
{
    if (!ContainFullSymImport() && !accessForTargetPkg) {
        AddImport();
    }
}

void FileRefactor::CheckAndDeleteImport()
{
    if (ContainFullPkgImport()) {
        DeleteImport();
    }
}

void FileRefactor::CheckAndChangeImport()
{
    std::string refFullPkg = CompilerCangjieProject::GetInstance()->GetFullPkgName(file);
    bool isSamePkg = FileRefactor::GetPackageRelation(refactorPkg, refFullPkg) == PackageRelation::SAME_PACKAGE;
    if (isSamePkg && !ContainFullSymImport()) {
        AddImport();
        return;
    }
    if (ContainFullPkgImport()) {
        ChangeImport();
    }
}

void FileRefactor::CheckAndDeleteImportForRe()
{
    if (ContainFullSymImportForReExport()) {
        DeleteImport();
    }
}

void FileRefactor::CheckAndChangeImportForRe()
{
    std::string refFullPkg = CompilerCangjieProject::GetInstance()->GetFullPkgName(file);
    bool isSamePkg = FileRefactor::GetPackageRelation(refactorPkg, refFullPkg) == PackageRelation::SAME_PACKAGE;
    if (isSamePkg && !ContainFullSymImportForReExport()) {
        AddImport();
        return;
    }
    if (ContainFullPkgImport()) {
        ChangeImport();
    }
}

void FileRefactor::InitMatcher() const
{
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorMoveFile, PackageRelation::CHILD,
            ark::lsp::Modifier::INTERNAL), &FileRefactor::CheckAndAddImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorMoveFile, PackageRelation::CHILD,
            ark::lsp::Modifier::PROTECTED), &FileRefactor::CheckAndAddImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorMoveFile, PackageRelation::CHILD,
            ark::lsp::Modifier::PUBLIC), &FileRefactor::CheckAndAddImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorMoveFile, PackageRelation::PARENT,
            ark::lsp::Modifier::PROTECTED), &FileRefactor::CheckAndAddImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorMoveFile, PackageRelation::PARENT,
            ark::lsp::Modifier::PUBLIC), &FileRefactor::CheckAndAddImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorMoveFile, PackageRelation::SAME_PACKAGE,
            ark::lsp::Modifier::UNDEFINED), &FileRefactor::DeleteImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorMoveFile, PackageRelation::SAME_MODULE,
            ark::lsp::Modifier::PROTECTED), &FileRefactor::CheckAndAddImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorMoveFile, PackageRelation::SAME_MODULE,
            ark::lsp::Modifier::PUBLIC), &FileRefactor::CheckAndAddImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorRefFile, PackageRelation::CHILD,
            ark::lsp::Modifier::INTERNAL), &FileRefactor::CheckAndChangeImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorRefFile, PackageRelation::CHILD,
            ark::lsp::Modifier::PROTECTED), &FileRefactor::CheckAndChangeImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorRefFile, PackageRelation::CHILD,
            ark::lsp::Modifier::PUBLIC), &FileRefactor::CheckAndChangeImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorRefFile, PackageRelation::PARENT,
            ark::lsp::Modifier::PROTECTED), &FileRefactor::CheckAndChangeImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorRefFile, PackageRelation::PARENT,
            ark::lsp::Modifier::PUBLIC), &FileRefactor::CheckAndChangeImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorRefFile, PackageRelation::SAME_PACKAGE,
            ark::lsp::Modifier::INTERNAL), &FileRefactor::CheckAndDeleteImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorRefFile, PackageRelation::SAME_PACKAGE,
            ark::lsp::Modifier::PROTECTED), &FileRefactor::CheckAndDeleteImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorRefFile, PackageRelation::SAME_PACKAGE,
            ark::lsp::Modifier::PUBLIC), &FileRefactor::CheckAndDeleteImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorRefFile, PackageRelation::SAME_MODULE,
            ark::lsp::Modifier::PROTECTED), &FileRefactor::CheckAndChangeImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorRefFile, PackageRelation::SAME_MODULE,
            ark::lsp::Modifier::PUBLIC), &FileRefactor::CheckAndChangeImport);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorReExport, PackageRelation::CHILD,
            ark::lsp::Modifier::INTERNAL), &FileRefactor::CheckAndChangeImportForRe);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorReExport, PackageRelation::CHILD,
            ark::lsp::Modifier::PROTECTED), &FileRefactor::CheckAndChangeImportForRe);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorReExport, PackageRelation::CHILD,
            ark::lsp::Modifier::PUBLIC), &FileRefactor::CheckAndChangeImportForRe);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorReExport, PackageRelation::PARENT,
            ark::lsp::Modifier::PROTECTED), &FileRefactor::CheckAndChangeImportForRe);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorReExport, PackageRelation::PARENT,
            ark::lsp::Modifier::PUBLIC), &FileRefactor::CheckAndChangeImportForRe);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorReExport, PackageRelation::SAME_PACKAGE,
            ark::lsp::Modifier::INTERNAL), &FileRefactor::CheckAndDeleteImportForRe);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorReExport, PackageRelation::SAME_PACKAGE,
            ark::lsp::Modifier::PROTECTED), &FileRefactor::CheckAndDeleteImportForRe);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorReExport, PackageRelation::SAME_PACKAGE,
            ark::lsp::Modifier::PUBLIC), &FileRefactor::CheckAndDeleteImportForRe);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorReExport, PackageRelation::SAME_MODULE,
            ark::lsp::Modifier::PROTECTED), &FileRefactor::CheckAndChangeImportForRe);
    RefactorMatcher::GetInstance().RegFunc(
        GetRefactorCode(FileRefactorKind::RefactorReExport, PackageRelation::SAME_MODULE,
            ark::lsp::Modifier::PUBLIC), &FileRefactor::CheckAndChangeImportForRe);
}

void FileRefactor::MatchRefactor(FileRefactorKind fileRefactorKind, PackageRelation packageRelation,
    ark::lsp::Modifier modifier)
{
    uint8_t operatorCode = GetRefactorCode(fileRefactorKind, packageRelation, modifier);
    auto func = RefactorMatcher::GetInstance().GetFunc(operatorCode);
    if (func) {
        (this->*(func))();
    }
}

std::string FileRefactor::GetImportFullPkg(const ImportContent &importContent)
{
    std::stringstream ss;
    for (const auto &prefix: importContent.prefixPaths) {
        ss << prefix << CONSTANTS::DOT;
    }
    if (importContent.kind != ImportKind::IMPORT_MULTI && importContent.kind != ImportKind::IMPORT_ALL
        && !importContent.isDecl) {
        ss << importContent.identifier << CONSTANTS::DOT;
    }
    return ss.str().substr(0, ss.str().size() - 1);
}

std::string FileRefactor::GetImportFullSym(const ImportContent &importContent)
{
    std::stringstream ss;
    for (int i = 0; i < importContent.prefixPaths.size(); ++i) {
        ss << importContent.prefixPaths[i];
        if (i != importContent.prefixPaths.size() - 1) {
            ss << CONSTANTS::DOT;
        }
    }
    if (importContent.kind != ImportKind::IMPORT_MULTI) {
        ss << CONSTANTS::DOT << importContent.identifier.Val();
    }
    if (importContent.kind == ImportKind::IMPORT_ALIAS) {
        ss << CONSTANTS::WHITE_SPACE << CONSTANTS::AS << CONSTANTS::WHITE_SPACE << importContent.aliasName.Val();
    }
    return ss.str();
}

std::string FileRefactor::GetImportFullSymWithoutAlias(const ImportContent &importContent)
{
    std::stringstream ss;
    for (int i = 0; i < importContent.prefixPaths.size(); ++i) {
        ss << importContent.prefixPaths[i];
        if (i != importContent.prefixPaths.size() - 1) {
            ss << CONSTANTS::DOT;
        }
    }
    if (importContent.kind != ImportKind::IMPORT_MULTI) {
        ss << CONSTANTS::DOT << importContent.identifier.Val();
    }
    return ss.str();
}

bool FileRefactor::GetDeletePosInMultiImport(std::vector<Ptr<ImportSpec>> &multiImports,
        ImportContent &singleImport, Position &beginPos, Position &endPos)
{
    std::string uri = kind == FileRefactorKind::RefactorMoveFile ? URI::URIFromAbsolutePath(targetPath).ToString()
                                                                 : URI::URIFromAbsolutePath(file).ToString();
    for (const auto &multiImport: multiImports) {
        if (!(multiImport->begin <= singleImport.begin && multiImport->end >= singleImport.end)) {
            continue;
        }
        // check whether to delete the entire multi-import
        Position multiImportEnd = multiImport->content.rightCurlPos;
        multiImportEnd.column++;
        size_t deleteCount = 0;
        for (const auto&change: result.changes[uri]) {
            if (change.type != FileRefactorChangeType::DELETED) {
                continue;
            }
            Range range = TransformFromIDE2Char(change.range);
            if (range.start == multiImport->begin && range.end == multiImportEnd) {
                beginPos = multiImport->begin;
                endPos = multiImportEnd;
                return true;
            }
            if (multiImport->begin <= range.start && multiImport->end >= range.end) {
                deleteCount++;
            }
        }
        if (deleteCount == multiImport->content.items.size() - 1) {
            beginPos = multiImport->begin;
            endPos = multiImportEnd;
            RemoveDuplicateDelete(beginPos, endPos, uri);
            return true;
        }

        endPos = singleImport.end;
        if (multiImport->content.commaPoses.empty()) {
            return true;
        }
        auto commaPoses = multiImport->content.commaPoses;
        std::sort(commaPoses.begin(), commaPoses.end());
        for (const auto& commaPos: commaPoses) {
            if (commaPos >= singleImport.end) {
                endPos = commaPos;
                endPos.column++;
                return true;
            }
        }
        return true;
    }
    return false;
}

void FileRefactor::RemoveDuplicateDelete(Position start, Position end, std::string &uri)
{
    for (auto it = result.changes[uri].begin(); it != result.changes[uri].end(); ) {
        Range range = TransformFromIDE2Char(it->range);
        if (it->type == FileRefactorChangeType::DELETED && start <= range.start && end >= range.end) {
            it = result.changes[uri].erase(it);
        } else {
            ++it;
        }
    }
}

uint8_t FileRefactor::GetRefactorCode(FileRefactorKind fileRefactorKind, PackageRelation packageRelation,
    ark::lsp::Modifier modifier) const
{
    return (static_cast<uint8_t>(fileRefactorKind) << 4)
           + (static_cast<uint8_t>(modifier) << 2)
           + static_cast<uint8_t>(packageRelation);
}

bool FileRefactor::ContainFullSymImport()
{
    if (!fileNode) {
        return false;
    }
    std::string refactorFullSym = refactorPkg + CONSTANTS::DOT + sym;
    if (kind == FileRefactorKind::RefactorMoveFile) {
        return FileContainFullSymImport(fileNode, true, refactorFullSym);
    }
    if (!fileNode->curPackage) {
        return false;
    }
    bool isContain = false;
    for (const auto&f: fileNode->curPackage->files) {
        isContain = FileContainFullSymImport(f.get(), f->filePath == file, refactorFullSym);
        if (isContain) {
            return true;
        }
    }
    return false;
}

bool FileRefactor::FileContainFullSymImport(const File *pFile, bool isRefFile, std::string refactorFullSym)
{
    for (auto &fileImport: pFile->imports) {
        if (!fileImport || fileImport->end.IsZero()) {
            continue;
        }
        auto importContent = fileImport.get()->content;
        if (importContent.kind == ImportKind::IMPORT_MULTI) {
            continue;
        }
        std::string importFullPkg = GetImportFullPkg(importContent);
        std::string importFullSym = GetImportFullSymWithoutAlias(importContent);
        bool isContain1 =
            isRefFile && ((importContent.kind == ImportKind::IMPORT_ALL && importFullPkg == refactorPkg) ||
                            (importFullSym == refactorFullSym));
        if (isContain1) {
            return true;
        }
        bool isContain2 = !isRefFile && fileImport->modifier &&
                          ((importContent.kind == ImportKind::IMPORT_ALL && importFullPkg == refactorPkg &&
                                FileRefactor::ReExportKinds.count(fileImport->modifier->modifier)) ||
                                (importFullSym == refactorFullSym &&
                                    FileRefactor::ReExportKinds.count(fileImport->modifier->modifier)));
        if (isContain2) {
            return true;
        }
    }
    return false;
}

bool FileRefactor::ContainFullSymImportForReExport()
{
    if (!fileNode) {
        return false;
    }
    if (!fileNode->curPackage) {
        return false;
    }

    std::string refactorFullSym = refactorPkg + CONSTANTS::DOT + sym;
    std::string originFullSym = reExportedPkg + CONSTANTS::DOT + sym;
    bool isContain = false;
    for (const auto &f: fileNode->curPackage->files) {
        isContain = FileContainFullSymImportForReExport(f.get(), f->filePath == file,
            refactorFullSym, originFullSym);
        if (isContain) {
            return true;
        }
    }
    return false;
}

bool FileRefactor::FileContainFullSymImportForReExport(const File *pFile, bool isRefFile,
    std::string refactorFullSym, std::string originFullSym)
{
    for (auto &fileImport: fileNode->imports) {
        if (!fileImport || fileImport->end.IsZero()) {
            continue;
        }
        auto importContent = fileImport.get()->content;
        if (importContent.kind == ImportKind::IMPORT_MULTI) {
            continue;
        }
        std::string importFullPkg = GetImportFullPkg(importContent);
        std::string importFullSym = GetImportFullSymWithoutAlias(importContent);
        bool isContain1 = isRefFile && ((importContent.kind == ImportKind::IMPORT_ALL &&
                                            (importFullPkg == refactorPkg || importFullPkg == reExportedPkg)) ||
                                            (importFullSym == refactorFullSym || importFullSym == originFullSym));
        if (isContain1) {
            return true;
        }
        bool isContain2 = !isRefFile && fileImport->modifier &&
                           FileRefactor::ReExportKinds.count(fileImport->modifier->modifier) &&
                           ((importContent.kind == ImportKind::IMPORT_ALL &&
                                (importFullPkg == refactorPkg || importFullPkg == reExportedPkg)) ||
                                (importFullSym == refactorFullSym || importFullSym == originFullSym));
        if (isContain2) {
            return true;
        }
    }
    return false;
}

bool FileRefactor::ContainFullPkgImport()
{
    if (!fileNode) {
        return false;
    }
    for (auto &fileImport: fileNode->imports) {
        if (!fileImport || fileImport->end.IsZero()) {
            continue;
        }
        auto importContent = fileImport.get()->content;
        if (importContent.kind == ImportKind::IMPORT_MULTI) {
            continue;
        }
        std::string importFullPkg = GetImportFullPkg(importContent);
        if (importFullPkg == refactorPkg) {
            return true;
        }
        if (importFullPkg.length() > refactorPkg.length()
            && importFullPkg.substr(0, refactorPkg.length()) == refactorPkg
            && importFullPkg[refactorPkg.length()] == '.') {
                return true;
            }
    }
    return false;
}

PackageRelation FileRefactor::GetPackageRelation(const std::string &fullPkgName,
    const std::string &targetFullPkgName)
{
    if (fullPkgName == targetFullPkgName) {
        return PackageRelation::SAME_PACKAGE;
    }
    if (fullPkgName.length() < targetFullPkgName.length()
        && targetFullPkgName.find(fullPkgName) == 0
        && targetFullPkgName[fullPkgName.length()] == '.') {
        return PackageRelation::PARENT;
    }
    if (fullPkgName.length() > targetFullPkgName.length()
        && fullPkgName.find(targetFullPkgName) == 0
        && fullPkgName[targetFullPkgName.length()] == '.') {
            return PackageRelation::CHILD;
    }
    auto srcRoot = Cangjie::Utils::SplitQualifiedName(fullPkgName).front();
    auto targetRoot = Cangjie::Utils::SplitQualifiedName(targetFullPkgName).front();
    return srcRoot == targetRoot ? PackageRelation::SAME_MODULE : PackageRelation::DIFF_MODULE;
}

lsp::Modifier FileRefactor::GetImportModifier(ImportSpec &importSpec)
{
    if (!importSpec.modifier) {
        return lsp::Modifier::UNDEFINED;
    }
    switch (importSpec.modifier->modifier) {
        case TokenKind::PRIVATE:
            return lsp::Modifier::PRIVATE;
        case TokenKind::INTERNAL:
            return lsp::Modifier::INTERNAL;
        case TokenKind::PROTECTED:
            return lsp::Modifier::PROTECTED;
        case TokenKind::PUBLIC:
            return lsp::Modifier::PUBLIC;
        default:
            return lsp::Modifier::UNDEFINED;
    }
}

bool FileRefactor::ValidReExportModifier(lsp::Modifier modifier)
{
    return modifier == lsp::Modifier::INTERNAL
        || modifier == lsp::Modifier::PROTECTED
        || modifier == lsp::Modifier::PUBLIC;
}
} // namespace ark