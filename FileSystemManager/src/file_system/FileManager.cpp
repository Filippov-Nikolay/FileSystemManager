#include "FileManager.h"
#include "../utils/Color.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace {

bool StartsWithCI(const std::string& str, const std::string& prefix)
{
    if (str.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i)
    {
        if (std::tolower(static_cast<unsigned char>(str[i])) !=
            std::tolower(static_cast<unsigned char>(prefix[i])))
            return false;
    }
    return true;
}

// Case-insensitive glob matching: * matches any sequence, ? matches one character
bool MatchGlob(const std::string& name, const std::string& pattern)
{
    size_t ni = 0, pi = 0;
    size_t starPi = std::string::npos, starNi = 0;

    while (ni < name.size())
    {
        if (pi < pattern.size() &&
            (pattern[pi] == '?' ||
             std::tolower(static_cast<unsigned char>(name[ni])) ==
             std::tolower(static_cast<unsigned char>(pattern[pi]))))
        {
            ++ni; ++pi;
        }
        else if (pi < pattern.size() && pattern[pi] == '*')
        {
            starPi = pi++;
            starNi = ni;
        }
        else if (starPi != std::string::npos)
        {
            pi = starPi + 1;
            ni = ++starNi;
        }
        else
        {
            return false;
        }
    }

    while (pi < pattern.size() && pattern[pi] == '*')
        ++pi;

    return pi == pattern.size();
}

} // namespace

FileManager::FileManager(const std::string& rootPath)
    : rootPath(rootPath), currentPath(rootPath)
{
    fs::create_directories(this->rootPath);
    this->rootPath = fs::weakly_canonical(this->rootPath);
    this->currentPath = this->rootPath;
}

fs::path FileManager::ResolvePath(const std::string& path)
{
    const fs::path resolved = fs::weakly_canonical(currentPath / path);

    auto resolvedIt = resolved.begin();
    auto rootIt = rootPath.begin();

    while (rootIt != rootPath.end())
    {
        if (resolvedIt == resolved.end() || *resolvedIt != *rootIt)
        {
            throw fs::filesystem_error(
                "Access denied: path is outside workspace",
                resolved,
                std::make_error_code(std::errc::permission_denied)
            );
        }
        ++rootIt;
        ++resolvedIt;
    }

    return resolved;
}

// ─── Directory ───────────────────────────────────────────────────────────────

