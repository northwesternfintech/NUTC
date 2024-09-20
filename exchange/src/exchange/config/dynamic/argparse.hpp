#pragma once

#include "common/util.hpp"

#include <argparse/argparse.hpp>
#include <fmt/format.h>

namespace nutc::exchange {

using mode = common::Mode;

mode process_arguments(int argc, const char** argv);

} // namespace nutc::exchange
