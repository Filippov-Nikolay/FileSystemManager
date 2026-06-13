#pragma once

#include "TestFramework.h"
#include "file_system/FileManager.h"

#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

// ─── Fixture ──────────────────────────────────────────────────────────────────

struct TempWorkspace
{
    fs::path path;

    TempWorkspace()
    {
        const auto ts = std::chrono::system_clock::now().time_since_epoch().count();
        path = fs::temp_directory_path() / ("fsm_test_" + std::to_string(ts));
        fs::create_directories(path);
    }

    ~TempWorkspace()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    FileManager make() { return FileManager(path.string()); }

    // Shorthand for building expected filesystem paths
    fs::path operator/(const std::string& rel) const { return path / rel; }
};

// ─── Directory — create / rename / copy / move / delete ──────────────────────

TEST(FileManager_CreateDirectory_Success)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    ASSERT_TRUE(fm.CreateDirectory("subdir"));
    ASSERT_TRUE(fs::is_directory(ws / "subdir"));
}

TEST(FileManager_CreateDirectory_AlreadyExists_ReturnsFalse)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.CreateDirectory("dir");
    ASSERT_FALSE(fm.CreateDirectory("dir"));
}

TEST(FileManager_CreateDirectory_TraversalBlocked)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    ASSERT_FALSE(fm.CreateDirectory("../outside"));
    ASSERT_FALSE(fs::exists(ws.path.parent_path() / "outside"));
}

TEST(FileManager_RenameDirectory_Success)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.CreateDirectory("old");
    ASSERT_TRUE(fm.RenameDirectory("old", "renamed"));
    ASSERT_FALSE(fs::exists(ws / "old"));
    ASSERT_TRUE(fs::exists(ws / "renamed"));
}

TEST(FileManager_CopyDirectory_LeavesSourceIntact)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.CreateDirectory("original");
    ASSERT_TRUE(fm.CopyDirectory("original", "copy"));
    ASSERT_TRUE(fs::exists(ws / "original"));
    ASSERT_TRUE(fs::exists(ws / "copy"));
}

TEST(FileManager_MoveDirectory_SourceGone)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.CreateDirectory("src");
    ASSERT_TRUE(fm.MoveDirectory("src", "dst"));
    ASSERT_FALSE(fs::exists(ws / "src"));
    ASSERT_TRUE(fs::exists(ws / "dst"));
}

TEST(FileManager_DeleteDirectory_Success)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.CreateDirectory("doomed");
    ASSERT_TRUE(fm.DeleteDirectory("doomed"));
    ASSERT_FALSE(fs::exists(ws / "doomed"));
}

TEST(FileManager_DeleteDirectory_NotFound_ReturnsFalse)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    ASSERT_FALSE(fm.DeleteDirectory("ghost"));
}

// ─── File — create / rename / copy / move / delete ───────────────────────────

TEST(FileManager_CreateFile_Success)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    ASSERT_TRUE(fm.CreateFile("test.txt"));
    ASSERT_TRUE(fs::is_regular_file(ws / "test.txt"));
}

TEST(FileManager_CreateFile_AlreadyExists_ReturnsFalse)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.CreateFile("test.txt");
    ASSERT_FALSE(fm.CreateFile("test.txt"));
}

TEST(FileManager_RenameFile_Success)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.CreateFile("before.txt");
    ASSERT_TRUE(fm.RenameFile("before.txt", "after.txt"));
    ASSERT_FALSE(fs::exists(ws / "before.txt"));
    ASSERT_TRUE(fs::exists(ws / "after.txt"));
}

TEST(FileManager_CopyFile_LeavesSource)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.WriteFile("original.txt", "data");
    ASSERT_TRUE(fm.CopyFile("original.txt", "copy.txt"));
    ASSERT_TRUE(fs::exists(ws / "original.txt"));
    ASSERT_TRUE(fs::exists(ws / "copy.txt"));
}

TEST(FileManager_MoveFile_SourceGone)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.WriteFile("from.txt", "data");
    ASSERT_TRUE(fm.MoveFile("from.txt", "to.txt"));
    ASSERT_FALSE(fs::exists(ws / "from.txt"));
    ASSERT_TRUE(fs::exists(ws / "to.txt"));
}

TEST(FileManager_DeleteFile_Success)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.CreateFile("temp.txt");
    ASSERT_TRUE(fm.DeleteFile("temp.txt"));
    ASSERT_FALSE(fs::exists(ws / "temp.txt"));
}

TEST(FileManager_DeleteFile_NotFound_ReturnsFalse)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    ASSERT_FALSE(fm.DeleteFile("ghost.txt"));
}

// ─── File — read / write / append ────────────────────────────────────────────

TEST(FileManager_WriteAndReadFile_Roundtrip)
{
    TempWorkspace ws;
    auto fm = ws.make();

    { SuppressOutput _; fm.WriteFile("notes.txt", "hello world"); }

    CaptureOutput cap;
    fm.ReadFile("notes.txt");
    ASSERT_TRUE(cap.str().find("hello world") != std::string::npos);
}

TEST(FileManager_WriteFile_OverwritesExisting)
{
    TempWorkspace ws;
    auto fm = ws.make();

    { SuppressOutput _; fm.WriteFile("f.txt", "first"); }
    { SuppressOutput _; fm.WriteFile("f.txt", "second"); }

    CaptureOutput cap;
    fm.ReadFile("f.txt");
    const auto out = cap.str();
    ASSERT_TRUE(out.find("second") != std::string::npos);
    ASSERT_TRUE(out.find("first")  == std::string::npos);
}

