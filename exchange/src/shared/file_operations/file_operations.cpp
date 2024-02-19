#include "file_operations.hpp"

#include <sys/stat.h>
#include <zip.h>

#include <cassert>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

// Low level file operations. Decoupled from functionality of exchange
namespace nutc {
namespace file_ops {

void
unzip_file(const std::string& src, const std::string& dest)
{
    assert(file_exists(src));

    int err = 0;
    zip* zipfile = zip_open(src.c_str(), 0, &err);

    assert(zipfile != nullptr);

    auto num_entries = static_cast<zip_uint64_t>(zip_get_num_entries(zipfile, 0));
    for (zip_uint64_t i = 0; i < num_entries; i++) {
        struct zip_stat zstat {};

        zip_stat_index(zipfile, i, 0, &zstat);

        zip_file* finto = zip_fopen_index(zipfile, i, 0);
        assert(finto != nullptr);

        char* contents = new char[zstat.size];
        zip_fread(finto, contents, zstat.size);

        // Construct the full path for the unzipped file
        std::string full_path = dest + "/" + zstat.name;

        // Write the file data to the destination directory
        std::ofstream out_file(full_path, std::ios::binary);
        out_file.write(contents, static_cast<zip_int64_t>(zstat.size));
        out_file.close();

        delete[] contents;

        zip_fclose(finto);
    }
    zip_close(zipfile);
}

bool
create_directory(const std::string& dir)
{
    std::filesystem::path dir_path{dir};
    if (!std::filesystem::exists(dir_path)) {
        if (!std::filesystem::create_directory(dir_path)) {
            return false;
        }
    }
    return true;
}

bool
file_exists(const std::string& filename) noexcept
{
    std::ifstream file(filename);
    return file.good();
}

std::string
read_file_content(const std::string& filename)
{
    assert(file_exists(filename));
    std::ifstream file(filename);
    std::stringstream buffer;

    assert(file.good());

    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace file_ops
} // namespace nutc
