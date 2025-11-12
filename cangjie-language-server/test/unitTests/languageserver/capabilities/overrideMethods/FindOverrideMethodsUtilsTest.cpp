// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "FindOverrideMethodsUtils.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "Utils.h"

using namespace ark;
using namespace Cangjie::AST;

TEST(FindOverrideMethodsUtilsTest, ResolveFuncParamListTest001)
{
    auto funcDecl = new FuncDecl();
    auto funcBody = new FuncBody();
    auto paramList = new FuncParamList();
    paramList->params.emplace_back(nullptr);
    funcBody->paramLists.emplace_back(paramList);
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);

    const FuncParamDetailList result = ResolveFuncParamList(Ptr<FuncDecl>(funcDecl));
    EXPECT_TRUE(result.params.empty());
}

TEST(FindOverrideMethodsUtilsTest, ResolveFuncParamListTest002)
{
    auto funcDecl = new FuncDecl();
    auto funcBody = new FuncBody();
    auto paramList = new FuncParamList();
    auto param = new FuncParam();
    param->identifier = "Int32";
    param->ty = new PrimitiveTy(TypeKind::TYPE_INT32);
    paramList->params.emplace_back(param);
    funcBody->paramLists.emplace_back(paramList);
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);

    const FuncParamDetailList result = ResolveFuncParamList(funcDecl);
    EXPECT_FALSE(result.params.empty());
    EXPECT_EQ(result.params[0].identifier, "`Int32`");
}

TEST(FindOverrideMethodsUtilsTest, ResolveFuncParamListTest003)
{
    auto funcDecl = new FuncDecl();
    auto funcBody = new FuncBody();
    auto paramList = new FuncParamList();
    auto param = new FuncParam();
    param->identifier = "Int32";
    param->ty = nullptr;
    paramList->params.emplace_back(param);
    funcBody->paramLists.emplace_back(paramList);
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);

    const FuncParamDetailList result = ResolveFuncParamList(funcDecl);
    EXPECT_TRUE(result.params.empty());
}

TEST(FindOverrideMethodsUtilsTest, ResolveFuncRetTypeTest001)
{
    auto funcDecl = new FuncDecl();
    funcDecl->funcBody = nullptr;
    const auto result = ResolveFuncRetType(funcDecl);
    EXPECT_FALSE(result);
}

TEST(FindOverrideMethodsUtilsTest, ResolveFuncRetTypeTest002)
{
    auto funcDecl = new FuncDecl();
    auto funcBody = new FuncBody();
    funcBody->retType = nullptr;
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);
    const auto result = ResolveFuncRetType(funcDecl);
    EXPECT_FALSE(result);
}

TEST(FindOverrideMethodsUtilsTest, ResolveFuncRetTypeTest003)
{
    auto funcDecl = new FuncDecl();
    auto funcBody = new FuncBody();
    auto retType = new Type();
    retType->ty = nullptr;
    funcBody->retType = OwnedPtr<Type>(retType);
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);
    const auto result = ResolveFuncRetType(funcDecl);
    EXPECT_FALSE(result);
}

TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest001)
{
    Ptr<Ty> type = nullptr;
    const auto result = ResolveType(type);
    EXPECT_EQ(result, nullptr);
}

// ClassTy
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest002)
{
    auto name = "TestClass";
    auto cd = new ClassDecl();
    cd->identifier = "TestClass";
    auto generic = new Generic();
    cd->generic = OwnedPtr<Generic>(generic);

    auto typeArgs = std::vector<Ptr<Ty>>();
    auto paramTy1 = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto paramTy2 = new PrimitiveTy(TypeKind::TYPE_FLOAT32);
    typeArgs.emplace_back(paramTy1);
    typeArgs.emplace_back(paramTy2);

    auto ty = new ClassTy(name, *cd, typeArgs);
    auto detail = ResolveType(ty);

    EXPECT_NE(detail, nullptr);
    auto classDetail = dynamic_cast<ClassLikeTypeDetail *>(detail.get());
    EXPECT_NE(classDetail, nullptr);
    EXPECT_EQ(classDetail->identifier, name);
    EXPECT_EQ(classDetail->generics.size(), 2);
}

// InterfaceTy
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest003)
{
    auto name = "TestInterface";
    auto id = new InterfaceDecl();
    id->identifier = "TestInterface";
    auto generic = new Generic();
    id->generic = OwnedPtr<Generic>(generic);

    auto typeArgs = std::vector<Ptr<Ty>>();
    auto paramTy1 = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto paramTy2 = new PrimitiveTy(TypeKind::TYPE_FLOAT32);
    typeArgs.emplace_back(paramTy1);
    typeArgs.emplace_back(paramTy2);

    auto ty = new InterfaceTy(name, *id, typeArgs);
    auto detail = ResolveType(ty);

    EXPECT_NE(detail, nullptr);
    auto classDetail = dynamic_cast<ClassLikeTypeDetail *>(detail.get());
    EXPECT_NE(classDetail, nullptr);
    EXPECT_EQ(classDetail->identifier, name);
    EXPECT_EQ(classDetail->generics.size(), 2);
}

// EnumTy
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest004)
{
    auto name = "TestEnum";
    auto ed = new EnumDecl();
    ed->identifier = "TestEnum";
    auto generic = new Generic();
    ed->generic = OwnedPtr<Generic>(generic);

    auto typeArgs = std::vector<Ptr<Ty>>();
    auto paramTy1 = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto paramTy2 = new PrimitiveTy(TypeKind::TYPE_FLOAT32);
    typeArgs.emplace_back(paramTy1);
    typeArgs.emplace_back(paramTy2);

    auto ty = new EnumTy(name, *ed, typeArgs);
    auto detail = ResolveType(ty);

    EXPECT_NE(detail, nullptr);
    auto classDetail = dynamic_cast<ClassLikeTypeDetail *>(detail.get());
    EXPECT_NE(classDetail, nullptr);
    EXPECT_EQ(classDetail->identifier, name);
    EXPECT_EQ(classDetail->generics.size(), 2);
}

