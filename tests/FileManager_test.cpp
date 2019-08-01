//
// Created by Dániel Molnár on 2019-07-22.
//

#include <FileManager/BinaryFile.hpp>
#include <FileManager/FileManager.hpp>
#include <FileManager/TextFile.hpp>

#include <fstream>
#include <map>
#include <vector>

#include <FileManager/Exceptions.hpp>
#include <gtest/gtest.h>

#include <Utils/Utils.hpp>
#include <Utils/TestUtil.hpp>

namespace fs = std::filesystem;

class FileTestFixture : public ::testing::Test {
  protected:
    FileTestFixture() : temp_file_manager({fs::temp_directory_path()}) {
        temp_file_exists_on_start = fs::temp_directory_path() / "exists";
        temp_file_missing_on_start = fs::temp_directory_path() / "missing";
        temp_text_file = fs::temp_directory_path() / "temp.txt";
        temp_text_file_with_content =
            fs::temp_directory_path() / "temp_content.txt";
        temp_binary_file = fs::temp_directory_path() / "temp.data";
        temp_binary_file_with_content =
            fs::temp_directory_path() / "temp_content.data";
        temp_inaccessible_file = fs::temp_directory_path() / "inaccessible";
    }

    Core::FileManager temp_file_manager;

    fs::path temp_file_exists_on_start;
    fs::path temp_file_missing_on_start;

    fs::path temp_text_file;
    fs::path temp_text_file_with_content;
    std::string text_file_content =
        "This is the content you are dealing with\nWith multiple "
        "lines and all, but no endl for additional tests";

    fs::path temp_binary_file;
    fs::path temp_binary_file_with_content;
    Core::BinaryFile::ByteSequence binary_file_content = {
        12, static_cast<std::byte>(12)};

    fs::path temp_inaccessible_file;

    void CreateIfMissing(const fs::path &path) { std::ofstream stream(path); }

    void RemoveIfPresent(const fs::path &path) {
        if (fs::exists(path))
            fs::remove(path);
    }

    void SetUp() override {
        RemoveIfPresent(temp_file_missing_on_start);
        RemoveIfPresent(temp_text_file);
        RemoveIfPresent(temp_binary_file);
        CreateIfMissing(temp_file_exists_on_start);
        CreateIfMissing(temp_inaccessible_file);
        fs::permissions(temp_inaccessible_file, fs::perms::none);

        std::ofstream stream(temp_text_file_with_content);
        stream << text_file_content;

        std::ofstream bstream(temp_binary_file_with_content);
        bstream.write(
            reinterpret_cast<const char *>(binary_file_content.data()),
            binary_file_content.size() *
                sizeof(Core::BinaryFile::ByteSequence::value_type));
    }

    void TearDown() override {
        fs::permissions(temp_inaccessible_file, fs::perms::all);

        RemoveIfPresent(temp_file_missing_on_start);
        RemoveIfPresent(temp_file_exists_on_start);
        RemoveIfPresent(temp_text_file);
        RemoveIfPresent(temp_text_file_with_content);
        RemoveIfPresent(temp_binary_file);
        RemoveIfPresent(temp_binary_file_with_content);
        RemoveIfPresent(temp_inaccessible_file);
    }
};

namespace {
struct dummy_struct {
    int a = 0;
    int b = 2;
    int c = 412;
};

} // namespace

using namespace Core;

TEST_F(FileTestFixture, CanCreateFiles) {
    TEST_INFO << "Inaccessible file location: " << std::endl;
    TEST_INFO << temp_inaccessible_file << std::endl;

    TEST_INFO << "Existing file location: " << std::endl;
    TEST_INFO << temp_file_exists_on_start << std::endl;

    ASSERT_FALSE(fs::exists(temp_file_missing_on_start));
    FileBase file(temp_file_missing_on_start);
    ASSERT_FALSE(file.exists());
    ASSERT_TRUE(file.create());
    ASSERT_TRUE(file.exists());

    ASSERT_TRUE(fs::is_regular_file(temp_file_missing_on_start));
}

TEST_F(FileTestFixture, CanRemoveFiles) {
    ASSERT_TRUE(fs::exists(temp_file_exists_on_start));
    FileBase file(temp_file_exists_on_start);
    ASSERT_TRUE(file.remove());
    ASSERT_FALSE(fs::exists(temp_file_exists_on_start));
}

TEST_F(FileTestFixture, IndicatesErrors) {
    FileBase existing_file(temp_file_exists_on_start);
    FileBase missing_file(temp_file_missing_on_start);
    ASSERT_FALSE(existing_file.create());
    ASSERT_FALSE(missing_file.remove());
    ASSERT_THROW(FileBase(fs::temp_directory_path()), Exceptions::InvalidPath);
}

