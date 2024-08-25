#pragma once

#include "shared/util.hpp"

#include <argparse/argparse.hpp>
#include <fmt/format.h>

namespace nutc::exchange {

using mode = shared::Mode;
using algorithm = shared::algorithm;

mode process_arguments(int argc, const char** argv);

} // namespace nutc::exchange