// StructTy
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest005)
{
    auto name = "TestStruct";
    auto sd = new StructDecl();
    sd->identifier = "TestStruct";
    auto generic = new Generic();
    sd->generic = OwnedPtr<Generic>(generic);

    auto typeArgs = std::vector<Ptr<Ty>>();
    auto paramTy1 = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto paramTy2 = new PrimitiveTy(TypeKind::TYPE_FLOAT32);
    typeArgs.emplace_back(paramTy1);
    typeArgs.emplace_back(paramTy2);

    auto ty = new StructTy(name, *sd, typeArgs);
    auto detail = ResolveType(ty);

    EXPECT_NE(detail, nullptr);
    auto classDetail = dynamic_cast<ClassLikeTypeDetail *>(detail.get());
    EXPECT_NE(classDetail, nullptr);
    EXPECT_EQ(classDetail->identifier, name);
    EXPECT_EQ(classDetail->generics.size(), 2);
}

// t.decl->generic == nullptr
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest006)
{
    auto name = "TestStruct";
    auto sd = new StructDecl();
    sd->identifier = "TestStruct";
    sd->generic = nullptr;

    auto typeArgs = std::vector<Ptr<Ty>>();
    auto paramTy1 = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto paramTy2 = new PrimitiveTy(TypeKind::TYPE_FLOAT32);
    typeArgs.emplace_back(paramTy1);
    typeArgs.emplace_back(paramTy2);

    auto ty = new StructTy(name, *sd, typeArgs);
    auto detail = ResolveType(ty);

    EXPECT_NE(detail, nullptr);
    auto classDetail = dynamic_cast<ClassLikeTypeDetail *>(detail.get());
    EXPECT_NE(classDetail, nullptr);
    EXPECT_EQ(classDetail->identifier, name);
    EXPECT_EQ(classDetail->generics.size(), 0);
}

// FuncTy
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest007)
{
    auto paramTy1 = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto paramTy2 = new PrimitiveTy(TypeKind::TYPE_FLOAT32);
    auto paramTys = std::vector<Ptr<Ty>>();
    paramTys.emplace_back(paramTy1);
    paramTys.emplace_back(paramTy2);
    auto retTy = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto funcTy = new FuncTy(paramTys, retTy);
    auto detail = ResolveType(funcTy);

    EXPECT_NE(detail, nullptr);
    auto funcDetail = dynamic_cast<FuncLikeTypeDetail *>(detail.get());
    EXPECT_NE(funcDetail, nullptr);
    EXPECT_EQ(funcDetail->params.size(), 2);
    EXPECT_EQ(funcDetail->ret->ToString(), "Int32");
}

// TypeAliasTy
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest008)
{
    auto name = "TestAlias";
    auto tad = new TypeAliasDecl();
    tad->identifier = "TestAlias";

    auto typeArgs = std::vector<Ptr<Ty>>();
    auto paramTy1 = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto paramTy2 = new PrimitiveTy(TypeKind::TYPE_FLOAT32);
    typeArgs.emplace_back(paramTy1);
    typeArgs.emplace_back(paramTy2);
    auto ty = new TypeAliasTy(name, *tad, typeArgs);
    auto detail = ResolveType(ty);

    EXPECT_NE(detail, nullptr);
    auto commonDetail = dynamic_cast<CommonTypeDetail *>(detail.get());
    EXPECT_NE(commonDetail, nullptr);
    EXPECT_EQ(commonDetail->identifier, name);
}

// GenericsTy
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest009)
{
    auto name = "TestGenerics";
    auto gpd = new GenericParamDecl();
    gpd->identifier = "TestGenerics";

    auto ty = new GenericsTy(name, *gpd);
    auto detail = ResolveType(ty);

    EXPECT_NE(detail, nullptr);
    auto commonDetail = dynamic_cast<CommonTypeDetail *>(detail.get());
    EXPECT_NE(commonDetail, nullptr);
    EXPECT_EQ(commonDetail->identifier, name);
}

// VArrayTy1
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest010)
{
    auto elemTy = new PrimitiveTy(TypeKind::TYPE_INT32);
    int64_t size = 10;
    auto varrayTy = new VArrayTy(elemTy, size);
    auto detail = ResolveType(varrayTy);

    EXPECT_NE(detail, nullptr);
    auto varrayDetail = dynamic_cast<VArrayTypeDetail *>(detail.get());
    EXPECT_NE(varrayDetail, nullptr);
    EXPECT_EQ(varrayDetail->identifier, "VArray");
    EXPECT_EQ(varrayDetail->size, 10);
    EXPECT_NE(varrayDetail->tyArg, nullptr);
    EXPECT_EQ(varrayDetail->tyArg->ToString(), "Int32");
}

// VArrayTy2
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest011)
{
    auto elemTy1 = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto elemTy2 = new PrimitiveTy(TypeKind::TYPE_FLOAT32);
    int64_t size = 10;
    auto varrayTy = new VArrayTy(elemTy1, size);
    varrayTy->typeArgs.emplace_back(elemTy2);
    auto detail = ResolveType(varrayTy);

    EXPECT_NE(detail, nullptr);
    auto varrayDetail = dynamic_cast<VArrayTypeDetail *>(detail.get());
    EXPECT_NE(varrayDetail, nullptr);
    EXPECT_EQ(varrayDetail->identifier, "");
    EXPECT_EQ(varrayDetail->size, 0);
    EXPECT_EQ(varrayDetail->tyArg, nullptr);
}

// TupleTy
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest012)
{
    bool isClosureTy = false;
    auto elemTys = std::vector<Ptr<Ty>>();
    auto elemTy1 = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto elemTy2 = new PrimitiveTy(TypeKind::TYPE_FLOAT32);
    elemTys.emplace_back(elemTy1);
    elemTys.emplace_back(elemTy2);

    auto ty = new TupleTy(elemTys, isClosureTy);
    auto detail = ResolveType(ty);

    EXPECT_NE(detail, nullptr);
    auto tupleDetail = dynamic_cast<TupleTypeDetail *>(detail.get());
    EXPECT_NE(tupleDetail, nullptr);
    EXPECT_EQ(tupleDetail->params.size(), 2);
    EXPECT_EQ(tupleDetail->params[0]->ToString(), "Int32");
    EXPECT_EQ(tupleDetail->params[1]->ToString(), "Float32");
}

