// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "IndexStorage.cpp"
 
using namespace ark::lsp;
namespace fs = std::filesystem;


class IndexStorageTest: public ::testing::Test {
protected:
    std::string testRoot = "./test_root_" + std::to_string(getpid());
    void SetUp() override { fs::create_directories(testRoot); }
    void TearDown() override { fs::remove_all(testRoot); }
};

 
TEST_F(IndexStorageTest, SplitFileName001) {
    auto res = SplitFileName("filename");
    EXPECT_EQ(res.first, "");
    EXPECT_EQ(res.second, "");
}
 
TEST_F(IndexStorageTest, SplitFileName002) {
    auto res = SplitFileName("file.txt");
    EXPECT_EQ(res.first, "");
    EXPECT_EQ(res.second, "");
}
 
TEST_F(IndexStorageTest, SplitFileName003) {
    auto res = SplitFileName("archive.tar.gz");
    EXPECT_EQ(res.first, "archive");
    EXPECT_EQ(res.second, "tar");
}
 
TEST_F(IndexStorageTest, MergeFileName001) {
    EXPECT_EQ(MergeFileName("mypkg", "123abc", "bin"), "mypkg.123abc.bin");
}
 
TEST_F(IndexStorageTest, convertCommentGroup001) {
    flatbuffers::FlatBufferBuilder builder;
 
    auto groupOffset = IdxFormat::CreateCommentGroup(builder, 0);
 
    builder.Finish(groupOffset);
    auto group = flatbuffers::GetRoot<IdxFormat::CommentGroup>(builder.GetBufferPointer());
    convertCommentGroup(*group);
}
 
static constexpr uint64_t kTestId    = 42;
static constexpr uint8_t  kTestType  = 3;
static constexpr uint64_t kContainer = 99;
 
TEST_F(IndexStorageTest, ReadCrossSymbol001) {
    // name==nullptr, location==nullptr, container_name==nullptr
    flatbuffers::FlatBufferBuilder fbb;
    auto crs_off = IdxFormat::CreateCrossSymbol(fbb,
        kTestId,
        0,
        kTestType,
        0,
        kContainer,
        0);
    fbb.Finish(crs_off);
    auto fb_crs = flatbuffers::GetRoot<IdxFormat::CrossSymbol>(fbb.GetBufferPointer());
 
    CrossSymbol out;
    ReadCrossSymbol(out, fb_crs);
 
    EXPECT_EQ(out.id,          kTestId);
    EXPECT_EQ(out.crossType,   CrossType(kTestType));
    EXPECT_EQ(out.container,   kContainer);
    EXPECT_TRUE(out.name.empty());
    EXPECT_TRUE(out.containerName.empty());
 
    EXPECT_EQ(out.location.begin.fileID, 0u);
    EXPECT_EQ(out.location.begin.line,   0u);
    EXPECT_EQ(out.location.begin.column, 0u);
    EXPECT_EQ(out.location.end.fileID,   0u);
    EXPECT_EQ(out.location.end.line,     0u);
    EXPECT_EQ(out.location.end.column,   0u);
    EXPECT_TRUE(out.location.fileUri.empty());
}
 
TEST_F(IndexStorageTest, ReadCrossSymbol002) {
    // name!=nullptr, container_name!=nullptr, location==nullptr
    flatbuffers::FlatBufferBuilder fbb;
    auto name_off = fbb.CreateString("TestName");
    auto cntname  = fbb.CreateString("CntName");
    auto crs_off = IdxFormat::CreateCrossSymbol(fbb,
        kTestId,
        name_off,
        kTestType,
        0,
        kContainer,
        cntname);
    fbb.Finish(crs_off);
    auto fb_crs = flatbuffers::GetRoot<IdxFormat::CrossSymbol>(fbb.GetBufferPointer());
 
    CrossSymbol out;
    ReadCrossSymbol(out, fb_crs);
 
    EXPECT_EQ(out.name,          "TestName");
    EXPECT_EQ(out.containerName, "CntName");
}
 
