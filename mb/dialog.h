#pragma once
#include <string>
#include <vector>

namespace component {

struct Dialog {
    std::vector<std::string> scripts;
    std::size_t current_line;
    bool is_active;
};

} // namespace component

namespace comp = component;