// Ty
TEST(FindOverrideMethodsUtilsTest, ResolveTypeTest013)
{
    auto ty = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto detail = ResolveType(ty);

    EXPECT_NE(detail, nullptr);
    EXPECT_EQ(detail->ToString(), "Int32");
}

TEST(FindOverrideMethodsUtilsTest, ResolveFuncDetailTest001)
{
    auto funcDecl = new FuncDecl();
    funcDecl->identifier = "myFunction";
    funcDecl->EnableAttr(Attribute::PUBLIC);
    funcDecl->EnableAttr(Attribute::STATIC);

    auto funcBody = new FuncBody();
    auto paramList = new FuncParamList();
    auto param = new FuncParam();
    param->identifier = "Int32";
    param->ty = new PrimitiveTy(TypeKind::TYPE_INT32);
    paramList->params.emplace_back(param);
    funcBody->paramLists.emplace_back(paramList);
    auto retType = new Type();
    retType->ty = new PrimitiveTy(TypeKind::TYPE_INT32);
    funcBody->retType = OwnedPtr<Type>(retType);
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);

    auto detail = ResolveFuncDetail(funcDecl);
    EXPECT_EQ(detail.identifier, "myFunction");
    EXPECT_EQ(detail.modifiers.size(), 2);
    EXPECT_EQ(detail.modifiers[0], "public");
    EXPECT_EQ(detail.modifiers[1], "static");
    EXPECT_EQ(detail.params.params.size(), 1);
    EXPECT_EQ(detail.params.params[0].identifier, "`Int32`");
    EXPECT_EQ(detail.params.params[0].type->ToString(), "Int32");
    EXPECT_EQ(detail.retType->ToString(), "Int32");
}

TEST(FindOverrideMethodsUtilsTest, ResolvePropDetailTest001)
{
    auto propDecl = new PropDecl();
    propDecl->identifier = "myProp";
    propDecl->EnableAttr(Attribute::PUBLIC);
    propDecl->EnableAttr(Attribute::STATIC);
    propDecl->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    auto detail = ResolvePropDetail(propDecl);
    EXPECT_EQ(detail.identifier, "myProp");
    EXPECT_EQ(detail.modifiers.size(), 2);
    EXPECT_EQ(detail.modifiers[0], "public");
    EXPECT_EQ(detail.modifiers[1], "static");
    EXPECT_EQ(detail.type->ToString(), "Int32");
}

TEST(FindOverrideMethodsUtilsTest, TypeDetailTest_DefaultConstructor)
{
    TypeDetail td;
    EXPECT_EQ(td.ToString(), "");
}

TEST(FindOverrideMethodsUtilsTest, TypeDetailTest_ExplicitConstructor)
{
    TypeDetail td("test123");
    EXPECT_EQ(td.ToString(), "test123");
}

TEST(FindOverrideMethodsUtilsTest, TypeDetailTest_ObjectLifetime)
{
    auto td = std::make_unique<TypeDetail>("test");
    EXPECT_EQ(td->ToString(), "test");
}

TEST(FindOverrideMethodsUtilsTest, TypeDetailTest_BasicSetIdentifier)
{
    std::unique_ptr<TypeDetail> td = std::make_unique<TypeDetail>("oldId");
    td->SetIdentifier("oldId", "newId");
    EXPECT_EQ(td->ToString(), "oldId");
}

TEST(FindOverrideMethodsUtilsTest, TypeDetailTest_DefaultComparable)
{
    std::unique_ptr<TypeDetail> td1 = std::make_unique<TypeDetail>("test1");
    std::unique_ptr<TypeDetail> td2 = std::make_unique<TypeDetail>("test2");
    EXPECT_FALSE(td1->Comparable(td2));
}

TEST(FindOverrideMethodsUtilsTest, TypeDetailTest_DefaultDiff)
{
    std::unique_ptr<TypeDetail> td1 = std::make_unique<TypeDetail>("test1");
    std::unique_ptr<TypeDetail> td2 = std::make_unique<TypeDetail>("test2");
    std::unordered_map<std::string, std::string> diffs;
    td1->Diff(td2, diffs);
    EXPECT_EQ(diffs.size(), 0);
}

TEST(FindOverrideMethodsUtilsTest, CommonTypeDetailTest_DefaultConstructor)
{
    CommonTypeDetail detail;
    EXPECT_EQ(detail.identifier, "");
}

TEST(FindOverrideMethodsUtilsTest, CommonTypeDetailTest_ExplicitConstructor)
{
    CommonTypeDetail detail("test_id");
    EXPECT_EQ(detail.identifier, "test_id");
}

TEST(FindOverrideMethodsUtilsTest, CommonTypeDetailTest_SetIdentifier_Success)
{
    CommonTypeDetail detail("old_id");
    detail.SetIdentifier("old_id", "new_id");
    EXPECT_EQ(detail.identifier, "new_id");
}

TEST(FindOverrideMethodsUtilsTest, CommonTypeDetailTest_SetIdentifier_NoChange)
{
    CommonTypeDetail detail("current_id");
    detail.SetIdentifier("wrong_id", "new_id");
    EXPECT_EQ(detail.identifier, "current_id");
}

TEST(FindOverrideMethodsUtilsTest, CommonTypeDetailTest_Comparable_ReturnsTrue)
{
    CommonTypeDetail detail;
    EXPECT_TRUE(detail.Comparable(nullptr));
}

TEST(FindOverrideMethodsUtilsTest, CommonTypeDetailTest_Diff_SameIdentifier)
{
    CommonTypeDetail detail("id");
    std::unique_ptr<TypeDetail> other = std::make_unique<CommonTypeDetail>("id");
    std::unordered_map<std::string, std::string> diffs;
    detail.Diff(other, diffs);
    EXPECT_TRUE(diffs.empty());
}

