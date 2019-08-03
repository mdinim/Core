//
// Created by Dániel Molnár on 2019-07-26.
//

#include <FileManager/FileBase.hpp>

#include <FileManager/Exceptions.hpp>
#include <iostream> //todo remove
#include <utility>

namespace Core {
FileBase::FileBase(FileManager::Path path) : _path(std::move(path)) {
    auto lock = FileManager::lock_shared(_path);
    if (std::filesystem::exists(_path) &&
        !std::filesystem::is_regular_file(_path)) {
        throw Exceptions::InvalidPath("Unexpected non-regular file path");
    }
}

bool FileBase::_exists() const {
    return std::filesystem::is_regular_file(_path);
}

bool FileBase::exists() const {
    auto lock = FileManager::lock_shared(_path);
    return _exists();
}

bool FileBase::remove() const {
    auto lock = FileManager::lock_unique(_path);
    if (_exists()) {
        std::filesystem::remove(_path);
        return true;
    }
    return false;
}

bool FileBase::create() {
    auto lock = FileManager::lock_unique(_path);
    if (_exists())
        return false;

    std::ofstream stream(_path);
    return true;
}
}