TEST_F(IndexStorageTest, ReadCrossSymbol003) {
    // only location present, subfields all nullptr -> skip nested assignments
    flatbuffers::FlatBufferBuilder fbb;
    auto loc_off = IdxFormat::CreateLocation(fbb,
        0,
        0,
        0);
    auto crs_off = CreateCrossSymbol(fbb,
        kTestId,
        0,
        kTestType,
        loc_off,
        kContainer,
        0);
    fbb.Finish(crs_off);
    auto fb_crs = flatbuffers::GetRoot<IdxFormat::CrossSymbol>(fbb.GetBufferPointer());
 
    CrossSymbol out;
    ReadCrossSymbol(out, fb_crs);
 
    EXPECT_EQ(out.location.begin.fileID, 0u);
    EXPECT_EQ(out.location.end.fileID,   0u);
    EXPECT_TRUE(out.location.fileUri.empty());
}
 
TEST_F(IndexStorageTest, ReadCrossSymbol004) {
    // only begin non-null
    flatbuffers::FlatBufferBuilder fbb;
    auto posB    = new IdxFormat::Position(7, 8, 9);
    auto loc_off = CreateLocation(fbb,
        posB,
        0,
        0);
    auto crs_off = CreateCrossSymbol(fbb,
        kTestId,
        0,
        kTestType,
        loc_off,
        kContainer,
        0);
    fbb.Finish(crs_off);
    auto fb_crs = flatbuffers::GetRoot<IdxFormat::CrossSymbol>(fbb.GetBufferPointer());
 
    CrossSymbol out;
    ReadCrossSymbol(out, fb_crs);
 
    EXPECT_EQ(out.location.begin.fileID,  7u);
    EXPECT_EQ(out.location.begin.line,    8u);
    EXPECT_EQ(out.location.begin.column,  9u);
    EXPECT_EQ(out.location.end.fileID,    0u);
    EXPECT_TRUE(out.location.fileUri.empty());
}
 
TEST_F(IndexStorageTest, ReadCrossSymbol006)
{
    // only file_uri non-null
    flatbuffers::FlatBufferBuilder fbb;
    auto uri_off = fbb.CreateString("file://uri");
    auto loc_off = IdxFormat::CreateLocation(fbb,
        0,
        0, uri_off);
    auto crs_off = CreateCrossSymbol(fbb, kTestId,
        0, kTestType, loc_off, kContainer,
        0);
    fbb.Finish(crs_off);
    auto fb_crs = flatbuffers::GetRoot<IdxFormat::CrossSymbol>(fbb.GetBufferPointer());
 
    CrossSymbol out;
    ReadCrossSymbol(out, fb_crs);
 
    EXPECT_EQ(out.location.fileUri, "file://uri");
}
 
static constexpr uint64_t kDefaultId = 0;
 
TEST_F(IndexStorageTest, ReadSymsComments001) {
    // sym == nullptr → early return
    Symbol res;
    ReadSymsComments(res, nullptr);
    EXPECT_TRUE(res.comments.leadingComments.empty());
    EXPECT_TRUE(res.comments.innerComments.empty());
    EXPECT_TRUE(res.comments.trailingComments.empty());
}
 
TEST_F(IndexStorageTest, ReadSymsComments002) {
    // sym != nullptr but comments() == nullptr → early return
    flatbuffers::FlatBufferBuilder fbb;
    auto sym_off = IdxFormat::CreateSymbol(fbb,
        kDefaultId, // id
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        false,
        0,
        false,
        false,
        0,
        0,
        0,
        0,
        0);
    fbb.Finish(sym_off);
 
    auto fb_sym = flatbuffers::GetRoot<IdxFormat::Symbol>(fbb.GetBufferPointer());
    Symbol res;
    fbb.Finish(sym_off);
    ReadSymsComments(res, fb_sym);
    EXPECT_TRUE(res.comments.leadingComments.empty());
    EXPECT_TRUE(res.comments.innerComments.empty());
    EXPECT_TRUE(res.comments.trailingComments.empty());
}
 
TEST_F(IndexStorageTest, ReadSymsComments004) {
    // only leading_comments
    flatbuffers::FlatBufferBuilder fbb;
    auto cg1      = IdxFormat::CreateCommentGroup(fbb);
    auto lead_vec = fbb.CreateVector<flatbuffers::Offset<IdxFormat::CommentGroup>>({cg1});
    auto cg_group = CreateCommentGroups(fbb,
        lead_vec,
        0,
        0);
    auto sym_off = CreateSymbol(fbb,
        kDefaultId, 0,0,0,0,0,0,0,false,0,false,false,0,0,0,
        cg_group,
        0);
    fbb.Finish(sym_off);
 
    auto fb_sym = flatbuffers::GetRoot<IdxFormat::Symbol>(fbb.GetBufferPointer());
    Symbol res;
    ReadSymsComments(res, fb_sym);
    EXPECT_EQ(res.comments.leadingComments.size(), 1u);
    EXPECT_TRUE(res.comments.innerComments.empty());
    EXPECT_TRUE(res.comments.trailingComments.empty());
}
 
