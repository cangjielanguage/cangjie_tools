// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "ItemResolverUtil.cpp"
#include <string>

using namespace ark;
using namespace Cangjie::AST;

// Test ResolveNameByNode function
TEST(ItemResolverUtilTest, ResolveNameByNode_MacroExpandDeclWithInvocation) {
    // Create MacroExpandDecl node with invocation declaration
    auto macroExpandDecl = OwnedPtr<MacroExpandDecl>(new MacroExpandDecl());
    auto invocationDecl = OwnedPtr<FuncDecl>(new FuncDecl());
    invocationDecl->identifier = "testFunction";
    macroExpandDecl->invocation.decl = std::move(invocationDecl);

    std::string result = ItemResolverUtil::ResolveNameByNode(*macroExpandDecl);
    EXPECT_EQ("testFunction", result);
}

TEST(ItemResolverUtilTest, ResolveNameByNode_MacroExpandDeclWithoutInvocation) {
    // Create MacroExpandDecl node without invocation declaration
    auto macroExpandDecl = OwnedPtr<MacroExpandDecl>(new MacroExpandDecl());
    macroExpandDecl->invocation.decl = nullptr;

    std::string result = ItemResolverUtil::ResolveNameByNode(*macroExpandDecl);
    EXPECT_EQ("", result);
}

TEST(ItemResolverUtilTest, ResolveNameByNode_DeclNode) {
    // Create Decl node
    auto decl = OwnedPtr<FuncDecl>(new FuncDecl());
    decl->identifier = "testFunction";

    std::string result = ItemResolverUtil::ResolveNameByNode(*decl);
    EXPECT_EQ("testFunction", result);
}

TEST(ItemResolverUtilTest, ResolveNameByNode_PackageNode) {
    // Create Package node
    auto package = OwnedPtr<Package>(new Package());
    package->fullPackageName = "test.package";

    std::string result = ItemResolverUtil::ResolveNameByNode(*package);
    EXPECT_EQ("test.package", result);
}

TEST(ItemResolverUtilTest, ResolveNameByNode_UnknownNode) {
    // Create unknown type node
    auto node = OwnedPtr<Node>(new Node());

    std::string result = ItemResolverUtil::ResolveNameByNode(*node);
    EXPECT_EQ("", result);
}

// Test ResolveKindByNode function
TEST(ItemResolverUtilTest, ResolveKindByNode_VarDecl) {
    // Create VarDecl node
    auto varDecl = OwnedPtr<VarDecl>(new VarDecl());

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*varDecl);
    EXPECT_EQ(CompletionItemKind::CIK_VARIABLE, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_FuncDeclConstructor) {
    // Create constructor FuncDecl node
    auto funcDecl = OwnedPtr<FuncDecl>(new FuncDecl());
    funcDecl->EnableAttr(Attribute::CONSTRUCTOR);

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*funcDecl);
    EXPECT_EQ(CompletionItemKind::CIK_CONSTRUCTOR, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_FuncDeclMethod) {
    // Create normal function FuncDecl node
    auto funcDecl = OwnedPtr<FuncDecl>(new FuncDecl());

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*funcDecl);
    EXPECT_EQ(CompletionItemKind::CIK_METHOD, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_ClassDecl) {
    // Create ClassDecl node
    auto classDecl = OwnedPtr<ClassDecl>(new ClassDecl());

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*classDecl);
    EXPECT_EQ(CompletionItemKind::CIK_CLASS, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_InterfaceDecl) {
    // Create InterfaceDecl node
    auto interfaceDecl = OwnedPtr<InterfaceDecl>(new InterfaceDecl());

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*interfaceDecl);
    EXPECT_EQ(CompletionItemKind::CIK_INTERFACE, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_EnumDecl) {
    // Create EnumDecl node
    auto enumDecl = OwnedPtr<EnumDecl>(new EnumDecl());

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*enumDecl);
    EXPECT_EQ(CompletionItemKind::CIK_ENUM, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_StructDecl) {
    // Create StructDecl node
    auto structDecl = OwnedPtr<StructDecl>(new StructDecl());

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*structDecl);
    EXPECT_EQ(CompletionItemKind::CIK_STRUCT, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_TypeAliasDecl) {
    // Create TypeAliasDecl node
    auto typeAliasDecl = OwnedPtr<TypeAliasDecl>(new TypeAliasDecl());

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*typeAliasDecl);
    EXPECT_EQ(CompletionItemKind::CIK_CLASS, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_MacroExpandDeclWithInvocation) {
    // Create MacroExpandDecl node with invocation declaration
    auto macroExpandDecl = OwnedPtr<MacroExpandDecl>(new MacroExpandDecl());
    auto invocationDecl = OwnedPtr<FuncDecl>(new FuncDecl());
    invocationDecl->identifier = "testFunction";
    macroExpandDecl->invocation.decl = std::move(invocationDecl);

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*macroExpandDecl);
    EXPECT_EQ(CompletionItemKind::CIK_METHOD, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_MacroExpandDeclWithoutInvocation) {
    // Create MacroExpandDecl node without invocation declaration
    auto macroExpandDecl = OwnedPtr<MacroExpandDecl>(new MacroExpandDecl());
    macroExpandDecl->invocation.decl = nullptr;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*macroExpandDecl);
    EXPECT_EQ(CompletionItemKind::CIK_MISSING, result);
}

