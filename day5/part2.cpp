#include "shared/shared.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <print>
#include <vector>

using Id = std::uint64_t;
using Ids = std::vector<Id>;

struct IdRange {
    Id start{0};
    Id end{0};
};

using IdRanges = std::vector<IdRange>;

std::tuple<std::string_view, std::string_view> splitInTwo(std::string_view str, char delim) {
    assert(std::ranges::count(str, delim) == 1);

    auto index = str.find(delim);
    assert((index != std::string_view::npos) && (index != 0) && ((index + 1) != str.size()));

    return {str.substr(0, index), str.substr(index + 1)};
}

Id parseId(std::string_view sv) {
    assert(sv.find_first_not_of("0123456789") == std::string_view::npos);

    auto id = Id{};
    std::from_chars(sv.begin(), sv.end(), id);
    return id;
}

IdRange parseIdRange(std::string_view str) {
    return std::apply([](auto... s) { return IdRange{parseId(s)...}; }, splitInTwo(str, '-'));
}

auto parse(const std::filesystem::path& path) {
    auto result = yieldLines(path) | std::views::take_while(std::not_fn(&std::string_view::empty)) |
                  std::views::transform(parseIdRange) | std::ranges::to<std::vector>();

    std::ranges::sort(result, {}, &IdRange::start);

    return result;
}

std::uint64_t solve(const IdRanges& ranges) {
    return std::ranges::fold_left(ranges, std::pair{std::uint64_t{0}, Id{0}},
                                  [](auto p, auto r) {
                                      auto [count, max] = p;
                                      if (max < r.end) {
                                          count += r.end + 1 - std::max(r.start, max + 1);
                                      }
                                      return std::pair{count, std::max(max, r.end)};
                                  })
        .first;
}

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};
    const auto fresh = parse(path);

    std::println("{}", solve(fresh));
}