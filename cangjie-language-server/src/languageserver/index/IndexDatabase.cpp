// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <iostream>
#include "../sql/wrapper/Configure.h"
#include "../sql/wrapper/Connection.h"
#include "../sql/wrapper/Memory.h"
#include "../common/Utils.h"
#include "IndexDatabase.h"

namespace ark {
namespace lsp {
namespace sql {

#define EXPAND(...) #__VA_ARGS__
#define SQL(...) EXPAND(__VA_ARGS__),

// SQL statements to prepare index database.
constexpr std::string_view PrepareDatabase[]{
#include "../sql/index/PrepareDB.sql"
};

// SQL statements to delete index database.
constexpr std::string_view DeleteDatabase[]{
#include "../sql/index/DeleteDB.sql"
};

// SQL statements to create index database.
constexpr std::string_view CreateDatabase[]{
#include "../sql/index/CreateDB.sql"
};

#undef SQL

// SQL statements to upgrade database
constexpr std::pair<int32_t, std::string_view> UpgradeDB[]{
#define SQL(Version, ...) {Version, EXPAND(__VA_ARGS__)},
#include "../sql/index/UpgradeDB.sql"
#undef SQL
};

// DML and DQL statements for index database.
#define SQL(Name, ...) constexpr std::string_view Name{EXPAND(__VA_ARGS__)}
#include "../sql/index/Statements.sql"
#undef SQL

#undef EXPAND
} // namespace sql

namespace {

constexpr int PACK_INDEX = 1;
constexpr int MODU_INDEX = 2;
constexpr int DIGEST_INDEX = 3;

auto Line(Position &pos)
{
    return [&](uint32_t line) { pos.line = line; };
}

auto Column(Position &pos)
{
    return [&](uint32_t column) { pos.column = column; };
}

auto FileID(Position &pos, unsigned int fileid)
{
    return [&](uint32_t fileid) { pos.fileID = fileid; };
}

uint64_t GetIDFromArray(IDArray info)
{
    uint64_t result = 0;
    uint64_t bit = 8;
    for (unsigned I = 0; I < info.size(); ++I) {
        result <<= bit;
        result |= (uint16_t)info[info.size() - I - 1];
    }
    return result;
}

void PopulateSymbol(const sqldb::Result &row, Symbol &sym)
{
    IDArray idArray;
    row.store(
        idArray, sym.kind, sym.symInfo.subKind, sym.symInfo.lang,
        sym.symInfo.properties, sym.name, sym.scope, sym.location.fileUri,
        Line(sym.location.begin), Column(sym.location.begin),
        Line(sym.location.end), Column(sym.location.end),
        sym.declaration.fileUri, Line(sym.declaration.begin),
        Column(sym.declaration.begin), Line(sym.declaration.end),
        Column(sym.declaration.end), sym.signature,
        sym.templateSpecializationArgs, sym.completionSnippetSuffix, sym.documentation,
        sym.returnType, sym.type, sym.flags, sym.isCjoSym, sym.isMemberParam, sym.modifier, sym.isDeprecated,
        sym.syscap, sym.pkgModifier, sym.curModule, sym.curMacroCall.fileUri, sym.curMacroCall.begin.line,
        sym.curMacroCall.begin.column, sym.curMacroCall.end.line, sym.curMacroCall.end.column);
    sym.id = GetIDFromArray(idArray);
}
// LCOV_EXCL_START
void PopulateSymbolAndCompletions(const sqldb::Result &row, Symbol &sym, CompletionItem &completion)
{
    IDArray idArray;
    row.store(
        // symbol
        idArray, sym.name, sym.modifier, sym.kind, sym.location.fileUri, sym.scope,
        sym.isCjoSym, sym.isDeprecated, sym.syscap, sym.pkgModifier,
        // completion
        idArray, completion.label, completion.insertText);
    sym.id = GetIDFromArray(idArray);
}

void PopulateCompletion(const sqldb::Result &row, CompletionItem &completion)
{
    IDArray idArray;
    row.store(idArray, completion.label, completion.insertText);
}

void PopulateExtendItemAndCompletions(const sqldb::Result &row, ExtendItem &extendItem, Symbol &sym,
    CompletionItem &completionItem, std::string &pkgName)
{
    IDArray extendId;
    IDArray idArray;
    row.store(
        // extendItem
        extendId, extendItem.id, extendItem.modifier, extendItem.interfaceName, pkgName,
        // symbol
        idArray, sym.name, sym.modifier, sym.kind, sym.curModule, sym.isCjoSym, sym.isDeprecated, sym.syscap,
        sym.signature,
        // completionItem
        idArray, completionItem.label, completionItem.insertText);
    sym.id = GetIDFromArray(idArray);
}
// LCOV_EXCL_STOP
void PopulateRef(const sqldb::Result &row, Ref &ref, SymbolID &symId)
{
    IDArray idArray;
    IDArray containerIdArray;
    row.store(
        idArray, ref.location.fileUri, Line(ref.location.begin),
        Column(ref.location.begin), Line(ref.location.end),
        Column(ref.location.end), ref.kind, containerIdArray, ref.isCjoRef, ref.isSuper);
    symId = GetIDFromArray(idArray);
    ref.container = GetIDFromArray(containerIdArray);
}

void PopulateRelation(const sqldb::Result &row, Relation &rel)
{
    IDArray subjectIdArray;
    IDArray objectIdArray;
    row.store(subjectIdArray, rel.predicate, objectIdArray);
    rel.subject = GetIDFromArray(subjectIdArray);
    rel.object = GetIDFromArray(objectIdArray);
}
// LCOV_EXCL_START
dberr_no PopulateSymbolWithRank(const sqldb::Result &row, Symbol &sym)
{
    IDArray idArray;
    row.store(
        idArray, sym.kind, sym.symInfo.subKind, sym.symInfo.lang,
        sym.symInfo.properties, sym.name, sym.scope, sym.location.fileUri,
        Line(sym.location.begin), Column(sym.location.begin),
        Line(sym.location.end), Column(sym.location.end),
        sym.declaration.fileUri, Line(sym.declaration.begin),
        Column(sym.declaration.begin), Line(sym.declaration.end),
        Column(sym.declaration.end), sym.signature,
        sym.templateSpecializationArgs, sym.completionSnippetSuffix, sym.documentation,
        sym.returnType, sym.type, sym.flags, sym.isCjoSym, sym.isMemberParam, sym.modifier, sym.isDeprecated,
        sym.syscap, sym.pkgModifier, sym.curModule, sym.curMacroCall.fileUri, sym.curMacroCall.begin.line,
        sym.curMacroCall.begin.column, sym.curMacroCall.end.line, sym.curMacroCall.end.column, sym.rank);
        sym.id = GetIDFromArray(idArray);
    return true;
}
// LCOV_EXCL_STOP
void PopulateComment(const sqldb::Result &row, Comment &comment)
{
    IDArray idArray;
    CompletionItem completionItem;
    row.store(idArray, comment.style, comment.kind, comment.commentStr);
}

void PopulateCrossSymbol(const sqldb::Result &row, CrossSymbol &crs)
{
    IDArray idArray;
    IDArray containerIdArray;
    std::string pkgName;

    row.store(pkgName, idArray, crs.name, containerIdArray, crs.containerName,
        crs.crossType, crs.location.fileUri,
        Line(crs.location.begin),
        Column(crs.location.begin),
        Line(crs.location.end),
        Column(crs.location.end),
        Line(crs.declaration.begin),
        Column(crs.declaration.begin),
        Line(crs.declaration.end),
        Column(crs.declaration.end));
    crs.id = GetIDFromArray(idArray);
    crs.container = GetIDFromArray(containerIdArray);
}

std::once_flag g_configureFlag;

dberr_no ConfigureSQLite()
{
    sqldb::setSerializedMode();
    return 0;
}

bool BusyHandlerCallback(int /* TryCount */)
{
    int sleepTime = 100;
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    return !ShutdownRequested();
}

dberr_no PrepareConnection(sqldb::Connection &sqlConnect)
{
    sqlConnect.progress(ShutdownRequested);
    sqlConnect.busy(BusyHandlerCallback);
#ifndef NO_EXCEPTIONS
    try {
#endif
        sqlConnect.tokenizer("identifier", GetIdentifierTokenizer());
#ifndef NO_EXCEPTIONS
    } catch (...) {
        std::cerr << " prepareConnection tokenizer fail\n";
        return 1;
    }
#endif
    for (std::string_view sql : sql::PrepareDatabase) {
#ifndef NO_EXCEPTIONS
        try {
#endif
            sqlConnect.execute(sql);
#ifndef NO_EXCEPTIONS
        } catch (...) {
            std::cerr << " prepareConnection execute fail\n";
            return 1;
        }
#endif
    }
    return 0;
}
// LCOV_EXCL_START
int32_t GetUserVersion(sqldb::Connection &sqlConnect)
{
    sqldb::Statement pragmaUserVersion = sqlConnect.prepare(sql::PragmaUserVersion);
    int32_t userVersion;
    pragmaUserVersion.execute(sqldb::into(userVersion));
    return userVersion;
}

std::string AddPercentAfterEachUTF8Char(const std::string& input)
{
    std::string result;
    for (size_t i = 0; i < input.size();) {
        unsigned char c = input[i];
        size_t charLen = 1;

        if ((c & 0x80) == 0x00) {
            // 1-byte (ASCII)
            charLen = 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte
            charLen = 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3-byte
            charLen = 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4-byte
            charLen = 4;
        } else {
            // invalid token
            ++i;
            continue;
        }

        result.append(input.substr(i, charLen));
        result += '%';
        i += charLen;
    }
    return result;
}

} // namespace

sqldb::Statement &IndexDatabase::DatabaseConnection::Use(std::string_view sql, bool useCacheStmt)
{
    auto it = statementLookup.try_emplace(sql.data()).first;
    if (useCacheStmt) {
        for (sqldb::Statement *Statement : it->second) {
            if (!Statement->isBusy()) {
                return *Statement;
            }
        }
    }
    statementCache.emplace_front();
    sqldb::Statement &statement = statementCache.front();
    statement = connection.prepare(sql);
    it->second.insert(it->second.begin(), &statement);
    return statement;
}

IndexDatabase::IndexDatabase() {}

void ConnectionTransaction(sqldb::Connection &sqlConnect, std::function<void()> callback)
{
    sqlConnect.execute("BEGIN");
#ifndef NO_EXCEPTIONS
    try {
#endif
        callback();
#ifndef NO_EXCEPTIONS
    } catch (...) {
        sqlConnect.execute("ROLLBACK");
    }
#endif
    sqlConnect.execute("COMMIT");
}

// add catch for db execute error
dberr_no IndexDatabase::Initialize(std::function<sqldb::Connection()> openConnection)
{
    std::call_once(g_configureFlag, [] {
        ConfigureSQLite();
    });
    sqldb::Connection sqlConnect = std::move(openConnection());
    if (dberr_no Err = PrepareConnection(sqlConnect)) {
        std::cerr << "---------open db fail\n";
        return 1;
    }
    sqldb::Statement selectSchemaEmpty = std::move(sqlConnect.prepare(sql::SelectSchemaEmpty));
    bool schemaEmpty;
    selectSchemaEmpty.execute(sqldb::into(schemaEmpty));
    if (schemaEmpty) {
        for (std::string_view SQL : sql::CreateDatabase) {
#ifndef NO_EXCEPTIONS
            try {
#endif
                sqlConnect.execute(SQL);
#ifndef NO_EXCEPTIONS
            } catch (...) {
                std::cerr << "---------CreateDatabase  db fail\n";
                return 1;
            }
#endif
        }
        databaseCache.Get([&sqlConnect] { return std::move(sqlConnect); });
    } else {
        sqldb::Statement PragmaApplicationID = std::move(sqlConnect.prepare(sql::PragmaApplicationID));
        int32_t ApplicationID;
#ifndef NO_EXCEPTIONS
        try {
#endif
            PragmaApplicationID.execute(sqldb::into(ApplicationID));
#ifndef NO_EXCEPTIONS
        } catch (...) {
            std::cerr << "---------PragmaApplicationID  db fail\n";
            return 1;
        }
#endif
        if (ApplicationID != DATABASE_MAGIC) {
            return 1;
        }
        int32_t UserVersion = std::move(GetUserVersion(sqlConnect));
        if (UserVersion != DATABASE_VERSION) {
            if (sqlConnect.isReadOnly()) {
                std::cerr << "---- DB read only\n";
                return 1;
            }
            upgradeFuture = std::async(
                std::launch::async,
                [this](sqldb::Connection Connection, int32_t UserVersion) {
                  std::lock_guard<std::mutex> Lock(updateMutex);
                  const auto *It = std::find_if(
                      std::begin(sql::UpgradeDB), std::end(sql::UpgradeDB),
                      [UserVersion](std::pair<std::int32_t, std::string_view> Pair) {
                        return UserVersion == Pair.first;
                      });
                  for (; It != std::end(sql::UpgradeDB); ++It) {
                      Connection.execute(It->second);
                      UserVersion = GetUserVersion(Connection);
                  }
                  if (UserVersion != DATABASE_VERSION) {
                      ConnectionTransaction(Connection, [&Connection] {
                            for (std::string_view SQL : sql::DeleteDatabase) {
                                Connection.execute(SQL);
                            }
                            for (std::string_view SQL : sql::CreateDatabase) {
                                Connection.execute(SQL);
                            }
                            return 0;
                          });
                  }
                },
                std::move(sqlConnect), UserVersion);
        }
    }
    openDatabase = [this, openConnect = std::move(openConnection)] {
        if (upgradeFuture.valid()) {
            upgradeFuture.wait();
        }
        sqldb::Connection connect = std::move(openConnect());
        auto failed = PrepareConnection(connect);
        if (failed) {
            std::cerr << " -- prepareConnection fail for OpenDatabase\n";
        }
        return DatabaseConnection(std::move(connect));
    };
    return 0;
}

dberr_no IndexDatabase::FileExists(std::string fileURI, bool &exists)
{
    Use(sql::SelectFileExists)
        .execute(sqldb::with(fileURI), sqldb::into(exists));
    return 0;
}
// LCOV_EXCL_STOP
dberr_no IndexDatabase::GetFileDigest(uint32_t fileId, std::string &digest)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectFileDigestWithId)
            .execute(sqldb::with(fileId), sqldb::into(digest));
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        std::cerr << "GetFileDigestWithID fail due to " << ex.what() << "\n";
    }