// Test ResolveKindByASTKind function
TEST(ItemResolverUtilTest, ResolveKindByASTKind_VarDecl) {
    // Test VAR_DECL type
    ASTKind astKind = ASTKind::VAR_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_VARIABLE, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_FuncDecl) {
    // Test FUNC_DECL type
    ASTKind astKind = ASTKind::FUNC_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_METHOD, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_ClassDecl) {
    // Test CLASS_DECL type
    ASTKind astKind = ASTKind::CLASS_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_CLASS, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_InterfaceDecl) {
    // Test INTERFACE_DECL type
    ASTKind astKind = ASTKind::INTERFACE_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_INTERFACE, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_EnumDecl) {
    // Test ENUM_DECL type
    ASTKind astKind = ASTKind::ENUM_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_ENUM, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_StructDecl) {
    // Test STRUCT_DECL type
    ASTKind astKind = ASTKind::STRUCT_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_STRUCT, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_TypeAliasDecl) {
    // Test TYPE_ALIAS_DECL type
    ASTKind astKind = ASTKind::TYPE_ALIAS_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_CLASS, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_Unknown) {
    // Test unknown type
    ASTKind astKind = ASTKind::NODE;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_MISSING, result);
}

// Test GetGenericParamByDecl function
TEST(ItemResolverUtilTest, GetGenericParamByDecl_NullGeneric) {
    // Test null Generic declaration
    Ptr<Generic> genericDecl = nullptr;

    std::string result = ItemResolverUtil::GetGenericParamByDecl(genericDecl);
    EXPECT_EQ("", result);
}

TEST(ItemResolverUtilTest, GetGenericParamByDecl_EmptyGeneric) {
    // Test empty Generic declaration (no type parameters)
    auto genericDecl = OwnedPtr<Generic>(new Generic());

    std::string result = ItemResolverUtil::GetGenericParamByDecl(genericDecl);
    EXPECT_EQ("<>", result);
}

TEST(ItemResolverUtilTest, GetGenericParamByDecl_SingleGenericParam) {
    // Test Generic declaration with single type parameter
    auto genericDecl = OwnedPtr<Generic>(new Generic());
    auto param = OwnedPtr<GenericParamDecl>(new GenericParamDecl());
    param->identifier = "T";
    genericDecl->typeParameters.emplace_back(std::move(param));

    std::string result = ItemResolverUtil::GetGenericParamByDecl(genericDecl);
    EXPECT_EQ("<T>", result);
}

