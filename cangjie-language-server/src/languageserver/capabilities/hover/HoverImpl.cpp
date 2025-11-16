// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "HoverImpl.h"
#include "cangjie/Lex/Token.h"

namespace {
    const int NUMBER_FOR_LINE_COMMENT = 2;       // length of "//"
    const int NUMBER_FOR_BLOCK_COMMENT = 2;      // length of "/*" or "*/"
    const int NUMBER_FOR_DOC_COMMENT = 3;        // length of "/**"
}

namespace ark {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace CONSTANTS;

std::string HoverImpl::curFilePath = "";

std::vector<std::string> HoverImpl::StringSplit(const std::string &src, const std::string &separateCharacter)
{
    std::vector<std::string> strs;
    size_t separateCharacterLen = separateCharacter.size();
    size_t lastPosition = 0;
    size_t index = 0;
    while ((index = src.find(separateCharacter, lastPosition)) != std::string::npos) {
        strs.push_back(src.substr(lastPosition, index - lastPosition));
        lastPosition = index + separateCharacterLen;
    }
    std::string lastString = src.substr(lastPosition);
    if (!lastString.empty())
        strs.push_back(lastString);
    return strs;
}

void HoverImpl::TrimSpaceAndTab(std::string &s)
{
    if (s.empty()) {
        return;
    }
    int begin = 0;
    int end = static_cast<int>(s.size()) - 1;
    // resolve spaces in full-width mode
    std::string fullWidthSpace = "\u3000";
    int fullWidthSpaceLen = static_cast<int>(fullWidthSpace.size());
    bool isInvalidHead = true;
    bool isInvalidTail = true;
    while (begin <= end && (isInvalidHead || isInvalidTail)) {
        if (isInvalidHead) {
            if (begin + fullWidthSpaceLen - 1 <= end && s.substr(begin, fullWidthSpaceLen) == fullWidthSpace) {
                begin += fullWidthSpaceLen;
            } else if (s[begin] == ' ' || s[begin] == '\t' || s[begin] == '\r') {
                begin++;
            } else {
                isInvalidHead = false;
            }
        }
        if (isInvalidTail) {
            if ((end - fullWidthSpaceLen) + 1 >= begin &&
                s.substr((end - fullWidthSpaceLen) + 1, fullWidthSpaceLen) == fullWidthSpace) {
                end -= fullWidthSpaceLen;
            } else if (s[end] == ' ' || s[end] == '\t' || s[end] == '\r') {
                end--;
            } else {
                isInvalidTail = false;
            }
        }
    }
    s = begin > end ? "" : s.substr(begin, (end - begin) + 1);
}

void HoverImpl::GetEffectiveContent(std::string &content, bool isHeadOrTail)
{
    TrimSpaceAndTab(content);
    if (isHeadOrTail) {
        if (content == "/**" ||
            (content.size() > NUMBER_FOR_DOC_COMMENT && content.substr(0, NUMBER_FOR_DOC_COMMENT) == "/**" &&
             content[NUMBER_FOR_DOC_COMMENT] != '/')) {
            content = content.substr(NUMBER_FOR_DOC_COMMENT);
        } else if (content.substr(0, NUMBER_FOR_BLOCK_COMMENT) == "/*") {
            content = content.substr(NUMBER_FOR_BLOCK_COMMENT);
        } else if (content.size() > 1 && content.substr(content.size() - NUMBER_FOR_BLOCK_COMMENT) == "*/") {
            content = content.substr(0, content.size() - NUMBER_FOR_BLOCK_COMMENT);
        }
    } else {
        auto index = content.find_first_not_of('*');
        if (index != std::string::npos) {
            content = content.substr(index);
        } else {
            content = "";
        }
    }
    TrimSpaceAndTab(content);
}

void HoverImpl::TrimBlankLines(std::vector<std::string> &result)
{
    if (result.empty()) {
        return;
    }
    bool isValidHead = false;
    bool isValidTail = false;
    for (auto &str : result) {
        TrimSpaceAndTab(str);
    }
    // the first and last element that is not a newline character, skip /** and */
    std::vector<std::string> str;
    // struct whether each result has been processed
    std::vector<bool> hasMarked(result.size());
    size_t begin = 0;
    size_t end = result.size() - 1;
    GetEffectiveContent(result[begin], true);
    GetEffectiveContent(result[end], true);
    while (begin < end && (!isValidHead || !isValidTail)) {
        if (!hasMarked[begin]) {
            hasMarked[begin] = true;
            GetEffectiveContent(result[begin], false);
        }
        if (!hasMarked[end]) {
            hasMarked[end] = true;
            GetEffectiveContent(result[end], false);
        }
        if (!isValidHead) {
            if (!result[begin].empty()) {
                isValidHead = true;
            } else {
                begin++;
            }
        }
        if (!isValidTail) {
            if (!result[end].empty()) {
                isValidTail = true;
            } else {
                end--;
            }
        }
    }
    for (; begin <= end; begin++) {
        if (!hasMarked[begin]) {
            hasMarked[begin] = true;
            GetEffectiveContent(result[begin], false);
        }
        str.push_back(result[begin]);
    }
    result = str;
}

std::string HoverImpl::GetHoverMessageForBlockComment(const std::vector<std::string> &result)
{
    std::string retBlockComment = "";
    for (const auto &str : result) {
        retBlockComment += str + CONSTANTS::BLANK;
    }
    if (!retBlockComment.empty()) {
        retBlockComment = retBlockComment.substr(0, retBlockComment.size() - CONSTANTS::BLANK.size());
    }
    return retBlockComment;
}

std::string HoverImpl::GetEffectiveDocComment(std::vector<std::string> &content, const std::string &blockTag)
{
    // 4 spaces for indent
    const std::string indent = "    ";
    std::string retContent;
    if (content.empty()) {
        return "";
    }
    if (blockTag != "@param") {
        for (auto str : content) {
            ConvertCarriageToSpace(str);
            TrimSpaceAndTab(str);
            if (blockTag != "@default" && blockTag != "@brief") {
                retContent += indent + str + CONSTANTS::BLANK;
                continue;
            }
            retContent += str + CONSTANTS::BLANK;
        }
        return retContent;
    }
    size_t maxLenParam = 0U;
    auto locationOfFirstSpace = std::string::npos;
    for (auto &str : content) {
        ConvertCarriageToSpace(str);
        TrimSpaceAndTab(str);
        locationOfFirstSpace = str.find_first_of(' ');
        if (locationOfFirstSpace != std::string::npos) {
            maxLenParam = std::max(maxLenParam, locationOfFirstSpace);
        }
    }
    for (const auto &str : content) {
        locationOfFirstSpace = str.find_first_of(' ');
        if (locationOfFirstSpace == std::string::npos) {
            retContent += indent + str + CONSTANTS::BLANK;
            continue;
        }
        std::string alignment;
        if (maxLenParam > locationOfFirstSpace) {
            alignment = std::string(maxLenParam - locationOfFirstSpace, ' ');
        }
        std::string tempStr = str;
        retContent += indent + tempStr.insert(locationOfFirstSpace, alignment) + CONSTANTS::BLANK;
    }
    return retContent;
}

bool HoverImpl::ValidTag(const char ch)
{
    // TagName can be @[Chinese][English][_]
    return (static_cast<unsigned char>(ch) & 0x80) || isalnum(ch) || ch == '_';
}

Decl* HoverImpl::GetRealDecl(const std::vector<Ptr<Decl>> &decls)
{
    Decl *decl = decls[0];
    if (decl->astKind == Cangjie::AST::ASTKind::FUNC_PARAM) {
        for (auto D : decls) {
            if (D->astKind == Cangjie::AST::ASTKind::VAR_DECL) {
                return D;
            }
        }
    }

    return decl;
}

void HoverImpl::ResolveDocMapAndDocKey(std::unordered_map<std::string, std::vector<std::string>> &doc,
                                       std::vector<std::string> &keyOfDoc, std::string resultToString)
{
    std::string docKey = "";
    std::string docValue = "";
    bool hasAt = false;
    // the head and tail of substring
    unsigned int begin = 0;
    unsigned int end = 0;
    unsigned int resultLength = static_cast<unsigned int>(resultToString.size());
    const auto numOfChineseChar = 3U;
    while (end < resultLength) {
        if (end + 1 < resultLength && resultToString[end] == '@' && ValidTag(resultToString[end + 1]) && !hasAt) {
            docValue = ((end >= 1 && begin == end - 1) ||
                    (end >= numOfChineseChar && begin == end - numOfChineseChar)) ?
                       "" : resultToString.substr(begin, end - begin);
            TrimSpaceAndTab(docValue);
            if (docKey.empty()) {
                doc["@default"].push_back(docValue);
            } else if (!doc.count(docKey) && docKey != "@brief") {
                keyOfDoc.push_back(docKey);
                doc[docKey].push_back(docValue);
            } else {
                doc[docKey].push_back(docValue);
            }
            begin = end;
            hasAt = true;
        } else if (!ValidTag(resultToString[end]) && hasAt) {
            docKey = resultToString.substr(begin, end - begin);
            if (resultToString[end] == '@') {
                end -= (end >= numOfChineseChar &&
                    (static_cast<unsigned char>(resultToString[end - numOfChineseChar]) & 0x80))
                    ? numOfChineseChar
                    : 1;
            }
            begin = end;
            hasAt = false;
        }
        end += (static_cast<unsigned char>(resultToString[end]) & 0x80) ?
               static_cast<unsigned int>(numOfChineseChar) : 1;
    }
    docValue = resultToString.substr(begin, end - begin);
    TrimSpaceAndTab(docValue);
    if (docKey.empty()) {
        doc["@default"].push_back(docValue);
    } else {
        if (!doc.count(docKey) && docKey != "@brief") {
            keyOfDoc.push_back(docKey);
        }
        doc[docKey].push_back(docValue);
    }
}

// cangjiedoc includes description and block tags. block tags include @brief, @param and  @return
std::string HoverImpl::GetHoverMessageForDocComment(const std::vector<std::string> &result)
{
    // key is block tag, value is content of block tag.
    std::unordered_map<std::string, std::vector<std::string>> doc;
    std::vector<std::string> keyOfDoc;
    (void)keyOfDoc.emplace_back("@brief");
    (void)keyOfDoc.emplace_back("@default");
    std::string resultToString = "";
    for (const auto str:result) {
        resultToString += str + CONSTANTS::BLANK;
    }
    ResolveDocMapAndDocKey(doc, keyOfDoc, resultToString);
    std::string retDocComment = "";
    if (doc.count("@brief")) {
        retDocComment += GetEffectiveDocComment(doc["@brief"], "@brief");
    }
    if (doc.count("@default")) {
        retDocComment += GetEffectiveDocComment(doc["@default"], "@default");
    }
    retDocComment += retDocComment.empty() ? "" : CONSTANTS::BLANK;
    for (size_t i = 2; i < keyOfDoc.size(); i++) {
        std::string str = keyOfDoc[i].substr(1);
        if (!str.empty() && isalpha(str[0])) {
            str[0] = static_cast<char>(std::toupper(static_cast<int>(str[0])));
        }
        retDocComment += str + CONSTANTS::BLANK;
        retDocComment += GetEffectiveDocComment(doc[keyOfDoc[i]], keyOfDoc[i]);
    }
    if (!retDocComment.empty()) {
        retDocComment = retDocComment.substr(0, retDocComment.size() - CONSTANTS::BLANK.size());
    }
    return retDocComment;
}

std::string HoverImpl::ResolveComment(const std::string &comment, const CommentKind kind)
{
    std::string retComment = "";
    if (comment.empty()) {
        return "";
    }
    switch (kind) {
        case CommentKind::NO_COMMENT: {
            break;
        }
        case CommentKind::LINE_COMMENT: {
            retComment = comment.substr(NUMBER_FOR_LINE_COMMENT);
            TrimSpaceAndTab(retComment);
            break;
        }
        case CommentKind::BLOCK_COMMENT: {
            std::vector<std::string> result = StringSplit(comment, CONSTANTS::BLANK);
            TrimBlankLines(result);
            retComment = GetHoverMessageForBlockComment(result);
            break;
        }
        case CommentKind::DOC_COMMENT: {
            std::vector<std::string> result = StringSplit(comment, CONSTANTS::BLANK);
            TrimBlankLines(result);
            retComment = GetHoverMessageForDocComment(result);
            break;
        }
        default:
            break;
    }
    return retComment;
}

std::string HoverImpl::GetDeclCommentOfBack(const std::vector<Cangjie::Token> &tokens, const int lastTokenIndexOnLine)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "HoverImpl::GetDeclCommentOfBack in.");
    std::string commentOfBack;
    auto index = static_cast<std::vector<Cangjie::Token>::size_type>(static_cast<unsigned int>(lastTokenIndexOnLine));
    std::string retCommentOfBack = "";
    if (index >= tokens.size()) { return retCommentOfBack; }
    if (tokens[index] == "\n" || tokens[index] == "\r\n") {
        commentOfBack = tokens[index - 1].Value();
    } else {
        commentOfBack = tokens[index].Value();
    }
    CommentKind kind = GetCommentKind(commentOfBack);
    if (lastTokenIndexOnLine > 1 && (kind == CommentKind::LINE_COMMENT || kind == CommentKind::BLOCK_COMMENT)) {
        retCommentOfBack = ResolveComment(commentOfBack, kind);
    }
    return retCommentOfBack;
}

