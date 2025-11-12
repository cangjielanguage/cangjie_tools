// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "FindDeclUsage.cpp"
#include <unordered_set>

using namespace ark;
using namespace Cangjie::AST;

// CheckTypeEqual test
TEST(FindDeclUsageTest, CheckTypeEqual_SameBasicTypes) {
    // Create two identical basic types
    auto srcType = new PrimitiveTy(TypeKind::TYPE_INT32);

    auto targetType = new PrimitiveTy(TypeKind::TYPE_INT32);

    EXPECT_TRUE(CheckTypeEqual(*srcType, *targetType));
}

TEST(FindDeclUsageTest, CheckTypeEqual_DifferentBasicTypes) {
    // Create two different basic types
    auto srcType = new PrimitiveTy(TypeKind::TYPE_INT32);;

    auto targetType = new PrimitiveTy(TypeKind::TYPE_INT64);;

    EXPECT_FALSE(CheckTypeEqual(*srcType, *targetType));
}

TEST(FindDeclUsageTest, CheckTypeEqual_ArrayTypesWithDifferentDims) {
    // Create two array types with different dimensions
    auto elemTy= new PrimitiveTy(TypeKind::TYPE_INT32);
    auto srcArrayTy = std::make_unique<ArrayTy>(elemTy, 1);
    auto targetArrayTy = std::make_unique<ArrayTy>(elemTy, 2);

    EXPECT_FALSE(CheckTypeEqual(*srcArrayTy, *targetArrayTy));
}

TEST(FindDeclUsageTest, CheckTypeEqual_ArrayTypesWithSameDims) {
    // Create two array types with same dimensions
    auto elemTy= new PrimitiveTy(TypeKind::TYPE_INT32);
    auto srcArrayTy = std::make_unique<ArrayTy>(elemTy, 1);
    auto targetArrayTy = std::make_unique<ArrayTy>(elemTy, 1);

    EXPECT_TRUE(CheckTypeEqual(*srcArrayTy, *targetArrayTy));
}

// CheckParamListEqual test
TEST(FindDeclUsageTest, CheckParamListEqual_SameParamLists) {
    // Create two identical parameter lists
    auto srcList = new FuncParamList();
    auto targetList = new FuncParamList();

    // Add identical parameters
    auto param1 = new FuncParam();
    param1->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    auto param2 = new FuncParam();
    param2->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    srcList->params.emplace_back(param1);
    targetList->params.emplace_back(param2);

    EXPECT_TRUE(CheckParamListEqual(*srcList, *targetList));
}

TEST(FindDeclUsageTest, CheckParamListEqual_DifferentParamCount) {
    // Create two parameter lists with different parameter counts
    auto srcList = new FuncParamList();
    auto targetList = new FuncParamList();

    auto param1 = new FuncParam();
    param1->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    auto param2 = new FuncParam();
    param2->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    auto param3 = new FuncParam();
    param3->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    srcList->params.emplace_back(param1);
    targetList->params.emplace_back(param2);
    targetList->params.emplace_back(param3);

    EXPECT_FALSE(CheckParamListEqual(*srcList, *targetList));
}

// CheckFunctionEqual test
TEST(FindDeclUsageTest, CheckFunctionEqual_BothFunctionsNullBody) {
    // Create two function declarations without function bodies
    FuncDecl srcFunc;
    FuncDecl targetFunc;

    // No function body should return false
    EXPECT_FALSE(CheckFunctionEqual(srcFunc, targetFunc));
}

TEST(FindDeclUsageTest, CheckFunctionEqual_DifferentParamListCount) {
    // Create two function declarations with different parameter list counts
    FuncDecl srcFunc;
    FuncDecl targetFunc;

    srcFunc.funcBody = OwnedPtr<FuncBody>(new FuncBody());
    targetFunc.funcBody = OwnedPtr<FuncBody>(new FuncBody());

    // Add different parameter list counts
    auto paramList1 = Ptr<FuncParamList>(new FuncParamList());
    auto paramList2 = Ptr<FuncParamList>(new FuncParamList());
    auto paramList3 = Ptr<FuncParamList>(new FuncParamList());

    srcFunc.funcBody->paramLists.emplace_back(paramList1);
    targetFunc.funcBody->paramLists.emplace_back(paramList2);
    targetFunc.funcBody->paramLists.emplace_back(paramList3);

    EXPECT_FALSE(CheckFunctionEqual(srcFunc, targetFunc));
}

