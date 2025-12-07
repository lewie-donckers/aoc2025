#include "shared/shared.hpp"

#include <cassert>
#include <filesystem>
#include <print>
#include <vector>

enum class Element { empty, start, splitter, beam };

using Row = std::vector<Element>;

Element parseElement(char c) {
    assert(std::string_view{".^S"}.contains(c));

    switch (c) {
        case '^':
            return Element::splitter;
        case 'S':
            return Element::start;
    }

    return Element::empty;
}

Row parseRow(std::string_view str) {
    assert(!str.empty());

    return std::views::transform(str, parseElement) | std::ranges::to<std::vector>();
}

struct State {
    std::vector<std::uint64_t> timelines;
};

State apply(const State& state, const Row& row) {
    auto result = state;

    if (result.timelines.empty()) {
        result.timelines = std::vector(row.size(), std::uint64_t{0});
    }

    assert(result.timelines.size() == row.size());

    for (auto [i, e] : std::views::enumerate(row)) {
        switch (e) {
            using enum Element;
            case empty:
                break;
            case start:
                assert(result.timelines[i] == 0);
                result.timelines[i] = 1;
                break;
            case beam:
                assert(false);
            case splitter:
                if (result.timelines[i] > 0) {
                    if (i > 0) {
                        result.timelines[i - 1] += result.timelines[i];
                    }
                    if (std::cmp_less(i + 1, result.timelines.size())) {
                        result.timelines[i + 1] += result.timelines[i];
                    }
                    result.timelines[i] = 0;
                }
                break;
        }
    }

    return result;
}

auto solve(const std::filesystem::path& path) {
    auto manifold = yieldLines(path) | std::views::transform(parseRow);

    return std::ranges::fold_left(std::ranges::fold_left(manifold, State{}, apply).timelines, std::uint64_t{0},
                                  std::plus<>{});
}

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};

    std::println("{}", solve(path));
}