TEST(FindOverrideMethodsUtilsTest, CommonTypeDetailTest_Diff_DifferentIdentifier)
{
    CommonTypeDetail detail("id1");
    std::unique_ptr<TypeDetail> other = std::make_unique<CommonTypeDetail>("id2");
    std::unordered_map<std::string, std::string> diffs;
    detail.Diff(other, diffs);
    EXPECT_EQ(diffs.size(), 1);
    EXPECT_EQ(diffs["id1"], "id2");
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_DefaultConstructor)
{
    ClassLikeTypeDetail detail;
    EXPECT_EQ(detail.identifier, "");
    EXPECT_TRUE(detail.generics.empty());
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_ExplicitConstructor)
{
    ClassLikeTypeDetail detail("test_id");
    EXPECT_EQ(detail.identifier, "test_id");
    EXPECT_TRUE(detail.generics.empty());
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_ToString_NoGenerics)
{
    ClassLikeTypeDetail detail("test_id");
    EXPECT_EQ(detail.ToString(), "test_id");
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_ToString_OneGeneric)
{
    ClassLikeTypeDetail detail("test_id");
    detail.generics.emplace_back(std::make_unique<ClassLikeTypeDetail>("generic1"));
    EXPECT_EQ(detail.ToString(), "test_id<generic1>");
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_ToString_MultipleGenerics)
{
    ClassLikeTypeDetail detail("test_id");
    detail.generics.emplace_back(std::make_unique<ClassLikeTypeDetail>("generic1"));
    detail.generics.emplace_back(std::make_unique<ClassLikeTypeDetail>("generic2"));
    EXPECT_EQ(detail.ToString(), "test_id<generic1, generic2>");
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_SetIdentifier_NoGenerics)
{
    ClassLikeTypeDetail detail("test_id");
    detail.SetIdentifier("old_id", "new_id");
    EXPECT_EQ(detail.identifier, "test_id");
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_SetIdentifier_OneGeneric)
{
    ClassLikeTypeDetail detail("test_id");
    detail.generics.emplace_back(std::make_unique<CommonTypeDetail>("old_id"));
    detail.SetIdentifier("old_id", "new_id");
    EXPECT_EQ(detail.generics[0]->identifier, "new_id");
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_SetIdentifier_MultipleGenerics)
{
    ClassLikeTypeDetail detail("test_id");
    detail.generics.emplace_back(std::make_unique<CommonTypeDetail>("old_id"));
    detail.generics.emplace_back(std::make_unique<CommonTypeDetail>("old_id"));
    detail.SetIdentifier("old_id", "new_id");
    for (const auto &generic : detail.generics) {
        EXPECT_EQ(generic->identifier, "new_id");
    }
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_Comparable_SameTypeAndGenerics)
{
    auto detail1 = std::make_unique<ClassLikeTypeDetail>("test_id");
    detail1->generics.emplace_back(std::make_unique<ClassLikeTypeDetail>("generic"));
    auto detail2 = std::make_unique<ClassLikeTypeDetail>("test_id");
    detail2->generics.emplace_back(std::make_unique<ClassLikeTypeDetail>("generic"));
    EXPECT_TRUE(detail1->Comparable(std::move(detail2)));
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_Comparable_DifferentType)
{
    auto detail1 = std::make_unique<ClassLikeTypeDetail>("test_id");
    auto detail2 = std::make_unique<CommonTypeDetail>("test_id");
    EXPECT_FALSE(detail1->Comparable(std::move(detail2)));
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_Comparable_SameTypeDifferentGenericsCount)
{
    auto detail1 = std::make_unique<ClassLikeTypeDetail>("test_id");
    detail1->generics.emplace_back(std::make_unique<ClassLikeTypeDetail>("generic"));
    auto detail2 = std::make_unique<ClassLikeTypeDetail>("test_id");
    EXPECT_FALSE(detail1->Comparable(std::move(detail2)));
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_Diff_NoGenerics)
{
    auto detail1 = std::make_unique<ClassLikeTypeDetail>("test_id");
    auto detail2 = std::make_unique<ClassLikeTypeDetail>("test_id");
    std::unordered_map<std::string, std::string> diffs;
    detail1->Diff(std::move(detail2), diffs);
    EXPECT_TRUE(diffs.empty());
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_Diff_OneGenericDifferent)
{
    auto detail1 = std::make_unique<ClassLikeTypeDetail>("test_id");
    detail1->generics.emplace_back(std::make_unique<CommonTypeDetail>("generic1"));
    auto detail2 = std::make_unique<ClassLikeTypeDetail>("test_id");
    detail2->generics.emplace_back(std::make_unique<CommonTypeDetail>("generic2"));
    std::unordered_map<std::string, std::string> diffs;
    detail1->Diff(std::move(detail2), diffs);
    EXPECT_EQ(diffs.size(), 1);
    EXPECT_EQ(diffs["generic1"], "generic2");
}

TEST(FindOverrideMethodsUtilsTest, ClassLikeTypeDetailTest_Diff_Uncomparable)
{
    auto detail1 = std::make_unique<ClassLikeTypeDetail>("test_id1");
    detail1->generics.emplace_back(std::make_unique<CommonTypeDetail>("generic1"));
    auto detail2 = std::make_unique<ClassLikeTypeDetail>("test_id2");
    detail2->generics.emplace_back(std::make_unique<CommonTypeDetail>("generic2"));
    std::unordered_map<std::string, std::string> diffs;
    detail1->Diff(std::move(detail2), diffs);
    EXPECT_EQ(diffs.size(), 0);
}

TEST(FindOverrideMethodsUtilsTest, FuncLikeTypeDetailTest_DefaultConstructorTest)
{
    FuncLikeTypeDetail detail;
    EXPECT_EQ(detail.params.size(), 0);
    EXPECT_EQ(detail.ret, nullptr);
}

TEST(FindOverrideMethodsUtilsTest, FuncLikeTypeDetailTest_ConstructorWithIdentifierTest)
{
    FuncLikeTypeDetail detail("test");
    EXPECT_EQ(detail.identifier, "test");
    EXPECT_EQ(detail.params.size(), 0);
    EXPECT_EQ(detail.ret, nullptr);
}

