#pragma once
#include <functional>
#include <string>
#include <vector>

namespace component {

struct Dialog_option {
    std::string reply;
    std::function<void()> action;
};

struct Dialog {
    std::vector<std::string> scripts;
    std::size_t current_line;
    std::vector<Dialog_option> options;
};

} // namespace component

namespace comp = component;