std::string HoverImpl::GetDeclCommentOfAbove(const std::vector<Cangjie::Token> &tokens,
                                             const unsigned int firstTokenIndexOnLine,
                                             bool &hasDocComment,
                                             const int prevLineFirstIndexOnLine)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "HoverImpl::GetDeclCommentOfAbove in.");
    // format is "comment + decl", so minus 1 to get comments
    if (tokens.empty() || firstTokenIndexOnLine < 1) {
        return "";
    }

    // only find preSibling element
    // -1 is line separator
    // -2 is effective preSibling element
    int indexForDocComment = static_cast<int>(firstTokenIndexOnLine) - 2;
    // reorganize the comment string
    std::string retAboveComment = "";
    if (indexForDocComment >= 0) {
        std::string value = tokens[indexForDocComment].Value();
        if (tokens[indexForDocComment].kind == TokenKind::COMMENT) {
            CommentKind kind = GetCommentKind(value);
            hasDocComment = kind == CommentKind::DOC_COMMENT;
            if (kind == CommentKind::LINE_COMMENT && prevLineFirstIndexOnLine < indexForDocComment) {
                return "";
            }
            std::string str = ResolveComment(value, kind);
            if (!str.empty()) {
                retAboveComment = str;
            }
        }
    }

    if (EndsWith(retAboveComment, CONSTANTS::BLANK)) {
        retAboveComment = retAboveComment.substr(0, retAboveComment.size() - CONSTANTS::BLANK.size());
    }

    return retAboveComment;
}

