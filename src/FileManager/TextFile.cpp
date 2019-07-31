//
// Created by Dániel Molnár on 2019-07-25.
//

#include <FileManager/TextFile.hpp>

#include <fstream>
#include <utility>

namespace Core {
TextFile::TextFile(FileManager::Path path)
    : FileBase(std::move(path)) {}

bool TextFile::clear() {
    auto lock = FileManager::lock_unique(_path);

    if (_exists()) {
        std::ofstream stream(_path, std::ios::trunc);
        return stream.is_open();
    }

    return false;
}

std::optional<std::string> TextFile::read() const {
    auto lock = FileManager::lock_shared(_path);

    if (std::ifstream stream{_path, std::ios::ate}) {
        auto size = stream.tellg();

        std::string result(size, '\0');
        stream.seekg(0);

        return stream.read(result.data(), size) ? std::optional{result}
                                             : std::nullopt;
    }

    return std::nullopt;
}

bool TextFile::append(const std::string &content) {
    auto lock = FileManager::lock_unique(_path);

    if (std::ofstream stream{_path, std::ios::app}) {
        stream << content;
        return true;
    }

    return false;
}

bool TextFile::write(const std::string &content) {
    auto lock = FileManager::lock_unique(_path);

    if (std::ofstream stream{_path}) {
        stream << content;
        return true;
    }

    return false;
}

}