TEST(ItemResolverUtilTest, GetGenericParamByDecl_MultipleGenericParams) {
    // Test Generic declaration with multiple type parameters
    auto genericDecl = OwnedPtr<Generic>(new Generic());
    auto param1 = OwnedPtr<GenericParamDecl>(new GenericParamDecl());
    param1->identifier = "T";
    auto param2 = OwnedPtr<GenericParamDecl>(new GenericParamDecl());
    param2->identifier = "U";
    genericDecl->typeParameters.emplace_back(std::move(param1));
    genericDecl->typeParameters.emplace_back(std::move(param2));

    std::string result = ItemResolverUtil::GetGenericParamByDecl(genericDecl);
    EXPECT_EQ("<T, U>", result);
}

// Test ResolveSignatureByNode function
TEST(ItemResolverUtilTest, ResolveSignatureByNode_VarDecl) {
    // Test signature resolution for VarDecl node
    auto varDecl = OwnedPtr<VarDecl>(new VarDecl());
    varDecl->identifier = "testVar";

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*varDecl, nullptr);
    EXPECT_EQ("testVar", result);
}

TEST(ItemResolverUtilTest, ResolveSignatureByNode_FuncDecl) {
    // Test signature resolution for FuncDecl node
    auto funcDecl = OwnedPtr<FuncDecl>(new FuncDecl());
    funcDecl->identifier = "testFunction";

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*funcDecl, nullptr);
    EXPECT_EQ("", result);
}

TEST(ItemResolverUtilTest, ResolveSignatureByNode_ClassDecl) {
    // Test signature resolution for ClassDecl node
    auto classDecl = OwnedPtr<ClassDecl>(new ClassDecl());
    classDecl->identifier = "TestClass";

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*classDecl, nullptr);
    EXPECT_EQ("TestClass", result);
}

TEST(ItemResolverUtilTest, ResolveSignatureByNode_ClassDeclWithGeneric) {
    // Test signature resolution for ClassDecl node with generic parameters
    auto classDecl = OwnedPtr<ClassDecl>(new ClassDecl());
    classDecl->identifier = "TestClass";
    auto generic = OwnedPtr<Generic>(new Generic());
    auto param = OwnedPtr<GenericParamDecl>(new GenericParamDecl());
    param->identifier = "T";
    generic->typeParameters.emplace_back(std::move(param));
    classDecl->generic = std::move(generic);

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*classDecl, nullptr);
    EXPECT_EQ("TestClass<T>", result);
}

// Test IsCustomAnnotation function
TEST(ItemResolverUtilTest, IsCustomAnnotation_NoOuterDecl) {
    // Test case where there is no outer declaration
    FuncDecl decl;

    bool result = ItemResolverUtil::IsCustomAnnotation(decl);
    EXPECT_FALSE(result);
}

TEST(ItemResolverUtilTest, IsCustomAnnotation_NoAnnotations) {
    // Test case where outer declaration has no annotations
    FuncDecl decl;
    auto outerDecl = OwnedPtr<FuncDecl>(new FuncDecl());
    decl.outerDecl = outerDecl.get();

    bool result = ItemResolverUtil::IsCustomAnnotation(decl);
    EXPECT_FALSE(result);
}

TEST(ItemResolverUtilTest, IsCustomAnnotation_WithCustomAnnotation) {
    // Test case with custom annotation
    FuncDecl decl;
    auto outerDecl = OwnedPtr<FuncDecl>(new FuncDecl());
    auto annotation = OwnedPtr<Annotation>(new Annotation("TestAnnotation", AnnotationKind::CUSTOM, Position{1, 1, 1}));
    outerDecl->annotations.emplace_back(std::move(annotation));
    decl.outerDecl = outerDecl.get();

    bool result = ItemResolverUtil::IsCustomAnnotation(decl);
    EXPECT_TRUE(result);
}

