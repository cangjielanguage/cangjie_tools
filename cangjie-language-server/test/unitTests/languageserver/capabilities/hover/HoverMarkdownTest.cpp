// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ArkServer.h"
#include "Protocol.h"
#include "gtest/gtest.h"

namespace {
ark::Hover MakeHover(std::initializer_list<std::string> markedStrings)
{
    ark::Hover hover;
    hover.markedString.assign(markedStrings.begin(), markedStrings.end());
    return hover;
}
} // namespace

TEST(HoverMarkdownTest, FormatsAllHoverSections)
{
    ark::Hover hover = MakeHover({
        "Declared in: row.cj\r\nPackage info: ohos.arkui.component.row\r\n",
        "// In class Row\n\n",
        "public func init(space!: Option<Length>, child!: () -> Unit)\n",
        "apiKey:ohos.arkui.component.row/Row.init(Option<Length>, () -> Unit)\r\n",
        "@!APILevel[\n    since: 22,\n]\n",
        "Creates a row layout container.\n",
        "Use it for horizontal layout.\n",
    });

    const std::string result = ark::BuildHoverMarkdown(hover);

    EXPECT_EQ(result,
        "Declared in: row.cj  \n"
        "Package info: ohos.arkui.component.row  \n"
        "\n"
        "```cangjie\n"
        "// In class Row\n"
        "public func init(space!: Option<Length>, child!: () -> Unit)\n"
        "apiKey:ohos.arkui.component.row/Row.init(Option<Length>, () -> Unit)\n"
        "@!APILevel[\n"
        "    since: 22,\n"
        "]\n"
        "```\n"
        "\n"
        "---\n"
        "\n"
        "Creates a row layout container.\n"
        "\n"
        "\n"
        "Use it for horizontal layout.\n");
}

TEST(HoverMarkdownTest, NormalizesLinuxMacAndWindowsLineEndings)
{
    const std::string expected =
        "Declared in: main.cj  \n"
        "Package info: test  \n"
        "\n"
        "```cangjie\n"
        "public func score(): Int64\n"
        "```\n";

    EXPECT_EQ(ark::BuildHoverMarkdown(MakeHover({
        "Declared in: main.cj\nPackage info: test\n",
        "public func score(): Int64\n",
    })), expected);

    EXPECT_EQ(ark::BuildHoverMarkdown(MakeHover({
        "Declared in: main.cj\r\nPackage info: test\r\n",
        "public func score(): Int64\r\n",
    })), expected);

    EXPECT_EQ(ark::BuildHoverMarkdown(MakeHover({
        "Declared in: main.cj\rPackage info: test\r",
        "public func score(): Int64\r",
    })), expected);
}

TEST(HoverMarkdownTest, SkipsEmptyBlocksAndOmitsEmptySections)
{
    EXPECT_TRUE(ark::BuildHoverMarkdown(MakeHover({"", " \r\n\t "})).empty());

    EXPECT_EQ(ark::BuildHoverMarkdown(MakeHover({
        "Declared in: only.cj\r\nPackage info: only\r\n",
    })),
        "Declared in: only.cj  \n"
        "Package info: only  \n"
        "\n");

    EXPECT_EQ(ark::BuildHoverMarkdown(MakeHover({
        "Just a resolved comment.",
    })),
        "```cangjie\n"
        "Just a resolved comment.\n"
        "```\n");
}

TEST(HoverMarkdownTest, EscapesMarkdownOutsideCodeFence)
{
    ark::Hover hover = MakeHover({
        "Declared in: [main].cj\r\nPackage info: test_pkg\r\n",
        "public func name(): String",
        "Use *bold* `code` [link] <tag> # title_1",
    });

    const std::string result = ark::BuildHoverMarkdown(hover);

    EXPECT_NE(result.find("Declared in: \\[main\\].cj  \n"), std::string::npos);
    EXPECT_NE(result.find("Package info: test\\_pkg  \n"), std::string::npos);
    EXPECT_NE(result.find("Use \\*bold\\* \\`code\\` \\[link\\] \\<tag\\> \\# title\\_1\n"), std::string::npos);
    EXPECT_NE(result.find("```cangjie\npublic func name(): String\n```\n"), std::string::npos);
}

TEST(HoverMarkdownTest, FormatsWithoutSourceInfo)
{
    ark::Hover hover = MakeHover({
        "// In struct Point",
        "public func x(): Int64",
        "apiKey:test/Point.x()",
        "Coordinate getter.",
    });

    EXPECT_EQ(ark::BuildHoverMarkdown(hover),
        "```cangjie\n"
        "// In struct Point\n"
        "public func x(): Int64\n"
        "apiKey:test/Point.x()\n"
        "```\n"
        "\n"
        "---\n"
        "\n"
        "Coordinate getter.\n");
}

TEST(HoverMarkdownTest, ClassifiesShortPrefixesAndAtBlocks)
{
    ark::Hover hover = MakeHover({
        "Declared",
        "api",
        "//",
        "@Deprecated[\n    message: \"use next\"\n]",
        "Trailing comment.",
    });

    EXPECT_EQ(ark::BuildHoverMarkdown(hover),
        "```cangjie\n"
        "Declared\n"
        "@Deprecated[\n"
        "    message: \"use next\"\n"
        "]\n"
        "```\n"
        "\n"
        "---\n"
        "\n"
        "api\n"
        "\n"
        "\n"
        "//\n"
        "\n"
        "\n"
        "Trailing comment.\n");
}