TEST(FindOverrideMethodsUtilsTest, FuncLikeTypeDetailTest_ToStringNoParamsNoReturnTest)
{
    FuncLikeTypeDetail detail;
    EXPECT_EQ(detail.ToString(), "() -> ");
}

TEST(FindOverrideMethodsUtilsTest, FuncLikeTypeDetailTest_ToStringOneParamNoReturnTest)
{
    FuncLikeTypeDetail detail;
    auto param = std::make_unique<TypeDetail>("int");
    detail.params.emplace_back(std::move(param));
    EXPECT_EQ(detail.ToString(), "(int) -> ");
}

TEST(FindOverrideMethodsUtilsTest, FuncLikeTypeDetailTest_ToStringMultipleParamsWithReturnTest)
{
    FuncLikeTypeDetail detail;
    auto param1 = std::make_unique<TypeDetail>("int");
    auto param2 = std::make_unique<TypeDetail>("float");
    detail.params.emplace_back(std::move(param1));
    detail.params.emplace_back(std::move(param2));
    auto ret = std::make_unique<TypeDetail>("void");
    detail.ret = std::move(ret);
    EXPECT_EQ(detail.ToString(), "(int, float) -> void");
}

TEST(FindOverrideMethodsUtilsTest, FuncLikeTypeDetailTest_SetIdentifierTest)
{
    FuncLikeTypeDetail detail;
    detail.params.emplace_back(std::make_unique<CommonTypeDetail>("int"));
    detail.ret = std::make_unique<CommonTypeDetail>("int");

    detail.SetIdentifier("int", "integer");

    EXPECT_EQ(detail.params[0]->ToString(), "integer");
    EXPECT_EQ(detail.ret->ToString(), "integer");
}

TEST(FindOverrideMethodsUtilsTest, FuncLikeTypeDetailTest_ComparableDifferentTypeTest)
{
    FuncLikeTypeDetail detail1;
    auto other = std::make_unique<TypeDetail>("test");
    EXPECT_FALSE(detail1.Comparable(other));
}

TEST(FindOverrideMethodsUtilsTest, FuncLikeTypeDetailTest_ComparableDifferentParamsCountTest)
{
    auto detail1 = std::make_unique<FuncLikeTypeDetail>();
    auto detail2 = std::make_unique<FuncLikeTypeDetail>();
    detail1->params.emplace_back(std::make_unique<CommonTypeDetail>("int"));
    EXPECT_FALSE(detail1->Comparable(std::move(detail2)));
}

TEST(FindOverrideMethodsUtilsTest, FuncLikeTypeDetailTest_ComparableSameParamsCountTest)
{
    auto detail1 = std::make_unique<FuncLikeTypeDetail>();
    auto detail2 = std::make_unique<FuncLikeTypeDetail>();
    detail1->params.emplace_back(std::make_unique<CommonTypeDetail>("int"));
    detail2->params.emplace_back(std::make_unique<CommonTypeDetail>("int"));
    EXPECT_TRUE(detail1->Comparable(std::move(detail2)));
}

TEST(FindOverrideMethodsUtilsTest, FuncLikeTypeDetailTest_DiffTest_Uncomparable)
{
    auto detail1 = std::make_unique<FuncLikeTypeDetail>();
    auto detail2 = std::make_unique<TypeDetail>();

    std::unordered_map<std::string, std::string> diffs;
    detail1->Diff(detail2, diffs);
    EXPECT_EQ(diffs.size(), 0);
}

TEST(FindOverrideMethodsUtilsTest, FuncLikeTypeDetailTest_DiffTest_Comparable)
{
    auto detail1 = std::make_unique<FuncLikeTypeDetail>();
    auto detail2 = std::make_unique<FuncLikeTypeDetail>();
    detail1->params.emplace_back(std::make_unique<CommonTypeDetail>("int"));
    detail2->params.emplace_back(std::make_unique<CommonTypeDetail>("float"));
    detail1->ret = std::make_unique<CommonTypeDetail>("void");
    detail2->ret = std::make_unique<CommonTypeDetail>("double");

    std::unordered_map<std::string, std::string> diffs;
    detail1->Diff(std::move(detail2), diffs);

    EXPECT_EQ(diffs.size(), 2);
    EXPECT_EQ(diffs["int"], "float");
    EXPECT_EQ(diffs["void"], "double");
}

TEST(FindOverrideMethodsUtilsTest, VArrayTypeDetailTest_Identifier)
{
    auto detail = std::make_unique<VArrayTypeDetail>();

    EXPECT_EQ(detail->size, 0);
    EXPECT_EQ(detail->tyArg, nullptr);
}

TEST(FindOverrideMethodsUtilsTest, VArrayTypeDetailTest_ToStringWithNullTyArg)
{
    VArrayTypeDetail detail;
    detail.identifier = "array";
    detail.size = 5;
    EXPECT_EQ(detail.ToString(), "array<, $5>");
}

TEST(FindOverrideMethodsUtilsTest, VArrayTypeDetailTest_ToStringWithTyArg)
{
    auto tyArg = std::make_unique<TypeDetail>();
    tyArg->identifier = "int";
    VArrayTypeDetail detail;
    detail.tyArg = std::move(tyArg);
    detail.identifier = "array";
    detail.size = 5;
    EXPECT_EQ(detail.ToString(), "array<int, $5>");
}

TEST(FindOverrideMethodsUtilsTest, VArrayTypeDetailTest_SetIdentifierWithTyArg)
{
    auto tyArg = std::make_unique<CommonTypeDetail>();
    tyArg->identifier = "int";
    VArrayTypeDetail detail;
    detail.tyArg = std::move(tyArg);
    detail.identifier = "array";
    detail.SetIdentifier("int", "integer");
    EXPECT_EQ(detail.tyArg->ToString(), "integer");
}

