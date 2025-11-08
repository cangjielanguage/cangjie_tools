// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_FORMATCODEPROCESSOR_H
#define CJFMT_FORMATCODEPROCESSOR_H

#include "Format/ASTToFormatSource.h"
#include "Format/Doc.h"
#include "cangjie/Basic/Print.h"
#include "cangjie/Parse/Parser.h"
#include "cangjie/Utils/FileUtil.h"
#include "cangjie/AST/Node.h"

#include <dirent.h>
#include <fstream>
#include <istream>
#include <map>
#include <optional>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <regex>

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

using namespace Cangjie;
using namespace Cangjie::AST;
namespace Cangjie::Format {
constexpr int OK = 0;
constexpr int ERR = 1;
const int DEPTH_OF_RECURSION = 0;
/* Sets the maximum recursion depth.The value -1 indicates that the recursion depth is not limited.It will be used as a
 * configuration option later. */
const int MAX_RECURSION_DEPTH = -1;

int FmtDir(const std::string& fmtDirPath, const std::string& dirOutputPath);
std::optional<std::string> FormatText(const std::string& rawCode, const std::string& filepath, Region regionToFormat);
bool FormatFile(std::string& rawCode, const std::string& filepath, std::string& sourceFormat, Region regionToFormat);
bool HasEnding(std::string const& fullString, std::string const& ending);
void TraveDepthLimitedDirs(
    const std::string& path, std::map<std::string, std::string>& fileMap, int depth, const int maxDepth);
std::string GetTargetName(
    const std::string& file, const std::string& fileName, const std::string& fmtDirPath, const std::string& outPath);
std::string PathJoin(const std::string& baseDir, const std::string& baseName);
void RegisterNode(ASTToFormatSource& toSourceManager, FormattingOptions& options);
void RegisterDeclNode(ASTToFormatSource& toSourceManager, FormattingOptions& options);
void RegisterExprNode(ASTToFormatSource& toSourceManager, FormattingOptions& options);
void RegisterPatternNode(ASTToFormatSource& toSourceManager, FormattingOptions& options);
void RegisterTypeNode(ASTToFormatSource& toSourceManager, FormattingOptions& options);
void RegisterDocProcessors(ASTToFormatSource& toSourceManager, FormattingOptions& options);

#ifdef _WIN32
static const std::string DELIMITER = "\\";
#else
static const std::string DELIMITER = "/";
#endif
static const int MAX_LINEBREAKS = 2;

class FormatCodeProcessor {
public:
    FormatCodeProcessor(const std::string& rawCode, const std::string& filepath, const Region region)
        : rawCode(rawCode), filepath(filepath), sm(), diag(), regionToFormat(region), tracker(region)
    {
        diag.SetSourceManager(&sm);
        OptionContext& optionContext = OptionContext::GetInstance();
        this->options = optionContext.GetConfigOptions();
    }

    std::optional<std::string> Run()
    {
        auto inputTokens = RunLexer(rawCode, filepath);
        if (inputTokens.empty()) {
            return "";
        }

        Parser parser(inputTokens, diag, sm);
#ifdef CANGJIE_AD
        (void)parser.SetForImport(true).SetGirAD(true);
#endif
        auto file = parser.ParseTopLevel();
        diag.EmitCategoryGroup();
        if (parser.GetDiagnosticEngine().GetErrorCount() > 0) {
            return std::nullopt;
        }

        std::string result;
        auto formatted = TransformAstToText(file);
        if (tracker.actuallyFormattedStart.has_value() && tracker.actuallyFormattedEnd.has_value()) {
            result = ConstructResultingText(file, inputTokens, formatted);
        } else {
            // couldn't find any ast nodes to format, should report this to user?
            result = rawCode;
        }

        if (result.back() != '\n') {
            result.push_back('\n');
        }
        return result;
    }

    std::string TransformAstToText(const OwnedPtr<File>& file)
    {
        ASTToFormatSource astTransformer(tracker, sm);
        SetupTranformer(astTransformer);
        Doc doc = astTransformer.ASTToDoc(file.get());
        return astTransformer.DocToString(doc);
    }
private:
    const std::string& rawCode;
    const std::string& filepath;
    SourceManager sm;
    DiagnosticEngine diag;
    Region regionToFormat;
    RegionFormattingTracker tracker;
    FormattingOptions options;

    std::vector<Token> GetTokensBetweenPositions(
        const std::vector<Token>& allTokens, const Cangjie::Position& start, const Cangjie::Position& end)
    {
        std::vector<Token> result;
        auto iterator = allTokens.begin();

        while (iterator != allTokens.end() && iterator->Begin() < start) {
            ++iterator;
        }
        while (iterator != allTokens.end() && iterator->End() <= end) {
            result.push_back(*iterator);
            ++iterator;
        }
        return result;
    }

    int CalculateIndentForCodeFragment(
        const std::vector<Token>& originalFragmentTokens, const std::vector<Token>& formattedFragmentTokens)
    {
        auto formattedFront = formattedFragmentTokens.front().Begin();
        auto originalFront = originalFragmentTokens.front().Begin();
        auto formattedPrefix =
            sm.GetContentBetween(Position(formattedFront.fileID, formattedFront.line, 1), formattedFront);
        auto isOnNewLineInFormattedCode = formattedPrefix.find_first_not_of(" \t") == std::string::npos;
        auto originalPrefix =
            sm.GetContentBetween(Position(originalFront.fileID, originalFront.line, 1), originalFront);
        auto isOnNewLineInOriginalCode = originalPrefix.find_first_not_of(" \t") == std::string::npos;
        int indentation;
        if (isOnNewLineInFormattedCode && isOnNewLineInOriginalCode) {
            indentation = formattedFront.column - 1;
        } else if (!isOnNewLineInFormattedCode && !isOnNewLineInOriginalCode) {
            auto lastSymbolInOriginalCodeColumn = formattedPrefix.find_last_not_of(" \t") + 1;
            auto indentationInFormattedText =
                (formattedFront.column - static_cast<int>(lastSymbolInOriginalCodeColumn)) - 1;
            indentation = indentationInFormattedText;
        } else if (isOnNewLineInOriginalCode && !isOnNewLineInFormattedCode) {
            indentation = originalFront.column - 1;
        } else { // !isOnNewLineInOriginal && isOnNewLineInFormattedCode
            indentation = 1;
        }
        return indentation;
    }

