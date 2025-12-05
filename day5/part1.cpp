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

bool isInRange(Id id, IdRange range) { return (id >= range.start) && (id <= range.end); }
bool isInRanges(Id id, const IdRanges& ranges) { return std::ranges::any_of(ranges, std::bind_front(isInRange, id)); }

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
    auto parts = yieldLines(path) | std::views::lazy_split(std::string_view{});

    auto it = parts.begin();
    assert(it != parts.end());
    auto fresh = *it | std::views::transform(parseIdRange) | std::ranges::to<std::vector>();

    ++it;
    assert(it != parts.end());
    auto available = *it | std::views::transform(parseId) | std::ranges::to<std::vector>();

    return std::tuple{fresh, available};
}

int solve(const IdRanges& fresh, const Ids& available) {
    const auto isFresh = std::bind_back(isInRanges, std::cref(fresh));

    return std::ranges::count_if(available, isFresh);
}

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};
    const auto [fresh, available] = parse(path);

    std::println("{}", solve(fresh, available));
}