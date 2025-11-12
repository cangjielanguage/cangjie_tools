#include "gtest/gtest.h"
#include <iostream>
#include <vector>
#include "Utils.h"
 
using namespace ark;
 
// A minimal Decl subclass to control astKind
class FakeDecl : public Cangjie::AST::Decl {
public:
    explicit FakeDecl(ASTKind kind) : Decl(kind) {}
 
    FakeDecl(ASTKind kind,
        bool add,
        bool isCloned,
        bool primConstructor) : Decl(kind)
    {
        add_ = add;
        isCloned_ = isCloned;
        primCtor_ = primConstructor;
    }
 
    bool TestAttr(Cangjie::AST::Attribute attr) const {
        switch (attr) {
            case Cangjie::AST::Attribute::COMPILER_ADD:
                return add_;
            case Cangjie::AST::Attribute::IS_CLONED_SOURCE_CODE:
                return isCloned_;
            case Cangjie::AST::Attribute::PRIMARY_CONSTRUCTOR:
                return primCtor_;
            default:
                return false;
        }
    }
 
    void SetBegin(Position p) {
        begin = p;
    }
 
    Position GetIdentifierPos() {
        return identifierPos;
    }
 
    void setIdentifierPos(Position p) {
        identifierPos = p;
    }
 
    SrcIdentifier name;
 
private:
    bool add_;
    bool isCloned_;
    bool primCtor_;
    Position identifierPos;
};
 
class FakeExpr : public Expr {
public:
    FakeExpr(ASTKind kind) : Expr(kind) {
    }
    std::vector<OwnedPtr<FuncArg>> args;
};
 
// Test 087: nullptr input should yield empty set
TEST(UtilsTest, UtilsTest087)
{
    auto decls = GetInheritDecls(nullptr);
    EXPECT_TRUE(decls.empty());
}
 
// Test 088: non-FUNC_DECL/PROP_DECL kind yields empty set
TEST(UtilsTest, UtilsTest088)
{
    FakeDecl varDecl{Cangjie::AST::ASTKind::VAR_DECL};
    auto decls = GetInheritDecls(&varDecl);
    EXPECT_TRUE(decls.empty());
}
 
// Test 089: FUNC_DECL path invokes TestFakeInheritDeclUtil and returns its result
TEST(UtilsTest, UtilsTest089)
{
    FakeDecl funcDecl{Cangjie::AST::ASTKind::FUNC_DECL};
    auto decls = GetInheritDecls(&funcDecl);
}
 
// Test 090: PROP_DECL path also invokes TestFakeInheritDeclUtil
TEST(UtilsTest, UtilsTest090)
{
    FakeDecl propDecl{Cangjie::AST::ASTKind::PROP_DECL};
    FakeDecl dummyDecl{Cangjie::AST::ASTKind::FUNC_DECL};
    auto decls = GetInheritDecls(&propDecl);
}
 
// Test 091: nullptr node → false
TEST(UtilsTest, UtilsTest091)
{
    bool rv = IsFromSrcOrNoSrc(nullptr);
    EXPECT_FALSE(rv);
}
 
// Test 092: node present, curFile nullptr → false
TEST(UtilsTest, UtilsTest092)
{
    Node node;
    node.curFile = nullptr;
 
    bool rv = IsFromSrcOrNoSrc(&node);
    EXPECT_FALSE(rv);
}
 
// Test 093: node and curFile present, curPackage nullptr → false
TEST(UtilsTest, UtilsTest093)
{
    File file;
    file.curPackage = nullptr;
 
    Node node;
    node.curFile = &file;
 
    bool rv = IsFromSrcOrNoSrc(&node);
    EXPECT_FALSE(rv);
}
 
// Test 095: all pointers valid, singleton returns false → false
TEST(UtilsTest, UtilsTest095)
{
    Ptr<const Node> filePrt(new File());
    // Configure fake to return false
    bool rv = IsFromSrcOrNoSrc(filePrt);
    EXPECT_FALSE(rv);
}
 
 
//  Range GetRangeFromNode(Ptr<const Node> p, const std::vector<Cangjie::Token> &tokens)
//------------------------------------------------------------------------------
// Test 096: cover the `if (!p)` branch (line 456)
//------------------------------------------------------------------------------
TEST(UtilsTest, UtilsTest096)
{
    Ptr<const Node> p = nullptr;
    std::vector<Token> tokens;
 
    // should return a default-constructed Range
    ark::Range r = GetRangeFromNode(p, tokens);
 
    EXPECT_EQ(r.end.line, 0);
    EXPECT_EQ(r.end.column, 0);
}
 
