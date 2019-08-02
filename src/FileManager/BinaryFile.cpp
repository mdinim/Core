//
// Created by Dániel Molnár on 2019-07-26.
//

#include <FileManager/BinaryFile.hpp>

#include <fstream>
#include <utility>

namespace Core {
BinaryFile::BinaryFile(Core::FileManager::Path path)
    : FileBase(std::move(path)) {}

bool BinaryFile::clear() {
    auto lock = FileManager::lock_unique(_path);

    if (_exists()) {
        std::ofstream stream(_path, std::ios::trunc | std::ios::binary);
        return stream.is_open();
    }

    return false;
}

std::optional<BinaryFile::ByteSequence> BinaryFile::read() const {
    auto lock = FileManager::lock_shared(_path);

    if (std::ifstream stream{_path, std::ios::ate | std::ios::binary}) {
        auto size = stream.tellg();

        ByteSequence sequence(size);
        stream.seekg(0);

        return stream.read(reinterpret_cast<char *>(sequence.data()), size)
                   ? std::optional{sequence}
                   : std::nullopt;
    }

    return std::nullopt;
}

bool BinaryFile::write(const Core::BinaryFile::ByteSequence &bytes) {
    auto lock = FileManager::lock_unique(_path);

    if (std::ofstream stream{_path, std::ios::app}) {
        stream.write(reinterpret_cast<const char *>(bytes.data()),
                     bytes.size() * sizeof(std::byte));
        return true;
    }

    return false;
}

}