// GetDefinedDecl test
TEST(FindDeclUsageTest, GetDefinedDecl_FuncDeclWithPropDecl) {
    // Create function declaration with propDecl
    FuncDecl funcDecl;
    auto propDecl = Ptr<PropDecl>(new PropDecl());
    funcDecl.propDecl = propDecl;

    auto result = GetDefinedDecl(Ptr<const Decl>(&funcDecl));
    EXPECT_EQ(propDecl.get(), result.get());
}

TEST(FindDeclUsageTest, GetDefinedDecl_FuncDeclWithoutPropDecl) {
    // Create function declaration without propDecl
    FuncDecl funcDecl;

    auto result = GetDefinedDecl(Ptr<const Decl>(&funcDecl));
    EXPECT_EQ(&funcDecl, result.get());
}

// CheckDeclEqual test
TEST(FindDeclUsageTest, CheckDeclEqual_SameTypeDecls) {
    // Create two identical type declarations
    ClassDecl srcDecl;
    ClassDecl targetDecl;

    srcDecl.fullPackageName = "test.package";
    srcDecl.identifier = "TestClass";

    targetDecl.fullPackageName = "test.package";
    targetDecl.identifier = "TestClass";

    // Type declarations should return true
    EXPECT_TRUE(CheckDeclEqual(srcDecl, targetDecl));
}

TEST(FindDeclUsageTest, CheckDeclEqual_DifferentPackageNames) {
    // Create two declarations with different package names
    ClassDecl srcDecl;
    ClassDecl targetDecl;

    srcDecl.fullPackageName = "test.package1";
    srcDecl.identifier = "TestClass";

    targetDecl.fullPackageName = "test.package2";
    targetDecl.identifier = "TestClass";

    EXPECT_FALSE(CheckDeclEqual(srcDecl, targetDecl));
}

TEST(FindDeclUsageTest, CheckDeclEqual_DifferentIdentifiers) {
    // Create two declarations with different identifiers
    ClassDecl srcDecl;
    ClassDecl targetDecl;

    srcDecl.fullPackageName = "test.package";
    srcDecl.identifier = "TestClass1";

    targetDecl.fullPackageName = "test.package";
    targetDecl.identifier = "TestClass2";

    EXPECT_FALSE(CheckDeclEqual(srcDecl, targetDecl));
}

TEST(FindDeclUsageTest, CheckDeclEqual_NonFunctionDeclsSameContext) {
    // Create two non-function declarations with same context
    VarDecl srcDecl;
    VarDecl targetDecl;

    srcDecl.fullPackageName = "test.package";
    srcDecl.identifier = "testVar";

    targetDecl.fullPackageName = "test.package";
    targetDecl.identifier = "testVar";

    EXPECT_TRUE(CheckDeclEqual(srcDecl, targetDecl));
}

// GetRealNode test
TEST(FindDeclUsageTest, GetRealNode_ExprWithSourceExpr) {
    // Create expression with sourceExpr
    auto expr = Ptr<RefExpr>(new RefExpr());
    auto sourceExpr = Ptr<RefExpr>(new RefExpr());
    expr->sourceExpr = sourceExpr;

    auto result = GetRealNode(expr);
    EXPECT_EQ(sourceExpr.get(), result.get());
}

TEST(FindDeclUsageTest, GetRealNode_MemberAccessWithBuiltinOperator) {
    // Create member access expression with builtin operator
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    // Assume "+" is one of the builtin operators
    ma->field = "+";

    auto callExpr = Ptr<CallExpr>(new CallExpr());
    auto sourceExpr = Ptr<RefExpr>(new RefExpr());
    callExpr->sourceExpr = sourceExpr;
    ma->callOrPattern = callExpr;

    auto result = GetRealNode(ma);
    EXPECT_EQ(sourceExpr.get(), result.get());
}

TEST(FindDeclUsageTest, GetRealNode_NormalNode) {
    // Create normal node
    auto node = Ptr<Decl>();

    auto result = GetRealNode(node);
    EXPECT_EQ(node.get(), result.get());
}

// checkMacroFunc test
TEST(FindDeclUsageTest, CheckMacroFunc_ValidMacroFunction) {
    // Create valid macro function
    Decl decl;
    auto target = Ptr<FuncDecl>(new FuncDecl());

    decl.isInMacroCall = true;
    decl.ty = Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT32));
    decl.identifier = "testFunc";

    target->ty = decl.ty; // Same type
    target->identifier = "testFunc";

    EXPECT_FALSE(checkMacroFunc(decl, target));
}

