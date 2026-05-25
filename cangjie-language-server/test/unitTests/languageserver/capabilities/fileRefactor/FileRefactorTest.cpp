// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "FileRefactor.h"

using namespace ark;

// FileRefactor constructor test
TEST(FileRefactorTest, Constructor) {
    FileRefactorRespParams result;
    FileRefactor refactor(result);

    // Verify initial state
    EXPECT_FALSE(refactor.moveDir);
    EXPECT_FALSE(refactor.accessForTargetPkg);
    EXPECT_EQ(refactor.fileNode, nullptr);
    EXPECT_TRUE(refactor.file.empty());
    EXPECT_TRUE(refactor.refactorPkg.empty());
    EXPECT_TRUE(refactor.newPkg.empty());
    EXPECT_TRUE(refactor.sym.empty());
    EXPECT_TRUE(refactor.reExportedPkg.empty());
    EXPECT_TRUE(refactor.targetPath.empty());
}

// Test GetImportFullPkg with IMPORT_MULTI
TEST(FileRefactorTest, GetImportFullPkgWithImportMulti) {
    ImportContent content;
    content.kind = ImportKind::IMPORT_MULTI;
    content.prefixPaths = {"com", "example"};

    std::string result = FileRefactor::GetImportFullPkg(content);
    EXPECT_EQ(result, "com.example");
}

// Test GetImportFullPkg with IMPORT_ALL
TEST(FileRefactorTest, GetImportFullPkgWithImportAll) {
    ImportContent content;
    content.kind = ImportKind::IMPORT_ALL;
    content.prefixPaths = {"com", "example"};
    content.isDecl = false;

    std::string result = FileRefactor::GetImportFullPkg(content);
    EXPECT_EQ(result, "com.example");
}

// Test GetImportFullPkg with normal import
TEST(FileRefactorTest, GetImportFullPkgWithNormalImport) {
    ImportContent content;
    content.kind = ImportKind::IMPORT_SINGLE;
    content.prefixPaths = {"com", "example"};
    content.identifier = Identifier();
    content.isDecl = false;

    std::string result = FileRefactor::GetImportFullPkg(content);
    EXPECT_EQ(result, "com.example., begin = (0, 0, 0), end = (0, 0, 0)");
}

// Test GetImportFullSym with IMPORT_MULTI
TEST(FileRefactorTest, GetImportFullSymWithImportMulti) {
    ImportContent content;
    content.kind = ImportKind::IMPORT_MULTI;
    content.prefixPaths = {"com", "example"};

    std::string result = FileRefactor::GetImportFullSym(content);
    EXPECT_EQ(result, "com.example");
}

// Test GetImportFullSym with IMPORT_ALIAS
TEST(FileRefactorTest, GetImportFullSymWithImportAlias) {
    ImportContent content;
    content.kind = ImportKind::IMPORT_ALIAS;
    content.prefixPaths = {"com", "example"};
    content.identifier = Identifier();
    content.aliasName = Identifier();

    std::string result = FileRefactor::GetImportFullSym(content);
    EXPECT_EQ(result, "com.example. as ");
}


// Test GetImportFullSymWithoutAlias
TEST(FileRefactorTest, GetImportFullSymWithoutAlias) {
    ImportContent content;
    content.kind = ImportKind::IMPORT_ALIAS;
    content.prefixPaths = {"com", "example"};
    content.identifier = Identifier();
    content.aliasName = Identifier();

    std::string result = FileRefactor::GetImportFullSymWithoutAlias(content);
    EXPECT_EQ(result, "com.example.");
}

// Test GetPackageRelation - SAME_PACKAGE
TEST(FileRefactorTest, GetPackageRelationSamePackage) {
    PackageRelation result = FileRefactor::GetPackageRelation("com.example", "com.example");
    EXPECT_EQ(result, PackageRelation::SAME_PACKAGE);
}

// Test GetPackageRelation - PARENT
TEST(FileRefactorTest, GetPackageRelationParent) {
    PackageRelation result = FileRefactor::GetPackageRelation("com", "com.example");
    EXPECT_EQ(result, PackageRelation::PARENT);
}

// Test GetPackageRelation - CHILD
TEST(FileRefactorTest, GetPackageRelationChild) {
    PackageRelation result = FileRefactor::GetPackageRelation("com.example", "com");
    EXPECT_EQ(result, PackageRelation::CHILD);
}

// Test GetPackageRelation - SAME_MODULE
TEST(FileRefactorTest, GetPackageRelationSameModule) {
    PackageRelation result = FileRefactor::GetPackageRelation("com.example1", "com.example2");
    EXPECT_EQ(result, PackageRelation::SAME_MODULE);
}

// Test GetPackageRelation - DIFF_MODULE
TEST(FileRefactorTest, GetPackageRelationDiffModule) {
    PackageRelation result = FileRefactor::GetPackageRelation("com.example", "org.example");
    EXPECT_EQ(result, PackageRelation::DIFF_MODULE);
}

// Test GetImportModifier without modifier
TEST(FileRefactorTest, GetImportModifierWithoutModifier) {
    ImportSpec spec;
    spec.modifier = nullptr;

    lsp::Modifier result = FileRefactor::GetImportModifier(spec);
    EXPECT_EQ(result, lsp::Modifier::UNDEFINED);
}

// Test ValidReExportModifier - INTERNAL
TEST(FileRefactorTest, ValidReExportModifierInternal) {
    bool result = FileRefactor::ValidReExportModifier(lsp::Modifier::INTERNAL);
    EXPECT_TRUE(result);
}

// Test ValidReExportModifier - PROTECTED
TEST(FileRefactorTest, ValidReExportModifierProtected) {
    bool result = FileRefactor::ValidReExportModifier(lsp::Modifier::PROTECTED);
    EXPECT_TRUE(result);
}

// Test ValidReExportModifier - PUBLIC
TEST(FileRefactorTest, ValidReExportModifierPublic) {
    bool result = FileRefactor::ValidReExportModifier(lsp::Modifier::PUBLIC);
    EXPECT_TRUE(result);
}

// Test ValidReExportModifier - PRIVATE
TEST(FileRefactorTest, ValidReExportModifierPrivate) {
    bool result = FileRefactor::ValidReExportModifier(lsp::Modifier::PRIVATE);
    EXPECT_FALSE(result);
}
