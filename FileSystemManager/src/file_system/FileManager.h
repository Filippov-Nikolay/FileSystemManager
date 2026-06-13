#pragma once

#include <filesystem>
#include <string>
#include <vector>

class FileManager
{
public:
    explicit FileManager(const std::string& rootPath);

    bool ChangeDirectory(const std::string& path);
    void PrintCurrentDirectory() const;
    std::string GetCurrentDirectory() const;
    std::string GetDisplayPath() const;
    std::vector<std::string> GetCompletions(const std::string& prefix) const;

    bool CreateDirectory(const std::string& path);
    bool RenameDirectory(const std::string& from, const std::string& where);
    bool CopyDirectory(const std::string& from, const std::string& where);
    bool MoveDirectory(const std::string& from, const std::string& where);
    bool DeleteDirectory(const std::string& path);
    bool ListDirectory(const std::string& path);
    bool GetDirectorySize(const std::string& path);

    bool CreateFile(const std::string& path);
    bool RenameFile(const std::string& from, const std::string& where);
    bool CopyFile(const std::string& from, const std::string& where);
    bool MoveFile(const std::string& from, const std::string& where);
    bool DeleteFile(const std::string& path);
    bool GetFileSize(const std::string& path);
    bool ReadFile(const std::string& path);
    bool WriteFile(const std::string& path, const std::string& content);
    bool AppendFile(const std::string& path, const std::string& content);

    bool SearchByMask(const std::string& path, const std::string& mask);

private:
    std::filesystem::path rootPath;
    std::filesystem::path currentPath;

    std::filesystem::path ResolvePath(const std::string& path);
    bool MoveOrRenameDirectory(const std::string& from, const std::string& where, const std::string& successMsg);
    unsigned long long CalculateDirectorySize(const std::filesystem::path& path);
};
