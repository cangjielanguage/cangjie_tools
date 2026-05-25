// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/FormatCodeProcessor.h"

#include <dirent.h>
#include <istream>
#include <map>
#include <optional>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "Format/ASTToFormatSource.h"
#include "Format/CommentHandler.h"
#include "Format/Doc.h"
#include "Format/DocProcessor/ArgsProcessor.h"
#include "Format/DocProcessor/BreakParentTypeProcessor.h"
#include "Format/DocProcessor/ConcatTypeProcessor.h"
#include "Format/DocProcessor/DotProcessor.h"
#include "Format/DocProcessor/FuncArgProcessor.h"
#include "Format/DocProcessor/GroupProcessor.h"
#include "Format/DocProcessor/LambdaBodyProcessor.h"
#include "Format/DocProcessor/LambdaProcessor.h"
#include "Format/DocProcessor/LineDotProcessor.h"
#include "Format/DocProcessor/LineProcessor.h"
#include "Format/DocProcessor/MemberAccessProcessor.h"
#include "Format/DocProcessor/SeparateProcessor.h"
#include "Format/DocProcessor/SoftLineTypeProcessor.h"
#include "Format/DocProcessor/SoftLineWithSpaceTypeProcessor.h"
#include "Format/DocProcessor/StringTypeProcessor.h"
#include "Format/NodeFomatter/Decl/ClassDeclFormatter.h"
#include "Format/NodeFomatter/Decl/EnumDeclFormatter.h"
#include "Format/NodeFomatter/Decl/ExtendDeclFormatter.h"
#include "Format/NodeFomatter/Decl/FuncDeclFormatter.h"
#include "Format/NodeFomatter/Decl/GenericParamDeclFormatter.h"
#include "Format/NodeFomatter/Decl/InterfaceDeclFormatter.h"
#include "Format/NodeFomatter/Decl/MacroDeclFormatter.h"
#include "Format/NodeFomatter/Decl/MacroExpandDeclFormatter.h"
#include "Format/NodeFomatter/Decl/MacroExpandParamFormatter.h"
#include "Format/NodeFomatter/Decl/MainDeclFormatter.h"
#include "Format/NodeFomatter/Decl/PrimaryCtorDeclFormatter.h"
#include "Format/NodeFomatter/Decl/PropDeclFormatter.h"
#include "Format/NodeFomatter/Decl/StructDeclFormatter.h"
#include "Format/NodeFomatter/Decl/TypeAliasDeclFormatter.h"
#include "Format/NodeFomatter/Decl/VarDeclFormatter.h"
#include "Format/NodeFomatter/Decl/VarWithPatternDeclFormatter.h"
#include "Format/NodeFomatter/Expr/ArrayExprFormatter.h"
#include "Format/NodeFomatter/Expr/ArrayLitFormatter.h"
#include "Format/NodeFomatter/Expr/AsExprFormatter.h"
#include "Format/NodeFomatter/Expr/AssignExprFormatter.h"
#include "Format/NodeFomatter/Expr/BinaryExprFormatter.h"
#include "Format/NodeFomatter/Expr/BlockFormatter.h"
#include "Format/NodeFomatter/Expr/CallExprFormatter.h"
#include "Format/NodeFomatter/Expr/DoWhileExprFormatter.h"
#include "Format/NodeFomatter/Expr/ForInExprFormatter.h"
#include "Format/NodeFomatter/Expr/IfExprFormatter.h"
#include "Format/NodeFomatter/Expr/IncOrDecExprFormatter.h"
#include "Format/NodeFomatter/Expr/IsExprFormatter.h"
#include "Format/NodeFomatter/Expr/JumpExprFormatter.h"
#include "Format/NodeFomatter/Expr/LambdaExprFormatter.h"
#include "Format/NodeFomatter/Expr/LetPatternDestructorFormatter.h"
#include "Format/NodeFomatter/Expr/LitConstExprFormatter.h"
#include "Format/NodeFomatter/Expr/MacroExpandExprFormatter.h"
#include "Format/NodeFomatter/Expr/MatchExprFormatter.h"
#include "Format/NodeFomatter/Expr/MemberAccessFormatter.h"
#include "Format/NodeFomatter/Expr/OptionalChainExprFormatter.h"
#include "Format/NodeFomatter/Expr/OptionalExprFormatter.h"
#include "Format/NodeFomatter/Expr/ParenExprFormatter.h"
#include "Format/NodeFomatter/Expr/PrimitiveTypeExprFormatter.h"
#include "Format/NodeFomatter/Expr/QuoteExprFormatter.h"
#include "Format/NodeFomatter/Expr/RangeExprFormatter.h"
#include "Format/NodeFomatter/Expr/RefExprFormatter.h"
#include "Format/NodeFomatter/Expr/ReturnExprFormatter.h"
#include "Format/NodeFomatter/Expr/SpawnExprFormatter.h"
#include "Format/NodeFomatter/Expr/SubscriptExprFormatter.h"
#include "Format/NodeFomatter/Expr/SynchronizedExprFormatter.h"
#include "Format/NodeFomatter/Expr/ThrowExprFormatter.h"
#include "Format/NodeFomatter/Expr/TrailingClosureExprFormatter.h"
#include "Format/NodeFomatter/Expr/TryExprFormatter.h"
#include "Format/NodeFomatter/Expr/TupleLitFormatter.h"
#include "Format/NodeFomatter/Expr/TypeConvExprFormatter.h"
#include "Format/NodeFomatter/Expr/UnaryExprFormatter.h"
#include "Format/NodeFomatter/Expr/WhileExprFormatter.h"
#include "Format/NodeFomatter/Expr/WildcardExprFormatter.h"
#include "Format/NodeFomatter/Node/AnnotationFormatter.h"
#include "Format/NodeFomatter/Node/ClassBodyFormatter.h"
#include "Format/NodeFomatter/Node/FileFormatter.h"
#include "Format/NodeFomatter/Node/FuncArgFormatter.h"
#include "Format/NodeFomatter/Node/FuncBodyFormatter.h"
#include "Format/NodeFomatter/Node/FuncParamFormatter.h"
#include "Format/NodeFomatter/Node/FuncParamListFormatter.h"
#include "Format/NodeFomatter/Node/GenericConstraintFormatter.h"
#include "Format/NodeFomatter/Node/ImportSpecFormatter.h"
#include "Format/NodeFomatter/Node/InterfaceBodyFormatter.h"
#include "Format/NodeFomatter/Node/MatchCaseFormatter.h"
#include "Format/NodeFomatter/Node/MatchCaseOtherFormatter.h"
#include "Format/NodeFomatter/Node/ModifierFormatter.h"
#include "Format/NodeFomatter/Node/PackageFormatter.h"
#include "Format/NodeFomatter/Node/PackageSpecFormatter.h"
#include "Format/NodeFomatter/Node/StructBodyFormatter.h"
#include "Format/NodeFomatter/Pattern/ConstPatternFormatter.h"
#include "Format/NodeFomatter/Pattern/EnumPatternFormatter.h"
#include "Format/NodeFomatter/Pattern/ExceptTypePatternFormatter.h"
#include "Format/NodeFomatter/Pattern/TuplePatternFormatter.h"
#include "Format/NodeFomatter/Pattern/TypePatternFormatter.h"
#include "Format/NodeFomatter/Pattern/VarOrEnumPatternFormatter.h"
#include "Format/NodeFomatter/Pattern/VarPatternFormatter.h"
#include "Format/NodeFomatter/Pattern/WildcardPatternFormatter.h"
#include "Format/NodeFomatter/Type/ConstantTypeFormatter.h"
#include "Format/NodeFomatter/Type/FuncTypeFormatter.h"
#include "Format/NodeFomatter/Type/OptionTypeFormatter.h"
#include "Format/NodeFomatter/Type/ParenTypeFormatter.h"
#include "Format/NodeFomatter/Type/PrimitiveTypeFormatter.h"
#include "Format/NodeFomatter/Type/QualifiedTypeFormatter.h"
#include "Format/NodeFomatter/Type/RefTypeFormatter.h"
#include "Format/NodeFomatter/Type/ThisTypeFormatter.h"
#include "Format/NodeFomatter/Type/TupleTypeFormatter.h"
#include "Format/NodeFomatter/Type/VArrayTypeFormatter.h"
#include "Format/OptionContext.h"
#include "Format/SimultaneousIterator.h"
#include "Format/TomlParser.h"
#include "cangjie/Basic/Print.h"
#include "cangjie/Parse/Parser.h"
#include "cangjie/Utils/FileUtil.h"
#ifdef CANGJIE_AD
#include "Format/NodeFomatter/Expr/AdjointExprFormatter.h"
#include "Format/NodeFomatter/Expr/GradExprFormatter.h"
#include "Format/NodeFomatter/Expr/VJPExprFormatter.h"
#include "Format/NodeFomatter/Expr/ValWithGradExprFormatter.h"
#endif

