//
// Created by Dániel Molnár on 2019-07-25.
//

#ifndef CORE_TEXTFILE_HPP
#define CORE_TEXTFILE_HPP

#include <optional>

#include <FileManager/FileBase.hpp>
#include <FileManager/FileManager.hpp>

namespace Core {

class TextFile : public FileBase {
  private:
    friend class FileManager;
  public:
    explicit TextFile(FileManager::Path path);

    bool clear();

    [[nodiscard]] std::optional<std::string> read() const;

    bool append(const std::string& content);
    bool write(const std::string& content);
};

} // namespace Core

#endif // CORE_TEXTFILE_HPP
