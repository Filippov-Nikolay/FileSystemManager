#pragma once

#include "TestFramework.h"
#include "file_system/CommandParser.h"

// ─── Parsing ──────────────────────────────────────────────────────────────────

TEST(Parser_EmptyInput_ReturnsEmptyCommand)
{
    CommandParser p;
    const auto cmd = p.Parse("");
    ASSERT_TRUE(cmd.name.empty());
    ASSERT_EQ(cmd.arguments.size(), 0u);
}

TEST(Parser_OnlySpaces_ReturnsEmptyCommand)
{
    CommandParser p;
    const auto cmd = p.Parse("   ");
    ASSERT_TRUE(cmd.name.empty());
}

TEST(Parser_SingleToken_SetsCommandName)
{
    CommandParser p;
    const auto cmd = p.Parse("help");
    ASSERT_EQ(cmd.name, std::string("help"));
    ASSERT_EQ(cmd.arguments.size(), 0u);
}

TEST(Parser_TwoTokens_SetsNameAndOneArgument)
{
    CommandParser p;
    const auto cmd = p.Parse("cd projects");
    ASSERT_EQ(cmd.name, std::string("cd"));
    ASSERT_EQ(cmd.arguments.size(), 1u);
    ASSERT_EQ(cmd.arguments[0], std::string("projects"));
}

TEST(Parser_ThreeTokens_TwoArguments)
{
    CommandParser p;
    const auto cmd = p.Parse("cpf src.txt dst.txt");
    ASSERT_EQ(cmd.name, std::string("cpf"));
    ASSERT_EQ(cmd.arguments.size(), 2u);
    ASSERT_EQ(cmd.arguments[0], std::string("src.txt"));
    ASSERT_EQ(cmd.arguments[1], std::string("dst.txt"));
}

TEST(Parser_QuotedArgument_PreservesInternalSpaces)
{
    CommandParser p;
    const auto cmd = p.Parse("crdr \"my folder\"");
    ASSERT_EQ(cmd.name, std::string("crdr"));
    ASSERT_EQ(cmd.arguments.size(), 1u);
    ASSERT_EQ(cmd.arguments[0], std::string("my folder"));
}

TEST(Parser_TwoQuotedArguments)
{
    CommandParser p;
    const auto cmd = p.Parse("mvf \"old name.txt\" \"new name.txt\"");
    ASSERT_EQ(cmd.name, std::string("mvf"));
    ASSERT_EQ(cmd.arguments.size(), 2u);
    ASSERT_EQ(cmd.arguments[0], std::string("old name.txt"));
    ASSERT_EQ(cmd.arguments[1], std::string("new name.txt"));
}

TEST(Parser_WriteCommand_MultiWordContent_SplitIntoArgs)
{
    // write joins args[1..] in the dispatcher — parser just tokenises
    CommandParser p;
    const auto cmd = p.Parse("write notes.txt hello world");
    ASSERT_EQ(cmd.name, std::string("write"));
    ASSERT_EQ(cmd.arguments.size(), 3u);
    ASSERT_EQ(cmd.arguments[0], std::string("notes.txt"));
    ASSERT_EQ(cmd.arguments[1], std::string("hello"));
    ASSERT_EQ(cmd.arguments[2], std::string("world"));
}

TEST(Parser_LeadingAndTrailingSpaces_Ignored)
{
    CommandParser p;
    const auto cmd = p.Parse("  cd  projects");
    ASSERT_EQ(cmd.name, std::string("cd"));
    ASSERT_EQ(cmd.arguments.size(), 1u);
    ASSERT_EQ(cmd.arguments[0], std::string("projects"));
}

TEST(Parser_MixedQuotedAndUnquoted)
{
    CommandParser p;
    const auto cmd = p.Parse("cpf plain.txt \"spaced name.txt\"");
    ASSERT_EQ(cmd.arguments.size(), 2u);
    ASSERT_EQ(cmd.arguments[0], std::string("plain.txt"));
    ASSERT_EQ(cmd.arguments[1], std::string("spaced name.txt"));
}
