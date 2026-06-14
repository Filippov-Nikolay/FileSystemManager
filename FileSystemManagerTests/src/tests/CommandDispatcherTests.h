#pragma once

#include "TestFramework.h"

#include "file_system/CommandDispatcher.h"
#include "file_system/Command.h"
#include "file_system/FileManager.h"
#include "logging/Logger.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <sstream>

namespace fs = std::filesystem;

// --- Helpers -----------------------------------------------------------------

namespace {

Command makeCmd(const std::string& name, std::vector<std::string> args = {})
{
    Command cmd;
    cmd.name      = name;
    cmd.arguments = std::move(args);
    return cmd;
}

// Redirects std::cin from a string for the duration of the scope
struct RedirectInput
{
    std::istringstream buffer;
    std::streambuf*    saved;

    explicit RedirectInput(const std::string& text)
        : buffer(text), saved(std::cin.rdbuf(buffer.rdbuf())) {}

    ~RedirectInput() { std::cin.rdbuf(saved); }
};

} // namespace

// --- Fixture -----------------------------------------------------------------

struct DispatchFixture
{
    fs::path wsPath;
    fs::path logPath;
    std::unique_ptr<FileManager>       fm;
    std::unique_ptr<Logger>            logger;
    std::unique_ptr<CommandDispatcher> dispatcher;

    DispatchFixture()
    {
        const auto ts = std::chrono::system_clock::now().time_since_epoch().count();
        wsPath  = fs::temp_directory_path() / ("fsm_disp_" + std::to_string(ts));
        logPath = fs::temp_directory_path() / ("fsm_log_"  + std::to_string(ts) + ".txt");
        fs::create_directories(wsPath);
        fm         = std::make_unique<FileManager>(wsPath.string());
        logger     = std::make_unique<Logger>(logPath.string());
        dispatcher = std::make_unique<CommandDispatcher>(*fm, *logger);
    }

    ~DispatchFixture()
    {
        dispatcher.reset();
        logger.reset();
        fm.reset();
        std::error_code ec;
        fs::remove_all(wsPath, ec);
        fs::remove(logPath, ec);
    }

    fs::path operator/(const std::string& rel) const { return wsPath / rel; }
};

// --- Dispatch behaviour ------------------------------------------------------

TEST(Dispatcher_UnknownCommand_PrintsError)
{
    DispatchFixture f;
    SuppressOutput _;
    const auto result = f.dispatcher->Dispatch(makeCmd("nonexistent"));
    ASSERT_TRUE(result == DispatchResult::Continue);
}

TEST(Dispatcher_EmptyCommand_ReturnsContinue)
{
    DispatchFixture f;
    SuppressOutput _;
    ASSERT_TRUE(f.dispatcher->Dispatch(makeCmd("")) == DispatchResult::Continue);
}

TEST(Dispatcher_clfm_ReturnsQuit)
{
    DispatchFixture f;
    SuppressOutput _;
    ASSERT_TRUE(f.dispatcher->Dispatch(makeCmd("clfm")) == DispatchResult::Quit);
}

TEST(Dispatcher_KnownCommand_ReturnsContinue)
{
    DispatchFixture f;
    SuppressOutput _;
    ASSERT_TRUE(f.dispatcher->Dispatch(makeCmd("pwd")) == DispatchResult::Continue);
}

// --- Directories -------------------------------------------------------------

TEST(Dispatcher_crdr_CreatesDirectory)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crdr", {"mydir"}));
    ASSERT_TRUE(fs::is_directory(f / "mydir"));
}

TEST(Dispatcher_rndr_RenamesDirectory)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crdr", {"before"}));
    f.dispatcher->Dispatch(makeCmd("rndr", {"before", "after"}));
    ASSERT_FALSE(fs::exists(f / "before"));
    ASSERT_TRUE(fs::is_directory(f / "after"));
}

TEST(Dispatcher_cpdr_CopiesDirectory)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crdr", {"orig"}));
    f.dispatcher->Dispatch(makeCmd("cpdr", {"orig", "copy"}));
    ASSERT_TRUE(fs::exists(f / "orig"));
    ASSERT_TRUE(fs::exists(f / "copy"));
}

TEST(Dispatcher_mvdr_MovesDirectory)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crdr", {"src"}));
    f.dispatcher->Dispatch(makeCmd("mvdr", {"src", "dst"}));
    ASSERT_FALSE(fs::exists(f / "src"));
    ASSERT_TRUE(fs::exists(f / "dst"));
}