#endif
    return 0;
}
// LCOV_EXCL_START
dberr_no IndexDatabase::GetSymbols(std::function<bool(const Symbol &sym)> callback)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectSymbols).execute([&](sqldb::Result Row) {
            Symbol resSym;
            PopulateSymbol(Row, resSym);
            callback(resSym);
            return true;
        });
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        Trace::Log("getsymbols fail due to: ", ex.what());
        Trace::Log(ex.what());
    }
#endif
    return 0;
}

dberr_no IndexDatabase::GetSymbolsByName(const std::string &name, std::function<bool(const Symbol &sym)> callback)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectSymbolByName).execute(sqldb::with(name, SymbolLanguage::CANGJIE), [&](sqldb::Result Row) {
            Symbol resSym;
            PopulateSymbol(Row, resSym);
            callback(resSym);
            return true;
        });
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        Trace::Log("getsymbols fail due to: ", ex.what());
        Trace::Log(ex.what());
    }
#endif
    return 0;
}

dberr_no IndexDatabase::GetSourceFileRelations(
    std::string fileURI,
    std::function<bool(std::string)> callback)
{
    return 0;
}

dberr_no IndexDatabase::GetHeaderFileRelations(
    std::string fileURI,
    std::function<bool(std::string)> callback)
{
    return 0;
}
// LCOV_EXCL_STOP
dberr_no IndexDatabase::GetSymbolByID(IDArray id, std::function<bool(const Symbol &sym)> callback)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectSymbol).execute(sqldb::with(id), [&](sqldb::Result Row) {
            Symbol resSym;
            PopulateSymbol(Row, resSym);
            callback(resSym);
            return true;
        });
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        std::cerr << "getsymbol fail due to " << ex.what() << "\n";
    }
