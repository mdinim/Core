//
// Created by Dániel Molnár on 2019-07-22.
//

#ifndef CORE_FILEMANAGER_HPP
#define CORE_FILEMANAGER_HPP

#include <string>

namespace Core {
class FileManager {
private:

public:
    static void read_file(const std::string& path);
};
}

#endif //CORE_FILEMANAGER_HPP