/**
 * @brief Get the comments above and after the decl.$$Comments consisting of multiple tokens can be supported later$$
 * @tokens All tokens in the curfile
 * @firstTokenIndexOnLine The first token in the row where the declaration is located.
 * @lastTokenIndexOnLine The last token in the row where the declaration is located.
 * @return A string consisting of comments
 * */
std::string HoverImpl::GetDeclCommentByIndex(const std::vector<Cangjie::Token> &tokens,
                                             const unsigned int firstTokenIndexOnLine, const int lastTokenIndexOnLine,
                                             const int prevLineFirstIndexOnLine)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "HoverImpl::GetDeclCommentByIndex in.");
    bool hasDocComment = false;
    // comments in the above
    std::string retCommentTop = GetDeclCommentOfAbove(tokens, firstTokenIndexOnLine,
                                                      hasDocComment, prevLineFirstIndexOnLine);
    // comments in the back
    std::string retCommentBack = "";
    if (!hasDocComment) {
        retCommentBack = GetDeclCommentOfBack(tokens, lastTokenIndexOnLine);
    }

    return ((retCommentTop.empty() || hasDocComment) ? retCommentTop :
            (retCommentTop + CONSTANTS::BLANK)) + retCommentBack;
}

std::string HoverImpl::GetHoverMessageByOuterDecl(const Decl &node)
{
    std::string detail;
    return Meta::match(node)(
        [&detail](const Cangjie::AST::ClassDecl &decl) {
            detail = "// In class " + decl.identifier + CONSTANTS::BLANK;
            return detail;
        },
        [&detail](const Cangjie::AST::InterfaceDecl &decl) {
            detail = "// In interface " + decl.identifier + CONSTANTS::BLANK;
            return detail;
        },
        [&detail](const Cangjie::AST::EnumDecl &decl) {
            detail = "// In enum " + decl.identifier + CONSTANTS::BLANK;
            return detail;
        },
        [&detail](const Cangjie::AST::StructDecl &decl) {
            detail = "// In struct " + decl.identifier + CONSTANTS::BLANK;
            return detail;
        },
        [&detail](const Cangjie::AST::ExtendDecl &decl) {
            if (decl.ty == nullptr) {
                return detail;
            }
            Ptr<Decl> realDecl = ItemResolverUtil::GetDeclByTy(decl.ty);
            if (realDecl == nullptr) {
                return detail;
            }
            detail = GetHoverMessageByOuterDecl(*realDecl);
            return detail;
        },
        // ----------- Match nothing --------------------
        [&detail]() { return detail; });
}