TEST(FindDeclUsageTest, CheckMacroFunc_NotInMacroCall) {
    // Create non-macro function
    Decl decl;
    auto target = Ptr<FuncDecl>(new FuncDecl());

    decl.isInMacroCall = false; // Not in macro call
    decl.ty = Ptr<Ty>();
    decl.identifier = "testFunc";

    target->ty = decl.ty;
    target->identifier = "testFunc";

    EXPECT_FALSE(checkMacroFunc(decl, target));
}

TEST(FindDeclUsageTest, CheckMacroFunc_DifferentIdentifiers) {
    // Create macro function with different identifiers
    Decl decl;
    auto target = Ptr<FuncDecl>(new FuncDecl());

    decl.isInMacroCall = true;
    decl.ty = Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT32));
    decl.identifier = "testFunc1";

    target->ty = Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT32));
    target->identifier = "testFunc2"; // Different identifier

    EXPECT_FALSE(checkMacroFunc(decl, target));
}

// FindNamedFuncParamUsage test
TEST(FindDeclUsageTest, FindNamedFuncParamUsage_NotNamedParam) {
    // Create non-named parameter
    FuncParam fp;
    fp.isNamedParam = false; // Non-named parameter

    Node root;
    auto result = FindNamedFuncParamUsage(fp, root);

    // Should directly return empty result set
    EXPECT_TRUE(result.empty());
}

TEST(FindDeclUsageTest, FindNamedFuncParamUsage_NoOuterDecl) {
    // Create named parameter without outer declaration
    FuncParam fp;
    fp.isNamedParam = true;
    fp.outerDecl = nullptr; // No outer declaration

    Node root;
    auto result = FindNamedFuncParamUsage(fp, root);

    // Should directly return empty result set
    EXPECT_TRUE(result.empty());
}

TEST(FindDeclUsageTest, FindNamedFuncParamUsage_OuterDeclNotFunc) {
    // Create named parameter with non-function outer declaration
    FuncParam fp;
    fp.isNamedParam = true;
    fp.outerDecl = Ptr<VarDecl>(); // Outer declaration is variable declaration

    Node root;
    auto result = FindNamedFuncParamUsage(fp, root);

    // Should directly return empty result set
    EXPECT_TRUE(result.empty());
}

// FindDeclUsage test
TEST(FindDeclUsageTest, FindDeclUsage_FuncParamDecl) {
    // Create function parameter declaration
    FuncParam fp;
    fp.isNamedParam = true;

    Node root;
    auto result = FindDeclUsage(fp, root, false);

    // Should call FindNamedFuncParamUsage
    EXPECT_TRUE(result.empty()); // Should be empty in this simple test
}

TEST(FindDeclUsageTest, FindDeclUsage_NormalDecl) {
    // Create normal declaration
    ClassDecl decl;
    decl.identifier = "TestClass";

    Node root;
    auto result = FindDeclUsage(decl, root, false);

    // Should call FindUsage
    EXPECT_TRUE(result.empty()); // Should be empty in this simple test
}

// CheckTypeEqual additional tests
TEST(FindDeclUsageTest, CheckTypeEqual_DifferentTypeArgCount) {
    // Create types with different type argument counts
    auto srcType = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto targetType = new PrimitiveTy(TypeKind::TYPE_INT32);

    // Add different number of type arguments
    srcType->typeArgs.emplace_back(new PrimitiveTy(TypeKind::TYPE_INT32));

    EXPECT_FALSE(CheckTypeEqual(*srcType, *targetType));
}

TEST(FindDeclUsageTest, CheckTypeEqual_OneNullTypeArg) {
    // Create types where one has null type argument
    auto srcType = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto targetType = new PrimitiveTy(TypeKind::TYPE_INT32);

    srcType->typeArgs.emplace_back(nullptr);
    targetType->typeArgs.emplace_back(new PrimitiveTy(TypeKind::TYPE_INT32));

    EXPECT_FALSE(CheckTypeEqual(*srcType, *targetType));
}