#endif
    return true;
}
// LCOV_EXCL_START
dberr_no IndexDatabase::GetCrossSymbolByID(IDArray id, std::function<void(const CrossSymbol &sym)> callback)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectCrossSymbolByID).execute(sqldb::with(id), [&](sqldb::Result Row) {
            CrossSymbol crs;
            PopulateCrossSymbol(Row, crs);
            callback(crs);
            return true;
        });
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        std::cerr << "GetCrossSymbolByID fail due to " << ex.what() << "\n";
    }
#endif
    return true;
}

dberr_no IndexDatabase::GetPkgSymbols(std::string pkgName, std::function<bool(const Symbol &sym)> callback)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        std::string scopePrefix = pkgName + ":";
        Use(sql::SelectSymbolsByPkgName).execute(sqldb::with(pkgName, scopePrefix),
            [&](sqldb::Result Row) {
            Symbol resSym;
            PopulateSymbol(Row, resSym);
            callback(resSym);
            return true;
        });
#ifndef NO_EXCEPTIONS
    } catch(std::exception &ex) {
        std::cerr << "getsymbol fail due to " << ex.what() << "\n";
    }
#endif
    return true;
}

dberr_no IndexDatabase::GetSymbolsAndCompletions(const std::string &prefix,
    std::function<void(const Symbol &sym, const CompletionItem &completion)> callback)
{
    std::string fuzzyPrefix = AddPercentAfterEachUTF8Char(prefix);
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectCompletions).execute(sqldb::with(fuzzyPrefix), [&](sqldb::Result Row) {
            Symbol resSym;
            CompletionItem resCompletion;
            PopulateSymbolAndCompletions(Row, resSym, resCompletion);
            callback(resSym, resCompletion);
            return true;
        });
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        Trace::Log("get symbol and completions failed", ex.what());
    }
#endif
    return true;
}

dberr_no IndexDatabase::GetCompletion(std::string symPackage, const Symbol &sym,
    std::function<void(const std::string &pkgName, const Symbol &sym, const CompletionItem &completion)> callback)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectCompletion).execute(sqldb::with(GetArrayFromID(sym.id)), [&](sqldb::Result Row) {
            CompletionItem resCompletion;
            PopulateCompletion(Row, resCompletion);
            callback(symPackage, sym, resCompletion);
            return true;
        });
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        std::cerr << "getsymbol fail due to " << ex.what() << "\n";
    }
#endif
    return true;
}

dberr_no IndexDatabase::GetExtendItem(IDArray id,
    std::function<void(const std::string &, const Symbol &, const ExtendItem &, const CompletionItem &)> callback)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectExtends).execute(sqldb::with(id), [&](sqldb::Result Row) {
            Symbol resSym;
            ExtendItem extendItem;
            CompletionItem completionItem;
            std::string packageName;
            PopulateExtendItemAndCompletions(Row, extendItem, resSym, completionItem, packageName);
            callback(packageName, resSym, extendItem, completionItem);
            return true;
        });
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        Trace::Log("get extem item failed: ", ex.what());
    }
#endif
    return true;
}
// LCOV_EXCL_STOP
dberr_no IndexDatabase::GetComment(IDArray id, std::function<bool(const Comment &comment)> callback)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectComment).execute(sqldb::with(id), [&](sqldb::Result Row) {
            Comment comment;
            PopulateComment(Row, comment);
            callback(comment);
            return true;
        });
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        std::cerr << "getsymbol fail due to " << ex.what() << "\n";
    }
#endif
    return true;
}
// LCOV_EXCL_START
dberr_no IndexDatabase::GetFileWithID(int fileId,
                                      std::function<bool(std::string, std::string)> callback)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectFileWithId).execute(sqldb::with(fileId), [&](sqldb::Result Row) {
            std::string fileName;
            std::string packageName;
            std::string moduleName;
            Row.store(std::ignore, fileName, std::ignore, std::ignore, packageName, moduleName);
            callback(packageName, moduleName);
            return true;
        });
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        std::cerr << "GetFileWithID fail due to " << ex.what() << "\n";
    }
#endif
    return true;
}