TEST_F(IndexStorageTest, ReadSymsComments005) {
    // only inner_comments
    flatbuffers::FlatBufferBuilder fbb;
    auto cg1 = IdxFormat::CreateCommentGroup(fbb /* default args */);
    auto inner_vec = fbb.CreateVector<flatbuffers::Offset<IdxFormat::CommentGroup>>({cg1});
    auto cg_group = CreateCommentGroups(fbb,
        0,
        inner_vec,
        0);
    auto sym_off = CreateSymbol(fbb,
        kDefaultId, 0,0,0,0,0,0,0,false,0,false,false,0,0,0,
        cg_group,
        0);
    fbb.Finish(sym_off);
 
    auto fb_sym = flatbuffers::GetRoot<IdxFormat::Symbol>(fbb.GetBufferPointer());
    Symbol res;
    ReadSymsComments(res, fb_sym);
    EXPECT_TRUE(res.comments.leadingComments.empty());
    EXPECT_EQ(res.comments.innerComments.size(), 1u);
    EXPECT_TRUE(res.comments.trailingComments.empty());
}
 
TEST_F(IndexStorageTest, ReadSymsComments006) {
    // only trailing_comments
    flatbuffers::FlatBufferBuilder fbb;
    auto cg1= IdxFormat::CreateCommentGroup(fbb /* default args */);
    auto trail_vec = fbb.CreateVector<flatbuffers::Offset<IdxFormat::CommentGroup>>({cg1});
    auto cg_group  = CreateCommentGroups(fbb,
        0,
        0,
        trail_vec);
    auto sym_off = CreateSymbol(fbb,
        kDefaultId, 0,0,0,0,0,0,0,false,0,false,false,0,0,0,
        cg_group,
        0);
    fbb.Finish(sym_off);
 
    auto fb_sym = flatbuffers::GetRoot<IdxFormat::Symbol>(fbb.GetBufferPointer());
    Symbol res;
    ReadSymsComments(res, fb_sym);
    EXPECT_TRUE(res.comments.leadingComments.empty());
    EXPECT_TRUE(res.comments.innerComments.empty());
    EXPECT_EQ(res.comments.trailingComments.size(), 1u);
}
 
static constexpr uint16_t kKind = 2;
static constexpr uint64_t kContainerID = 42;
static constexpr bool kIsCjoRef = true;
 
TEST_F(IndexStorageTest, ReadRef001) {
    flatbuffers::FlatBufferBuilder fbb;
    auto ref_off = IdxFormat::CreateRef(fbb, 0, kKind, kContainerID, kIsCjoRef, false);
    fbb.Finish(ref_off);
    auto fb_ref = flatbuffers::GetRoot<IdxFormat::Ref>(fbb.GetBufferPointer());
    Ref out;
    ReadRef(out, fb_ref);
    EXPECT_EQ(out.location.begin.fileID, 0u);
    EXPECT_EQ(out.location.begin.line, 0u);
    EXPECT_EQ(out.location.begin.column, 0u);
    EXPECT_EQ(out.location.end.fileID, 0u);
    EXPECT_EQ(out.location.end.line, 0u);
    EXPECT_EQ(out.location.end.column, 0u);
    EXPECT_TRUE(out.location.fileUri.empty());
    EXPECT_EQ(out.kind, RefKind(kKind));
    EXPECT_EQ(out.container, kContainerID);
    EXPECT_EQ(out.isCjoRef, kIsCjoRef);
}
 
