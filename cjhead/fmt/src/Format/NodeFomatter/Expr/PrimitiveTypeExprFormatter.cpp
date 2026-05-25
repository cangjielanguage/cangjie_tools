// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/PrimitiveTypeExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"
#include <string>

namespace Cangjie::Format {
using namespace Cangjie::AST;

namespace {
const std::unordered_map<TypeKind, std::string> TYPEKIND_TO_STRING_MAP{
    {TypeKind::TYPE_UNIT, "Unit"},
    {TypeKind::TYPE_INT8, "Int8"},
    {TypeKind::TYPE_INT16, "Int16"},
    {TypeKind::TYPE_INT32, "Int32"},
    {TypeKind::TYPE_INT64, "Int64"},
    {TypeKind::TYPE_INT_NATIVE, "IntNative"},
    {TypeKind::TYPE_UINT8, "UInt8"},
    {TypeKind::TYPE_UINT16, "UInt16"},
    {TypeKind::TYPE_UINT32, "UInt32"},
    {TypeKind::TYPE_UINT64, "UInt64"},
    {TypeKind::TYPE_UINT_NATIVE, "UIntNative"},
    {TypeKind::TYPE_FLOAT16, "Float16"},
    {TypeKind::TYPE_FLOAT32, "Float32"},
    {TypeKind::TYPE_FLOAT64, "Float64"},
    {TypeKind::TYPE_RUNE, "Rune"},
    {TypeKind::TYPE_BOOLEAN, "Bool"},
    {TypeKind::TYPE_IDEAL_INT, "Int"},
    {TypeKind::TYPE_IDEAL_FLOAT, "Float"},
    {TypeKind::TYPE_TUPLE, "Tuple"},
    {TypeKind::TYPE_ENUM, "Enum"},
    {TypeKind::TYPE_FUNC, "Function"},
    {TypeKind::TYPE_ARRAY, "Array"},
    {TypeKind::TYPE_CLASS, "Class"},
    {TypeKind::TYPE_INTERFACE, "Interface"},
    {TypeKind::TYPE, "TypeAlias"},
    {TypeKind::TYPE_STRUCT, "Struct"},
    {TypeKind::TYPE_NOTHING, "Nothing"}
};
}

void Cangjie::Format::PrimitiveTypeExprFormatter::ASTToDoc(
    Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto primitiveTypeExpr = StaticAs<ASTKind::PRIMITIVE_TYPE_EXPR>(node);
    AddPrimitiveTypeExpr(doc, *primitiveTypeExpr, level);
}

void Cangjie::Format::PrimitiveTypeExprFormatter::AddPrimitiveTypeExpr(
    Doc& doc, const Cangjie::AST::PrimitiveTypeExpr& primitiveTypeExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    auto iter = TYPEKIND_TO_STRING_MAP.find(primitiveTypeExpr.typeKind);
    if (iter != TYPEKIND_TO_STRING_MAP.end()) {
        std::string typeStr = iter->second;
        doc.members.emplace_back(DocType::STRING, level, typeStr);
    } else {
        Error("Can't find type kind, Please report this to Cangjie Tools team.");
    }
}
} // namespace Cangjie::Format