dberr_no IndexDatabase::GetFileWithUri(std::string fileUri,
                                       std::function<bool(std::string, std::string)> callback)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectFileWithUri).execute(sqldb::with(fileUri), [&](sqldb::Result Row) {
            std::string fileName;
            std::string packageName;
            std::string moduleName;
            Row.store(std::ignore, fileName, std::ignore, std::ignore, packageName, moduleName);
            callback(packageName, moduleName);
            return true;
        });
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        std::cerr << "GetFileWithUri fail due to " << ex.what() << "\n";
    }
#endif
    return true;
}

dberr_no IndexDatabase::GetSymbol(std::string filePath, size_t line, size_t col,
                                  std::function<bool(const Symbol &sym)> callback)
{
    return true;
}

dberr_no IndexDatabase::GetMatchingSymbols(
    std::string query, std::function<bool(const Symbol &sym)> callback,
    std::optional<std::string> scope,
    std::optional<Symbol::SymbolFlag> flags)
{
    std::stringstream patternStream;
    auto Tokenize = GetIdentifierTokenizer();
    Tokenize(query, [&patternStream](std::string_view token, size_t) {
        patternStream << '"' << token << '"' << '*' << ' ';
    });
    std::string pattern = patternStream.str();
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use (sql::SelectMatchingSymbols).execute(sqldb::with(pattern, flags, scope), [&](sqldb::Result row) {
            Symbol resSym;
            if (PopulateSymbolWithRank(row, resSym)) {
                return true;
            }
            if (PopulateReferenceCount(resSym)) {
                return true;
            }
            return callback(resSym) ? RESULT_NEXT : RESULT_DONE;
        });
#ifndef NO_EXCEPTIONS
    } catch (std::exception &ex) {
        std::cerr << "GetMatchingSymbols fail due to " << ex.what() << "\n";
        return true;
    }
#endif
    return true;
}
// LCOV_EXCL_STOP
dberr_no IndexDatabase::GetReferences(const SymbolID &id, RefKind kind,
                                      std::function<bool(const Ref &ref)> callback)
{
    IDArray idArray = GetArrayFromID(id);
    Use(sql::SelectReference)
        .execute(sqldb::with(idArray, kind), [&](sqldb::Result row) {
            Ref ref;
            SymbolID symId;
            PopulateRef(row, ref, symId);
            callback(ref);
            return true;
        });
    return true;
}
// LCOV_EXCL_START
dberr_no IndexDatabase::GetFileReferences(const std::string &fileUri, RefKind kind,
    std::function<bool(const Ref &ref, const SymbolID symId)> callback) 
{
    Use(sql::SelectFileReference)
        .execute(sqldb::with(fileUri, kind), [&](sqldb::Result row) {
            Ref ref;
            SymbolID symId;
            PopulateRef(row, ref, symId);
            callback(ref, symId);
            return true;
        });
    return true;
}
// LCOV_EXCL_STOP
dberr_no IndexDatabase::GetReferred(const SymbolID &id,
                                    std::function<void(const SymbolID &, const Ref &)> callback)
{
    IDArray idArray = GetArrayFromID(id);
    Use(sql::SelectReferred)
        .execute(sqldb::with(idArray), [&](sqldb::Result row) {
            Ref ref;
            SymbolID symId;
            PopulateRef(row, ref, symId);
            callback(symId, ref);
            return true;
        });
    return true;
}
// LCOV_EXCL_START
dberr_no IndexDatabase::GetUsedSymbols(std::string fileURI,
                                       std::set<SymbolID> &symbols)
{
    return true;
}

dberr_no IndexDatabase::GetReferenceAt(std::string fileURI, RefKind kind,
                                       Position pos, SymbolID &id)
{
    return true;
}

dberr_no IndexDatabase::GetRelations(
    const SymbolID &subjectID, RelationKind kind,
    std::function<bool(const Relation &rel)> callback)
{
    IDArray idArray = GetArrayFromID(subjectID);
    if (kind == RelationKind::OVERRIDES) {
        Use(sql::SelectSubjectFromRelations)
            .execute(sqldb::with(idArray, RelationKind::RIDDEND_BY),
                     [&](sqldb::Result Row) {
                         IDArray objArray;
                         Row.store(objArray);
                         SymbolID objectID = GetIDFromArray(objArray);
                         callback({subjectID, kind, objectID});
                         return true;
                     });
        return true;
    }
    Use(sql::SelectRelation)
        .execute(sqldb::with(idArray, kind), [&](sqldb::Result Row) {
            Relation rel;
            PopulateRelation(Row, rel);
            callback(rel);
            return true;
        });
    Use(sql::SelectReverseRelation)
        .execute(sqldb::with(idArray, kind), [&](sqldb::Result Row) {
            Relation rel;
            PopulateRelation(Row, rel);
            callback(rel);
            return true;
        });
    return true;
}

dberr_no IndexDatabase::GetCallRelations(
    const SymbolID &subjectID, CallRelationKind kind,
    std::function<bool(const CallRelation &rel)> callback)
{
    return true;
}
// LCOV_EXCL_STOP
dberr_no IndexDatabase::GetRelationsRiddenUp(
    const SymbolID &objectID, RelationKind kind,
    std::function<bool(const Relation &rel)> callback)
{
    IDArray objArray = GetArrayFromID(objectID);
    Use(sql::SelectReverseRelation)
        .execute(sqldb::with(objArray, kind), [&](sqldb::Result row) {
            Relation rel;
            PopulateRelation(row, rel);
            callback(rel);
            return true;
        });
    return true;
}

dberr_no IndexDatabase::GetRelationsRiddenDown(
    const SymbolID &subjectID, RelationKind kind,
    std::function<bool(const Relation &rel)> callback)
{
    IDArray subArray = GetArrayFromID(subjectID);
    Use(sql::SelectRelation)
        .execute(sqldb::with(subArray, kind), [&](sqldb::Result row) {
            Relation rel;
            PopulateRelation(row, rel);
            callback(rel);
            return true;
        });
    return true;
}

dberr_no IndexDatabase::GetCrossSymbols(
    const std::string &pkgName, const std::string &symName, const std::function<void(const CrossSymbol &)> &callback)
{
    Use(sql::SelectCrossSymbol)
        .execute(sqldb::with(pkgName, symName), [&](sqldb::Result row) {
            CrossSymbol crs;
            PopulateCrossSymbol(row, crs);
            callback(crs);
            return true;
        });
    return true;
}

dberr_no IndexDatabase::DBUpdate::InsertFileWithId(int fileID, std::vector<std::string> &fileInfo)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        db.Use(sql::InsertFileWithID)
            .execute(sqldb::with(fileID, fileInfo[0], fileInfo[DIGEST_INDEX],
                                 fileInfo[PACK_INDEX], fileInfo[MODU_INDEX]));
#ifndef NO_EXCEPTIONS
    }
    catch (std::exception &ex) {
        std::cerr << "exception in InsertFileWithID: " << ex.what() << "\n";
    }