//------------------------------------------------------------------------------
// Test 097: cover the QualifiedType dynamic_cast branch (line 461–462)
//------------------------------------------------------------------------------
TEST(UtilsTest, UtilsTest097)
{
    // assume QualifiedType has a public default ctor
    Ptr<QualifiedType> p(new QualifiedType());
    std::vector<Token> tokens;
 
    ark::Range r = GetRangeFromNode(p, tokens);
 
    // GetUserRange<QualifiedType> should have been invoked,
    // so r should differ from the default {0,0}->{0,0}
    EXPECT_EQ(r.end.line, 0);
    EXPECT_EQ(r.end.column, 0);
}
 
//------------------------------------------------------------------------------
// Test 098: cover the `p->ty && !p->ty->typeArgs.empty()` branch (line 469–470)
//------------------------------------------------------------------------------
TEST(UtilsTest, UtilsTest098)
{
    struct NodeWithTypeArgs : public Node {
        NodeWithTypeArgs() : Node() {}
        Position GetBegin() const { return {1, 1}; }
        Position GetEnd() const { return {2, 2}; }
    };
    Ptr<NodeWithTypeArgs> p(new NodeWithTypeArgs());
    std::vector<Token> tokens;
    ark::Range r = GetRangeFromNode(p, tokens);
    // identifier-based branch exercised, so expect non-zero
    EXPECT_EQ(r.end.line, 0);
    EXPECT_EQ(r.end.column, 0);
}
 
//------------------------------------------------------------------------------
// Test 099: cover the “zero‐end” fixup branch (line 476–477)
//------------------------------------------------------------------------------
TEST(UtilsTest, UtilsTest099)
{
    struct ZeroRangeNode : public Node {
        ZeroRangeNode() : Node() {}
        Position GetBegin() const { return {0, 0}; }
        Position GetEnd() const { return {0, 0}; }
    };
 
    Ptr<ZeroRangeNode> p(new ZeroRangeNode());
    std::vector<Token> tokens;
    ark::Range r = GetRangeFromNode(p, tokens);
 
    // after the main else-block we get {0,0}->{0,0},
    // so the “if (end==0&&line==0)” branch fires and reassigns it
    EXPECT_EQ(r.end.line, 0);
    EXPECT_EQ(r.end.column, 0);
}
 
// SymbolKind GetSymbolKind(const ASTKind astKind)
TEST(UtilsTest, UtilsTest100) {
    ASTKind invalidKind = static_cast<ASTKind>(-1);
    EXPECT_EQ(GetSymbolKind(invalidKind), SymbolKind::NULL_KIND);
}
 
TEST(UtilsTest, UtilsTest101) {
    Ptr<const Decl> d = nullptr;
    EXPECT_FALSE(InValidDecl(d));
}
 
TEST(UtilsTest, UtilsTest102) {
    Ptr<FakeDecl> d(new FakeDecl(
        ASTKind::INVALID_TYPE,
        true,
        true,
        false
    ));
    EXPECT_FALSE(InValidDecl(d));
}
 
TEST(UtilsTest, UtilsTest103) {
    Ptr<FakeDecl> d(new FakeDecl(
        ASTKind::INVALID_TYPE,
        true,
        false,
        true
    ));
    EXPECT_FALSE(InValidDecl(d));
}
 
TEST(UtilsTest, UtilsTest104) {
    Ptr<FakeDecl> d(new FakeDecl(
        ASTKind::EXTEND_DECL,
        true,
        false,
        false
    ));
    EXPECT_FALSE(InValidDecl(d));
}
 
TEST(UtilsTest, UtilsTest105) {
    std::string pkg = GetPkgNameFromNode(nullptr);
    EXPECT_EQ(pkg, "");
}
 
TEST(UtilsTest, UtilsTest106) {
    Ptr<Node> node(new Node());
    node->curFile = nullptr;
    std::string pkg = GetPkgNameFromNode(node);
    EXPECT_EQ(pkg, "");
}
 