TEST(FindOverrideMethodsUtilsTest, VArrayTypeDetailTest_ComparableWithSameType)
{
    auto other = std::make_unique<VArrayTypeDetail>();
    VArrayTypeDetail detail;
    EXPECT_TRUE(detail.Comparable(std::move(other)));
}

TEST(FindOverrideMethodsUtilsTest, VArrayTypeDetailTest_ComparableWithDifferentType)
{
    auto other = std::make_unique<TypeDetail>();
    VArrayTypeDetail detail;
    EXPECT_FALSE(detail.Comparable(other));
}

TEST(FindOverrideMethodsUtilsTest, VArrayTypeDetailTest_Diff_Uncomparable)
{
    auto other = std::make_unique<TypeDetail>();
    VArrayTypeDetail detail;
    std::unordered_map<std::string, std::string> diffs;
    detail.Diff(other, diffs);
    EXPECT_EQ(diffs.size(), 0);
}

TEST(FindOverrideMethodsUtilsTest, VArrayTypeDetailTest_DiffWithDifferentTyArg)
{
    auto tyArg1 = std::make_unique<CommonTypeDetail>();
    tyArg1->identifier = "int";
    auto tyArg2 = std::make_unique<CommonTypeDetail>();
    tyArg2->identifier = "float";
    VArrayTypeDetail detail1;
    detail1.tyArg = std::move(tyArg1);
    detail1.identifier = "array";
    detail1.size = 5;
    auto detail2 = std::make_unique<VArrayTypeDetail>();
    detail2->tyArg = std::move(tyArg2);
    detail2->identifier = "array";
    detail2->size = 5;
    std::unordered_map<std::string, std::string> diffs;
    detail1.Diff(std::move(detail2), diffs);
    EXPECT_EQ(diffs.size(), 1);
    EXPECT_EQ(diffs["int"], "float");
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_DefaultConstructorTest)
{
    TupleTypeDetail detail;
    EXPECT_EQ(detail.params.size(), 0);
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_ToStringWithEmptyParams)
{
    TupleTypeDetail detail;
    EXPECT_EQ(detail.ToString(), "()");
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_ToStringWithSingleParam)
{
    auto param = std::make_unique<TypeDetail>();
    param->identifier = "int";
    TupleTypeDetail detail;
    detail.params.emplace_back(std::move(param));
    EXPECT_EQ(detail.ToString(), "(int)");
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_ToStringWithMultipleParams)
{
    auto param1 = std::make_unique<TypeDetail>();
    param1->identifier = "int";
    auto param2 = std::make_unique<TypeDetail>();
    param2->identifier = "float";
    TupleTypeDetail detail;
    detail.params.emplace_back(std::move(param1));
    detail.params.emplace_back(std::move(param2));
    EXPECT_EQ(detail.ToString(), "(int, float)");
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_SetIdentifierWithSingleParam)
{
    auto param = std::make_unique<CommonTypeDetail>();
    param->identifier = "int";
    TupleTypeDetail detail;
    detail.params.emplace_back(std::move(param));
    detail.SetIdentifier("int", "integer");
    EXPECT_EQ(detail.params[0]->ToString(), "integer");
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_ComparableWithDifferentType)
{
    TupleTypeDetail detail1;
    auto detail2 = std::make_unique<TypeDetail>();
    EXPECT_FALSE(detail1.Comparable(detail2));
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_ComparableWithDifferentParamCount)
{
    TupleTypeDetail detail1;
    detail1.params.emplace_back(std::make_unique<CommonTypeDetail>());
    auto detail2 = std::make_unique<TupleTypeDetail>();
    EXPECT_FALSE(detail1.Comparable(std::move(detail2)));
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_ComparableWithSameParams)
{
    auto param1 = std::make_unique<CommonTypeDetail>();
    param1->identifier = "int";
    TupleTypeDetail detail1;
    detail1.params.emplace_back(std::move(param1));
    auto param2 = std::make_unique<CommonTypeDetail>();
    param2->identifier = "int";
    auto detail2 = std::make_unique<TupleTypeDetail>();
    detail2->params.emplace_back(std::move(param2));
    EXPECT_TRUE(detail1.Comparable(std::move(detail2)));
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_ComparableWithDifferentParams)
{
    auto param1 = std::make_unique<TypeDetail>();
    param1->identifier = "int";
    TupleTypeDetail detail1;
    detail1.params.emplace_back(std::move(param1));
    auto param2 = std::make_unique<TypeDetail>();
    param2->identifier = "int";
    auto detail2 = std::make_unique<TupleTypeDetail>();
    detail2->params.emplace_back(std::move(param2));
    EXPECT_FALSE(detail1.Comparable(std::move(detail2)));
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_DiffWithSameParams)
{
    auto param1 = std::make_unique<CommonTypeDetail>();
    param1->identifier = "int";
    TupleTypeDetail detail1;
    detail1.params.emplace_back(std::move(param1));
    auto param2 = std::make_unique<CommonTypeDetail>();
    param2->identifier = "int";
    auto detail2 = std::make_unique<TupleTypeDetail>();
    detail2->params.emplace_back(std::move(param2));
    std::unordered_map<std::string, std::string> diffs;
    detail1.Diff(std::move(detail2), diffs);
    EXPECT_TRUE(diffs.empty());
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_DiffWithDifferentParams)
{
    auto param1 = std::make_unique<CommonTypeDetail>();
    param1->identifier = "int";
    TupleTypeDetail detail1;
    detail1.params.emplace_back(std::move(param1));
    auto param2 = std::make_unique<CommonTypeDetail>();
    param2->identifier = "float";
    auto detail2 = std::make_unique<TupleTypeDetail>();
    detail2->params.emplace_back(std::move(param2));
    std::unordered_map<std::string, std::string> diffs;
    detail1.Diff(std::move(detail2), diffs);
    EXPECT_FALSE(diffs.empty());
    EXPECT_EQ(diffs["int"], "float");
}