int HoverImpl::GetHoverMessage(Ptr<Decl> decl, Hover &result, const ArkAST &ast)
{
    auto instance = CompilerCangjieProject::GetInstance();
    std::string path = instance->GetPathBySource(curFilePath, decl->identifier.Begin().fileID);
    // get source declared
    std::string source = ItemResolverUtil::ResolveSourceByNode(decl, path);
    source = (source.empty()) ? source : (source + CONSTANTS::BLANK + CONSTANTS::BLANK);
    result.markedString.push_back(source);

    // get outer message
    if (decl->outerDecl != nullptr) {
        result.markedString.push_back(GetHoverMessageByOuterDecl(*decl->outerDecl));
    }

    // get signature
    std::string detail = ItemResolverUtil::ResolveDetailByNode(*decl, ast.sourceManager);
    detail = (detail.empty()) ? detail : (detail + CONSTANTS::BLANK);
    result.markedString.push_back(detail);
    // get apiKey
    if (MessageHeaderEndOfLine::GetIsDeveco()) {
        result.markedString.push_back(GetDeclApiKey(decl));
    }
    // get decl arkAST
    if (!decl) {
        return 0;
    }
    ArkAST *declAST = instance->GetArkAST(path);
    if (declAST == nullptr || declAST->tokens.empty()) {
        return 0;
    }
    // get comment
    int firstTokenIndexOnLine = GetFirstTokenOnCurLine(declAST->tokens, decl->GetIdentifierPos().line);
    int lastTokenIndexOnLine = GetLastTokenOnCurLine(declAST->tokens, decl->GetIdentifierPos().line);
    if (firstTokenIndexOnLine == -1 || lastTokenIndexOnLine == -1) {
        return 1;
    }
    // prev line first token index
    int prevLineFirstIndexOnLine = GetFirstTokenOnCurLine(declAST->tokens, decl->GetIdentifierPos().line - 1);
    std::string comment = GetDeclCommentByIndex(declAST->tokens, static_cast<const unsigned int>(firstTokenIndexOnLine),
        lastTokenIndexOnLine, prevLineFirstIndexOnLine);
    result.markedString.push_back(comment);
    return 1;
}

