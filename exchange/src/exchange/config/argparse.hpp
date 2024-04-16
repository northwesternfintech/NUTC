#pragma once

#include "shared/util.hpp"

#include <argparse/argparse.hpp>
#include <fmt/format.h>

namespace nutc {
namespace config {

using mode = util::Mode;
using algorithm = util::algorithm;

std::tuple<mode, std::optional<algorithm>>
process_arguments(int argc, const char** argv);

} // namespace config
} // namespace nutc