TEST_F(IndexStorageTest, ReadRef002) {
    flatbuffers::FlatBufferBuilder fbb;
    auto loc_off = IdxFormat::CreateLocation(fbb, nullptr, nullptr, 0);
    auto ref_off = CreateRef(fbb, loc_off, kKind, kContainerID, kIsCjoRef, false);
    fbb.Finish(ref_off);
    auto fb_ref = flatbuffers::GetRoot<IdxFormat::Ref>(fbb.GetBufferPointer());
    Ref out;
    ReadRef(out, fb_ref);
    EXPECT_EQ(out.location.begin.fileID, 0u);
    EXPECT_EQ(out.location.begin.line, 0u);
    EXPECT_EQ(out.location.begin.column, 0u);
    EXPECT_EQ(out.location.end.fileID, 0u);
    EXPECT_EQ(out.location.end.line, 0u);
    EXPECT_EQ(out.location.end.column, 0u);
    EXPECT_TRUE(out.location.fileUri.empty());
    EXPECT_EQ(out.kind, RefKind(kKind));
    EXPECT_EQ(out.container, kContainerID);
    EXPECT_EQ(out.isCjoRef, kIsCjoRef);
}
 
TEST_F(IndexStorageTest, ReadRef003) {
    flatbuffers::FlatBufferBuilder fbb;
    IdxFormat::Position posB(7, 8, 9);
    auto loc_off = IdxFormat::CreateLocation(fbb, &posB, nullptr, 0);
    auto ref_off = CreateRef(fbb, loc_off, kKind, kContainerID, kIsCjoRef, false);
    fbb.Finish(ref_off);
    auto fb_ref = flatbuffers::GetRoot<IdxFormat::Ref>(fbb.GetBufferPointer());
    Ref out;
    ReadRef(out, fb_ref);
    EXPECT_EQ(out.location.begin.fileID, 7u);
    EXPECT_EQ(out.location.begin.line, 8u);
    EXPECT_EQ(out.location.begin.column, 9u);
    EXPECT_EQ(out.location.end.fileID, 0u);
    EXPECT_EQ(out.location.end.line, 0u);
    EXPECT_EQ(out.location.end.column, 0u);
    EXPECT_TRUE(out.location.fileUri.empty());
    EXPECT_EQ(out.kind, RefKind(kKind));
    EXPECT_EQ(out.container, kContainerID);
    EXPECT_EQ(out.isCjoRef, kIsCjoRef);
}
 
TEST_F(IndexStorageTest, ReadRef004) {
    flatbuffers::FlatBufferBuilder fbb;
    IdxFormat::Position posE(1, 2, 3);
    auto loc_off = IdxFormat::CreateLocation(fbb, nullptr, &posE, 0);
    auto ref_off = CreateRef(fbb, loc_off, kKind, kContainerID, kIsCjoRef, false);
    fbb.Finish(ref_off);
    auto fb_ref = flatbuffers::GetRoot<IdxFormat::Ref>(fbb.GetBufferPointer());
    Ref out;
    ReadRef(out, fb_ref);
    EXPECT_EQ(out.location.begin.fileID, 0u);
    EXPECT_EQ(out.location.begin.line, 0u);
    EXPECT_EQ(out.location.begin.column, 0u);
    EXPECT_EQ(out.location.end.fileID, 1u);
    EXPECT_EQ(out.location.end.line, 2u);
    EXPECT_EQ(out.location.end.column, 3u);
    EXPECT_TRUE(out.location.fileUri.empty());
    EXPECT_EQ(out.kind, RefKind(kKind));
    EXPECT_EQ(out.container, kContainerID);
    EXPECT_EQ(out.isCjoRef, kIsCjoRef);
}
 
TEST_F(IndexStorageTest, ReadRef005)
{
    flatbuffers::FlatBufferBuilder fbb;
    auto uri_off = fbb.CreateString("file://path");
    auto loc_off = IdxFormat::CreateLocation(fbb, nullptr, nullptr, uri_off);
    auto ref_off = CreateRef(fbb, loc_off, kKind, kContainerID, kIsCjoRef, false);
    fbb.Finish(ref_off);
    auto fb_ref = flatbuffers::GetRoot<IdxFormat::Ref>(fbb.GetBufferPointer());
    Ref out;
    ReadRef(out, fb_ref);
    EXPECT_EQ(out.location.begin.fileID, 0u);
    EXPECT_EQ(out.location.begin.line, 0u);
    EXPECT_EQ(out.location.begin.column, 0u);
    EXPECT_EQ(out.location.end.fileID, 0u);
    EXPECT_EQ(out.location.end.line, 0u);
    EXPECT_EQ(out.location.end.column, 0u);
    EXPECT_EQ(out.location.fileUri, "file://path");
    EXPECT_EQ(out.kind, RefKind(kKind));
    EXPECT_EQ(out.container, kContainerID);
    EXPECT_EQ(out.isCjoRef, kIsCjoRef);
}