std::string HoverImpl::GetDeclApiKey(const Ptr<Decl> &decl)
{
    std::string apiKey = "apiKey:";
    apiKey += decl->fullPackageName + '/';
    if (decl->outerDecl) {
        apiKey += decl->outerDecl->identifier.Val();
        if (decl->outerDecl->generic) {
            apiKey += "<";
            bool firstGeneric = true;
            for (const auto &type : decl->outerDecl->generic->typeParameters) {
                if (!firstGeneric) {
                    apiKey += ", ";
                }
                apiKey += type->identifier;
                firstGeneric = false;
            }
            apiKey += ">";
        }
        if (!decl->outerDecl->identifier.Val().empty()) {
            apiKey += ".";
        }
    }
    std::string signature;
    if (auto fd = DynamicCast<FuncDecl>(decl)) {
        signature += fd->identifier.Val();
        if (!fd->funcBody) { return ""; }
        if (fd->funcBody->generic) {
            signature += "<";
            bool firstGeneric = true;
            for (const auto &type : fd->funcBody->generic->typeParameters) {
                if (!firstGeneric) {
                    signature += ", ";
                }
                signature += type->identifier;
                firstGeneric = false;
            }
            signature += ">";
        }
        signature += '(';
        bool firstTy = true;
        for (const auto &param : fd->funcBody->paramLists[0]->params) {
            if (!firstTy) {
                signature += ", ";
            }
            signature += GetString(*param->ty);
            firstTy = false;
        }
        signature += ')';
        apiKey += signature;
        return apiKey;
    }
    // return empty apiKey:
    return "apiKey:";
}

