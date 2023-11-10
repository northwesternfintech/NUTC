#include "file_management.hpp"

#include "logging.hpp"

#include <sys/stat.h>
#include <zip.h>

#include <fstream>
#include <iostream>
#include <sstream>

namespace nutc {
namespace file_mgmt {

void
unzip_file(const std::string& src, const std::string& dest)
{
    // TODO: handle properly
    if (!file_exists(src)) {
        log_e(dev_mode, "Zip file {} does not exist.", src);
        return;
    }

    int err = 0;
    log_i(dev_mode, "Unzipping file {}", src);
    zip* z = zip_open(src.c_str(), 0, &err);
    if (z == nullptr) {
        zip_error_t error;
        zip_error_init_with_code(&error, err);
        log_e(dev_mode, "Error opening zip file: {}", zip_error_strerror(&error));
        zip_error_fini(&error);
        return;
    }

    zip_uint64_t num_entries = static_cast<zip_uint64_t>(zip_get_num_entries(z, 0));
    for (zip_uint64_t i = 0; i < num_entries; i++) {
        struct zip_stat st;
        zip_stat_index(z, i, 0, &st);

        zip_file* f = zip_fopen_index(z, i, 0);
        if (f == nullptr) {
            log_e(dev_mode, "Error opening file in zip: {}", zip_strerror(z));
            continue;
        }

        char* contents = new char[st.size];
        zip_fread(f, contents, st.size);

        // Construct the full path for the unzipped file
        std::string full_path = dest + "/" + st.name;

        // Write the file data to the destination directory
        std::ofstream out_file(full_path, std::ios::binary);
        out_file.write(contents, static_cast<zip_int64_t>(st.size));
        out_file.close();

        delete[] contents;

        zip_fclose(f);
    }
    zip_close(z);
}

bool
create_directory(std::string dir_name)
{
    struct stat st;
    if (stat(dir_name.c_str(), &st) != 0) {
        if (mkdir(dir_name.c_str(), 0777) != 0) {
            log_e(dev_mode, "{}", "Failed to create directory.");
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
    std::ifstream file(filename);
    std::stringstream buffer;

    if (!file) {
        log_e(dev_mode, "File {} does not exist or could not be opened.", filename);
        return "";
    }

    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace file_mgmt
} // namespace nutc