    std::string ConstructTextForCodeFragment(
        const std::vector<Token>& originalFileAllTokens, const std::vector<Token>& formattedTextTokens)
    {
        auto formattedBegin = tracker.actuallyFormattedStart.value();
        auto formattedEnd = tracker.actuallyFormattedEnd.value();
        auto originalFragmentTokens = GetTokensBetweenPositions(originalFileAllTokens, formattedBegin, formattedEnd);
        auto formattedFragmentTokens = formattedTextTokens;
        auto preciseFragmentBounds = tracker.GetPreciseFragmentBounds(sm);
        if (preciseFragmentBounds.has_value()) {
            auto smallerFragments = ExtractTokensBetweenPositions(originalFragmentTokens, formattedFragmentTokens,
                preciseFragmentBounds->first, preciseFragmentBounds->second);
            if (smallerFragments.has_value()) {
                std::tie(originalFragmentTokens, formattedFragmentTokens) = smallerFragments.value();
            }
        }

        auto fragmentText = InsertComments(originalFragmentTokens, formattedFragmentTokens, sm);
        auto contentBefore =
            sm.GetContentBetween(originalFileAllTokens.front().Begin(), originalFragmentTokens.front().Begin());
        auto contentAfter =
            sm.GetContentBetween(originalFragmentTokens.back().End(), originalFileAllTokens.back().End());

        (void)contentBefore.erase(contentBefore.find_last_not_of(" \t") + 1);

        auto blankLinesBeforeSnippet = originalFragmentTokens.front().Begin().line - tracker.shouldFormatBegin.line;
        if (blankLinesBeforeSnippet > 1) {
            TrimTrailingEmptyLines(contentBefore, static_cast<size_t>(blankLinesBeforeSnippet));
        }
        (void)contentBefore.erase(contentBefore.find_last_not_of(" ") + 1);

        auto blankLinesAfterSnippet = tracker.shouldFormatEnd.line - originalFragmentTokens.back().Begin().line;
        if (blankLinesAfterSnippet > 1) {
            TrimLeadingEmptyLines(contentAfter, static_cast<size_t>(blankLinesAfterSnippet - 1));
        }

        auto indentation = CalculateIndentForCodeFragment(originalFragmentTokens, formattedFragmentTokens);
        for (int i = 0; i < indentation; ++i) {
            contentBefore.push_back(' ');
        }

        return contentBefore + fragmentText + contentAfter;
    }

    std::string ConstructResultingText(OwnedPtr<File>&, const std::vector<Token>& inputTokens, std::string& formatted)
    {
        (void)formatted.erase(0, formatted.find_first_not_of(" \t\n"));
        (void)formatted.erase(formatted.find_last_not_of(" \t\n") + 1);
        auto formattedTokens = RunLexer(formatted, "___formattedSnippet.cj");
        if (!regionToFormat.isWholeFile) {
            return ConstructTextForCodeFragment(inputTokens, formattedTokens);
        } else {
            return InsertComments(inputTokens, formattedTokens, sm);
        }
    }

    std::vector<Token> RunLexer(const std::string& code, const std::string& path)
    {
        auto fileID = sm.AddSource(path, code);
        auto lexer = Lexer(fileID, code, diag, sm);
        return lexer.GetTokens();
    }

    void SetupTranformer(ASTToFormatSource& transformer)
    {
        RegisterNode(transformer, options);
        RegisterDeclNode(transformer, options);
        RegisterExprNode(transformer, options);
        RegisterPatternNode(transformer, options);
        RegisterTypeNode(transformer, options);
        RegisterDocProcessors(transformer, options);
    }

    void TrimTrailingEmptyLines(std::string& str, size_t maxLinesToTrim)
    {
        size_t currentPos = str.length();
        size_t lineBreakCount = 0;
        for (size_t i = str.length() - 1; i > 0; i--) {
            auto ch = str[i];
            if (!std::isspace(ch)) {
                break;
            }
            if (ch == '\n') {
                lineBreakCount++;
                currentPos = i + 1;
                if (lineBreakCount > maxLinesToTrim) {
                    break;
                }
            }
        }
        str.erase(lineBreakCount >= MAX_LINEBREAKS ? currentPos + 1 : currentPos);
    }

    void TrimLeadingEmptyLines(std::string& str, size_t maxLinesToTrim)
    {
        size_t currentPos = 0;
        size_t lineBreakCount = 0;
        for (size_t i = 0; i < str.length() - 1; i++) {
            auto ch = str[i];
            if (!std::isspace(ch)) {
                break;
            }
            if (ch == '\n') {
                lineBreakCount++;
                currentPos = i;
                if (lineBreakCount > maxLinesToTrim) {
                    break;
                }
            }
        }
        str.erase(0, currentPos);
    }
};

} // namespace
#endif // CJFMT_FORMATCODEPROCESSOR_H