// Test ResolveNameByNode function - MacroExpandDecl path
TEST(ItemResolverUtilTest, ResolveNameByNode_MacroExpandDeclWithDecl) {
    // Create MacroExpandDecl node with valid declaration
    auto macroExpandDecl = OwnedPtr<MacroExpandDecl>(new MacroExpandDecl());
    auto innerDecl = OwnedPtr<VarDecl>(new VarDecl());
    innerDecl->identifier = "innerVar";
    macroExpandDecl->invocation.decl = std::move(innerDecl);

    std::string result = ItemResolverUtil::ResolveNameByNode(*macroExpandDecl);
    EXPECT_EQ("innerVar", result);
}

// Test ResolveKindByNode function - uncovered branches
TEST(ItemResolverUtilTest, ResolveKindByNode_MacroDecl) {
    // Create MacroDecl node
    auto macroDecl = OwnedPtr<MacroDecl>(new MacroDecl());

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*macroDecl);
    EXPECT_EQ(CompletionItemKind::CIK_METHOD, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_GenericParamDecl) {
    // Create GenericParamDecl node
    auto genericParamDecl = OwnedPtr<GenericParamDecl>(new GenericParamDecl());

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*genericParamDecl);
    EXPECT_EQ(CompletionItemKind::CIK_VARIABLE, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_PrimaryCtorDecl) {
    // Create PrimaryCtorDecl node
    auto primaryCtorDecl = OwnedPtr<PrimaryCtorDecl>(new PrimaryCtorDecl());

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*primaryCtorDecl);
    EXPECT_EQ(CompletionItemKind::CIK_METHOD, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_VarWithPatternDecl) {
    // Create VarWithPatternDecl node
    auto varWithPatternDecl = OwnedPtr<VarWithPatternDecl>(new VarWithPatternDecl());

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*varWithPatternDecl);
    EXPECT_EQ(CompletionItemKind::CIK_VARIABLE, result);
}

// Test ResolveKindByASTKind function - uncovered branches
TEST(ItemResolverUtilTest, ResolveKindByASTKind_MacroDecl) {
    ASTKind astKind = ASTKind::MACRO_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_METHOD, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_GenericParamDecl) {
    ASTKind astKind = ASTKind::GENERIC_PARAM_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_VARIABLE, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_BuiltInDecl) {
    ASTKind astKind = ASTKind::BUILTIN_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_CLASS, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_PrimaryCtorDecl) {
    ASTKind astKind = ASTKind::PRIMARY_CTOR_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_METHOD, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_VarWithPatternDecl) {
    ASTKind astKind = ASTKind::VAR_WITH_PATTERN_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_VARIABLE, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_MacroExpandDecl) {
    ASTKind astKind = ASTKind::MACRO_EXPAND_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_METHOD, result);
}

// Test ResolveSignatureByNode function - uncovered branches
TEST(ItemResolverUtilTest, ResolveSignatureByNode_MainDecl) {
    // Create MainDecl node
    auto mainDecl = OwnedPtr<MainDecl>(new MainDecl());
    mainDecl->identifier = "main";

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*mainDecl, nullptr);
    EXPECT_EQ("main()", result);
}

TEST(ItemResolverUtilTest, ResolveSignatureByNode_Package) {
    // Create Package node
    auto package = OwnedPtr<Package>(new Package());
    package->fullPackageName = "test.package";

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*package, nullptr);
    EXPECT_EQ("test.package", result);
}

TEST(ItemResolverUtilTest, ResolveSignatureByNode_TypeAliasDecl) {
    // Create TypeAliasDecl node
    auto typeAliasDecl = OwnedPtr<TypeAliasDecl>(new TypeAliasDecl());
    typeAliasDecl->identifier = "MyType";

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*typeAliasDecl, nullptr);
    EXPECT_EQ("MyType", result);
}

