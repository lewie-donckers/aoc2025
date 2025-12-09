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

template <typename T>
struct as_t {};

template <typename T>
auto as() {
    return as_t<T>{};
}

template <typename T, typename R>
auto operator|(R&& range, as_t<T>) {
    return std::forward<R>(range) | std::views::transform([](const auto& e) { return T{e}; });
}

template <typename T>
T parseInt(std::string_view sv) {
    assert(sv.find_first_not_of("0123456789") == std::string_view::npos);

    auto result = T{};
    std::from_chars(sv.begin(), sv.end(), result);
    return result;
}

template <typename T, std::size_t N, typename R>
T makeFromRange(R&& range) {
    static constexpr auto element = [](R& range, auto& it, std::size_t) {
        assert(it != range.end());
        return *(it++);
    };
    static constexpr auto impl = []<std::size_t... Indices>(R& range, std::index_sequence<Indices...>) {
        auto it = range.begin();
        auto result = T{element(range, it, Indices)...};
        assert(it == range.end());
        return result;
    };

    return impl(range, std::make_index_sequence<N>());
}
