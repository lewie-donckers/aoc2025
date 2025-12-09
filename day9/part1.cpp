#include "shared/shared.hpp"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <print>
#include <vector>

struct Location {
    std::uint64_t x{0};
    std::uint64_t y{0};
};

std::uint64_t area(Location a, Location b) {
    static constexpr auto dist = [](auto i, auto j) { return 1 + ((i < j) ? j - i : i - j); };
    return dist(a.x, b.x) * dist(a.y, b.y);
}

using Locations = std::vector<Location>;

Location parseLocation(std::string_view str) {
    return makeFromRange<Location, 2>(str | std::views::split(',') | as<std::string_view>() |
                                      std::views::transform(parseInt<std::uint64_t>));
}

Locations parse(const std::filesystem::path& path) {
    return yieldLines(path) | std::views::transform(parseLocation) | std::ranges::to<std::vector>();
}

auto solve(const Locations& locations) {
    const auto size = locations.size();
    auto combinations =
        std::views::iota(0uz, size) | std::views::transform([size](auto i) {
            return std::views::iota(i + 1, size) | std::views::transform([i](auto j) { return std::pair{i, j}; });
        }) |
        std::views::join;

    return std::ranges::max(combinations | std::views::transform([&locations](auto p) {
                                const auto [indexA, indexB] = p;
                                const auto& locationA = locations.at(indexA);
                                const auto& locationB = locations.at(indexB);
                                return area(locationA, locationB);
                            }));
}

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};
    const auto problem = parse(path);

    std::println("{}", solve(problem));
}