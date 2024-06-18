#include "lint/lint.hpp"

#include <glaze/core/read.hpp>
#include <glaze/json/write.hpp>
#include <pybind11/embed.h>

#include <cstdlib>

#include <iostream>
#include <string>

int
main()
{
    std::string algo_code;
    std::string line;
    while (std::getline(std::cin, line)) {
        algo_code += line + '\n';
    }

    // Initialize py
    pybind11::initialize_interpreter();

    auto lint_result = nutc::lint::lint(algo_code);

    pybind11::finalize_interpreter();
    std::cout << glz::write_json(lint_result) << "\n";

    return 0;
}