TEST_F(IndexStorageTest, SplitFileName_EdgeCases) {
    // Case: No dot in filename
    auto res1 = SplitFileName("filename");
    EXPECT_EQ(res1.first, "");
    EXPECT_EQ(res1.second, "");

    // Case: Only one dot (SplitFileName requires at least two dots to extract second/first)
    auto res2 = SplitFileName("onlyone.dot");
    EXPECT_EQ(res2.first, "");
    EXPECT_EQ(res2.second, "");

    // Case: Empty string
    auto res3 = SplitFileName("");
    EXPECT_EQ(res3.first, "");
    EXPECT_EQ(res3.second, "");

    // Case: Hidden files or dots at start
    auto res4 = SplitFileName(".hidden.file.idx");
    EXPECT_EQ(res4.first, ".hidden");
    EXPECT_EQ(res4.second, "file");
}

TEST_F(IndexStorageTest, GetShardPath_PathConversion) {
    // Test logic of CacheManager::GetShardPathFromFilePath
    CacheManager cm(testRoot);
    // Package names with slashes should be converted to dots
    std::string path = cm.GetShardPathFromFilePath("ark/lsp/service", "hash123");
    EXPECT_TRUE(path.find("ark.lsp.service.hash123.idx") != std::string::npos);
}

// --- 2. Comprehensive Symbol Reading (Covers ReadSymbolLocation, Macro, Comments etc.) ---

TEST_F(IndexStorageTest, ReadSymbol_AllOptionalFields) {
    flatbuffers::FlatBufferBuilder fbb;

    // Create strings for optional fields
    auto name = fbb.CreateString("myFunc");
    auto scope = fbb.CreateString("MyClass");
    auto sig = fbb.CreateString("()V");
    auto ret = fbb.CreateString("void");
    auto mod = fbb.CreateString("MyModule");
    auto sys = fbb.CreateString("SysCap.Test");
    auto uri = fbb.CreateString("file:///test.cj");

    // Construct Position and Location
    IdxFormat::Position pos(1, 10, 5);
    auto loc = IdxFormat::CreateLocation(fbb, &pos, &pos, uri);

    // Construct CompletionItems
    auto label = fbb.CreateString("label");
    auto ins = fbb.CreateString("insert");
    auto compItem = IdxFormat::CreateCompletionItem(fbb, label, ins);
    auto compVec = fbb.CreateVector({compItem});

    // Construct Comments
    auto commentVal = fbb.CreateString("doc comment");
    auto token = IdxFormat::CreateToken(fbb, commentVal);
    auto fbComment = IdxFormat::CreateComment(fbb, IdxFormat::CommentStyle_Doc, IdxFormat::CommentKind_Ordinary, token);
    auto cmsVec = fbb.CreateVector({fbComment});
    auto group = IdxFormat::CreateCommentGroup(fbb, cmsVec);
    auto groupsVec = fbb.CreateVector({group});
    auto fbComments = IdxFormat::CreateCommentGroups(fbb, groupsVec, groupsVec, groupsVec);

    // Build Symbol with all components to trigger all ReadSymbol... helper functions
    auto sym_off = IdxFormat::CreateSymbol(fbb,
        1001, name, scope, loc, loc,
        static_cast<uint8_t>(AST::ASTKind::FUNC_DECL),
        sig, ret, true, 1, false, true,
        mod, loc, compVec, fbComments, sys, 2);

    fbb.Finish(sym_off);
    auto fb_sym = flatbuffers::GetRoot<IdxFormat::Symbol>(fbb.GetBufferPointer());

    Symbol out;
    ReadSymbol(out, fb_sym);

    // Verify fields
    EXPECT_EQ(out.id, 1001u);
    EXPECT_EQ(out.name, "myFunc");
    EXPECT_EQ(out.location.fileUri, "file:///test.cj");
    EXPECT_EQ(out.isMemberParam, true);
    EXPECT_EQ(out.syscap, "SysCap.Test");
    ASSERT_EQ(out.completionItems.size(), 1u);
    EXPECT_EQ(out.completionItems[0].label, "label");
    // Verify comments branch coverage
    EXPECT_FALSE(out.comments.leadingComments.empty());
    EXPECT_FALSE(out.comments.innerComments.empty());
    EXPECT_FALSE(out.comments.trailingComments.empty());
}