#endif
    return true;
}

dberr_no IndexDatabase::DBUpdate::InsertFile(std::string fileURI, FileDigest digest)
{
    return true;
}

dberr_no IndexDatabase::DBUpdate::DeleteFile(const std::string &fileURI)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        db.Use(sql::DeleteFile).execute(sqldb::with(fileURI));
#ifndef NO_EXCEPTIONS
    }
    catch (std::exception &ex) {
        std::cerr << "exception in DeleteFile: " << ex.what() << "\n";
    }
#endif
    return true;
}
// LCOV_EXCL_START
dberr_no IndexDatabase::DBUpdate::UpdateFilesNotEnumerated()
{
    return true;
}

dberr_no IndexDatabase::DBUpdate::UpdateFileEnumerated(std::string fileURI)
{
    return true;
}

dberr_no IndexDatabase::DBUpdate::RenameFile(
    std::pair<std::string, std::string> oldNewURI)
{
    return true;
}
// LCOV_EXCL_STOP
dberr_no IndexDatabase::DBUpdate::InsertSymbol(const Symbol &sym)
{
    if (sym.id == 0) {
        return true;
    }
    auto InsertSymbol = [this](const Symbol &insertSym) -> dberr_no {
#ifndef NO_EXCEPTIONS
        try {
#endif
            db.Use(sql::InsertSymbol)
                .execute(sqldb::with(
                    (GetArrayFromID(insertSym.id)), insertSym.kind, insertSym.symInfo.subKind, insertSym.symInfo.lang,
                    insertSym.symInfo.properties, insertSym.name, insertSym.scope, insertSym.location.fileUri,
                    insertSym.location.begin.line, insertSym.location.begin.column,
                    insertSym.location.end.line, insertSym.location.end.column,
                    insertSym.declaration.fileUri,
                    insertSym.declaration.begin.line,
                    insertSym.declaration.begin.column,
                    insertSym.declaration.end.line,
                    insertSym.declaration.end.column, insertSym.signature,
                    insertSym.templateSpecializationArgs, insertSym.completionSnippetSuffix,
                    insertSym.documentation, insertSym.returnType, insertSym.type, insertSym.flags,
                    insertSym.isCjoSym, insertSym.isMemberParam, insertSym.modifier, insertSym.isDeprecated,
                    insertSym.syscap, insertSym.pkgModifier, insertSym.curModule, insertSym.curMacroCall.fileUri,
                    insertSym.curMacroCall.begin.line, insertSym.curMacroCall.begin.column,
                    insertSym.curMacroCall.end.line, insertSym.curMacroCall.end.column));
#ifndef NO_EXCEPTIONS
        } catch (const std::exception &e) {
            Trace::Log("err in insert symbol: ", e.what());
            return 1;
        }
#endif
        return true;
    };
    InsertSymbol(sym);
    return true;
}

void IndexDatabase::DBUpdate::DealSymbols(const std::vector<Symbol> &syms)
{
    size_t maxMultiInsertIndex = 0;
    if (syms.size() >= MUTI_INSERT_MAX_SIZE) {
        maxMultiInsertIndex = syms.size() - (syms.size() % MUTI_INSERT_MAX_SIZE);
        std::ostringstream oss;
        oss << sql::MultiInsertSymbolsHead;
        bool isNeedCommon = false;
        for (size_t i = 0; i < MUTI_INSERT_MAX_SIZE; i++) {
            if (isNeedCommon) {
                oss << ",";
            }
            oss << sql::MultiInsertSymbolsValue;
            isNeedCommon = true;
        }
        int Index = 0;
        sqldb::Statement &stmt = db.Use(oss.str(), false);
        for (size_t i = 0; i < maxMultiInsertIndex + 1; i++) {
            if (i >= MUTI_INSERT_MAX_SIZE && i % MUTI_INSERT_MAX_SIZE == 0) {
                Index = 0;
                stmt.execute();
            }
            const auto &insertSym = syms[i];
            const auto &bind = sqldb::with((insertSym.idArray), insertSym.kind, insertSym.symInfo.subKind,
                insertSym.symInfo.lang, insertSym.symInfo.properties, insertSym.name, insertSym.scope,
                insertSym.location.fileUri, insertSym.location.begin.line, insertSym.location.begin.column,
                insertSym.location.end.line, insertSym.location.end.column, insertSym.declaration.fileUri,
                insertSym.declaration.begin.line, insertSym.declaration.begin.column,
                insertSym.declaration.end.line, insertSym.declaration.end.column, insertSym.signature,
                insertSym.templateSpecializationArgs, insertSym.completionSnippetSuffix, insertSym.documentation,
                insertSym.returnType, insertSym.type, insertSym.flags, insertSym.isCjoSym, insertSym.isMemberParam,
                insertSym.modifier, insertSym.isDeprecated, insertSym.syscap, insertSym.pkgModifier,
                insertSym.curModule, insertSym.curMacroCall.fileUri, insertSym.curMacroCall.begin.line,
                insertSym.curMacroCall.begin.column, insertSym.curMacroCall.end.line,
                insertSym.curMacroCall.end.column);
            BindValue(bind, stmt.GetStmt(), Index);
        }
    }

    // deal remaining
    std::ostringstream oss;
    oss << sql::MultiInsertSymbolsHead;
    bool isNeedCommon = false;
    for (size_t i = 0; i < syms.size() - maxMultiInsertIndex; i++) {
        if (isNeedCommon) {
            oss << ",";
        }
        oss << sql::MultiInsertSymbolsValue;
        isNeedCommon = true;
    }
    sqldb::Statement &stmt = db.Use(oss.str(), false);
    int Index = 0;
    for (size_t j = maxMultiInsertIndex; j < syms.size(); j++) {
        const auto &insertSym = syms[j];
        const auto &bind = sqldb::with((insertSym.idArray), insertSym.kind, insertSym.symInfo.subKind,
            insertSym.symInfo.lang, insertSym.symInfo.properties, insertSym.name, insertSym.scope,
            insertSym.location.fileUri, insertSym.location.begin.line, insertSym.location.begin.column,
            insertSym.location.end.line, insertSym.location.end.column, insertSym.declaration.fileUri,
            insertSym.declaration.begin.line, insertSym.declaration.begin.column, insertSym.declaration.end.line,
            insertSym.declaration.end.column, insertSym.signature, insertSym.templateSpecializationArgs,
            insertSym.completionSnippetSuffix, insertSym.documentation, insertSym.returnType, insertSym.type,
            insertSym.flags, insertSym.isCjoSym, insertSym.isMemberParam, insertSym.modifier, insertSym.isDeprecated,
            insertSym.syscap, insertSym.pkgModifier, insertSym.curModule, insertSym.curMacroCall.fileUri,
            insertSym.curMacroCall.begin.line, insertSym.curMacroCall.begin.column, insertSym.curMacroCall.end.line,
            insertSym.curMacroCall.end.column);
        BindValue(bind, stmt.GetStmt(), Index);
    }
    stmt.execute();
}