TEST(Dispatcher_rmdr_Confirmed_DeletesDirectory)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crdr", {"doomed"}));
    ASSERT_TRUE(fs::exists(f / "doomed"));

    RedirectInput in("y\n");
    f.dispatcher->Dispatch(makeCmd("rmdr", {"doomed"}));
    ASSERT_FALSE(fs::exists(f / "doomed"));
}

TEST(Dispatcher_rmdr_Cancelled_DirectoryRemains)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crdr", {"safe"}));

    RedirectInput in("n\n");
    f.dispatcher->Dispatch(makeCmd("rmdr", {"safe"}));
    ASSERT_TRUE(fs::exists(f / "safe"));
}

TEST(Dispatcher_lsdr_ShowsEntries)
{
    DispatchFixture f;
    { SuppressOutput _; f.dispatcher->Dispatch(makeCmd("crdr", {"subdir"})); }

    CaptureOutput cap;
    f.dispatcher->Dispatch(makeCmd("lsdr", {"."}));
    ASSERT_TRUE(cap.str().find("subdir") != std::string::npos);
}

TEST(Dispatcher_szdr_ShowsSize)
{
    DispatchFixture f;
    SuppressOutput _;

    CaptureOutput cap;
    f.dispatcher->Dispatch(makeCmd("szdr", {"."}));
    ASSERT_TRUE(cap.str().find("bytes") != std::string::npos);
}

// --- Files -------------------------------------------------------------------

TEST(Dispatcher_crf_CreatesFile)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crf", {"test.txt"}));
    ASSERT_TRUE(fs::is_regular_file(f / "test.txt"));
}

TEST(Dispatcher_rnf_RenamesFile)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crf", {"old.txt"}));
    f.dispatcher->Dispatch(makeCmd("rnf", {"old.txt", "new.txt"}));
    ASSERT_FALSE(fs::exists(f / "old.txt"));
    ASSERT_TRUE(fs::exists(f / "new.txt"));
}

TEST(Dispatcher_cpf_CopiesFile)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("write", {"src.txt", "data"}));
    f.dispatcher->Dispatch(makeCmd("cpf", {"src.txt", "dst.txt"}));
    ASSERT_TRUE(fs::exists(f / "src.txt"));
    ASSERT_TRUE(fs::exists(f / "dst.txt"));
}

TEST(Dispatcher_mvf_MovesFile)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("write", {"from.txt", "data"}));
    f.dispatcher->Dispatch(makeCmd("mvf", {"from.txt", "to.txt"}));
    ASSERT_FALSE(fs::exists(f / "from.txt"));
    ASSERT_TRUE(fs::exists(f / "to.txt"));
}

TEST(Dispatcher_rmf_Confirmed_DeletesFile)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crf", {"temp.txt"}));

    RedirectInput in("y\n");
    f.dispatcher->Dispatch(makeCmd("rmf", {"temp.txt"}));
    ASSERT_FALSE(fs::exists(f / "temp.txt"));
}

TEST(Dispatcher_rmf_Cancelled_FileRemains)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crf", {"keep.txt"}));

    RedirectInput in("n\n");
    f.dispatcher->Dispatch(makeCmd("rmf", {"keep.txt"}));
    ASSERT_TRUE(fs::exists(f / "keep.txt"));
}

TEST(Dispatcher_szf_ShowsSize)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("write", {"file.txt", "hello"}));

    CaptureOutput cap;
    f.dispatcher->Dispatch(makeCmd("szf", {"file.txt"}));
    ASSERT_TRUE(cap.str().find("bytes") != std::string::npos);
}

TEST(Dispatcher_cat_PrintsFileContent)
{
    DispatchFixture f;
    { SuppressOutput _; f.dispatcher->Dispatch(makeCmd("write", {"note.txt", "hello world"})); }

    CaptureOutput cap;
    f.dispatcher->Dispatch(makeCmd("cat", {"note.txt"}));
    ASSERT_TRUE(cap.str().find("hello world") != std::string::npos);
}