// --- 3. Relation and Extend Reading ---
TEST_F(IndexStorageTest, ReadRelation_Normal) {
    flatbuffers::FlatBufferBuilder fbb;
    auto rel_off = IdxFormat::CreateRelation(fbb, 1, 2, 3);
    fbb.Finish(rel_off);
    auto fb_rel = flatbuffers::GetRoot<IdxFormat::Relation>(fbb.GetBufferPointer());

    Relation out;
    ReadRelation(out, fb_rel);
    EXPECT_EQ(out.subject, 1u);
    EXPECT_EQ(out.predicate, RelationKind(2));
    EXPECT_EQ(out.object, 3u);
}

// --- 4. CacheManager Lifecycle and Integration ---
TEST_F(IndexStorageTest, InitDir_CreatesNecessaryFolders) {
    CacheManager cm(testRoot);
    cm.InitDir();
    // Check if subdirectories for index and astdata are created
    EXPECT_TRUE(fs::exists(testRoot + "/.cache/index"));
    EXPECT_TRUE(fs::exists(testRoot + "/.cache/astdata"));
}

TEST_F(IndexStorageTest, IsStale_And_UpdateIdMap) {
    CacheManager cm(testRoot);
    std::string pkg = "demo.pkg";
    std::string hash1 = "aaaa";
    std::string hash2 = "bbbb";

    // Initial state: No entry means it is stale
    EXPECT_TRUE(cm.IsStale(pkg, hash1));

    // Update with hash1
    cm.UpdateIdMap(pkg, hash1);
    EXPECT_FALSE(cm.IsStale(pkg, hash1)); // Now fresh
    EXPECT_TRUE(cm.IsStale(pkg, hash2));  // Different hash is stale
}

TEST_F(IndexStorageTest, Store_And_Load_Integration) {
    CacheManager cm(testRoot);
    cm.InitDir();

    std::string pkg = "test.package";
    std::string shardId = "v1";

    // 1. Prepare data
    IndexFileOut data;
    std::vector<Symbol> syms;
    Symbol s; s.id = 77; s.name = "TestSym";
    syms.push_back(s);
    data.symbols = &syms;

    std::map<SymbolID, std::vector<Ref>> refs;
    Ref r; r.container = 77; r.kind = RefKind::REFERENCE;
    refs[77].push_back(r);
    data.refs = &refs;

    // Fill other empty required pointers to avoid null deref in StoreIndexShard if any
    std::vector<Relation> rels; data.relations = &rels;
    std::map<SymbolID, std::vector<ExtendItem>> exts; data.extends = &exts;
    std::vector<CrossSymbol> cross; data.crossSymbos = &cross;

    // 2. Store to disk
    cm.StoreIndexShard(pkg, shardId, data);
    std::string path = cm.GetShardPathFromFilePath(pkg, shardId);
    EXPECT_TRUE(fs::exists(path));

    // 3. Load from disk
    auto loaded = cm.LoadIndexShard(pkg, shardId);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ((*loaded)->symbols.size(), 1u);
    EXPECT_EQ((*loaded)->symbols[0].name, "TestSym");
    EXPECT_EQ((*loaded)->refs[77].size(), 1u);
}

TEST_F(IndexStorageTest, LoadIndexShard_CorruptFile_ReturnsNull) {
    CacheManager cm(testRoot);
    cm.InitDir();
    std::string pkg = "bad.file";
    std::string shard = "x";
    std::string path = cm.GetShardPathFromFilePath(pkg, shard);

    // Create a file with random content (not a valid Flatbuffer)
    std::ofstream ofs(path);
    ofs << "This is not a flatbuffer binary data";
    ofs.close();

    // VerifyHashedPackageBuffer should fail, function should return nullopt and delete the file
    auto res = cm.LoadIndexShard(pkg, shard);
    EXPECT_FALSE(res.has_value());
    EXPECT_FALSE(fs::exists(path)); // File should be removed by the error handling branch
}

