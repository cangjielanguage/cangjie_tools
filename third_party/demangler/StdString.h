// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


#ifndef CANGJIE_DEMANGLER_STD_STRING_H
#define CANGJIE_DEMANGLER_STD_STRING_H

#include <string>

namespace Cangjie {
/**
 * @brief This is a std::string proxy type.
 * Since the Demangler and DemanglerInfo template classes are based on the CString type we created in the Base library,
 * we need to maintain API consistency with the CString class. The following methods must all be implemented. This type
 * also helps remove the dependency on CString and securec (an indirect dependency) of the cangjie-demangle library that
 * we export to other users.
 */
class StdString : public std::string {
public:
    /**
     * @brief The constructor of class StdString with none.
     *
     * @return StdString The instance of StdString.
     */
    StdString() : std::string() {}

    /**
     * @brief The constructor of class StdString with character.
     *
     * @param c The character.
     * @return StdString The instance of StdString.
     */
    StdString(char c) : std::string(1, c) {}

    /**
     * @brief The constructor of class StdString with char*.
     *
     * @param initStr The pointer to character.
     * @return StdString The instance of StdString.
     */
    StdString(const char* initStr) : std::string(initStr) {}

    /**
     * @brief The constructor of class StdString with std::string.
     *
     * @param other The string.
     * @return StdString The instance of StdString.
     */
    StdString(const std::string& other) : std::string(other) {}

    /**
     * @brief The constructor of class StdString with StdString.
     *
     * @param other The StdString.
     * @return StdString The instance of StdString.
     */
    StdString(const StdString& other) : std::string(other.Str()) {}

    /**
     * @brief This function ensures that StdString self-assignment.
     *
     * @param other The StdString.
     * @return StdString The new instance of StdString.
     */
    StdString& operator=(const StdString& other)
    {
        std::string::operator=(other.Str());
        return *this;
    }

    /**
     * @brief Get the length of StdString object.
     *
     * @return size_t The length.
     */
    size_t Length() const { return this->size(); }

    /**
     * @brief Get the "char *" format of StdString object.
     *
     * @return char* The string.
     */
    const char* Str() const noexcept { return this->c_str(); }

    /**
     * @brief Determine if StdString object is empty.
     *
     * @return bool Return true if the StdString object is empty, Otherwise, false is returned.
     */
    bool IsEmpty() const { return this->empty(); }

    /**
     * @brief Search for the first occurrence of the specified pattern in the string starting from the given position.
     *
     * @param pattern The specified pattern.
     * @param begin The string starting position.
     * @return int Return the index of the first match, or -1 if no match is found.
     */
    int Find(const char* pattern, size_t begin = 0) const { return this->find(pattern, begin); }

    /**
     * @brief Search for the first occurrence of the specified character in the string starting from
     * the given position.
     *
     * @param pattern The specified character.
     * @param begin The string starting position.
     * @return int Return the index of the first match, or -1 if no match is found.
     */
    int Find(const char pattern, size_t begin = 0) const { return this->find(pattern, begin); }

    /**
     * @brief Return a substring starting from the specified index.
     *
     * @param begin The string starting position.
     * @return StdString The substring.
     */
    StdString SubStr(size_t index) const { return this->substr(index); }

    /**
     * @brief Return a substring starting from the specified index and with the specified length.
     *
     * @param begin The string starting position.
     * @param len The substring length.
     * @return StdString The substring.
     */
    StdString SubStr(size_t index, size_t len) const { return this->substr(index, len); }

    /**
     * @brief Check whether the string ends with the specified suffix.
     *
     * @param suffix The specified suffix.
     * @return bool Return true if the string ends with the specified suffix, Otherwise, false is returned.
     */
    bool EndsWith(const StdString& suffix) const
    {
        return size() >= suffix.size() && substr(size() - suffix.size()) == suffix;
    }

    /**
     * @brief Truncate the string to the specified index.
     *
     * @param index The truncate position.
     * @return StdString The truncated string.
     */
    StdString& Truncate(size_t index)
    {
        this->resize(index);
        return *this;
    }
};
} // namespace Cangjie
#endif // CANGJIE_DEMANGLER_STD_STRING_H