dberr_no IndexDatabase::DBUpdate::InsertSymbols(const std::vector<Symbol> &syms)
{
    if (syms.empty()) {
        return true;
    }
#ifndef NO_EXCEPTIONS
    try {
#endif
        DealSymbols(syms);
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert symbol: ", e.what());
    }
#endif
    return true;
}

dberr_no IndexDatabase::DBUpdate::InsertCompletion(const Symbol &sym,  const CompletionItem &completionItem)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        db.Use(sql::InsertCompletion)
                        .execute(sqldb::with(
                            GetArrayFromID(sym.id), completionItem.label, completionItem.insertText));
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert completion: ", e.what());
    }
#endif
    return true;
}

void IndexDatabase::DBUpdate::DealCompletions(const std::vector<std::pair<IDArray, CompletionItem>> &completions)
{
    size_t maxMultiInsertIndex = 0;
    if (completions.size() >= MUTI_INSERT_MAX_SIZE) {
        maxMultiInsertIndex = completions.size() - (completions.size() % MUTI_INSERT_MAX_SIZE);
        std::ostringstream oss;
        oss << sql::MultiInsertCompletionsHead;
        bool isNeedCommon = false;
        for (size_t i = 0; i < MUTI_INSERT_MAX_SIZE; i++) {
            if (isNeedCommon) {
                oss << ",";
            }
            oss << sql::MultiInsertCompletionsValue;
            isNeedCommon = true;
        }
        int Index = 0;
        sqldb::Statement &stmt = db.Use(oss.str(), false);
        for (size_t i = 0; i < maxMultiInsertIndex + 1; i++) {
            if (i >= MUTI_INSERT_MAX_SIZE && i % MUTI_INSERT_MAX_SIZE == 0) {
                Index = 0;
                stmt.execute();
            }
            const auto &array = completions[i].first;
            const auto &insertCompletion = completions[i].second;
            const auto &bind = sqldb::with(array, insertCompletion.label, insertCompletion.insertText);
            BindValue(bind, stmt.GetStmt(), Index);
        }
    }

    // deal remaining
    std::ostringstream oss;
    oss << sql::MultiInsertCompletionsHead;
    bool isNeedCommon = false;
    for (size_t i = 0; i < completions.size() - maxMultiInsertIndex; i++) {
        if (isNeedCommon) {
            oss << ",";
        }
        oss << sql::MultiInsertCompletionsValue;
        isNeedCommon = true;
    }
    sqldb::Statement &stmt = db.Use(oss.str(), false);
    int Index = 0;
    for (size_t j = maxMultiInsertIndex; j < completions.size(); j++) {
        const auto &array = completions[j].first;
        const auto &insertCompletion = completions[j].second;
        const auto &bind = sqldb::with(array, insertCompletion.label, insertCompletion.insertText);
        BindValue(bind, stmt.GetStmt(), Index);
    }
    stmt.execute();
}

dberr_no IndexDatabase::DBUpdate::InsertCompletions(const std::vector<std::pair<IDArray, CompletionItem>> &completions)
{
    if (completions.empty()) {
        return true;
    }
#ifndef NO_EXCEPTIONS
    try {
#endif
        DealCompletions(completions);
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert completion: ", e.what());
    }
#endif
    return true;
}

dberr_no IndexDatabase::DBUpdate::InsertComment(const Symbol &sym, const AST::Comment &comment)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        db.Use(sql::InsertComment)
                        .execute(sqldb::with(
                            GetArrayFromID(sym.id), comment.style, comment.kind, comment.info.Value()));
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert comment: ", e.what());
    }
#endif
    return true;
}
// LCOV_EXCL_START
void IndexDatabase::DBUpdate::DealComments(const std::vector<std::pair<IDArray, Comment>> &comments)
{
    size_t maxMultiInsertIndex = 0;
    if (comments.size() >= MUTI_INSERT_MAX_SIZE) {
        maxMultiInsertIndex = comments.size() - (comments.size() % MUTI_INSERT_MAX_SIZE);
        std::ostringstream oss;
        oss << sql::MultiInsertCommentsHead;
        bool isNeedCommon = false;
        for (size_t i = 0; i < MUTI_INSERT_MAX_SIZE; i++) {
            if (isNeedCommon) {
                oss << ",";
            }
            oss << sql::MultiInsertCommentsValue;
            isNeedCommon = true;
        }
        int Index = 0;
        sqldb::Statement &stmt = db.Use(oss.str(), false);
        for (size_t i = 0; i < maxMultiInsertIndex + 1; i++) {
            if (i >= MUTI_INSERT_MAX_SIZE && i % MUTI_INSERT_MAX_SIZE == 0) {
                Index = 0;
                stmt.execute();
            }
            const auto &array = comments[i].first;
            const auto &insertComment = comments[i].second;
            const auto &bind =
                sqldb::with(array, insertComment.style, insertComment.kind, insertComment.commentStr);
            BindValue(bind, stmt.GetStmt(), Index);
        }
    }

    // deal remaining
    std::ostringstream oss;
    oss << sql::MultiInsertCommentsHead;
    bool isNeedCommon = false;
    for (size_t i = 0; i < comments.size() - maxMultiInsertIndex; i++) {
        if (isNeedCommon) {
            oss << ",";
        }
        oss << sql::MultiInsertCommentsValue;
        isNeedCommon = true;
    }
    sqldb::Statement &stmt = db.Use(oss.str(), false);
    int Index = 0;
    for (size_t j = maxMultiInsertIndex; j < comments.size(); j++) {
        const auto &array = comments[j].first;
        const auto &insertComment = comments[j].second;
        const auto &bind =
            sqldb::with(array, insertComment.style, insertComment.kind, insertComment.commentStr);
        BindValue(bind, stmt.GetStmt(), Index);
    }
    stmt.execute();
}
// LCOV_EXCL_STOP
dberr_no IndexDatabase::DBUpdate::InsertComments(const std::vector<std::pair<IDArray, Comment>> &comments)
{
    if (comments.empty()) {
        return true;
    }
#ifndef NO_EXCEPTIONS
    try {
#endif
        DealComments(comments);
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert comment: ", e.what());
    }
#endif
    return true;
}

dberr_no IndexDatabase::DBUpdate::InsertReference(const IDArray &id, const Ref &ref)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        db.Use(sql::InsertReference)
            .execute(sqldb::with(id, ref.location.fileUri, ref.location.begin.line,
                                 ref.location.begin.column, ref.location.end.line,
                                 ref.location.end.column, ref.kind, GetArrayFromID(ref.container),
                                 ref.isCjoRef, ref.isSuper));
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert ref: ", e.what());
    }
#endif
    return true;
}

