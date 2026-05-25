// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "capabilities/refactor/tweaks/IntroduceField.h"
#include "gtest/gtest.h"
#include <memory>

using namespace ark;
using namespace Cangjie::AST;

namespace {
template <typename T> OwnedPtr<T> MakeOwner()
{
    return OwnedPtr<T>(new T());
}

void ExpectTarget(FuncDecl &funcDecl, bool supported, bool member, bool isStatic)
{
    EXPECT_EQ(IntroduceField::IsSupportedTargetFunc(funcDecl), supported);
    EXPECT_EQ(IntroduceField::IsMemberFieldTarget(funcDecl), member);
    EXPECT_EQ(IntroduceField::IsStaticFieldTarget(funcDecl), isStatic);
}
} // namespace

TEST(IntroduceFieldTest, TargetKinds)
{
    FuncDecl topLevelFunc;
    ExpectTarget(topLevelFunc, true, false, false);

    auto classDecl = MakeOwner<ClassDecl>();
    FuncDecl classFunc;
    classFunc.outerDecl = classDecl.get();
    ExpectTarget(classFunc, true, true, false);
    classFunc.EnableAttr(Attribute::STATIC);
    ExpectTarget(classFunc, true, true, true);

    auto structDecl = MakeOwner<StructDecl>();
    FuncDecl structFunc;
    structFunc.outerDecl = structDecl.get();
    ExpectTarget(structFunc, true, true, false);

    auto interfaceDecl = MakeOwner<InterfaceDecl>();
    FuncDecl interfaceFunc;
    interfaceFunc.outerDecl = interfaceDecl.get();
    ExpectTarget(interfaceFunc, false, false, false);

    auto enumDecl = MakeOwner<EnumDecl>();
    FuncDecl enumFunc;
    enumFunc.outerDecl = enumDecl.get();
    ExpectTarget(enumFunc, false, false, false);

    auto extendDecl = MakeOwner<ExtendDecl>();
    FuncDecl extendFunc;
    extendFunc.outerDecl = extendDecl.get();
    ExpectTarget(extendFunc, false, false, false);
}

TEST(IntroduceFieldTest, StaticOnlyAppliesToSupportedMemberFunctions)
{
    FuncDecl topLevelStaticFunc;
    topLevelStaticFunc.EnableAttr(Attribute::STATIC);
    ExpectTarget(topLevelStaticFunc, true, false, false);

    auto interfaceDecl = MakeOwner<InterfaceDecl>();
    FuncDecl interfaceStaticFunc;
    interfaceStaticFunc.outerDecl = interfaceDecl.get();
    interfaceStaticFunc.EnableAttr(Attribute::STATIC);
    ExpectTarget(interfaceStaticFunc, false, false, false);

    auto structDecl = MakeOwner<StructDecl>();
    FuncDecl structStaticFunc;
    structStaticFunc.outerDecl = structDecl.get();
    structStaticFunc.EnableAttr(Attribute::STATIC);
    ExpectTarget(structStaticFunc, true, true, true);
}

TEST(IntroduceFieldTest, ExtraOptionsReturnStoredOptions)
{
    IntroduceField introduceField;
    EXPECT_TRUE(introduceField.ExtraOptions().empty());

    introduceField.extraOptions["ErrorCode"] = "4";
    introduceField.extraOptions["suggestName"] = "fieldValue";

    auto options = introduceField.ExtraOptions();
    EXPECT_EQ(options.at("ErrorCode"), "4");
    EXPECT_EQ(options.at("suggestName"), "fieldValue");

    options["ErrorCode"] = "changed";
    EXPECT_EQ(introduceField.ExtraOptions().at("ErrorCode"), "4");
}
