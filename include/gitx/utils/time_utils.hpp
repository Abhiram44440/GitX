#pragma once

#include <string>
#include <cstdint>

namespace gitx {

std::string current_timestamp_iso8601();
std::string format_timestamp(uint64_t seconds_since_epoch);

} // namespace gitx
