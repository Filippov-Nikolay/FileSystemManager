#include "FileManager.h"

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
        if (fs::exists(target)) { std::cout << "Directory already exists.\n"; return false; }
        fs::create_directories(target);
        std::cout << "Directory created successfully.\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::MoveOrRenameDirectory(const std::string& from, const std::string& where, const std::string& successMsg)
{
    try
    {
        const auto src = ResolvePath(from);
        const auto dst = ResolvePath(where);
        if (!fs::exists(src) || !fs::is_directory(src)) { std::cout << "Source directory does not exist.\n"; return false; }
        if (fs::exists(dst)) { std::cout << "Destination path already exists.\n"; return false; }
        fs::rename(src, dst);
        std::cout << successMsg << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
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
        if (!fs::exists(src) || !fs::is_directory(src)) { std::cout << "Source directory does not exist.\n"; return false; }
        fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        std::cout << "Directory copied successfully.\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::DeleteDirectory(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_directory(target)) { std::cout << "Directory does not exist.\n"; return false; }
        fs::remove_all(target);
        std::cout << "Directory deleted successfully.\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::ListDirectory(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_directory(target)) { std::cout << "Directory does not exist.\n"; return false; }

        for (const auto& entry : fs::directory_iterator(target))
        {
            if      (entry.is_directory())    std::cout << "[DIR]   " << entry.path().filename().string() << '\n';
            else if (entry.is_regular_file()) std::cout << "[FILE]  " << entry.path().filename().string() << '\n';
            else                              std::cout << "[OTHER] " << entry.path().filename().string() << '\n';
        }
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::GetDirectorySize(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_directory(target)) { std::cout << "Directory does not exist.\n"; return false; }
        std::cout << "Directory size: " << CalculateDirectorySize(target) << " bytes\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
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
        if (fs::exists(target)) { std::cout << "File already exists.\n"; return false; }
        if (target.has_parent_path()) fs::create_directories(target.parent_path());
        std::ofstream file(target);
        if (!file.is_open()) { std::cout << "Failed to create file.\n"; return false; }
        std::cout << "File created successfully.\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::RenameFile(const std::string& from, const std::string& where)
{
    try
    {
        const auto src = ResolvePath(from);
        const auto dst = ResolvePath(where);
        if (!fs::exists(src) || !fs::is_regular_file(src)) { std::cout << "Source file does not exist.\n"; return false; }
        if (fs::exists(dst)) { std::cout << "Destination file already exists.\n"; return false; }
        if (dst.has_parent_path()) fs::create_directories(dst.parent_path());
        fs::rename(src, dst);
        std::cout << "File renamed successfully.\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::CopyFile(const std::string& from, const std::string& where)
{
    try
    {
        const auto src = ResolvePath(from);
        const auto dst = ResolvePath(where);
        if (!fs::exists(src) || !fs::is_regular_file(src)) { std::cout << "Source file does not exist.\n"; return false; }
        if (dst.has_parent_path()) fs::create_directories(dst.parent_path());
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
        std::cout << "File copied successfully.\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::MoveFile(const std::string& from, const std::string& where)
{
    try
    {
        const auto src = ResolvePath(from);
        const auto dst = ResolvePath(where);
        if (!fs::exists(src) || !fs::is_regular_file(src)) { std::cout << "Source file does not exist.\n"; return false; }
        if (fs::exists(dst)) { std::cout << "Destination file already exists.\n"; return false; }
        if (dst.has_parent_path()) fs::create_directories(dst.parent_path());
        fs::rename(src, dst);
        std::cout << "File moved successfully.\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::DeleteFile(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_regular_file(target)) { std::cout << "File does not exist.\n"; return false; }
        fs::remove(target);
        std::cout << "File deleted successfully.\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::GetFileSize(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_regular_file(target)) { std::cout << "File does not exist.\n"; return false; }
        std::cout << "File size: " << fs::file_size(target) << " bytes\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::ReadFile(const std::string& path)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_regular_file(target)) { std::cout << "File does not exist.\n"; return false; }
        std::ifstream file(target);
        if (!file.is_open()) { std::cout << "Failed to open file.\n"; return false; }
        std::string line;
        while (std::getline(file, line)) std::cout << line << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::WriteFile(const std::string& path, const std::string& content)
{
    try
    {
        const auto target = ResolvePath(path);
        if (target.has_parent_path()) fs::create_directories(target.parent_path());
        std::ofstream file(target, std::ios::trunc);
        if (!file.is_open()) { std::cout << "Failed to open file for writing.\n"; return false; }
        file << content << '\n';
        std::cout << "File written successfully.\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

bool FileManager::AppendFile(const std::string& path, const std::string& content)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_regular_file(target)) { std::cout << "File does not exist.\n"; return false; }
        std::ofstream file(target, std::ios::app);
        if (!file.is_open()) { std::cout << "Failed to open file for appending.\n"; return false; }
        file << content << '\n';
        std::cout << "Content appended successfully.\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

// ─── Search ──────────────────────────────────────────────────────────────────

bool FileManager::SearchByMask(const std::string& path, const std::string& mask)
{
    try
    {
        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_directory(target)) { std::cout << "Directory does not exist.\n"; return false; }

        bool found = false;

        for (const auto& entry : fs::recursive_directory_iterator(target))
        {
            if (entry.is_regular_file() && MatchGlob(entry.path().filename().string(), mask))
            {
                try
                {
                    std::cout << fs::relative(entry.path(), rootPath).string() << '\n';
                }
                catch (...)
                {
                    std::cout << entry.path().string() << '\n';
                }
                found = true;
            }
        }

        if (!found) std::cout << "No files found.\n";
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
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
            std::cout << "Directory changed to: " << GetDisplayPath() << '\n';
            return true;
        }

        const auto target = ResolvePath(path);
        if (!fs::exists(target) || !fs::is_directory(target)) { std::cout << "Directory does not exist.\n"; return false; }
        currentPath = target;
        std::cout << "Directory changed to: " << GetDisplayPath() << '\n';
        return true;
    }
    catch (const fs::filesystem_error& e) { std::cout << "Error: " << e.what() << '\n'; return false; }
}

void FileManager::PrintCurrentDirectory() const
{
    std::cout << "Current directory: " << GetDisplayPath() << '\n';
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
