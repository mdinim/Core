//
// Created by Dániel Molnár on 2019-07-26.
//

#ifndef CORE_BINARYFILE_HPP
#define CORE_BINARYFILE_HPP

#include <optional>

#include <FileManager/FileManager.hpp>
#include <FileManager/FileBase.hpp>

namespace Core {

class BinaryFile : public FileBase {
  private:
    friend class FileManager;

  public:
    using ByteSequence = std::vector<std::byte>;

    explicit BinaryFile(FileManager::Path path);

    bool clear();

    [[nodiscard]] std::optional<ByteSequence> read() const;

    bool write(const ByteSequence& bytes);
};

} // namespace Core

#endif // CORE_BINARYFILE_HPP
