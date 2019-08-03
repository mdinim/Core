//
// Created by Dániel Molnár on 2019-07-22.
//

#ifndef CORE_FILEMANAGER_HPP
#define CORE_FILEMANAGER_HPP

#include <filesystem>
#include <vector>
#include <fstream>
#include <map>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>

namespace Core {

class TextFile;
class BinaryFile;
class FileBase;

class FileManager {
  public:
    using Path = std::filesystem::path;

  private:
    friend class TextFile;
    friend class BinaryFile;
    friend class FileBase;

    using SharedLock = std::shared_lock<std::shared_mutex>;
    using UniqueLock = std::unique_lock<std::shared_mutex>;

    static std::mutex _guards_guard;
    static std::map<Path, std::shared_mutex> _guards;
    static std::shared_mutex &get_guard(const Path &path);

    inline static SharedLock lock_shared(const Path &path) {
        return SharedLock(get_guard(path));
    }

    inline static UniqueLock lock_unique(const Path &path) {
        return UniqueLock(get_guard(path));
    }

    const std::vector<Path> _search_paths;


  public:
    explicit FileManager(std::vector<Path> search_paths);

    [[nodiscard]] std::optional<Path> find(const std::string &file_name) const;
    std::optional<TextFile> text_file(const std::string &file_name);
    std::optional<BinaryFile> binary_file(const std::string &file_name);

    static std::optional<TextFile> text_file(const Path &path, bool create);
    static std::optional<BinaryFile> binary_file(const Path &path, bool create);
};

} // namespace Core

#endif // CORE_FILEMANAGER_HPP