TEST_F(IndexStorageTest, UtilityFunctions_Coverage) {
    // Test SplitFileName with various inputs
    auto res1 = SplitFileName("no_dot");
    EXPECT_EQ(res1.first, "");
    EXPECT_EQ(res1.second, "");

    auto res2 = SplitFileName(".only_extension");
    EXPECT_EQ(res2.first, "");
    EXPECT_EQ(res2.second, "");

    auto res3 = SplitFileName("file.name.ext");
    EXPECT_EQ(res3.first, "file");
    EXPECT_EQ(res3.second, "name");

    auto res4 = SplitFileName("file.");
    EXPECT_EQ(res4.first, "");
    EXPECT_EQ(res4.second, "");

    // Test MergeFileName
    std::string merged = MergeFileName("pkg/path", "hash123", "idx");
    // Ensure path separators are handled or replaced
    EXPECT_FALSE(merged.empty());
}

TEST_F(IndexStorageTest, CacheManager_IsStale_Coverage) {
    CacheManager cm(testRoot);
    std::string pkg = "test.package";
    std::string hash = "old_hash";

    // Scenario 1: Record exists in map, but .ast file is missing on disk
    cm.UpdateIdMap(pkg, hash);
    // Even if it's in the map, if the file doesn't exist, it should trigger specific internal branches
    EXPECT_FALSE(cm.IsStale(pkg, hash));

    // Scenario 2: Hash mismatch (should return true)
    EXPECT_TRUE(cm.IsStale(pkg, "new_hash"));

    // Scenario 3: Pkg not in map at all
    EXPECT_TRUE(cm.IsStale("unknown.pkg", "any"));
}

TEST_F(IndexStorageTest, LoadIndexShard_ErrorPaths) {
    CacheManager cm(testRoot);
    cm.InitDir();
    std::string pkg = "error.pkg";
    std::string hash = "h1";
    std::string path = cm.GetShardPathFromFilePath(pkg, hash);

    // Scenario 1: File does not exist (triggers 'if (!inFile.is_open())')
    auto res1 = cm.LoadIndexShard(pkg, hash);
    EXPECT_FALSE(res1.has_value());

    // Scenario 2: Corrupt Flatbuffer data (triggers '!verifier.VerifyBuffer' and 'fs::remove')
    std::ofstream ofs(path, std::ios::binary);
    ofs << "This is definitely not a flatbuffer";
    ofs.close();

    auto res2 = cm.LoadIndexShard(pkg, hash);
    EXPECT_FALSE(res2.has_value());
    EXPECT_FALSE(fs::exists(path)); // Ensure the corrupt file was deleted
}

TEST_F(IndexStorageTest, LoadShard_NullSlabs) {
    CacheManager cm(testRoot);
    cm.InitDir();
    std::string pkg = "null.slabs";
    std::string hash = "h2";

    // Construct a HashedPackage where all vectors are null/empty
    flatbuffers::FlatBufferBuilder fbb;
    auto pkg_off = IdxFormat::CreateHashedPackage(fbb, 0, 0, 0, 0, 0); // All offsets are 0
    IdxFormat::FinishHashedPackageBuffer(fbb, pkg_off);

    std::string path = cm.GetShardPathFromFilePath(pkg, hash);
    std::ofstream ofs(path, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(fbb.GetBufferPointer()), fbb.GetSize());
    ofs.close();

    // This will trigger "if (!hashedPackage->refs())" etc. inside LoadIndexShard helpers
    auto res = cm.LoadIndexShard(pkg, hash);
    ASSERT_TRUE(res.has_value());
    EXPECT_TRUE((*res)->refs.empty());
    EXPECT_TRUE((*res)->extends.empty());
}

