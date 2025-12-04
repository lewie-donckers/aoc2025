#pragma once

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <generator>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

std::generator<std::string_view> yieldLines(const std::filesystem::path& path) {
    auto input = std::ifstream{path};
    auto line = std::string{};
    while (std::getline(input, line)) {
        co_yield line;
    }
}