TEST(Dispatcher_write_WritesContent)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("write", {"f.txt", "first"}));
    f.dispatcher->Dispatch(makeCmd("write", {"f.txt", "second"}));

    CaptureOutput cap;
    f.dispatcher->Dispatch(makeCmd("cat", {"f.txt"}));
    const auto out = cap.str();
    ASSERT_TRUE(out.find("second") != std::string::npos);
    ASSERT_TRUE(out.find("first")  == std::string::npos);
}

TEST(Dispatcher_append_AppendsContent)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("write",  {"log.txt", "line one"}));
    f.dispatcher->Dispatch(makeCmd("append", {"log.txt", "line two"}));

    CaptureOutput cap;
    f.dispatcher->Dispatch(makeCmd("cat", {"log.txt"}));
    const auto out = cap.str();
    ASSERT_TRUE(out.find("line one") != std::string::npos);
    ASSERT_TRUE(out.find("line two") != std::string::npos);
}

// --- Navigation --------------------------------------------------------------

TEST(Dispatcher_pwd_PrintsPath)
{
    DispatchFixture f;
    CaptureOutput cap;
    f.dispatcher->Dispatch(makeCmd("pwd"));
    ASSERT_FALSE(cap.str().empty());
}

TEST(Dispatcher_cd_ChangesDirectory)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crdr", {"sub"}));
    f.dispatcher->Dispatch(makeCmd("cd",   {"sub"}));
    f.dispatcher->Dispatch(makeCmd("crf",  {"marker.txt"}));
    ASSERT_TRUE(fs::exists(f / "sub" / "marker.txt"));
}

TEST(Dispatcher_cd_NoArg_ReturnsToRoot)
{
    DispatchFixture f;
    SuppressOutput _;
    f.dispatcher->Dispatch(makeCmd("crdr", {"deep"}));
    f.dispatcher->Dispatch(makeCmd("cd",   {"deep"}));
    f.dispatcher->Dispatch(makeCmd("cd",   {}));
    f.dispatcher->Dispatch(makeCmd("crf",  {"root.txt"}));
    ASSERT_TRUE(fs::exists(f / "root.txt"));
    ASSERT_FALSE(fs::exists(f / "deep" / "root.txt"));
}

TEST(Dispatcher_shmsk_FindsMatchingFiles)
{
    DispatchFixture f;
    { SuppressOutput _; f.dispatcher->Dispatch(makeCmd("write", {"notes.txt", ""})); }
    { SuppressOutput _; f.dispatcher->Dispatch(makeCmd("crf",   {"readme.md"})); }

    CaptureOutput cap;
    f.dispatcher->Dispatch(makeCmd("shmsk", {".", "*.txt"}));
    const auto out = cap.str();
    ASSERT_TRUE(out.find("notes.txt") != std::string::npos);
    ASSERT_TRUE(out.find("readme.md") == std::string::npos);
}

// --- System ------------------------------------------------------------------

TEST(Dispatcher_help_ContainsCategories)
{
    DispatchFixture f;
    CaptureOutput cap;
    f.dispatcher->Dispatch(makeCmd("help"));
    const auto out = cap.str();
    ASSERT_TRUE(out.find("Directories") != std::string::npos);
    ASSERT_TRUE(out.find("Files")       != std::string::npos);
    ASSERT_TRUE(out.find("Navigation")  != std::string::npos);
    ASSERT_TRUE(out.find("System")      != std::string::npos);
}

TEST(Dispatcher_help_ContainsAllCommands)
{
    DispatchFixture f;
    CaptureOutput cap;
    f.dispatcher->Dispatch(makeCmd("help"));
    const auto out = cap.str();
    for (const auto& cmd : {"crdr","rndr","cpdr","mvdr","rmdr","lsdr","szdr",
                             "crf","rnf","cpf","mvf","rmf","szf","cat","write","append",
                             "pwd","cd","shmsk","help","clear","clfm"})
    {
        ASSERT_TRUE(out.find(cmd) != std::string::npos);
    }
}

TEST(Dispatcher_clear_ReturnsContinue)
{
    DispatchFixture f;
    SuppressOutput _;
    ASSERT_TRUE(f.dispatcher->Dispatch(makeCmd("clear")) == DispatchResult::Continue);
}

TEST(Dispatcher_MissingArgument_PrintsUsageError)
{
    DispatchFixture f;
    CaptureOutput cap;
    f.dispatcher->Dispatch(makeCmd("crdr")); // requires 1 arg
    ASSERT_TRUE(cap.str().find("requires") != std::string::npos);
}