bool FileManager::CreateDirectory(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (fs::exists(target)) { std::cout << Color::Yellow << "Directory already exists." << Color::Reset << '\n'; return false; }
        fs::create_directories(target);
        std::cout << Color::Green << "Directory created successfully." << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::MoveOrRenameDirectory(const std::string& from, const std::string& where, const std::string& successMsg)
{
    try
    {
        const auto src = ResolvePath(from);
        const auto dst = ResolvePath(where);
        if (!fs::exists(src) || !fs::is_directory(src)) { std::cout << Color::Red << "Source directory does not exist." << Color::Reset << '\n'; return false; }
        if (fs::exists(dst)) { std::cout << Color::Yellow << "Destination path already exists." << Color::Reset << '\n'; return false; }
        fs::rename(src, dst);
        std::cout << Color::Green << successMsg << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::RenameDirectory(const std::string& from, const std::string& where)
{
    return MoveOrRenameDirectory(from, where, "Directory renamed successfully.");
}

bool FileManager::MoveDirectory(const std::string& from, const std::string& where)
{
    return MoveOrRenameDirectory(from, where, "Directory moved successfully.");
}

bool FileManager::CopyDirectory(const std::string& from, const std::string& where)
{
    try
    {
        const auto src = ResolvePath(from);
        const auto dst = ResolvePath(where);
        if (!fs::exists(src) || !fs::is_directory(src)) { std::cout << Color::Red << "Source directory does not exist." << Color::Reset << '\n'; return false; }
        fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        std::cout << Color::Green << "Directory copied successfully." << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::DeleteDirectory(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_directory(target)) { std::cout << Color::Red << "Directory does not exist." << Color::Reset << '\n'; return false; }
        fs::remove_all(target);
        std::cout << Color::Green << "Directory deleted successfully." << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::ListDirectory(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_directory(target)) { std::cout << Color::Red << "Directory does not exist." << Color::Reset << '\n'; return false; }

        for (const auto& entry : fs::directory_iterator(target))
        {
            if      (entry.is_directory())    std::cout << Color::Cyan  << "[DIR]  " << Color::Reset << ' ' << entry.path().filename().string() << '\n';
            else if (entry.is_regular_file()) std::cout << Color::White << "[FILE] " << Color::Reset << ' ' << entry.path().filename().string() << '\n';
            else                              std::cout << Color::Yellow << "[OTHER]" << Color::Reset << ' ' << entry.path().filename().string() << '\n';
        }
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::GetDirectorySize(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_directory(target)) { std::cout << Color::Red << "Directory does not exist." << Color::Reset << '\n'; return false; }
        std::cout << Color::Yellow << "Directory size: " << CalculateDirectorySize(target) << " bytes" << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

unsigned long long FileManager::CalculateDirectorySize(const fs::path& path)
{
    unsigned long long size = 0;
    for (const auto& entry : fs::recursive_directory_iterator(path))
        if (entry.is_regular_file()) size += entry.file_size();
    return size;
}

// ─── File ────────────────────────────────────────────────────────────────────

bool FileManager::CreateFile(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (fs::exists(target)) { std::cout << Color::Yellow << "File already exists." << Color::Reset << '\n'; return false; }
        if (target.has_parent_path()) fs::create_directories(target.parent_path());
        std::ofstream file(target);
        if (!file.is_open()) { std::cout << Color::Red << "Failed to create file." << Color::Reset << '\n'; return false; }
        std::cout << Color::Green << "File created successfully." << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::RenameFile(const std::string& from, const std::string& where)
{
    try
    {
        const auto src = ResolvePath(from);
        const auto dst = ResolvePath(where);
        if (!fs::exists(src) || !fs::is_regular_file(src)) { std::cout << Color::Red << "Source file does not exist." << Color::Reset << '\n'; return false; }
        if (fs::exists(dst)) { std::cout << Color::Yellow << "Destination file already exists." << Color::Reset << '\n'; return false; }
        if (dst.has_parent_path()) fs::create_directories(dst.parent_path());
        fs::rename(src, dst);
        std::cout << Color::Green << "File renamed successfully." << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::CopyFile(const std::string& from, const std::string& where)
{
    try
    {
        const auto src = ResolvePath(from);
        const auto dst = ResolvePath(where);
        if (!fs::exists(src) || !fs::is_regular_file(src)) { std::cout << Color::Red << "Source file does not exist." << Color::Reset << '\n'; return false; }
        if (dst.has_parent_path()) fs::create_directories(dst.parent_path());
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
        std::cout << Color::Green << "File copied successfully." << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::MoveFile(const std::string& from, const std::string& where)
{
    try
    {
        const auto src = ResolvePath(from);
        const auto dst = ResolvePath(where);
        if (!fs::exists(src) || !fs::is_regular_file(src)) { std::cout << Color::Red << "Source file does not exist." << Color::Reset << '\n'; return false; }
        if (fs::exists(dst)) { std::cout << Color::Yellow << "Destination file already exists." << Color::Reset << '\n'; return false; }
        if (dst.has_parent_path()) fs::create_directories(dst.parent_path());
        fs::rename(src, dst);
        std::cout << Color::Green << "File moved successfully." << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::DeleteFile(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_regular_file(target)) { std::cout << Color::Red << "File does not exist." << Color::Reset << '\n'; return false; }
        fs::remove(target);
        std::cout << Color::Green << "File deleted successfully." << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::GetFileSize(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_regular_file(target)) { std::cout << Color::Red << "File does not exist." << Color::Reset << '\n'; return false; }
        std::cout << Color::Yellow << "File size: " << fs::file_size(target) << " bytes" << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::ReadFile(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_regular_file(target)) { std::cout << Color::Red << "File does not exist." << Color::Reset << '\n'; return false; }
        std::ifstream file(target);
        if (!file.is_open()) { std::cout << Color::Red << "Failed to open file." << Color::Reset << '\n'; return false; }
        std::string line;
        while (std::getline(file, line)) std::cout << line << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::WriteFile(const std::string& path, const std::string& content)
{
    try
    {
        const auto target = ResolvePath(path);
        if (target.has_parent_path()) fs::create_directories(target.parent_path());
        std::ofstream file(target, std::ios::trunc);
        if (!file.is_open()) { std::cout << Color::Red << "Failed to open file for writing." << Color::Reset << '\n'; return false; }
        file << content << '\n';
        std::cout << Color::Green << "File written successfully." << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

bool FileManager::AppendFile(const std::string& path, const std::string& content)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_regular_file(target)) { std::cout << Color::Red << "File does not exist." << Color::Reset << '\n'; return false; }
        std::ofstream file(target, std::ios::app);
        if (!file.is_open()) { std::cout << Color::Red << "Failed to open file for appending." << Color::Reset << '\n'; return false; }
        file << content << '\n';
        std::cout << Color::Green << "Content appended successfully." << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

// ─── Search ──────────────────────────────────────────────────────────────────

bool FileManager::SearchByMask(const std::string& path, const std::string& mask)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_directory(target)) { std::cout << Color::Red << "Directory does not exist." << Color::Reset << '\n'; return false; }

        bool found = false;

        for (const auto& entry : fs::recursive_directory_iterator(target))
        {
            if (entry.is_regular_file() && MatchGlob(entry.path().filename().string(), mask))
            {
                try
                {
                    std::cout << Color::Cyan << fs::relative(entry.path(), rootPath).string() << Color::Reset << '\n';
                }
                catch (...)
                {
                    std::cout << Color::Cyan << entry.path().string() << Color::Reset << '\n';
                }
                found = true;
            }
        }

        if (!found) std::cout << Color::Yellow << "No files found." << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

// ─── Navigation ──────────────────────────────────────────────────────────────

bool FileManager::ChangeDirectory(const std::string& path)
{
    try
    {
        // cd with no argument → go back to workspace root
        if (path.empty())
        {
            currentPath = rootPath;
            std::cout << Color::Cyan << "Directory changed to: " << GetDisplayPath() << Color::Reset << '\n';
            return true;
        }

        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_directory(target)) { std::cout << Color::Red << "Directory does not exist." << Color::Reset << '\n'; return false; }
        currentPath = target;
        std::cout << Color::Cyan << "Directory changed to: " << GetDisplayPath() << Color::Reset << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << Color::Red << "Error: " << e.what() << Color::Reset << '\n'; return false; }
}

void FileManager::PrintCurrentDirectory() const
{
    std::cout << Color::Cyan << "Current directory: " << GetDisplayPath() << Color::Reset << '\n';
}

std::string FileManager::GetCurrentDirectory() const
{
    return currentPath.string();
}

std::string FileManager::GetDisplayPath() const
{
    try
    {
        const auto rel = fs::relative(currentPath, rootPath);
        const auto relStr = rel.string();
        const auto rootName = rootPath.filename().string();
        return rootName + (relStr == "." ? "" : "\\" + relStr);
    }
    catch (...) { return currentPath.string(); }
}

// ─── Completions ─────────────────────────────────────────────────────────────

std::vector<std::string> FileManager::GetCompletions(const std::string& prefix) const
{
    std::string dirPart;
    std::string namePart;

    const auto lastSep = prefix.find_last_of("/\\");
    if (lastSep != std::string::npos)
    {
        dirPart  = prefix.substr(0, lastSep + 1);
        namePart = prefix.substr(lastSep + 1);
    }
    else
    {
        namePart = prefix;
    }

    const fs::path searchDir = currentPath / dirPart;
    if (!fs::exists(searchDir) || !fs::is_directory(searchDir)) return {};

    std::vector<std::string> results;

    try
    {
        for (const auto& entry : fs::directory_iterator(searchDir))
        {
            const std::string name = entry.path().filename().string();
            if (namePart.empty() || StartsWithCI(name, namePart))
            {
                std::string completion = dirPart + name;
                if (entry.is_directory()) completion += '\\';
                results.push_back(completion);
            }
        }
    }
    catch (...) {}

    std::sort(results.begin(), results.end());
    return results;
}