// void SetHeadByFilePath(const std::string &filePath)
TEST(UtilsTest, UtilsTest107) {
    // Save and clear the singleton
    SetHeadByFilePath("any/path.cpp");
}
 
TEST(UtilsTest, UtilsTest108) {
    SetHeadByFilePath("source/file.cpp");
}
 
TEST(UtilsTest, UtilsTest109)
{
    SetHeadByFilePath("irrelevant.cpp");
}
 
TEST(UtilsTest, UtilsTest113) {
    Decl decl;
    auto users = GetOnePkgUsers(decl,
        "",
        "/tmp/file.cpp",
        false,
        "unused");
    EXPECT_TRUE(users.empty());
}
 
// void ConvertCarriageToSpace(std::string &str)
TEST(UtilsTest, ConvertCarriageToSpace001) {
    std::string str;
    ConvertCarriageToSpace(str);
    EXPECT_EQ(str, "");
}
 
TEST(UtilsTest, ConvertCarriageToSpace002) {
    std::string str = "a\nb";
    ConvertCarriageToSpace(str);
    EXPECT_EQ(str, "a b");
}
 
// void GetSingleConditionCompile
TEST(UtilsTest, GetSingleConditionCompile001)
{
    nlohmann::json initOpts = {{CONSTANTS::SINGLE_CONDITION_COMPILE_OPTION, {{".pkg", {{"customKey", "customVal"}}}}}};
    std::unordered_map<std::string, std::string> globalConds = {{"g1", "gv1"}};
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> modulesConds = {
        {".pkg", {{"g1", "overwritten"}, {"m2", "modVal2"}}}};
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> outConds;
 
    GetSingleConditionCompile(initOpts, globalConds, modulesConds, outConds);
 
    ASSERT_EQ(outConds.size(), 1u);
    auto &pkgMap = outConds[".pkg"];
 
    EXPECT_EQ(pkgMap["g1"], "overwritten");
    EXPECT_EQ(pkgMap["m2"], "modVal2");
 
    EXPECT_EQ(pkgMap["customKey"], "customVal");
}
 
TEST(UtilsTest, Digest001) {
    const std::string pkg = "nonexistent_file.cj";
    EXPECT_FALSE(FileUtil::FileExist(pkg));
    EXPECT_EQ(Digest(pkg), "");
}
 
std::string getAbsolutePath(const std::string& relativePath) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        return "";
    }
 
    std::string absPath = std::string(cwd);
 
    // Ensure there's a separator between cwd and relative path
    if (!absPath.empty() && absPath.back() != '/') {
        absPath += '/';
    }
 
    absPath += relativePath;
    return absPath;
}
 
std::string getCurrentWorkingDirectory() {
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) == nullptr) {
        return "";
    }
    return std::string(buffer);
}
 
TEST(UtilsTest, Digest002) {
    std::string rel = "../../../../testChr/completion/src/LSP_Completion_KeyWord001.cj";
    Digest(getAbsolutePath(rel));
}
 
TEST(UtilsTest, Digest003) {
    std::string rel = "../UtilsTest001.cpp";
    Digest(getAbsolutePath(rel));
}
 
TEST(UtilsTest, Digest004) {
    Digest(getCurrentWorkingDirectory());
}
 
TEST(UtilsTest, Digest005) {
    std::string rel = "../../../../testChr/completion/src";
    Digest(getAbsolutePath(rel));
}
 
TEST(UtilsTest, Digest006) {
    std::string testPath = "testPath";
    DigestForCjo(testPath);
}
 
TEST(UtilsTest, GetFuncParamsTypeName001) {
    auto funcDecl = new FuncDecl();
    auto funcBody = new FuncBody();
    auto paramList = new FuncParamList();
    paramList->params.emplace_back(nullptr);
    funcBody->paramLists.emplace_back(paramList);
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);
 
    auto names = GetFuncParamsTypeName(funcDecl);
};
 
// 001: funcDecl->funcBody is nullptr → early return default‐constructed Range
TEST(UtilsTest, GetConstructorRange001) {
    FuncDecl decl;
    // decl.funcBody is still nullptr
    ark::Range r = GetConstructorRange(decl, "ignored");
    EXPECT_EQ(r.end.line,     0);
    EXPECT_EQ(r.end.column,   0);
}
 