TEST_F(FileTestFixture, TextFileWriteAppend) {
    TextFile text_file(temp_text_file);
    ASSERT_FALSE(text_file.exists());

    ASSERT_TRUE(text_file.write("Hello World"));
    ASSERT_TRUE(text_file.exists());

    std::ifstream stream(temp_text_file);
    ASSERT_TRUE(stream.is_open());

    std::string line;
    std::getline(stream, line);
    ASSERT_EQ(line, "Hello World");

    ASSERT_TRUE(text_file.append("!"));
    stream.seekg(0);
    std::getline(stream, line);
    ASSERT_EQ(line, "Hello World!");

    text_file.clear();
    stream.seekg(0);
    std::getline(stream, line);
    ASSERT_EQ(line, "");
}

TEST_F(FileTestFixture, TextFileRead) {
    TextFile text_file(temp_text_file_with_content);
    ASSERT_TRUE(text_file.exists());

    auto content = text_file.read();
    ASSERT_EQ(content, text_file_content);
}

TEST_F(FileTestFixture, BinaryFileWrite) {
    std::vector<unsigned short> data = {1,   24, 32,  59, 29,
                                        158, 59, 255, 0,  214};
    BinaryFile::ByteSequence binary_data;
    std::transform(
        data.begin(), data.end(), std::back_inserter(binary_data),
        [](const auto &item) { return static_cast<std::byte>(item); });

    BinaryFile binary_file(temp_binary_file);

    ASSERT_FALSE(binary_file.exists());
    ASSERT_TRUE(binary_file.write(binary_data));
    ASSERT_TRUE(binary_file.exists());

    auto content = binary_file.read();
    ASSERT_TRUE(content);
    ASSERT_EQ(*content, binary_data);

    ASSERT_TRUE(binary_file.clear());
    content = binary_file.read();
    ASSERT_EQ(*content, BinaryFile::ByteSequence());
}

TEST_F(FileTestFixture, BinaryFileRead) {
    BinaryFile file(temp_binary_file_with_content);
    ASSERT_TRUE(file.exists());
    auto content = file.read();
    ASSERT_TRUE(content);
    ASSERT_EQ(*content, binary_file_content);
}

TEST_F(FileTestFixture, Errors) {
    TextFile text_file(temp_inaccessible_file);
    TextFile missing_temp_file(temp_text_file);
    ASSERT_FALSE(text_file.write("Gibberish"));
    ASSERT_FALSE(text_file.append("Gibberish"));
    ASSERT_FALSE(missing_temp_file.clear());
    ASSERT_FALSE(text_file.read());

    BinaryFile binary_file(temp_inaccessible_file);
    BinaryFile missing_binary_file(temp_binary_file);
    ASSERT_FALSE(binary_file.write({static_cast<std::byte>(0)}));
    ASSERT_FALSE(binary_file.clear());
    ASSERT_FALSE(missing_binary_file.clear());
    ASSERT_FALSE(binary_file.read());
}

TEST_F(FileTestFixture, FactoryFunctions) {
    ASSERT_TRUE(temp_file_manager.text_file(temp_file_exists_on_start));
    ASSERT_TRUE(temp_file_manager.binary_file(temp_file_exists_on_start));
    ASSERT_FALSE(temp_file_manager.text_file(temp_file_missing_on_start));
    ASSERT_FALSE(temp_file_manager.binary_file(temp_file_missing_on_start));

    ASSERT_FALSE(FileManager::text_file(temp_file_missing_on_start, false));
    ASSERT_FALSE(FileManager::binary_file(temp_file_missing_on_start, false));

    ASSERT_FALSE(FileManager::binary_file("relative.data", false));
    ASSERT_FALSE(FileManager::text_file("relative.txt", false));

    ASSERT_TRUE(temp_file_manager.text_file(temp_file_missing_on_start, true));
    {
        TextFile file(temp_file_missing_on_start);
        ASSERT_TRUE(file.exists());
        ASSERT_TRUE(file.remove());
    }

    ASSERT_TRUE(
        temp_file_manager.binary_file(temp_file_missing_on_start, true));
    {
        BinaryFile file(temp_file_missing_on_start);
        ASSERT_TRUE(file.exists());
        ASSERT_TRUE(file.remove());
    }
}

// TODO move this to a separate *_test file
TEST(UtilsTest, Test) {
    struct dummy {};
    class vector_like : public std::vector<int> {};
    ASSERT_TRUE(is_container<std::vector<int>>::value);
    bool is_map_container = is_container<std::map<int, int>>::value;
    ASSERT_TRUE(is_map_container);
    ASSERT_TRUE(is_container<vector_like>::value);
    ASSERT_FALSE(is_container<int>::value);
    ASSERT_FALSE(is_container<dummy>::value);
}