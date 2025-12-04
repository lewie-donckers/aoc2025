#include "shared/shared.hpp"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <print>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

struct Location {
    int row{0};
    int col{0};
};

struct Offset {
    int row{0};
    int col{0};
};

struct Size {
    int rows{0};
    int cols{0};
};

bool isValid(Location l, Size s) { return (l.col >= 0) && (l.col < s.cols) && (l.row >= 0) && (l.row < s.rows); }

Location operator+(Location l, Offset o) { return Location{.row = l.row + o.row, .col = l.col + o.col}; }

static constexpr auto neighbors =
    std::array<Offset, 8>{{{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}}};

auto validNeighbors(Location l, Size s) {
    return neighbors | std::views::transform([l](auto o) { return l + o; }) |
           std::views::filter([s](auto l) { return isValid(l, s); });
}

auto locations(Size s) {
    return std::views::cartesian_product(std::views::iota(0, s.rows), std::views::iota(0, s.cols)) |
           std::views::transform([](auto t) { return Location{std::get<0>(t), std::get<1>(t)}; });
}

class Grid {
public:
    void append(std::string_view line) {
        assert(!line.empty());
        assert(line.find_first_not_of(".@") == std::string::npos);
        assert(data_.empty() || (line.size() == data_.front().size()));

        data_.emplace_back(line);
    }

    Size getSize() const {
        assert(!data_.empty());
        return Size{.rows = static_cast<int>(data_.size()), .cols = static_cast<int>(data_.front().size())};
    }

    char element(Location l) const {
        assert(!data_.empty());
        assert(isValid(l, getSize()));

        return data_[l.row][l.col];
    }

private:
    std::vector<std::string> data_;
};

bool isRoll(char c) { return c == '@'; }

auto parse(const std::filesystem::path& path) {
    auto result = Grid{};

    for (auto line : yieldLines(path)) {
        result.append(line);
    }

    return result;
}

auto solve(const Grid& grid) {
    const auto element = std::bind_front(&Grid::element, &grid);
    const auto size = grid.getSize();

    return std::ranges::count(locations(size) | std::views::transform([&](Location l) {
                                  return isRoll(element(l)) &&
                                         std::ranges::count_if(validNeighbors(l, size) | std::views::transform(element),
                                                               isRoll) < 4;
                              }),
                              true);
}

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};
    const auto grid = parse(path);

    std::println("{}", solve(grid));
}