TEST(FindOverrideMethodsUtilsTest, TupleTypeDetailTest_DiffWithDifferentParamCount)
{
    TupleTypeDetail detail1;
    detail1.params.emplace_back(std::make_unique<CommonTypeDetail>());
    auto detail2 = std::make_unique<TupleTypeDetail>();
    std::unordered_map<std::string, std::string> diffs;
    detail1.Diff(std::move(detail2), diffs);
    EXPECT_TRUE(diffs.empty());
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailTest_Identifier)
{
    auto detail = std::make_unique<FuncParamDetail>();
    EXPECT_EQ(detail->identifier, "");
    EXPECT_EQ(detail->type, nullptr);
    EXPECT_EQ(detail->isNamed, false);
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailTest_ToStringWithTypeNullAndIsNamedTrue)
{
    FuncParamDetail param;
    param.identifier = "param1";
    param.isNamed = true;
    EXPECT_EQ(param.ToString(), "param1!: ");
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailTest_ToStringWithTypeNotNullAndIsNamedFalse)
{
    auto type = std::make_unique<TypeDetail>();
    type->identifier = "int";
    FuncParamDetail param;
    param.type = std::move(type);
    param.identifier = "param1";
    param.isNamed = false;
    EXPECT_EQ(param.ToString(), "param1: int");
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailTest_ToStringWithTypeNotNullAndIsNamedTrue)
{
    auto type = std::make_unique<TypeDetail>();
    type->identifier = "int";
    FuncParamDetail param;
    param.type = std::move(type);
    param.identifier = "param1";
    param.isNamed = true;
    EXPECT_EQ(param.ToString(), "param1!: int");
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailTest_SetGenericTypeWithMatch)
{
    auto type = std::make_unique<CommonTypeDetail>();
    type->identifier = "int";
    FuncParamDetail param;
    param.type = std::move(type);
    param.identifier = "param1";
    param.isNamed = false;
    param.SetGenericType("int", "integer");
    EXPECT_EQ(param.type->ToString(), "integer");
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailListTest_Identifier)
{
    auto detail = std::make_unique<FuncParamDetailList>();
    EXPECT_EQ(detail->params.size(), 0);
    EXPECT_EQ(detail->isVariadic, false);
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailListTest_SetGenericTypeWithSingleParam)
{
    auto type = std::make_unique<CommonTypeDetail>();
    type->identifier = "int";
    auto param = std::make_unique<FuncParamDetail>();
    param->type = std::move(type);
    param->identifier = "param1";
    param->isNamed = false;
    FuncParamDetailList list;
    list.params.emplace_back(std::move(*param));
    list.SetGenericType("int", "integer");
    EXPECT_EQ(list.params[0].type->ToString(), "integer");
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailListTest_ToStringWithEmptyParamsAndNotVariadic)
{
    FuncParamDetailList list;
    EXPECT_EQ(list.ToString(), "");
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailListTest_ToStringWithSingleParamAndNotVariadic)
{
    auto type = std::make_unique<CommonTypeDetail>();
    type->identifier = "int";
    auto param = std::make_unique<FuncParamDetail>();
    param->type = std::move(type);
    param->identifier = "param1";
    param->isNamed = false;
    FuncParamDetailList list;
    list.params.emplace_back(std::move(*param));
    EXPECT_EQ(list.ToString(), "param1: int");
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailListTest_ToStringWithMultipleParamsAndNotVariadic)
{
    auto type1 = std::make_unique<CommonTypeDetail>();
    type1->identifier = "int";
    auto param1 = std::make_unique<FuncParamDetail>();
    param1->type = std::move(type1);
    param1->identifier = "param1";
    param1->isNamed = false;

    auto type2 = std::make_unique<CommonTypeDetail>();
    type2->identifier = "float";
    auto param2 = std::make_unique<FuncParamDetail>();
    param2->type = std::move(type2);
    param2->identifier = "param2";
    param2->isNamed = false;

    FuncParamDetailList list;
    list.params.emplace_back(std::move(*param1));
    list.params.emplace_back(std::move(*param2));
    EXPECT_EQ(list.ToString(), "param1: int, param2: float");
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailListTest_ToStringWithEmptyParamsAndVariadic)
{
    FuncParamDetailList list;
    list.isVariadic = true;
    EXPECT_EQ(list.ToString(), ", ...");
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailListTest_ToStringWithSingleParamAndVariadic)
{
    auto type = std::make_unique<CommonTypeDetail>();
    type->identifier = "int";
    auto param = std::make_unique<FuncParamDetail>();
    param->type = std::move(type);
    param->identifier = "param1";
    param->isNamed = false;
    FuncParamDetailList list;
    list.params.emplace_back(std::move(*param));
    list.isVariadic = true;
    EXPECT_EQ(list.ToString(), "param1: int, ...");
}

TEST(FindOverrideMethodsUtilsTest, FuncParamDetailListTest_ToStringWithMultipleParamsAndVariadic)
{
    auto type1 = std::make_unique<CommonTypeDetail>();
    type1->identifier = "int";
    auto param1 = std::make_unique<FuncParamDetail>();
    param1->type = std::move(type1);
    param1->identifier = "param1";
    param1->isNamed = false;

    auto type2 = std::make_unique<CommonTypeDetail>();
    type2->identifier = "float";
    auto param2 = std::make_unique<FuncParamDetail>();
    param2->type = std::move(type2);
    param2->identifier = "param2";
    param2->isNamed = false;

    FuncParamDetailList list;
    list.params.emplace_back(std::move(*param1));
    list.params.emplace_back(std::move(*param2));
    list.isVariadic = true;
    EXPECT_EQ(list.ToString(), "param1: int, param2: float, ...");
}

TEST(FindOverrideMethodsUtilsTest, FuncDetailTest_Identifier)
{
    auto detail = std::make_unique<FuncDetail>();
    EXPECT_EQ(detail->modifiers.size(), 0);
    EXPECT_EQ(detail->identifier, "");
    EXPECT_EQ(detail->retType, nullptr);
}