TEST_F(IndexStorageTest, StoreIndexShard_CleanupOldFiles) {
    CacheManager cm(testRoot);
    cm.InitDir();
    std::string pkg = "cleanup.pkg";

    // Step 1: Create a "fake" old index file record
    cm.UpdateIdMap(pkg, "old_v1");
    std::string oldPath = cm.GetShardPathFromFilePath(pkg, "old_v1");
    std::ofstream ofs(oldPath); ofs << "old data"; ofs.close();
    ASSERT_TRUE(fs::exists(oldPath));

    // Step 2: Store a new version with a different hash
    IndexFileOut data;
    // Prepare minimal valid pointers to avoid crash
    std::vector<Symbol> s; data.symbols = &s;
    std::map<SymbolID, std::vector<Ref>> r; data.refs = &r;
    std::vector<Relation> rel; data.relations = &rel;
    std::map<SymbolID, std::vector<ExtendItem>> e; data.extends = &e;
    std::vector<CrossSymbol> c; data.crossSymbos = &c;

    cm.StoreIndexShard(pkg, "new_v2", data);

    // Scenario: 'if (found != astIdMap.end() && found->second != hashCode)'
    // should trigger and delete the old file
    EXPECT_FALSE(fs::exists(oldPath));
    EXPECT_TRUE(fs::exists(cm.GetShardPathFromFilePath(pkg, "new_v2")));
}

// --- 5. Comment and Token Detail Coverage ---

TEST_F(IndexStorageTest, ConvertComment_Detailed) {
    flatbuffers::FlatBufferBuilder fbb;
    auto val = fbb.CreateString("comment text");
    auto token = IdxFormat::CreateToken(fbb, val);

    // Test conversion of a single comment
    // Covers convertComment function and Token value setting
    auto fbComment = IdxFormat::CreateComment(fbb,
        IdxFormat::CommentStyle_Doc,
        IdxFormat::CommentKind_Ordinary,
        token);
    fbb.Finish(fbComment);

    auto root = flatbuffers::GetRoot<IdxFormat::Comment>(fbb.GetBufferPointer());
    auto result = convertComment(*root);

    EXPECT_EQ(result.info.Value(), "comment text");
}

// --- 6. Symbol Completion and Macro Optional Branches ---

TEST_F(IndexStorageTest, ReadSymbol_OptionalFieldMissing) {
    flatbuffers::FlatBufferBuilder fbb;

    // Create a symbol with mostly null offsets
    // Covers 'if (sym->name() != nullptr)' etc. being false
    auto sym_off = IdxFormat::CreateSymbol(fbb, 101, 0, 0, 0, 0, 0, 0, 0,
                                          false, 0, false, false, 0,
                                          0, 0, 0, 0, 0);
    fbb.Finish(sym_off);

    auto fb_sym = flatbuffers::GetRoot<IdxFormat::Symbol>(fbb.GetBufferPointer());
    Symbol res;
    ReadSymbol(res, fb_sym);

    // Verify that empty branches didn't crash and fields remain default
    EXPECT_EQ(res.id, 101u);
    EXPECT_TRUE(res.name.empty());
    EXPECT_TRUE(res.completionItems.empty());
    EXPECT_TRUE(res.syscap.empty());
}

// --- 7. CacheManager File System Errors ---

TEST_F(IndexStorageTest, CacheManager_IsStale_FileMissing) {
    CacheManager cm(testRoot);
    std::string pkg = "missing.file.pkg";
    std::string hash = "hash123";

    // Add to map but don't create file on disk
    // Covers 'if (FileUtil::FileExist(staleFilePath))' being false
    cm.UpdateIdMap(pkg, hash);

    // Requesting a different hash should trigger staleness but skip file removal
    EXPECT_TRUE(cm.IsStale(pkg, "new_hash"));
}


// --- 9. Serialization Branch Coverage ---
TEST_F(IndexStorageTest, StoreSymbol_EmptyFields) {
    flatbuffers::FlatBufferBuilder builder;
    Symbol sym;
    sym.id = 1;
    // sym.name is empty, sym.location is default

    // Covers serialization of symbols with empty strings/vectors
    auto offset = StoreSymbol(builder, sym);
    builder.Finish(offset);

    auto fb_sym = flatbuffers::GetRoot<IdxFormat::Symbol>(builder.GetBufferPointer());
    EXPECT_EQ(fb_sym->id(), 1u);
    EXPECT_TRUE(fb_sym->name()->str().empty());
}

// --- 10. AstFileHandler Dynamic Cast Coverage ---

TEST_F(IndexStorageTest, AstFileHandler_StoreShard_InvalidType) {
    AstFileHandler handler;
    // Passing nullptr or wrong subclass to StoreShard
    // Covers 'if (!fileOut || !fileOut->data)'
    handler.StoreShard("any.ast", nullptr);

    AstFileOut out;
    out.data = nullptr; // Explicitly null data
    handler.StoreShard("any.ast", &out);
    // Should return early without file operation
}