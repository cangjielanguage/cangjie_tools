// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "IndexStorage.cpp"
 
using namespace ark::lsp;
 
TEST(IndexStorageTest, SplitFileName001) {
    auto res = SplitFileName("filename");
    EXPECT_EQ(res.first, "");
    EXPECT_EQ(res.second, "");
}
 
TEST(IndexStorageTest, SplitFileName002) {
    auto res = SplitFileName("file.txt");
    EXPECT_EQ(res.first, "");
    EXPECT_EQ(res.second, "");
}
 
TEST(IndexStorageTest, SplitFileName003) {
    auto res = SplitFileName("archive.tar.gz");
    EXPECT_EQ(res.first, "archive");
    EXPECT_EQ(res.second, "tar");
}
 
TEST(IndexStorageTest, MergeFileName001) {
    EXPECT_EQ(MergeFileName("mypkg", "123abc", "bin"), "mypkg.123abc.bin");
}
 
TEST(IndexStorageTest, convertCommentGroup001) {
    flatbuffers::FlatBufferBuilder builder;
 
    auto groupOffset = IdxFormat::CreateCommentGroup(builder, 0);
 
    builder.Finish(groupOffset);
    auto group = flatbuffers::GetRoot<IdxFormat::CommentGroup>(builder.GetBufferPointer());
    convertCommentGroup(*group);
}
 
static constexpr uint64_t kTestId    = 42;
static constexpr uint8_t  kTestType  = 3;
static constexpr uint64_t kContainer = 99;
 
TEST(IndexStorageTest, ReadCrossSymbol001) {
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
 
TEST(IndexStorageTest, ReadCrossSymbol002) {
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
 
TEST(IndexStorageTest, ReadCrossSymbol003) {
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
 
TEST(IndexStorageTest, ReadCrossSymbol004) {
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
 
TEST(IndexStorageTest, ReadCrossSymbol006)
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
 
TEST(IndexStorageTest, ReadSymsComments001) {
    // sym == nullptr → early return
    Symbol res;
    ReadSymsComments(res, nullptr);
    EXPECT_TRUE(res.comments.leadingComments.empty());
    EXPECT_TRUE(res.comments.innerComments.empty());
    EXPECT_TRUE(res.comments.trailingComments.empty());
}
 
TEST(IndexStorageTest, ReadSymsComments002) {
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
 
TEST(IndexStorageTest, ReadSymsComments004) {
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
 
TEST(IndexStorageTest, ReadSymsComments005) {
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
 
TEST(IndexStorageTest, ReadSymsComments006) {
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
 
TEST(IndexStorageTest, ReadRef001) {
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
 
TEST(IndexStorageTest, ReadRef002) {
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
 
TEST(IndexStorageTest, ReadRef003) {
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
 
TEST(IndexStorageTest, ReadRef004) {
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
 
TEST(IndexStorageTest, ReadRef005)
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