namespace Cangjie::Format {
using namespace Cangjie;
using namespace Cangjie::AST;

void RegisterNode(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterNode<PackageFormatter>(ASTKind::PACKAGE, options);
    toSourceManager.RegisterNode<PackageSpecFormatter>(ASTKind::PACKAGE_SPEC, options);
    toSourceManager.RegisterNode<FileFormatter>(ASTKind::FILE, options);
    toSourceManager.RegisterNode<ImportSpecFormatter>(ASTKind::IMPORT_SPEC, options);
    toSourceManager.RegisterNode<ClassBodyFormatter>(ASTKind::CLASS_BODY, options);
    toSourceManager.RegisterNode<StructBodyFormatter>(ASTKind::STRUCT_BODY, options);
    toSourceManager.RegisterNode<InterfaceBodyFormatter>(ASTKind::INTERFACE_BODY, options);
    toSourceManager.RegisterNode<FuncBodyFormatter>(ASTKind::FUNC_BODY, options);
    toSourceManager.RegisterNode<FuncParamFormatter>(ASTKind::FUNC_PARAM, options);
    toSourceManager.RegisterNode<FuncParamListFormatter>(ASTKind::FUNC_PARAM_LIST, options);
    toSourceManager.RegisterNode<FuncArgFormatter>(ASTKind::FUNC_ARG, options);
    toSourceManager.RegisterNode<AnnotationFormatter>(ASTKind::ANNOTATION, options);
    toSourceManager.RegisterNode<GenericConstraintFormatter>(ASTKind::GENERIC_CONSTRAINT, options);
    toSourceManager.RegisterNode<MatchCaseFormatter>(ASTKind::MATCH_CASE, options);
    toSourceManager.RegisterNode<MatchCaseOtherFormatter>(ASTKind::MATCH_CASE_OTHER, options);
    toSourceManager.RegisterNode<ModifierFormatter>(ASTKind::MODIFIER, options);
}

void RegisterDeclNode(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterNode<ClassDeclFormatter>(ASTKind::CLASS_DECL, options);
    toSourceManager.RegisterNode<InterfaceDeclFormatter>(ASTKind::INTERFACE_DECL, options);
    toSourceManager.RegisterNode<EnumDeclFormatter>(ASTKind::ENUM_DECL, options);
    toSourceManager.RegisterNode<MacroDeclFormatter>(ASTKind::MACRO_DECL, options);
    toSourceManager.RegisterNode<ExtendDeclFormatter>(ASTKind::EXTEND_DECL, options);
    toSourceManager.RegisterNode<MainDeclFormatter>(ASTKind::MAIN_DECL, options);
    toSourceManager.RegisterNode<FuncDeclFormatter>(ASTKind::FUNC_DECL, options);
    toSourceManager.RegisterNode<TypeAliasDeclFormatter>(ASTKind::TYPE_ALIAS_DECL, options);
    toSourceManager.RegisterNode<GenericParamDeclFormatter>(ASTKind::GENERIC_PARAM_DECL, options);
    toSourceManager.RegisterNode<MacroExpandDeclFormatter>(ASTKind::MACRO_EXPAND_DECL, options);
    toSourceManager.RegisterNode<MacroExpandParamFormatter>(ASTKind::MACRO_EXPAND_PARAM, options);
    toSourceManager.RegisterNode<VarDeclFormatter>(ASTKind::VAR_DECL, options);
    toSourceManager.RegisterNode<VarWithPatternDeclFormatter>(ASTKind::VAR_WITH_PATTERN_DECL, options);
    toSourceManager.RegisterNode<PropDeclFormatter>(ASTKind::PROP_DECL, options);
    toSourceManager.RegisterNode<StructDeclFormatter>(ASTKind::STRUCT_DECL, options);
    toSourceManager.RegisterNode<PrimaryCtorDeclFormatter>(ASTKind::PRIMARY_CTOR_DECL, options);
}

void RegisterExprNode(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterNode<BlockFormatter>(ASTKind::BLOCK, options);
    toSourceManager.RegisterNode<LetPatternDestructorFormatter>(ASTKind::LET_PATTERN_DESTRUCTOR, options);
    toSourceManager.RegisterNode<LitConstExprFormatter>(ASTKind::LIT_CONST_EXPR, options);
#ifdef CANGJIE_AD
    toSourceManager.RegisterNode<GradExprFormatter>(ASTKind::GRAD_EXPR, options);
    toSourceManager.RegisterNode<VJPExprFormatter>(ASTKind::VJP_EXPR, options);
    toSourceManager.RegisterNode<ValWithGradExprFormatter>(ASTKind::VALWITHGRAD_EXPR, options);
    toSourceManager.RegisterNode<AdjointExprFormatter>(ASTKind::ADJOINT_EXPR, options);
#endif
    toSourceManager.RegisterNode<CallExprFormatter>(ASTKind::CALL_EXPR, options);
    toSourceManager.RegisterNode<MemberAccessFormatter>(ASTKind::MEMBER_ACCESS, options);
    toSourceManager.RegisterNode<RefExprFormatter>(ASTKind::REF_EXPR, options);
    toSourceManager.RegisterNode<PrimitiveTypeExprFormatter>(ASTKind::PRIMITIVE_TYPE_EXPR, options);
    toSourceManager.RegisterNode<ArrayLitFormatter>(ASTKind::ARRAY_LIT, options);
    toSourceManager.RegisterNode<TypeConvExprFormatter>(ASTKind::TYPE_CONV_EXPR, options);
    toSourceManager.RegisterNode<SpawnExprFormatter>(ASTKind::SPAWN_EXPR, options);
    toSourceManager.RegisterNode<SynchronizedExprFormatter>(ASTKind::SYNCHRONIZED_EXPR, options);
    toSourceManager.RegisterNode<TryExprFormatter>(ASTKind::TRY_EXPR, options);
    toSourceManager.RegisterNode<ThrowExprFormatter>(ASTKind::THROW_EXPR, options);
    toSourceManager.RegisterNode<LambdaExprFormatter>(ASTKind::LAMBDA_EXPR, options);
    toSourceManager.RegisterNode<ReturnExprFormatter>(ASTKind::RETURN_EXPR, options);
    toSourceManager.RegisterNode<UnaryExprFormatter>(ASTKind::UNARY_EXPR, options);
    toSourceManager.RegisterNode<BinaryExprFormatter>(ASTKind::BINARY_EXPR, options);
    toSourceManager.RegisterNode<AssignExprFormatter>(ASTKind::ASSIGN_EXPR, options);
    toSourceManager.RegisterNode<IfExprFormatter>(ASTKind::IF_EXPR, options);
    toSourceManager.RegisterNode<IsExprFormatter>(ASTKind::IS_EXPR, options);
    toSourceManager.RegisterNode<AsExprFormatter>(ASTKind::AS_EXPR, options);
    toSourceManager.RegisterNode<JumpExprFormatter>(ASTKind::JUMP_EXPR, options);
    toSourceManager.RegisterNode<WildcardExprFormatter>(ASTKind::WILDCARD_EXPR, options);
    toSourceManager.RegisterNode<IncOrDecExprFormatter>(ASTKind::INC_OR_DEC_EXPR, options);
    toSourceManager.RegisterNode<ForInExprFormatter>(ASTKind::FOR_IN_EXPR, options);
    toSourceManager.RegisterNode<QuoteExprFormatter>(ASTKind::QUOTE_EXPR, options);
    toSourceManager.RegisterNode<MacroExpandExprFormatter>(ASTKind::MACRO_EXPAND_EXPR, options);
    toSourceManager.RegisterNode<WhileExprFormatter>(ASTKind::WHILE_EXPR, options);
    toSourceManager.RegisterNode<DoWhileExprFormatter>(ASTKind::DO_WHILE_EXPR, options);
    toSourceManager.RegisterNode<ParenExprFormatter>(ASTKind::PAREN_EXPR, options);
    toSourceManager.RegisterNode<RangeExprFormatter>(ASTKind::RANGE_EXPR, options);
    toSourceManager.RegisterNode<MatchExprFormatter>(ASTKind::MATCH_EXPR, options);
    toSourceManager.RegisterNode<SubscriptExprFormatter>(ASTKind::SUBSCRIPT_EXPR, options);
    toSourceManager.RegisterNode<TrailingClosureExprFormatter>(ASTKind::TRAIL_CLOSURE_EXPR, options);
    toSourceManager.RegisterNode<ArrayExprFormatter>(ASTKind::ARRAY_EXPR, options);
    toSourceManager.RegisterNode<OptionalChainExprFormatter>(ASTKind::OPTIONAL_CHAIN_EXPR, options);
    toSourceManager.RegisterNode<OptionalExprFormatter>(ASTKind::OPTIONAL_EXPR, options);
    toSourceManager.RegisterNode<TupleLitFormatter>(ASTKind::TUPLE_LIT, options);
}

void RegisterPatternNode(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterNode<EnumPatternFormatter>(ASTKind::ENUM_PATTERN, options);
    toSourceManager.RegisterNode<VarOrEnumPatternFormatter>(ASTKind::VAR_OR_ENUM_PATTERN, options);
    toSourceManager.RegisterNode<TuplePatternFormatter>(ASTKind::TUPLE_PATTERN, options);
    toSourceManager.RegisterNode<VarPatternFormatter>(ASTKind::VAR_PATTERN, options);
    toSourceManager.RegisterNode<ConstPatternFormatter>(ASTKind::CONST_PATTERN, options);
    toSourceManager.RegisterNode<TypePatternFormatter>(ASTKind::TYPE_PATTERN, options);
    toSourceManager.RegisterNode<ExceptTypePatternFormatter>(ASTKind::EXCEPT_TYPE_PATTERN, options);
    toSourceManager.RegisterNode<WildcardPatternFormatter>(ASTKind::WILDCARD_PATTERN, options);
}

void RegisterTypeNode(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterNode<PrimitiveTypeFormatter>(ASTKind::PRIMITIVE_TYPE, options);
    toSourceManager.RegisterNode<RefTypeFormatter>(ASTKind::REF_TYPE, options);
    toSourceManager.RegisterNode<ThisTypeFormatter>(ASTKind::THIS_TYPE, options);
    toSourceManager.RegisterNode<ParenTypeFormatter>(ASTKind::PAREN_TYPE, options);
    toSourceManager.RegisterNode<TupleTypeFormatter>(ASTKind::TUPLE_TYPE, options);
    toSourceManager.RegisterNode<QualifiedTypeFormatter>(ASTKind::QUALIFIED_TYPE, options);
    toSourceManager.RegisterNode<FuncTypeFormatter>(ASTKind::FUNC_TYPE, options);
    toSourceManager.RegisterNode<OptionTypeFormatter>(ASTKind::OPTION_TYPE, options);
    toSourceManager.RegisterNode<VArrayTypeFormatter>(ASTKind::VARRAY_TYPE, options);
    toSourceManager.RegisterNode<ConstantTypeFormatter>(ASTKind::CONSTANT_TYPE, options);
}

void RegisterDocProcessors(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterDocProcessors<StringTypeProcessor>(DocType::STRING, options);
    toSourceManager.RegisterDocProcessors<ConcatTypeProcessor>(DocType::CONCAT, options);
    toSourceManager.RegisterDocProcessors<LineProcessor>(DocType::LINE, options);
    toSourceManager.RegisterDocProcessors<ArgsProcessor>(DocType::ARGS, options);
    toSourceManager.RegisterDocProcessors<FuncArgProcessor>(DocType::FUNC_ARG, options);
    toSourceManager.RegisterDocProcessors<LambdaBodyProcessor>(DocType::LAMBDA_BODY, options);
    toSourceManager.RegisterDocProcessors<MemberAccessProcessor>(DocType::MEMBER_ACCESS, options);
    toSourceManager.RegisterDocProcessors<LambdaProcessor>(DocType::LAMBDA, options);
    toSourceManager.RegisterDocProcessors<LineDotProcessor>(DocType::LINE_DOT, options);
    toSourceManager.RegisterDocProcessors<DotProcessor>(DocType::DOT, options);
    toSourceManager.RegisterDocProcessors<SeparateProcessor>(DocType::SEPARATE, options);
    toSourceManager.RegisterDocProcessors<GroupProcessor>(DocType::GROUP, options);
    toSourceManager.RegisterDocProcessors<SoftLineWithSpaceTypeProcessor>(DocType::SOFTLINE_WITH_SPACE, options);
    toSourceManager.RegisterDocProcessors<SoftLineTypeProcessor>(DocType::SOFTLINE, options);
    toSourceManager.RegisterDocProcessors<BreakParentTypeProcessor>(DocType::BREAK_PARENT, options);
}

std::optional<std::string> FormatText(const std::string& rawCode, const std::string& filepath, const Region region)
{
    auto formatCode = FormatCodeProcessor(rawCode, filepath, region);
    return formatCode.Run();
}

bool FormatFile(std::string& rawCode, const std::string& filepath, std::string& sourceFormat, Region regionToFormat)
{
    std::ifstream instream(filepath);
    std::string buffer((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
    instream.close();
    rawCode = buffer;
    if (regionToFormat.startLine == -1 || regionToFormat.endLine == -1) {
        regionToFormat = Region::wholeFile;
    }
    auto formatted = FormatText(rawCode, filepath, regionToFormat);
    if (!formatted) {
        return false;
    }
    sourceFormat = formatted.value();
    return true;
}

std::string GetTargetName(
    const std::string& file, const std::string& fileName, const std::string& fmtDirPath, const std::string& outPath)
{
    std::string outPathRel;
    std::string targetName;
    if (!outPath.empty()) {
        std::string relPathNotFName = file.substr(0, file.size() - fileName.size());
        auto relPath = FileUtil::GetRelativePath(fmtDirPath, relPathNotFName) | FileUtil::IdenticalFunc;
        outPathRel = outPath + DELIMITER + relPath;
        int creatRel = 0;
        if (!FileUtil::IsDir(outPathRel)) {
            creatRel = FileUtil::CreateDirs(outPathRel);
        }
        if (creatRel != -1) {
            targetName = outPath + DELIMITER + relPath + fileName;
        }
    } else {
        targetName = file;
    }
    return targetName;
}

int FmtDir(const std::string& fmtDirPath, const std::string& dirOutputPath)
{
    std::regex reg = std::regex("[*|;&$><`?!\\n]+");
    if (std::regex_search(fmtDirPath, reg)) {
        Errorln("The path cannot contain invalid characters: *|;&$><`?!");
        return ERR;
    }
    auto absDirPath = FileUtil::GetAbsPath(fmtDirPath) | FileUtil::IdenticalFunc;
    std::map<std::string, std::string> filesMap;
    /* Obtain the path and name of the source file. Save to file map. */
    TraveDepthLimitedDirs(absDirPath, filesMap, DEPTH_OF_RECURSION, MAX_RECURSION_DEPTH);
    for (auto& file : filesMap) {
        std::string sourceFormat, targetName, rawCode;
        /* Format the file content and save the formatted content to sourceFormat. */
        if (!FormatFile(rawCode, file.first, sourceFormat, Region::wholeFile)) {
            sourceFormat = rawCode;
        }
        /* Obtain the output file path based on the output path and retain the source file directory structure. */
        targetName = GetTargetName(file.first, file.second, absDirPath, dirOutputPath);
        if (targetName.empty()) {
            Errorln("Create target file error.");
            return ERR;
        }
        std::ofstream outStream(targetName, std::ios::out | std::ofstream::binary);
        if (!outStream) {
            Errorln("Create target file error.");
            return ERR;
        }
        auto length = sourceFormat.size();
        (void)outStream.write(sourceFormat.c_str(), static_cast<long>(length));
        outStream.close();
    }
    return OK;
}

std::string PathJoin(const std::string& baseDir, const std::string& baseName)
{
    std::string lastChar(1, baseDir.at(baseDir.length() - 1));

    if (lastChar == DELIMITER) {
        return baseDir + baseName;
    } else {
        return baseDir + DELIMITER + baseName;
    }
}

bool HasEnding(std::string const& fullString, std::string const& ending)
{
    if (fullString.length() >= ending.length()) {
        return (fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0);
    }
    return false;
}

void TraveDepthLimitedDirs(
    const std::string& path, std::map<std::string, std::string>& fileMap, int depth, const int maxDepth)
{
    if (depth >= maxDepth && maxDepth != MAX_RECURSION_DEPTH) {
        return;
    }
    DIR* pd = opendir(path.c_str());
    if (!pd) {
        std::cout << "cant open dirs(" << path << "), opendir fail" << std::endl;
        return;
    }
    dirent* dir = nullptr;
    struct stat statBuf {};
    while ((dir = readdir(pd)) != nullptr) {
        std::string dName = std::string(dir->d_name);
        std::string subFileOrDir = PathJoin(path, dName);
        (void)stat(subFileOrDir.c_str(), &statBuf); // use lstat if want to ignore symbolic-link-dir
        if (S_ISDIR(statBuf.st_mode)) {
            // continue if present dir or  parent dir
            if (dName == "." || dName == "..") {
                continue;
            }
            // traverse sub-dir
            TraveDepthLimitedDirs(subFileOrDir, fileMap, depth + 1, maxDepth);
        } else {
            std::string name = dir->d_name;
            if (HasEnding(name, ".cj")) {
                (void)fileMap.emplace(subFileOrDir, name);
            }
            continue;
        }
    }
    (void)closedir(pd);
}
}
