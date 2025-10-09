// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_URI_H
#define LSPSERVER_URI_H

#include <string>
#include <tuple>

/**
 * According to the language service protocol to create structure
 * see https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/#baseProtocol
 */
namespace ark {
constexpr long URI_SECOND_POS = 2;
constexpr unsigned int HEXADECIMAL = 16;
class URI {
public:
    URI(const std::string &scheme, const std::string &authority, const std::string &body);
    ~URI() {}

    std::string ToString() const;

    static URI Parse(const std::string& origUri);

    static std::string Resolve(const URI &u);

    static std::string Resolve(const std::string &fileURI);

    friend bool operator==(const URI &lhs, const URI &rhs)
    {
        return std::tie(lhs.scheme, lhs.authority, lhs.body) ==
               std::tie(rhs.scheme, rhs.authority, rhs.body);
    }

    friend bool operator<(const URI &lhs, const URI &rhs)
    {
        return std::tie(lhs.scheme, lhs.authority, lhs.body) <
               std::tie(rhs.scheme, rhs.authority, rhs.body);
    }

    static std::string GetAbsolutePath(std::string bodyPath);

    static URI URIFromAbsolutePath(const std::string absolutePath);

private:
    URI() = default;
    std::string scheme;
    std::string authority;
    std::string body;
};
} // namespace ark

#endif // LSPSERVER_URI_H
