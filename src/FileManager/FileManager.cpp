//
// Created by Dániel Molnár on 2019-07-22.
//

#include <FileManager/FileManager.hpp>

#include <utility>

#include <FileManager/BinaryFile.hpp>
#include <FileManager/TextFile.hpp>

namespace {

template <class T>
static std::optional<T> file(const Core::FileManager::Path &path, bool create) {
    namespace fs = std::filesystem;

    if (path.is_relative())
        return std::nullopt;

    if (!fs::is_regular_file(path)) {
        if (create) {
            fs::create_directories(path.parent_path());
            std::ofstream stream(path);
        } else {
            return std::nullopt;
        }
    }

    return T(path);
}

} // namespace

namespace Core {

std::mutex FileManager::_guards_guard = {};
std::map<FileManager::Path, std::shared_mutex> FileManager::_guards = {};

FileManager::FileManager(std::vector<Path> search_paths)
    : _search_paths(std::move(search_paths)) {}

std::optional<FileManager::Path>
FileManager::find(const std::string &file_name) const {
    namespace fs = std::filesystem;

    for (const auto &directory : _search_paths) {
        if (auto path = directory / file_name; fs::is_regular_file(path))
            return path;
    }
    return std::nullopt;
}

std::shared_mutex &FileManager::get_guard(const std::filesystem::path &path) {
    std::unique_lock<std::mutex> lock(_guards_guard);
    return _guards[path];
}

std::optional<TextFile> FileManager::text_file(const std::string &file_name) {
    namespace fs = std::filesystem;

    if (auto maybe_path = find(file_name))
        return FileManager::text_file(*maybe_path, false);
    return std::nullopt;
}

std::optional<TextFile> FileManager::text_file(const FileManager::Path &path,
                                               bool create) {
    return file<TextFile>(path, create);
}

std::optional<BinaryFile>
FileManager::binary_file(const std::string &file_name) {
    namespace fs = std::filesystem;

    if (auto maybe_path = find(file_name))
        return FileManager::binary_file(*maybe_path, false);
    return std::nullopt;
}

std::optional<BinaryFile>
FileManager::binary_file(const FileManager::Path &path, bool create) {
    return file<BinaryFile>(path, create);
}

}