void IndexDatabase::DBUpdate::DealReferences(const std::vector<std::pair<IDArray, Ref>> &refs)
{
    size_t maxMultiInsertIndex = 0;
    if (refs.size() >= MUTI_INSERT_MAX_SIZE) {
        maxMultiInsertIndex = refs.size() - (refs.size() % MUTI_INSERT_MAX_SIZE);
        std::ostringstream oss;
        oss << sql::MultiInsertReferencesHead;
        bool isNeedCommon = false;
        for (size_t i = 0; i < MUTI_INSERT_MAX_SIZE; i++) {
            if (isNeedCommon) {
                oss << ",";
            }
            oss << sql::MultiInsertReferencesValue;
            isNeedCommon = true;
        }
        int Index = 0;
        sqldb::Statement &stmt = db.Use(oss.str(), false);
        for (size_t i = 0; i < maxMultiInsertIndex + 1; i++) {
            if (i >= MUTI_INSERT_MAX_SIZE && i % MUTI_INSERT_MAX_SIZE == 0) {
                Index = 0;
                stmt.execute();
            }
            const auto &array = refs[i].first;
            const auto &insertRef = refs[i].second;
            const auto &containerArray = GetArrayFromID(insertRef.container);
            const auto &bind = sqldb::with(array, insertRef.location.fileUri, insertRef.location.begin.line,
                insertRef.location.begin.column, insertRef.location.end.line, insertRef.location.end.column,
                insertRef.kind, containerArray, insertRef.isCjoRef, insertRef.isSuper);
            BindValue(bind, stmt.GetStmt(), Index);
        }
    }

    // deal remaining
    std::ostringstream oss;
    oss << sql::MultiInsertReferencesHead;
    bool isNeedCommon = false;
    for (size_t i = 0; i < refs.size() - maxMultiInsertIndex; i++) {
        if (isNeedCommon) {
            oss << ",";
        }
        oss << sql::MultiInsertReferencesValue;
        isNeedCommon = true;
    }
    sqldb::Statement &stmt = db.Use(oss.str(), false);
    int Index = 0;
    for (size_t j = maxMultiInsertIndex; j < refs.size(); j++) {
        const auto &array = refs[j].first;
        const auto &insertRef = refs[j].second;
        const auto &containerArray = GetArrayFromID(insertRef.container);
        const auto &bind = sqldb::with(array, insertRef.location.fileUri, insertRef.location.begin.line,
            insertRef.location.begin.column, insertRef.location.end.line, insertRef.location.end.column,
            insertRef.kind, containerArray, insertRef.isCjoRef, insertRef.isSuper);
        BindValue(bind, stmt.GetStmt(), Index);
    }
    stmt.execute();
}

dberr_no IndexDatabase::DBUpdate::InsertReferences(const std::vector<std::pair<IDArray, Ref>> &refs)
{
    if (refs.empty()) {
        return true;
    }
#ifndef NO_EXCEPTIONS
    try {
#endif
        DealReferences(refs);
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert ref: ", e.what());
    }
#endif
    return true;
}

dberr_no IndexDatabase::DBUpdate::InsertRelation(const IDArray &subject, RelationKind predicate, const IDArray &object)
{
    db.Use(sql::InsertRelation).execute(sqldb::with(subject, predicate, object));
    return true;
}

void IndexDatabase::DBUpdate::DealRelations(const std::vector<Relation> &relations)
{
    size_t maxMultiInsertIndex = 0;
    if (relations.size() >= MUTI_INSERT_MAX_SIZE) {
        maxMultiInsertIndex = relations.size() - (relations.size() % MUTI_INSERT_MAX_SIZE);
        std::ostringstream oss;
        oss << sql::MultiInsertRelationsHead;
        bool isNeedCommon = false;
        for (size_t i = 0; i < MUTI_INSERT_MAX_SIZE; i++) {
            if (isNeedCommon) {
                oss << ",";
            }
            oss << sql::MultiInsertRelationsValue;
            isNeedCommon = true;
        }
        int Index = 0;
        sqldb::Statement &stmt = db.Use(oss.str(), false);
        for (size_t i = 0; i < maxMultiInsertIndex + 1; i++) {
            if (i >= MUTI_INSERT_MAX_SIZE && i % MUTI_INSERT_MAX_SIZE == 0) {
                Index = 0;
                stmt.execute();
            }
            const auto &relation = relations[i];
            IDArray subject = GetArrayFromID(relation.subject);
            IDArray object = GetArrayFromID(relation.object);
            const auto &bind = sqldb::with(subject, relation.predicate, relation.object);
            BindValue(bind, stmt.GetStmt(), Index);
        }
    }

    // deal remaining
    std::ostringstream oss;
    oss << sql::MultiInsertRelationsHead;
    bool isNeedCommon = false;
    for (size_t i = 0; i < relations.size() - maxMultiInsertIndex; i++) {
        if (isNeedCommon) {
            oss << ",";
        }
        oss << sql::MultiInsertRelationsValue;
        isNeedCommon = true;
    }
    sqldb::Statement &stmt = db.Use(oss.str(), false);
    int Index = 0;
    for (size_t j = maxMultiInsertIndex; j < relations.size(); j++) {
        const auto &relation = relations[j];
        IDArray subject = GetArrayFromID(relation.subject);
        IDArray object = GetArrayFromID(relation.object);
        const auto &bind = sqldb::with(subject, relation.predicate, relation.object);
        BindValue(bind, stmt.GetStmt(), Index);
    }
    stmt.execute();
}

dberr_no IndexDatabase::DBUpdate::InsertRelations(const std::vector<Relation> &relations)
{
    if (relations.empty()) {
        return true;
    }
#ifndef NO_EXCEPTIONS
    try {
#endif
        DealRelations(relations);
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert relations: ", e.what());
    }
#endif
    return true;
}

dberr_no IndexDatabase::DBUpdate::InsertExtend(const IDArray &extendId, const IDArray &id, const Modifier modifier,
    const std::string &name, const std::string &curPkgName)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        db.Use(sql::InsertExtend)
            .execute(sqldb::with(extendId, id, modifier, name, curPkgName));
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert extend: ", e.what());
    }
#endif
    return true;
}

dberr_no IndexDatabase::DBUpdate::InsertExtends(
    const std::map<std::pair<std::string, SymbolID>, std::vector<ExtendItem>> &extends)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        sqldb::Statement &stmt = db.Use(sql::InsertExtend);
        for (const auto &extend : extends) {
            const auto &curPkgName = extend.first.first;
            const auto &extendId = extend.first.second;
            for (const auto &extenItem : extend.second) {
                stmt.execute(sqldb::with(GetArrayFromID(extendId), GetArrayFromID(extenItem.id),
                    extenItem.modifier, extenItem.interfaceName, curPkgName));
            }
        }
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert extend: ", e.what());
    }
#endif
    return true;
}