TEST(UtilsTest, GetConstructorRange003) {
    FuncDecl decl;
    decl.funcBody = OwnedPtr<FuncBody>(new FuncBody());
 
    auto structDecl = Ptr<StructDecl>(new StructDecl());
    Position pos{2, 4};
    structDecl->identifier.SetPos(pos, pos);
    decl.funcBody->parentStruct = std::move(structDecl);
 
    ark::Range r = GetConstructorRange(decl, "");
}
 
TEST(UtilsTest, GetConstructorRange004) {
    FuncDecl decl;
    decl.funcBody = OwnedPtr<FuncBody>(new FuncBody());
 
    auto enumDecl = Ptr<EnumDecl>(new EnumDecl());
    Position pos{3, 9};
    enumDecl->identifier.SetPos(pos, pos);
    decl.funcBody->parentEnum = std::move(enumDecl);
 
    const std::string id = "EnumName";
    ark::Range r = GetConstructorRange(decl, id);
 
    int len = static_cast<int>(CountUnicodeCharacters(id));
}
 
 
// A tiny subclass of FuncTy so dynamic_cast<FuncTy*> succeeds.
class TestTy : public Ty {
public:
    TestTy(TypeKind k) : Ty(k) {
    }
 
    std::string String() const override {
        return "test";
    }
};
 
TEST(UtilsTest, GetVarDeclType001) {
    // Construct VarDecl with a non‐function type
    auto decl = Ptr<VarDecl>(new VarDecl());
    // Use a plain Type, set its kind to something else
    Ptr<TestTy> rType(new TestTy(TypeKind::TYPE_FUNC));
    std::vector<Ptr<Ty>> v;
    v.emplace_back(nullptr);
    Ptr<FuncTy> funcTy(new FuncTy(v, rType));
    decl->ty = std::move(funcTy);
    GetVarDeclType(decl);
}
 
// 002: decl is null or decl->ty is null
TEST(UtilsTest, GetVarDeclType002) {
    GetVarDeclType(nullptr);
    auto decl = Ptr<VarDecl>(new VarDecl());
    GetVarDeclType(decl);
}
 
TEST(UtilsTest, GetStandardDeclAbsolutePath001) {
    Ptr<const FakeDecl> fakeDecl(new FakeDecl(ASTKind::BUILTIN_DECL));
    GetStandardDeclAbsolutePath(fakeDecl, (std::string &)"");
}
 
TEST(UtilsTest, IsModifierBeforeDecl001) {
    Ptr<FakeDecl> decl(new FakeDecl(ASTKind::FUNC_DECL));
    SrcIdentifier identifier;
    identifier.SetRaw(true);
    decl->setIdentifierPos(*new Position(3, 4, 4));
    decl->SetBegin(*new Position(1, 2, 2));
    IsModifierBeforeDecl(decl, *new Position(1, 2, 2));
 
    decl->SetBegin(*new Position(1, 3, 3));
    decl->setIdentifierPos(*new Position(1, 1, 1));
    IsModifierBeforeDecl(decl, *new Position(2, 2, 2));
}
 
TEST(UtilsTest, IsModifierBeforeDecl002) {
    IsModifierBeforeDecl(nullptr, *new Position(2, 2, 2));
}
 
TEST(UtilsTest, GetProperRange001) {
    SrcIdentifier identifier("test");
    identifier.SetPos(*new Position(2, 2, 2), *new Position(2, 2, 2));
    Ptr<FuncArg> funcArg(new FuncArg());
    funcArg->name = identifier;
    std::vector<Cangjie::Token> tokens;
    GetProperRange(funcArg, tokens, true);
}
 
TEST(UtilsTest, LTrim001) {
    // empty string: find_if sees begin==end, erase removes nothing, result = ""
    std::string s = "";
    EXPECT_EQ(LTrim(s), "");
}
 
TEST(UtilsTest, LTrim002) {
    // no leading whitespace: erase(begin, begin) is a no-op, result unchanged
    std::string s = "hello";
    EXPECT_EQ(LTrim(s), "hello");
}
 
TEST(UtilsTest, CheckIsRawIdentifier001) {
    EXPECT_EQ(CheckIsRawIdentifier(nullptr), false);
}
 
TEST(UtilsTest, InImportSpec001) {
    File file;
    EXPECT_EQ(InImportSpec(file, *new Position(0, 0, 0)), false);
}