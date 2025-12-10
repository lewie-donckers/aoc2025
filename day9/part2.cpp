#include "shared/shared.hpp"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <print>
#include <variant>
#include <vector>

struct Row {
    std::uint64_t r{0};

    constexpr auto operator<=>(const Row&) const noexcept = default;
};

struct Col {
    std::uint64_t c{0};

    constexpr auto operator<=>(const Col&) const noexcept = default;
};

struct Location {
    Col col;
    Row row;
};

struct Rectangle {
    Location a;
    Location b;
};

std::uint64_t area(Rectangle r) {
    static constexpr auto dist = [](auto i, auto j) { return 1 + ((i < j) ? j - i : i - j); };
    return dist(r.a.row.r, r.b.row.r) * dist(r.a.col.c, r.b.col.c);
}

using Locations = std::vector<Location>;

Location parseLocation(std::string_view str) {
    return makeFromRange<Location, 2>(str | std::views::split(',') | as<std::string_view>() |
                                      std::views::transform(parseInt<std::uint64_t>));
}

Locations parse(const std::filesystem::path& path) {
    return yieldLines(path) | std::views::transform(parseLocation) | std::ranges::to<std::vector>();
}

template <typename T, typename R>
std::generator<T> yieldMerged(R& r1, R& r2) {
    auto i1 = r1.begin();
    auto i2 = r2.begin();

    while ((i1 != r1.end()) && (i2 != r2.end())) {
        if (*i1 < *i2) {
            co_yield *i1++;
        } else {
            co_yield *i2++;
        }
    }

    while (i1 != r1.end()) {
        co_yield *i1++;
    }

    while (i2 != r2.end()) {
        co_yield *i2++;
    }
}

class Floor {
public:
    void addLine(Location a, Location b) {
        assert((a.row == b.row) || (a.col == b.col));

        if (a.col == b.col) {
            return;
        }

        const auto index = a.row.r;
        if (index >= data_.size()) {
            data_.resize(index + 1);
        }

        const auto [minCol, maxCol] = std::minmax(a.col, b.col);
        data_.at(a.row.r).emplace_back(minCol, maxCol);
    }

    void normalize() {
        std::ranges::for_each(data_, std::ranges::sort);

        auto curState = std::vector<ColRange>{};
        std::ranges::for_each(data_, [&curState](auto& curRow) {
            auto nextRow = std::vector<ColRange>{};
            auto nextState = std::vector<ColRange>{};

            for (auto toProcess : yieldMerged<ColRange>(curState, curRow)) {
                [&nextRow](auto b) {
                    if (nextRow.empty()) {
                        nextRow.push_back(b);
                        return;
                    }

                    auto& a = nextRow.back();
                    assert(a.begin <= b.begin);

                    if (a.begin == b.begin) {
                        if (a.end < b.end) {
                            // -aaa---
                            // -bbbbb-
                            // ---xxx-
                            a = b;
                        }
                    } else if (a.end == b.begin) {
                        // -aaa---
                        // ---bbb-
                        // -xxxxx-
                        a.end = b.end;
                    } else if (a.end < b.begin) {
                        // -aaa-----
                        // -----bbb-
                        // -xxx-xxx-
                        nextRow.push_back(b);
                    }
                }(toProcess);

                [&nextState](auto b) {
                    if (nextState.empty()) {
                        nextState.push_back(b);
                        return;
                    }

                    auto& a = nextState.back();
                    assert(a.begin <= b.begin);

                    if (a == b) {
                        // -aaa-
                        // -bbb-
                        // -----
                        nextState.pop_back();
                    } else if (a.begin == b.begin) {
                        if (a.end < b.end) {
                            // -aaa---
                            // -bbbbb-
                            // ---xxx-
                            a.begin = a.end;
                            a.end = b.end;
                        } else if (a.end > b.end) {
                            // -aaaaa-
                            // -bbb---
                            // ---xxx-
                            a.begin = b.end;
                        } else {
                            assert(false);
                        }
                    } else if (a.end == b.end) {
                        // -aaaaa-
                        // ---bbb-
                        // -xxx---
                        a.end = b.begin;
                    } else if (a.end == b.begin) {
                        // -aaa---
                        // ---bbb-
                        // -xxxxx-
                        a.end = b.end;
                    } else if (a.end < b.begin) {
                        // -aaa-----
                        // -----bbb-
                        // -xxx-xxx-
                        assert(a.end.c + 1 < b.begin.c);
                        nextState.push_back(b);
                    } else if ((a.begin < b.begin) && (a.end > b.end)) {
                        // -aaaaaaa-
                        // ---bbb---
                        // -xxx-xxx-
                        a.end = b.begin;
                        nextState.emplace_back(b.end, a.end);
                    } else {
                        assert(false);
                    }
                }(toProcess);
            }

            curState = nextState;
            curRow = nextRow;
        });
    }

    bool isTiled(Rectangle rect) const {
        const auto [rowMin, rowMax] = std::minmax(rect.a.row, rect.b.row);
        const auto rows = std::span{std::next(data_.begin(), rowMin.r), 1 + rowMax.r - rowMin.r};
        const auto cols = std::make_from_tuple<ColRange>(std::minmax(rect.a.col, rect.b.col));

        return std::ranges::all_of(rows, [cols](const auto& row) {
            return std::ranges::any_of(
                row, [cols](auto range) { return (range.begin <= cols.begin) && (range.end >= cols.end); });
        });
    }

private:
    struct ColRange {
        Col begin;
        Col end;

        constexpr auto operator<=>(const ColRange&) const noexcept = default;
    };

    std::vector<std::vector<ColRange>> data_;
};

Floor makeFloor(const Locations& locations) {
    assert(!std::ranges::contains(locations | std::views::adjacent_transform<3>([](auto a, auto b, auto c) {
                                      return ((a.row == b.row) && (b.row == c.row)) ||
                                             ((a.col == b.col) && (b.col == c.col));
                                  }),
                                  true));
    auto floor = Floor{};

    assert(locations.size() >= 2);

    floor.addLine(locations.back(), locations.front());
    std::ranges::for_each(locations | std::views::adjacent<2>, [&floor](auto t) {
        const auto [a, b] = t;
        floor.addLine(a, b);
    });

    floor.normalize();

    return floor;
}

auto solve(const Locations& locations) {
    const auto floor = makeFloor(locations);

    const auto size = locations.size();
    auto rectangles = std::views::iota(0uz, size) | std::views::transform([&locations, size](auto i) {
                          return std::views::iota(i + 1, size) | std::views::transform([&locations, i](auto j) {
                                     return Rectangle{locations.at(i), locations.at(j)};
                                 });
                      }) |
                      std::views::join;

    return std::ranges::max(rectangles | std::views::filter([&floor](auto r) { return floor.isTiled(r); }) |
                            std::views::transform(area));
}

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};
    const auto problem = parse(path);

    std::println("{}", solve(problem));
}