dberr_no IndexDatabase::DBUpdate::InsertCrossSymbol(const std::string &curPkgName, const CrossSymbol &crsSym)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        db.Use(sql::InsertCrossSymbol)
            .execute(sqldb::with(curPkgName, GetArrayFromID(crsSym.id), crsSym.name, GetArrayFromID(crsSym.container),
                    crsSym.containerName, crsSym.crossType, crsSym.location.fileUri, crsSym.location.begin.line,
                    crsSym.location.begin.column, crsSym.location.end.line, crsSym.location.end.column,
                    crsSym.declaration.begin.line, crsSym.declaration.begin.column, crsSym.declaration.end.line,
                    crsSym.declaration.end.column));
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert crossSymbol: ", e.what());
    }
#endif
    return true;
}
// LCOV_EXCL_START
void IndexDatabase::DBUpdate::DealCrossSymbols(const std::vector<std::pair<std::string, CrossSymbol>> &crsSyms)
{
    size_t maxMultiInsertIndex = 0;
    if (crsSyms.size() >= MUTI_INSERT_MAX_SIZE) {
        maxMultiInsertIndex = crsSyms.size() - (crsSyms.size() % MUTI_INSERT_MAX_SIZE);
        std::ostringstream oss;
        oss << sql::MultiInsertCrossSymbolsHead;
        bool isNeedCommon = false;
        for (size_t i = 0; i < MUTI_INSERT_MAX_SIZE; i++) {
            if (isNeedCommon) {
                oss << ",";
            }
            oss << sql::MultiInsertCrossSymbolsValue;
            isNeedCommon = true;
        }
        int Index = 0;
        sqldb::Statement &stmt = db.Use(oss.str(), false);
        for (size_t i = 0; i < maxMultiInsertIndex + 1; i++) {
            if (i >= MUTI_INSERT_MAX_SIZE && i % MUTI_INSERT_MAX_SIZE == 0) {
                Index = 0;
                stmt.execute();
            }
            const auto &curPkgName = crsSyms[i].first;
            const auto &crs = crsSyms[i].second;
            const auto &bind = sqldb::with(curPkgName, crs.id, crs.name, crs.container, crs.containerName,
                crs.crossType, crs.location.fileUri, crs.location.begin.line, crs.location.begin.column,
                crs.location.end.line, crs.location.end.column);
            BindValue(bind, stmt.GetStmt(), Index);
        }
    }

    // deal remaining
    std::ostringstream oss;
    oss << sql::MultiInsertCrossSymbolsHead;
    bool isNeedCommon = false;
    for (size_t i = 0; i < crsSyms.size() - maxMultiInsertIndex; i++) {
        if (isNeedCommon) {
            oss << ",";
        }
        oss << sql::MultiInsertCrossSymbolsValue;
        isNeedCommon = true;
    }
    sqldb::Statement &stmt = db.Use(oss.str(), false);
    int Index = 0;
    for (size_t j = maxMultiInsertIndex; j < crsSyms.size(); j++) {
        const auto &curPkgName = crsSyms[j].first;
        const auto &crs = crsSyms[j].second;
        const auto &bind = sqldb::with(curPkgName, crs.id, crs.name, crs.container, crs.containerName,
            crs.crossType, crs.location.fileUri, crs.location.begin.line, crs.location.begin.column,
            crs.location.end.line, crs.location.end.column);
        BindValue(bind, stmt.GetStmt(), Index);
    }
    stmt.execute();
}

dberr_no IndexDatabase::DBUpdate::InsertCrossSymbols(const std::vector<std::pair<std::string, CrossSymbol>> &crsSyms)
{
    if (crsSyms.empty()) {
        return true;
    }
#ifndef NO_EXCEPTIONS
    try {
#endif
        DealCrossSymbols(crsSyms);
#ifndef NO_EXCEPTIONS
    } catch (const std::exception &e) {
        Trace::Log("err in insert crossSymbol: ", e.what());
    }
#endif
    return true;
}

dberr_no IndexDatabase::Update(std::function<dberr_no(DBUpdate)> callback)
{
    DatabaseConnection &dbConnect = Database();
    std::unique_lock<std::mutex> lock(updateMutex);
#ifndef NO_EXCEPTIONS
    try {
#endif
        dbConnect.Use(sql::Begin).execute();
#ifndef NO_EXCEPTIONS
    } catch (...) {
        std::cerr << " err in update begin\n";
        return 1;
    }
    try {
#endif
        callback(DBUpdate(dbConnect));
#ifndef NO_EXCEPTIONS
    } catch (...) {
        std::cerr << " err in update callback\n";
        dbConnect.Use(sql::Rollback).execute();
        return 1;
    }
    try {
#endif
        dbConnect.Use(sql::Commit).execute();
#ifndef NO_EXCEPTIONS
    } catch (...) {
        std::cerr << " err in update commit\n";
        dbConnect.Use(sql::Rollback).execute();
        return 1;
    }
#endif
    return true;
}

dberr_no IndexDatabase::AnalyzeDatabase()
{
    return true;
}

dberr_no IndexDatabase::CompactDatabase()
{
    return true;
}

size_t IndexDatabase::ReleaseMemory() { return sqldb::releaseMemory(); }

size_t IndexDatabase::GetMemoryUsed() { return sqldb::getMemoryUsed(); }

int IndexDatabase::GetChanges() { return Database()->getChanges(); }

dberr_no IndexDatabase::PopulateReferenceCount(Symbol &sym)
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        Use(sql::SelectReferenceCount)
            .execute(sqldb::with(sym.idArray, RefKind::REFERENCE),
                     sqldb::into(sym.references));
#ifndef NO_EXCEPTIONS
    } catch (...) {
        return 1;
    }
#endif
    return true;
}

sqldb::Statement &IndexDatabase::UseSelectConfig()
{
    return Use(sql::SelectConfig);
}

sqldb::Statement &IndexDatabase::UseInsertConfig()
{
    return Use(sql::InsertConfig);
}

dberr_no OpenIndexDatabase(IndexDatabase &db, const std::string &file,
                           IndexDatabase::Options opts)
{
    return db.Initialize([dbfile = file, dbOpts = std::move(opts)] {
        if (dbOpts.openInMemory) {
            return (sqldb::memdb)(
                dbfile, (unsigned int)sqldb::OpenURI | (dbOpts.openReadOnly ? (unsigned int)sqldb::OpenReadOnly
                                                          : ((unsigned int)sqldb::OpenCreate |
                                                             (unsigned int)sqldb::OpenReadWrite)));
        } else {
            return (sqldb::open)(
                dbfile, (unsigned int)sqldb::OpenURI | (dbOpts.openReadOnly ? (unsigned int)sqldb::OpenReadOnly
                                                        : ((unsigned int)sqldb::OpenCreate |
                                                           (unsigned int)sqldb::OpenReadWrite)));
        }
    });
}

} // namespace lsp
} // namespace ark
// LCOV_EXCL_STOP