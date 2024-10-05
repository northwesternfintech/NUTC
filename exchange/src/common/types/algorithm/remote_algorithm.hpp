#pragma once

#include "base_algorithm.hpp"
#include "common/compilation/compile_cpp.hpp"
#include "common/file_operations/file_operations.hpp"

#include <boost/filesystem/operations.hpp>

#include <fstream>

namespace nutc::common {
// TODO
class RemoteAlgorithm : public BaseAlgorithm {
    std::string id_;
    std::string algo_data_;

public:
    RemoteAlgorithm(AlgoLanguage language, std::string algo_id, std::string algo_data) :
        BaseAlgorithm{language}, id_{std::move(algo_id)},
        algo_data_{std::move(algo_data)}
    {}

    std::string
    get_algo_string() const
    {
        if (get_language() == AlgoLanguage::cpp) {
            // TODO: clean up
            std::string binary_output =
                (boost::filesystem::temp_directory_path()
                 / boost::filesystem::unique_path("%%%%-%%%%-%%%%.tmp"))
                    .string();

            std::ofstream algo_file(binary_output);
            algo_file << algo_data_ << std::flush;
            algo_file.close();

            return common::read_file_content(common::compile_cpp(binary_output));
        }
        if (get_language() == AlgoLanguage::python) {
            return algo_data_;
        }

        throw std::runtime_error("Unknown algo language");
    }

    std::string
    get_id() const
    {
        return id_;
    }
};
} // namespace nutc::common
