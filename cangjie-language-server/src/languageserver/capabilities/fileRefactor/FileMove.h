// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


#ifndef CANGJIE_LSP_FILEMOVE_H
#define CANGJIE_LSP_FILEMOVE_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "../../CompilerCangjieProject.h"
#include "../../index/Symbol.h"
#include "../../common/Utils.h"
#include "FileRefactor.h"

namespace ark {
class FileMove {
public:
    static void FileMoveRefactor(const ArkAST *ast, FileRefactorRespParams &result, const std::string &file, const std::string &selectedElement, const std::string &target);

private:
    static void FindFileRefactor(const ArkAST *ast, const std::string &file, const std::string &targetPkg, const std::string & targetPath, FileRefactorRespParams &result);

    static void DealMoveFile(const ArkAST *ast, const std::string &file, const std::string &targetPkg, const std::string &targetPath, FileRefactor &refactor);

    static void DealRefFile(const ArkAST *ast, const std::string &file, const std::string &targetPkg, FileRefactor &refactor);

    static void DealReExport(const ArkAST *ast, const std::string &file, const std::string &targetPkg, FileRefactor &refactor);

    static void DealMoveFilePackageName(const ArkAST *ast, const std::string &targetPkg, const std::string &targetPath, FileRefactorRespParams &result);

    static std::string GetFullPkgBySymScope(const std::string &symScope);

    static ArkAST* GetRefArkAST(const std::string &filePath);

    static std::unique_ptr<PackageInstance> GetPackageInstance(LSPCompilerInstance *ci);

    static std::unique_ptr<ArkAST> CreateArkAST(LSPCompilerInstance *ci, PackageInstance *pkgInstance, const std::string &filePath);

    static void Clear();

    static std::string GetRealImportSymName(const lsp::Symbol &sym);

    static bool IsValidExportSym(const lsp::Symbol &sym, const std::string &exportedPkg);

    static bool IsValidExportSym(const lsp::Symbol &sym, const std::string &exportedPkg, const std::string &fullPkgSym);

    static std::string GetPkgNameAfterMove(const std::string &pathBeforeMove, std::string pkgBeforeMove);

    static File* GetFileNode(const ArkAST *ast, std::string filePath);

    static bool ExistImportForTargetPkg(lsp::SymbolID symbolID, std::string targetPkg, std::string moveFile);

    static std::string GetTargetPath(std::string file);

    static std::unordered_map<std::string, std::unique_ptr<Cangjie::LSPCompilerInstance>> ciMap;

    static std::unordered_map<std::string, std::unique_ptr<ArkAST>> astMap;

    static std::unordered_map<std::string, std::unique_ptr<PackageInstance>> pkgInstanceMap;

    // if moving a folder, store the move dir
    static std::string moveDirPath;

    static std::string targetDir;
};
}


#endif // CANGJIE_LSP_FILEMOVE_H