int HoverImpl::FindHover(const ArkAST &ast, Hover &result, Cangjie::Position pos)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "HoverImpl::FindHover in.");

    // update pos fileID
    pos.fileID = ast.fileID;

    // check current token is the kind which required in function CheckTokenKind(TokenKind)
    pos = PosFromIDE2Char(pos);
    PositionIDEToUTF8(ast.tokens, pos, *ast.file);

    // get curFilePath
    curFilePath = ast.file ? ast.file->filePath : "";
    int index = ast.GetCurTokenByPos(pos, 0, static_cast<int>(ast.tokens.size()) - 1);
    if (index < 0 || ast.tokens[static_cast<size_t>(index)].kind == TokenKind::MAIN) {
        logger.LogMessage(MessageType::MSG_INFO, "this token does not need to hover.");
        return -1;
    }
    if (index - 1 >= 0) {
        Token preToken = ast.tokens[static_cast<size_t>(index - 1)];
        if (NAME_TO_ANNO_KIND.count(ast.tokens[static_cast<size_t>(index)].Value())
            && preToken.kind == TokenKind::AT) {
            return -1;
        }
    }

    std::string query = "_ = (" + std::to_string(pos.fileID) + ", "
                        + std::to_string(pos.line) + ", " + std::to_string(pos.column) + ")";
    if (!ast.packageInstance) {
        return -1;
    }
    auto syms = SearchContext(ast.packageInstance->ctx, query);
    if (syms.empty()) {
        logger.LogMessage(MessageType::MSG_WARNING, "the result of search is empty.");
        return -1;
    }
    Token curToken = ast.tokens[index];
    if (auto mee = DynamicCast<MacroExpandExpr*>(syms[0]->node)) {
        bool isInvalid = mee->invocation.leftParenPos <= curToken.Begin() &&
            curToken.End() <= mee->invocation.rightParenPos;
        if (isInvalid) {
            Trace::Wlog("no hover info in macro expand expr");
            return -1;
        }
    }
    bool isMarkPosition = !syms[0] || IsMarkPos(syms[0]->node, pos);
    if (isMarkPosition) { return -1; }
    std::vector<Ptr<Decl> > decls = ast.FindRealDecl(ast, syms, query, pos, {true, false});
    bool isDeclEmpty = decls.empty() || decls[0] == nullptr;
    if (isDeclEmpty) {
        logger.LogMessage(MessageType::MSG_WARNING, "the decl is null.");
        return -1;
    }
    Ptr<Decl> decl = GetRealDecl(decls);
    if (decl->TestAttr(Attribute::DEFAULT, Attribute::COMPILER_ADD)) {
        query = "_ = (" + std::to_string(decl->identifier.Begin().fileID) +
            ", " + std::to_string(decl->identifier.Begin().line) +
            ", " + std::to_string(decl->identifier.Begin().column) + ")";
        auto curSyms = SearchContext(ast.packageInstance->ctx, query);
        for (const auto &sym: curSyms) {
            if (sym->name != decl->identifier.Val()) { continue; }
            if (auto realDecl = DynamicCast<Decl*>(sym->node)) {
                decl = realDecl;
            }
        }
    }
    if (IsModifierBeforeDecl(decl, curToken.Begin())) {
        logger.LogMessage(MessageType::MSG_WARNING, "this token does not need to hover");
        return -1;
    }
    Range range = {
        curToken.Begin(),
        {
            curToken.Begin().fileID,
            curToken.Begin().line,
            curToken.Begin().column + static_cast<int>(CountUnicodeCharacters(curToken.Value()))
        }
    };
    Ptr<Cangjie::AST::Node> node = syms[0]->node;
    SetRangForInterpolatedString(curToken, node, range);
    if (curToken.Value().substr(0, 1) == "`" && curToken.Value().substr(curToken.Value().size() - 1, 1) == "`") {
        range.start = range.start + 1;
        range.end = range.end - 1;
    }
    UpdateRange(ast.tokens, range, *node);
    result.range = TransformFromChar2IDE(range);
    int ret = GetHoverMessage(decl, result, ast);

    return ret;
}
} // namespace ark