TEST(FileManager_AppendFile_AddsBelowExistingContent)
{
    TempWorkspace ws;
    auto fm = ws.make();

    { SuppressOutput _; fm.WriteFile("log.txt", "line one"); }
    { SuppressOutput _; fm.AppendFile("log.txt", "line two"); }

    CaptureOutput cap;
    fm.ReadFile("log.txt");
    const auto out = cap.str();
    ASSERT_TRUE(out.find("line one") != std::string::npos);
    ASSERT_TRUE(out.find("line two") != std::string::npos);
}

TEST(FileManager_ReadFile_NotFound_ReturnsFalse)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    ASSERT_FALSE(fm.ReadFile("ghost.txt"));
}

// ─── Navigation ───────────────────────────────────────────────────────────────

TEST(FileManager_ChangeDirectory_Success)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.CreateDirectory("sub");
    ASSERT_TRUE(fm.ChangeDirectory("sub"));
}

TEST(FileManager_ChangeDirectory_NonExistent_ReturnsFalse)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    ASSERT_FALSE(fm.ChangeDirectory("ghost"));
}

TEST(FileManager_ChangeDirectory_TraversalBlocked)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    ASSERT_FALSE(fm.ChangeDirectory("../outside"));
}

TEST(FileManager_ChangeDirectory_NoArg_GoesToWorkspaceRoot)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.CreateDirectory("deep");
    fm.ChangeDirectory("deep");
    ASSERT_TRUE(fm.ChangeDirectory(""));

    // A file created now should land at root, not inside "deep"
    fm.CreateFile("marker.txt");
    ASSERT_TRUE(fs::exists(ws / "marker.txt"));
    ASSERT_FALSE(fs::exists(ws / "deep" / "marker.txt"));
}

TEST(FileManager_ChangeDirectory_NestedPath)
{
    TempWorkspace ws;
    SuppressOutput _;
    auto fm = ws.make();

    fm.CreateDirectory("a");
    fm.ChangeDirectory("a");
    fm.CreateDirectory("b");

    ASSERT_TRUE(fm.ChangeDirectory("b"));
    fm.CreateFile("leaf.txt");
    ASSERT_TRUE(fs::exists(ws / "a" / "b" / "leaf.txt"));
}

// ─── Search ───────────────────────────────────────────────────────────────────

TEST(FileManager_SearchByMask_StarExtension_FindsMatches)
{
    TempWorkspace ws;
    auto fm = ws.make();

    { SuppressOutput _; fm.WriteFile("notes.txt", ""); fm.WriteFile("readme.md", ""); fm.WriteFile("data.txt", ""); }

    CaptureOutput cap;
    fm.SearchByMask(".", "*.txt");
    const auto out = cap.str();

    ASSERT_TRUE(out.find("notes.txt") != std::string::npos);
    ASSERT_TRUE(out.find("data.txt")  != std::string::npos);
    ASSERT_TRUE(out.find("readme.md") == std::string::npos);
}

TEST(FileManager_SearchByMask_QuestionMarkWildcard)
{
    TempWorkspace ws;
    auto fm = ws.make();

    { SuppressOutput _; fm.WriteFile("a1.log", ""); fm.WriteFile("b2.log", ""); fm.WriteFile("abc.log", ""); }

    CaptureOutput cap;
    fm.SearchByMask(".", "??.log");
    const auto out = cap.str();

    ASSERT_TRUE(out.find("a1.log")  != std::string::npos);
    ASSERT_TRUE(out.find("b2.log")  != std::string::npos);
    ASSERT_TRUE(out.find("abc.log") == std::string::npos); // 3 chars before .log
}

TEST(FileManager_SearchByMask_PrefixAndSuffix)
{
    TempWorkspace ws;
    auto fm = ws.make();

    { SuppressOutput _; fm.WriteFile("test_01.cpp", ""); fm.WriteFile("test_02.cpp", ""); fm.WriteFile("main.cpp", ""); }

    CaptureOutput cap;
    fm.SearchByMask(".", "test_*.cpp");
    const auto out = cap.str();

    ASSERT_TRUE(out.find("test_01.cpp") != std::string::npos);
    ASSERT_TRUE(out.find("test_02.cpp") != std::string::npos);
    ASSERT_TRUE(out.find("main.cpp")    == std::string::npos);
}

TEST(FileManager_SearchByMask_CaseInsensitive)
{
    TempWorkspace ws;
    auto fm = ws.make();

    { SuppressOutput _; fm.WriteFile("README.MD", ""); }

    CaptureOutput cap;
    fm.SearchByMask(".", "*.md");
    ASSERT_TRUE(cap.str().find("README.MD") != std::string::npos);
}

TEST(FileManager_SearchByMask_NoMatches_PrintsMessage)
{
    TempWorkspace ws;
    auto fm = ws.make();

    { SuppressOutput _; fm.CreateFile("notes.txt"); }

    CaptureOutput cap;
    fm.SearchByMask(".", "*.cpp");
    ASSERT_TRUE(cap.str().find("No files found") != std::string::npos);
}

TEST(FileManager_SearchByMask_SearchesRecursively)
{
    TempWorkspace ws;
    auto fm = ws.make();

    {
        SuppressOutput _;
        fm.CreateDirectory("sub");
        fm.ChangeDirectory("sub");
        fm.WriteFile("nested.txt", "");
        fm.ChangeDirectory("");
    }

    CaptureOutput cap;
    fm.SearchByMask(".", "*.txt");
    ASSERT_TRUE(cap.str().find("nested.txt") != std::string::npos);
}
