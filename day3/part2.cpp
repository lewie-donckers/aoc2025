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
    static constexpr auto digits = 12;
    assert(str.size() >= digits);
    assert(str.find_first_not_of("0123456789") == std::string_view::npos);

    auto result = T{0};
    auto it = str.begin();

    for (auto i = digits; i > 0; --i) {
        it = std::ranges::max_element(it, std::prev(str.end(), i - 1), {}, ctoi);
        result = (result * 10) + ctoi(*it);
        ++it;
    }

    return result;
}

T solve(const std::filesystem::path& path) { return sum(read(path) | std::views::transform(fixBank)); }

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};
    std::println("{}", solve(path));
}