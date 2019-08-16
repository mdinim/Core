//
// Created by Dániel Molnár on 2019-07-26.
//

#ifndef CORE_FILEBASE_HPP
#define CORE_FILEBASE_HPP

#include <FileManager/FileManager.hpp>

namespace Core {

class FileBase {

  private:
  protected:
    const FileManager::Path _path;

    bool _exists() const;
  public:
    explicit FileBase(FileManager::Path path);

    bool exists() const;

    bool remove() const;

    const FileManager::Path& path() const {
        return _path;
    }

    bool create();
};

} // namespace Core

#endif // CORE_FILEBASE_HPP