TEST(FindDeclUsageTest, CheckTypeEqual_FuncTypeInvalidCast) {
    // Create function types where dynamic cast fails
    auto srcType = new PrimitiveTy(TypeKind::TYPE_FUNC);
    auto targetType = new PrimitiveTy(TypeKind::TYPE_FUNC);

    // PrimitiveTy cannot be cast to FuncTy
    EXPECT_FALSE(CheckTypeEqual(*srcType, *targetType));
}

// CheckParamListEqual additional tests
TEST(FindDeclUsageTest, CheckParamListEqual_NullParam) {
    // Create parameter lists with null parameters
    auto srcList = new FuncParamList();
    auto targetList = new FuncParamList();

    srcList->params.emplace_back(nullptr);
    targetList->params.emplace_back(new FuncParam());

    EXPECT_FALSE(CheckParamListEqual(*srcList, *targetList));
}

TEST(FindDeclUsageTest, CheckParamListEqual_NullParamType) {
    // Create parameter lists with parameters having null types
    auto srcList = new FuncParamList();
    auto targetList = new FuncParamList();

    auto srcParam = new FuncParam();
    auto targetParam = new FuncParam();

    srcParam->ty = nullptr;
    targetParam->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    srcList->params.emplace_back(srcParam);
    targetList->params.emplace_back(targetParam);

    EXPECT_FALSE(CheckParamListEqual(*srcList, *targetList));
}

TEST(FindDeclUsageTest, CheckParamListEqual_DifferentParamTypes) {
    // Create parameter lists with different parameter types
    auto srcList = new FuncParamList();
    auto targetList = new FuncParamList();

    auto srcParam = new FuncParam();
    auto targetParam = new FuncParam();

    srcParam->ty = new PrimitiveTy(TypeKind::TYPE_INT32);
    targetParam->ty = new PrimitiveTy(TypeKind::TYPE_INT64);

    srcList->params.emplace_back(srcParam);
    targetList->params.emplace_back(targetParam);

    EXPECT_FALSE(CheckParamListEqual(*srcList, *targetList));
}

// CheckFunctionEqual additional tests
TEST(FindDeclUsageTest, CheckFunctionEqual_DifferentParamLists) {
    // Create functions with different parameter lists
    FuncDecl srcFunc;
    FuncDecl targetFunc;

    srcFunc.funcBody = OwnedPtr<FuncBody>(new FuncBody());
    targetFunc.funcBody = OwnedPtr<FuncBody>(new FuncBody());

    auto srcParamList = Ptr<FuncParamList>(new FuncParamList());
    auto targetParamList = Ptr<FuncParamList>(new FuncParamList());

    // Add different parameters to make lists unequal
    auto srcParam = new FuncParam();
    auto targetParam = new FuncParam();

    srcParam->ty = new PrimitiveTy(TypeKind::TYPE_INT32);
    targetParam->ty = new PrimitiveTy(TypeKind::TYPE_INT64);

    srcParamList->params.emplace_back(srcParam);
    targetParamList->params.emplace_back(targetParam);

    srcFunc.funcBody->paramLists.emplace_back(srcParamList);
    targetFunc.funcBody->paramLists.emplace_back(targetParamList);

    EXPECT_FALSE(CheckFunctionEqual(srcFunc, targetFunc));
}

// CheckDeclEqual additional tests
TEST(FindDeclUsageTest, CheckDeclEqual_DifferentASTKind) {
    // Create declarations with different AST kinds
    ClassDecl srcDecl;
    VarDecl targetDecl;

    srcDecl.fullPackageName = "test.package";
    srcDecl.identifier = "test";
    targetDecl.fullPackageName = "test.package";
    targetDecl.identifier = "test";

    EXPECT_FALSE(CheckDeclEqual(srcDecl, targetDecl));
}

TEST(FindDeclUsageTest, CheckDeclEqual_FuncDeclDifferentOuter) {
    // Create function declarations with different outer contexts
    FuncDecl srcFunc;
    FuncDecl targetFunc;

    srcFunc.fullPackageName = "test.package";
    srcFunc.identifier = "testFunc";
    targetFunc.fullPackageName = "test.package";
    targetFunc.identifier = "testFunc";

    // Create different outer declarations
    ClassDecl srcOuter;
    ClassDecl targetOuter;

    srcOuter.identifier = "Class1";
    targetOuter.identifier = "Class2";

    srcFunc.outerDecl = Ptr<Decl>(&srcOuter);
    targetFunc.outerDecl = Ptr<Decl>(&targetOuter);

    // Functions should be different due to different outer contexts
    EXPECT_FALSE(CheckDeclEqual(srcFunc, targetFunc));
}

