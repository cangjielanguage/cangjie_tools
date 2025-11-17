// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "IndexStorage.h"

#include <algorithm>
#include <fstream>
#include <ios>
#include <regex>
#include <string>
#include <utility>
#include "../Options.h"
#include "../common/FileStore.h"

namespace ark {
namespace lsp {
namespace {
std::pair<std::string, std::string> SplitFileName(const std::string &file)
{
    std::pair<std::string, std::string> res;
    auto found = file.find_last_of(".");
    if (found == std::string::npos) {
        return res;
    }
    std::string temp = file.substr(0, found);
    if (found = temp.find_last_of("."), found != std::string::npos) {
        res.second = temp.substr(found + 1);
        res.first = temp.substr(0, found);
    }
    return res;
}

std::string MergeFileName(const std::string& fullPkgName, const std::string &hashCode,
                          const std::string &extension)
{
    return fullPkgName + "." + hashCode + "." + extension;
}

void ReadSymbol(Symbol &res, const IdxFormat::Symbol *sym)
{
    res.id = sym->id();
    if (sym->name() != nullptr) {
        res.name = sym->name()->str();
    }
    if (sym->scope() != nullptr) {
        res.scope = sym->scope()->str();
    }
    if (sym->location() != nullptr) {
        if (sym->location()->begin() != nullptr) {
            res.location.begin.fileID = sym->location()->begin()->file_id();
            res.location.begin.line = sym->location()->begin()->line();
            res.location.begin.column = sym->location()->begin()->column();
        }
        if (sym->location()->end() != nullptr) {
            res.location.end.fileID = sym->location()->end()->file_id();
            res.location.end.line = sym->location()->end()->line();
            res.location.end.column = sym->location()->end()->column();
        }
        if (sym->location()->file_uri() != nullptr) {
            res.location.fileUri = sym->location()->file_uri()->str();
        }
    }
    if (sym->declaration() != nullptr) {
        if (sym->declaration()->begin() != nullptr) {
            res.declaration.begin.fileID = sym->declaration()->begin()->file_id();
            res.declaration.begin.line = sym->declaration()->begin()->line();
            res.declaration.begin.column = sym->declaration()->begin()->column();
        }
        if (sym->declaration()->end() != nullptr) {
            res.declaration.end.fileID = sym->declaration()->end()->file_id();
            res.declaration.end.line = sym->declaration()->end()->line();
            res.declaration.end.column = sym->declaration()->end()->column();
        }
        if (sym->declaration()->file_uri() != nullptr) {
            res.declaration.fileUri = sym->declaration()->file_uri()->str();
        }
    }
    if (sym->cur_macro_call() != nullptr) {
        if (sym->cur_macro_call()->begin() != nullptr) {
            res.curMacroCall.begin.fileID = sym->cur_macro_call()->begin()->file_id();
            res.curMacroCall.begin.line = sym->cur_macro_call()->begin()->line();
            res.curMacroCall.begin.column = sym->cur_macro_call()->begin()->column();
        }
        if (sym->cur_macro_call()->end() != nullptr) {
            res.curMacroCall.end.fileID = sym->cur_macro_call()->end()->file_id();
            res.curMacroCall.end.line = sym->cur_macro_call()->end()->line();
            res.curMacroCall.end.column = sym->cur_macro_call()->end()->column();
        }
        if (sym->cur_macro_call()->file_uri() != nullptr) {
            res.curMacroCall.fileUri = sym->cur_macro_call()->file_uri()->str();
        }
    }
    res.kind = AST::ASTKind(sym->kind());
    if (sym->signature() != nullptr) {
        res.signature = sym->signature()->str();
    }
    if (sym->return_type() != nullptr) {
        res.returnType = sym->return_type()->str();
    }
    res.isMemberParam = sym->is_member_param();
    res.modifier = Modifier(sym->modifier());
    res.isCjoSym = sym->is_cjo_sym();
    res.isDeprecated = sym->is_deprecated();
    if (sym->insert_text()) {
        res.insertText = sym->insert_text()->str();
    }
    if (sym->cur_module()) {
        res.curModule = sym->cur_module()->str();
    }
}

void ReadRef(Ref &res, const IdxFormat::Ref *ref)
{
    if (ref->location() != nullptr) {
        if (ref->location()->begin() != nullptr) {
            res.location.begin.fileID = ref->location()->begin()->file_id();
            res.location.begin.line = ref->location()->begin()->line();
            res.location.begin.column = ref->location()->begin()->column();
        }
        if (ref->location()->end() != nullptr) {
            res.location.end.fileID = ref->location()->end()->file_id();
            res.location.end.line = ref->location()->end()->line();
            res.location.end.column = ref->location()->end()->column();
        }
        if (ref->location()->file_uri() != nullptr) {
            res.location.fileUri = ref->location()->file_uri()->str();
        }
    }
    res.kind = RefKind(ref->kind());
    res.container = ref->id();
    res.isCjoRef = ref->is_cjo_ref();
}

void ReadRelation(Relation &res, const IdxFormat::Relation *relation)
{
    res.subject = relation->subject();
    res.predicate = RelationKind(relation->kind());
    res.object = relation->object();
}

auto StoreSymbol(flatbuffers::FlatBufferBuilder &builder, const Symbol &sym)
{
    auto name = builder.CreateString(sym.name);
    auto scope = builder.CreateString(sym.scope);
    auto begin = IdxFormat::Position(sym.location.begin.fileID, sym.location.begin.line,
                                     sym.location.begin.column);
    auto end = IdxFormat::Position(sym.location.end.fileID, sym.location.end.line,
                                   sym.location.end.column);
    auto uri = builder.CreateString(sym.location.fileUri);
    auto loc = IdxFormat::CreateLocation(builder, &begin, &end, uri);
    auto decl_begin = IdxFormat::Position(sym.declaration.begin.fileID, sym.declaration.begin.line,
                                          sym.declaration.begin.column);
    auto decl_end = IdxFormat::Position(sym.declaration.end.fileID, sym.declaration.end.line,
                                        sym.declaration.end.column);
    auto decl_uri = builder.CreateString(sym.declaration.fileUri);
    auto decl_loc = IdxFormat::CreateLocation(builder, &decl_begin, &decl_end, decl_uri);
    auto sig = builder.CreateString(sym.signature);
    auto ret = builder.CreateString(sym.returnType);
    auto text = builder.CreateString(sym.insertText);
    auto module = builder.CreateString(sym.curModule);
    auto macro_call_begin = IdxFormat::Position(sym.curMacroCall.begin.fileID,
                                                sym.curMacroCall.begin.line, sym.curMacroCall.begin.column);
    auto macro_call_end = IdxFormat::Position(sym.curMacroCall.end.fileID,
                                              sym.curMacroCall.end.line, sym.curMacroCall.end.column);
    auto macro_call_uri = builder.CreateString(sym.curMacroCall.fileUri);
    auto macro = IdxFormat::CreateLocation(builder, &macro_call_begin,
                                           &macro_call_end, macro_call_uri);
    return IdxFormat::CreateSymbol(builder, sym.id, name, scope, loc, decl_loc,
                                   static_cast<uint16_t>(sym.kind), sig, ret, sym.isMemberParam,
                                   static_cast<uint8_t>(sym.modifier), sym.isCjoSym, sym.isDeprecated,
                                   text, module, macro);
}

auto StoreRef(flatbuffers::FlatBufferBuilder &builder, const Ref &ref)
{
    auto begin = IdxFormat::Position(ref.location.begin.fileID, ref.location.begin.line,
                                     ref.location.begin.column);
    auto end = IdxFormat::Position(ref.location.end.fileID, ref.location.end.line,
                                   ref.location.end.column);
    auto uri = builder.CreateString(ref.location.fileUri);
    auto loc = IdxFormat::CreateLocation(builder, &begin, &end, uri);
    return IdxFormat::CreateRef(builder, loc, static_cast<uint16_t>(ref.kind), ref.container, ref.isCjoRef);
}

auto StoreExtend(flatbuffers::FlatBufferBuilder &builder, const ExtendItem &extendItem)
{
    auto interfaceName = builder.CreateString(extendItem.interfaceName);
    return IdxFormat::CreateExtend(builder, extendItem.id,
        static_cast<uint8_t>(extendItem.modifier), interfaceName);
}

auto StoreRelation(flatbuffers::FlatBufferBuilder &builder, const Relation &re)
{
    return IdxFormat::CreateRelation(builder, re.subject, static_cast<uint16_t>(re.predicate),
                                     re.object);
}
} // namespace

std::optional<std::unique_ptr<FileIn>> AstFileHandler::LoadShard(std::string filePath) const
{
    auto in = std::make_unique<AstFileIn>();
    std::string reason;
    bool ret = FileUtil::ReadBinaryFileToBuffer(filePath, in->data, reason);
    if (!ret) {
        Trace::Elog(reason);
        return std::nullopt;
    }
    return std::move(in);
}

void AstFileHandler::StoreShard(std::string filePath, const FileOut *out) const
{
    auto fileOut = dynamic_cast<const AstFileOut *>(out);
    if (!fileOut || !fileOut->data) {
        return;
    }
    bool ret = FileUtil::WriteBufferToASTFile(filePath, *fileOut->data);
    if (!ret) {
        Trace::Log("ast file write");
    }
}

void CacheManager::InitDir()
{
    std::string cacheRoot = FileUtil::JoinPath(basePath, ".cache");
    astdataDir = FileUtil::JoinPath(cacheRoot, "astdata/");
    indexDir = FileUtil::JoinPath(cacheRoot, "index/");
    if (!FileUtil::FileExist(astdataDir)) {
        auto ret = FileUtil::CreateDirs(astdataDir);
        if (ret == -1) {
            return;
        }
    }

    if (!FileUtil::FileExist(indexDir)) {
        auto ret = FileUtil::CreateDirs(indexDir);
        if (ret == -1) {
            return;
        }
    }

    for (const auto &iter : FileUtil::GetAllFilesUnderCurrentPath(astdataDir, "ast")) {
        (void)astIdMap.emplace(SplitFileName(iter));
    }
}

bool CacheManager::IsStale(const std::string &pkgName, const std::string &digest)
{
    auto found = astIdMap.find(pkgName);
    // Package do not have cache in disk.
    if (found == astIdMap.end()) {
        return true;
    }
    // Cache is fresh, not to rebuild.
    if (found->second == digest) {
        return false;
    }

    auto staleFileName = MergeFileName(pkgName, found->second, "ast");
    auto staleFilePath = FileUtil::JoinPath(astdataDir, staleFileName);
    // Cache is stale, delete cache file and rebuild package.
    if (FileUtil::FileExist(staleFilePath)) {
        (void)FileUtil::Remove(staleFilePath);
    }
    return true;
}

void CacheManager::UpdateIdMap(const std::string &pkgName, const std::string &digest)
{
    std::lock_guard<std::mutex> lock(cacheMtx);
    astIdMap.insert_or_assign(pkgName, digest);
}

std::optional<std::unique_ptr<FileIn>> CacheManager::Load(const std::string &pkgName)
{
    auto found = astIdMap.find(pkgName);
    if (found == astIdMap.end()) {
        return std::nullopt;
    }
    std::string fileName = MergeFileName(pkgName, found->second, "ast");
    std::string filePath = FileUtil::JoinPath(astdataDir, fileName);
    return astLoader->LoadShard(filePath);
}

void CacheManager::Store(const std::string &pkgName, const std::string &digest, const std::vector<uint8_t> &buffer)
{
    if (digest.empty() || Options::GetInstance().IsOptionSet("--test")) {
        return;
    }
    lsp::AstFileOut out;
    out.shardId = digest;
    out.data = &buffer;

    auto found = astIdMap.find(pkgName);
    if (found != astIdMap.end()) {
        auto staleFileName = MergeFileName(pkgName, found->second, "ast");
        auto staleFilePath = FileUtil::JoinPath(astdataDir, staleFileName);
        if (FileUtil::FileExist(staleFilePath)) {
            (void)FileUtil::Remove(staleFilePath);
        }
    }
    UpdateIdMap(pkgName, digest);
    std::string fileName = MergeFileName(pkgName, digest, "ast");
    std::string filePath = FileUtil::JoinPath(astdataDir, fileName);
    if (!filePath.empty()) {
        astLoader->StoreShard(filePath, &out);
    }
}

std::optional<std::unique_ptr<IndexFileIn>> CacheManager::LoadIndexShard(const std::string &curPkgName,
                                                                         const std::string &shardIdentifier) const
{
    std::string idxFilePath = GetShardPathFromFilePath(curPkgName, shardIdentifier);
    std::vector<uint8_t> buffer;
    std::string reason;

    (void)FileUtil::ReadBinaryFileToBuffer(idxFilePath, buffer, reason);

    flatbuffers::Verifier verifier = flatbuffers::Verifier(buffer.data(), buffer.size());
    bool ok = IdxFormat::VerifyHashedPackageBuffer(verifier);
    if (!ok) {
        (void)FileUtil::Remove(idxFilePath);
        return std::nullopt;
    }

    auto ifi = std::make_unique<IndexFileIn>();
    auto hashedPackage = IdxFormat::GetHashedPackage(buffer.data());
    if (hashedPackage == nullptr) {
        return std::nullopt;
    }
    // read symbols
    auto symbolSlab = hashedPackage->symbol_slab();
    if (symbolSlab != nullptr) {
        for (flatbuffers::uoffset_t i = 0; i < symbolSlab->size(); ++i) {
            auto sym = symbolSlab->Get(i);
            Symbol res;
            ReadSymbol(res, sym);
            (void)ifi->symbols.emplace_back(res);
        }
    }

    // read refs
    readRefs(*hashedPackage, ifi);

    // read relations
    auto relationSlab = hashedPackage->relation_slab();
    if (relationSlab != nullptr) {
        for (flatbuffers::uoffset_t i = 0; i < relationSlab->size(); ++i) {
            auto relation = relationSlab->Get(i);
            Relation res;
            ReadRelation(res, relation);
            (void)ifi->relations.emplace_back(res);
        }
    }

    // read extends
    readExtends(*hashedPackage, ifi);
    return std::move(ifi);
}

void CacheManager::readRefs(
    const IdxFormat::HashedPackage &package, std::unique_ptr<ark::lsp::IndexFileIn> &ifi) const
{
    auto refSlab = package.ref_slab();
    if (refSlab == nullptr) {
        return;
    }
    for (flatbuffers::uoffset_t i = 0; i < refSlab->size(); ++i) {
        std::pair<SymbolID, std::vector<Ref>> resPair;
        auto sym2Ref = refSlab->Get(i);
        resPair.first = sym2Ref->id();
        auto refs = sym2Ref->refs();
        if (refs != nullptr) {
            for (flatbuffers::uoffset_t j = 0; j < refs->size(); ++j) {
                auto ref = refs->Get(j);
                Ref res;
                ReadRef(res, ref);
                (void)resPair.second.emplace_back(res);
            }
        }
        (void)ifi->refs.emplace(resPair);
    }
}

void CacheManager::readExtends(
    const IdxFormat::HashedPackage &package, std::unique_ptr<ark::lsp::IndexFileIn> &ifi) const
{
    auto extendSlab = package.extend_slab();
    if (extendSlab != nullptr) {
        for (flatbuffers::uoffset_t i = 0; i < extendSlab->size(); ++i) {
            auto extendItem = extendSlab->Get(i);
            if (!extendItem) {
                continue;
            }
            std::pair<SymbolID, std::vector<ExtendItem>> extendPair;
            extendPair.first = extendItem->id();
            auto extends = extendItem->extends();
            if (extends == nullptr) {
                continue;
            }
            for (flatbuffers::uoffset_t j = 0; j < extends->size(); ++j) {
                auto extend = extends->Get(j);
                ExtendItem res = {.id = extend->id(),
                                  .modifier = Modifier(extend->modifier()),
                                  .interfaceName = extend->interface() ? extend->interface()->str() : ""};
                (void)extendPair.second.emplace_back(res);
            }
            (void)ifi->extends.emplace(extendPair);
        }
    }
}

void CacheManager::StoreIndexShard(const std::string &curPkgName, const std::string &shardIdentifier,
                                   const IndexFileOut &shard) const
{
    auto found = astIdMap.find(curPkgName);
    if (found != astIdMap.end()) {
        auto staleFileName = MergeFileName(curPkgName, found->second, "idx");
        auto staleFilePath = FileUtil::JoinPath(indexDir, staleFileName);
        if (FileUtil::FileExist(staleFilePath)) {
            (void)FileUtil::Remove(staleFilePath);
        }
    }

    std::string idxFilePath =
        FileUtil::Normalize(GetShardPathFromFilePath(curPkgName, shardIdentifier));
    flatbuffers::FlatBufferBuilder builder;

    // serialize symbols
    std::vector<flatbuffers::Offset<IdxFormat::Symbol>> symbolVec;
    for (const auto &sym : *shard.symbols) {
        symbolVec.push_back(StoreSymbol(builder, sym));
    }
    auto symbolSlab = builder.CreateVector(symbolVec);

    // serialize refs
    std::vector<flatbuffers::Offset<IdxFormat::Sym2Ref>> sym2RefVec;
    for (const auto &pair : *shard.refs) {
        std::vector<flatbuffers::Offset<IdxFormat::Ref>> refVec;
        for (const auto &ref : pair.second) {
            refVec.push_back(StoreRef(builder, ref));
        }
        auto refs = builder.CreateVector(refVec);
        auto sym2Ref = IdxFormat::CreateSym2Ref(builder, pair.first, refs);
        sym2RefVec.push_back(sym2Ref);
    }
    auto refSlab = builder.CreateVector(sym2RefVec);

    // serialize relations
    std::vector<flatbuffers::Offset<IdxFormat::Relation>> relationVec;
    for (const auto &re : *shard.relations) {
        (void)relationVec.emplace_back(StoreRelation(builder, re));
    }
    auto relationSlab = builder.CreateVector(relationVec);
    // serialize symbol with extends
    std::vector<flatbuffers::Offset<IdxFormat::Sym2Extend>> sym2ExtendVec;
    for (const auto &pair : *shard.extends) {
        std::vector<flatbuffers::Offset<IdxFormat::Extend>> extendVec;
        for (const auto &extend : pair.second) {
            extendVec.push_back(StoreExtend(builder, extend));
        }
        auto extends = builder.CreateVector(extendVec);
        auto sym2Extend = IdxFormat::CreateSym2Extend(builder, pair.first, extends);
        sym2ExtendVec.push_back(sym2Extend);
    }
    auto extendSlab = builder.CreateVector(sym2ExtendVec);
    auto hashedPackage = IdxFormat::CreateHashedPackage(builder, symbolSlab, refSlab, relationSlab, extendSlab);
    IdxFormat::FinishHashedPackageBuffer(builder, hashedPackage);

    std::ofstream outFile{FileStore::NormalizePath(idxFilePath).c_str(), std::ios::binary | std::ios::out};
    (void)outFile.write(reinterpret_cast<char *>(builder.GetBufferPointer()), builder.GetSize());
    outFile.close();
}

std::string CacheManager::GetShardPathFromFilePath(std::string curPkgName,
                                                   const std::string &shardIdentifier) const
{
    std::replace(curPkgName.begin(), curPkgName.end(), '/', '.');
    std::string idxFileName = curPkgName + "." + shardIdentifier + ".idx";
    auto sr = FileUtil::JoinPath(indexDir, idxFileName);
    return sr;
}
} // namespace lsp
} // namespace ark