TEST(ItemResolverUtilTest, ResolveSignatureByNode_TypeAliasDeclWithGeneric) {
    // Create TypeAliasDecl node with generic parameters
    auto typeAliasDecl = OwnedPtr<TypeAliasDecl>(new TypeAliasDecl());
    typeAliasDecl->identifier = "MyType";
    auto generic = OwnedPtr<Generic>(new Generic());
    auto param = OwnedPtr<GenericParamDecl>(new GenericParamDecl());
    param->identifier = "T";
    generic->typeParameters.emplace_back(std::move(param));
    typeAliasDecl->generic = std::move(generic);

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*typeAliasDecl, nullptr);
    EXPECT_EQ("MyType<T>", result);
}

// Test ResolveInsertByNode function - basic coverage
TEST(ItemResolverUtilTest, ResolveInsertByNode_VarDecl) {
    // Create VarDecl node
    auto varDecl = OwnedPtr<VarDecl>(new VarDecl());
    varDecl->identifier = "testVar";

    std::string result = ItemResolverUtil::ResolveInsertByNode(*varDecl, nullptr);
    EXPECT_EQ("testVar", result);
}

TEST(ItemResolverUtilTest, ResolveInsertByNode_Package) {
    // Create Package node
    auto package = OwnedPtr<Package>(new Package());
    package->fullPackageName = "test.package";

    std::string result = ItemResolverUtil::ResolveInsertByNode(*package, nullptr);
    EXPECT_EQ("test.package", result);
}

TEST(ItemResolverUtilTest, IsCustomAnnotation_WithNullAnnotation) {
    // Test case with null annotation
    FuncDecl decl;
    auto outerDecl = OwnedPtr<FuncDecl>(new FuncDecl());
    outerDecl->annotations.emplace_back(nullptr);
    decl.outerDecl = outerDecl.get();

    bool result = ItemResolverUtil::IsCustomAnnotation(decl);
    EXPECT_FALSE(result);
}

// Test ResolveSourceByNode function
TEST(ItemResolverUtilTest, ResolveSourceByNode_NullDecl) {
    // Test with null declaration pointer
    Ptr<Decl> decl = nullptr;

    std::string result = ItemResolverUtil::ResolveSourceByNode(decl, "test/path");
    EXPECT_EQ("", result);
}

// Test FetchTypeString function - basic coverage
TEST(ItemResolverUtilTest, FetchTypeString_NullType) {
    // Test with null type
    Type type;
    type.ty = nullptr;

    std::string result = ItemResolverUtil::FetchTypeString(type);
    EXPECT_EQ("", result);
}

// Test AddGenericInsertByDecl function
TEST(ItemResolverUtilTest, AddGenericInsertByDecl_NullGeneric) {
    std::string detail = "test";
    int result = ItemResolverUtil::AddGenericInsertByDecl(detail, nullptr);
    EXPECT_EQ(0, result);
    EXPECT_EQ("test", detail);
}

TEST(ItemResolverUtilTest, AddGenericInsertByDecl_WithGeneric) {
    auto genericDecl = OwnedPtr<Generic>(new Generic());
    auto param = OwnedPtr<GenericParamDecl>(new GenericParamDecl());
    param->identifier = "T";
    genericDecl->typeParameters.emplace_back(std::move(param));

    std::string detail = "test";
    int result = ItemResolverUtil::AddGenericInsertByDecl(detail, genericDecl);
    EXPECT_EQ(2, result); // Starts from 1, so with one parameter it becomes 2
    EXPECT_EQ("test<${1:T}>", detail);
}

// Test GetDeclByTy function
TEST(ItemResolverUtilTest, GetDeclByTy_NullTy) {
    Ty* ty = nullptr;
    Ptr<Decl> result = ItemResolverUtil::GetDeclByTy(ty);
    EXPECT_EQ(nullptr, result);
}