TEST(FindDeclUsageTest, CheckDeclEqual_FuncDeclInvalidCast) {
    // Test function declaration comparison with invalid casts
    FuncDecl srcFunc;
    ClassDecl targetDecl; // Wrong type

    srcFunc.fullPackageName = "test.package";
    srcFunc.identifier = "test";
    targetDecl.fullPackageName = "test.package";
    targetDecl.identifier = "test";

    EXPECT_FALSE(CheckDeclEqual(srcFunc, targetDecl));
}

// GetRealNode additional tests
TEST(FindDeclUsageTest, GetRealNode_MemberAccessWithoutBuiltinOperator) {
    // Create member access without builtin operator
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "normalField"; // Not a builtin operator

    auto result = GetRealNode(ma);
    EXPECT_EQ(ma.get(), result.get());
}

TEST(FindDeclUsageTest, GetRealNode_MemberAccessWithNullCall) {
    // Create member access with null callOrPattern
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "+"; // Builtin operator
    ma->callOrPattern = nullptr;

    auto result = GetRealNode(ma);
    EXPECT_EQ(ma.get(), result.get());
}

TEST(FindDeclUsageTest, GetRealNode_MemberAccessCallWithoutSource) {
    // Create member access with call expression without source
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    ma->field = "+"; // Builtin operator

    auto callExpr = Ptr<CallExpr>(new CallExpr());
    callExpr->sourceExpr = nullptr; // No source expression
    ma->callOrPattern = callExpr;

    auto result = GetRealNode(ma);
    EXPECT_EQ(ma.get(), result.get());
}

// checkMacroFunc additional tests
TEST(FindDeclUsageTest, CheckMacroFunc_NullTarget) {
    // Test checkMacroFunc with null target
    Decl decl;
    decl.isInMacroCall = true;
    decl.ty = Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_FUNC));
    decl.identifier = "testFunc";

    EXPECT_FALSE(checkMacroFunc(decl, nullptr));
}

TEST(FindDeclUsageTest, CheckMacroFunc_DifferentTypes) {
    // Test checkMacroFunc with different types
    Decl decl;
    auto target = Ptr<FuncDecl>(new FuncDecl());

    decl.isInMacroCall = true;
    decl.ty = Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT32));
    target->ty = Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT64));
    decl.identifier = "testFunc";
    target->identifier = "testFunc";

    EXPECT_FALSE(checkMacroFunc(decl, target));
}

TEST(FindDeclUsageTest, CheckMacroFunc_NonFunctionType) {
    // Test checkMacroFunc with non-function type
    Decl decl;
    auto target = Ptr<FuncDecl>(new FuncDecl());

    decl.isInMacroCall = true;
    decl.ty = Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT32)); // Not function type
    target->ty = Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT32));
    decl.identifier = "testFunc";
    target->identifier = "testFunc";

    EXPECT_FALSE(checkMacroFunc(decl, target));
}

// FindNamedFuncParamUsage additional tests
TEST(FindDeclUsageTest, FindNamedFuncParamUsage_ValidCase) {
    // Test valid named parameter usage finding
    FuncParam fp;
    fp.isNamedParam = true;
    fp.identifier = "testParam";

    // Create function declaration as outer
    auto funcDecl = Ptr<FuncDecl>(new FuncDecl());
    fp.outerDecl = funcDecl;

    Node root;

    // This should exercise the full logic path
    auto result = FindNamedFuncParamUsage(fp, root);
    EXPECT_TRUE(result.empty()); // Empty in simple test
}

// FindUsage and FindDeclUsage additional tests
TEST(FindDeclUsageTest, FindUsage_WithRenameAndAliasTarget) {
    // Test FindUsage with rename and alias target
    ClassDecl decl;
    decl.identifier = "TestClass";

    Node root;

    // Test with isRename = true
    auto result = FindUsage(decl, root, true);
    EXPECT_TRUE(result.empty());
}

TEST(FindDeclUsageTest, FindDeclUsage_WithRename) {
    // Test FindDeclUsage with rename flag
    ClassDecl decl;
    decl.identifier = "TestClass";

    Node root;

    auto result = FindDeclUsage(decl, root, true);
    EXPECT_TRUE(result.empty());
}