TEST(FindOverrideMethodsUtilsTest, FuncDetailTest_SetGenericTypeWithSingleParamMatch)
{
    auto type = std::make_unique<CommonTypeDetail>();
    type->identifier = "int";
    auto param = std::make_unique<FuncParamDetail>();
    param->type = std::move(type);
    param->identifier = "param1";
    param->isNamed = false;
    auto paramList = std::make_unique<FuncParamDetailList>();
    paramList->params.emplace_back(std::move(*param));
    FuncDetail func;
    func.params = std::move(*paramList);
    func.SetGenericType("int", "integer");
    EXPECT_EQ(func.params.params[0].type->ToString(), "integer");
}

TEST(FindOverrideMethodsUtilsTest, FuncDetailTest_SetGenericTypeWithRetTypeMatch)
{
    auto retType = std::make_unique<CommonTypeDetail>();
    retType->identifier = "int";
    FuncDetail func;
    func.retType = std::move(retType);
    func.SetGenericType("int", "integer");
    EXPECT_EQ(func.retType->ToString(), "integer");
}

TEST(FindOverrideMethodsUtilsTest, FuncDetailTest_ToStringWithEmptyModifiersParamsAndRetType)
{
    FuncDetail func;
    func.identifier = "func1";
    EXPECT_EQ(func.ToString(), "func func1(): ");
}

TEST(FindOverrideMethodsUtilsTest, FuncDetailTest_ToStringWithMultipleModifiersParamsAndRetType)
{
    FuncDetail func;
    func.modifiers.emplace_back("public");
    func.modifiers.emplace_back("static");
    func.identifier = "func1";
    auto paramType1 = std::make_unique<CommonTypeDetail>();
    paramType1->identifier = "int";
    auto param1 = std::make_unique<FuncParamDetail>();
    param1->type = std::move(paramType1);
    param1->identifier = "param1";
    param1->isNamed = false;

    auto paramType2 = std::make_unique<CommonTypeDetail>();
    paramType2->identifier = "float";
    auto param2 = std::make_unique<FuncParamDetail>();
    param2->type = std::move(paramType2);
    param2->identifier = "param2";
    param2->isNamed = true;

    auto paramList = std::make_unique<FuncParamDetailList>();
    paramList->params.emplace_back(std::move(*param1));
    paramList->params.emplace_back(std::move(*param2));
    func.params = std::move(*paramList);

    auto retType = std::make_unique<CommonTypeDetail>();
    retType->identifier = "void";
    func.retType = std::move(retType);
    EXPECT_EQ(func.ToString(), "public static func func1(param1: int, param2!: float): void");
}

TEST(FindOverrideMethodsUtilsTest, FuncDetailTest_GetFunctionNameWithEmptyParams)
{
    FuncDetail func;
    func.identifier = "func1";
    EXPECT_EQ(func.GetFunctionName(), "func1()");
}

TEST(FindOverrideMethodsUtilsTest, FuncDetailTest_GetFunctionNameWithMultipleParams)
{
    FuncDetail func;
    func.identifier = "func1";
    auto paramType1 = std::make_unique<CommonTypeDetail>();
    paramType1->identifier = "int";
    auto param1 = std::make_unique<FuncParamDetail>();
    param1->type = std::move(paramType1);
    param1->identifier = "param1";
    param1->isNamed = false;

    auto paramType2 = std::make_unique<CommonTypeDetail>();
    paramType2->identifier = "float";
    auto param2 = std::make_unique<FuncParamDetail>();
    param2->type = std::move(paramType2);
    param2->identifier = "param2";
    param2->isNamed = true;

    auto paramList = std::make_unique<FuncParamDetailList>();
    paramList->params.emplace_back(std::move(*param1));
    paramList->params.emplace_back(std::move(*param2));
    func.params = std::move(*paramList);
    EXPECT_EQ(func.GetFunctionName(), "func1(param1: int, param2!: float)");
}

TEST(FindOverrideMethodsUtilsTest, FuncDetailTest_GetFunctionWithRetWithNullRetType)
{
    FuncDetail func;
    func.identifier = "func1";
    EXPECT_EQ(func.GetFunctionWithRet(), "func1()");
}

TEST(FindOverrideMethodsUtilsTest, FuncDetailTest_GetFunctionWithRetWithValidRetType)
{
    FuncDetail func;
    func.identifier = "func1";
    auto retType = std::make_unique<CommonTypeDetail>();
    retType->identifier = "void";
    func.retType = std::move(retType);
    EXPECT_EQ(func.GetFunctionWithRet(), "func1(): void");
}

TEST(FindOverrideMethodsUtilsTest, PropDetailTest_Identifier)
{
    auto detail = std::make_unique<PropDetail>();
    EXPECT_EQ(detail->modifiers.size(), 0);
    EXPECT_EQ(detail->identifier, "");
    EXPECT_EQ(detail->type, nullptr);
}

TEST(FindOverrideMethodsUtilsTest, PropDetailTest_SetGenericTypeWithMatch)
{
    auto type = std::make_unique<CommonTypeDetail>();
    type->identifier = "int";
    PropDetail prop;
    prop.type = std::move(type);
    prop.identifier = "prop1";
    prop.SetGenericType("int", "integer");
    EXPECT_EQ(prop.type->ToString(), "integer");
}

TEST(FindOverrideMethodsUtilsTest, PropDetailTest_ToStringWithEmptyModifiersAndType)
{
    PropDetail prop;
    prop.identifier = "prop1";
    EXPECT_EQ(prop.ToString(), " prop prop1: ");
}

TEST(FindOverrideMethodsUtilsTest, PropDetailTest_ToStringWithSingleModifierAndNoType)
{
    PropDetail prop;
    prop.modifiers.emplace_back("public");
    prop.identifier = "prop1";
    EXPECT_EQ(prop.ToString(), "public prop prop1: ");
}

TEST(FindOverrideMethodsUtilsTest, PropDetailTest_ToStringWithMultipleModifiersAndType)
{
    PropDetail prop;
    prop.modifiers.emplace_back("public");
    prop.modifiers.emplace_back("static");
    prop.identifier = "prop1";
    auto type = std::make_unique<CommonTypeDetail>();
    type->identifier = "int";
    prop.type = std::move(type);
    EXPECT_EQ(prop.ToString(), "public static prop prop1: int");
}