#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <generator>
#include <print>

using T = std::uint64_t;

template <typename R>
T sum(R&& range) {
    return std::ranges::fold_left(std::forward<R>(range), T{0}, std::plus<T>{});
}

std::generator<std::string_view> read(const std::filesystem::path& path) {
    auto input = std::ifstream{path};
    auto line = std::string{};
    while (std::getline(input, line)) {
        co_yield line;
    }
}

T ctoi(char c) {
    auto result = T{};
    std::from_chars<T>(&c, &c + 1, result);
    return result;
}

T fixBank(std::string_view str) {
    assert(str.size() >= 2);
    assert(str.find_first_not_of("0123456789") == std::string_view::npos);

    const auto it1 = std::ranges::max_element(str.begin(), std::prev(str.end()), {}, ctoi);
    const auto it2 = std::ranges::max_element(std::next(it1), str.end(), {}, ctoi);

    return (ctoi(*it1) * 10) + ctoi(*it2);
}

T solve(const std::filesystem::path& path) { return sum(read(path) | std::views::transform(fixBank)); }

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};
    std::println("{}", solve(path));
}