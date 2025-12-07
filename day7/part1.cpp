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
    std::uint64_t nrSplits{0};
    Row beams;
};

State apply(const State& state, const Row& row) {
    auto result = state;

    if (result.beams.empty()) {
        result.beams = std::vector(row.size(), Element::empty);
    }

    assert(result.beams.size() == row.size());

    for (auto [i, e] : std::views::enumerate(row)) {
        switch (e) {
            using enum Element;
            case empty:
                break;
            case start:
                result.beams[i] = beam;
                break;
            case beam:
                assert(false);
            case splitter:
                if (result.beams[i] == beam) {
                    ++result.nrSplits;

                    result.beams[i] = empty;
                    if (i > 0) {
                        result.beams[i - 1] = beam;
                    }
                    if (std::cmp_less(i + 1, result.beams.size())) {
                        result.beams[i + 1] = beam;
                    }
                }
                break;
        }
    }

    return result;
}

auto solve(const std::filesystem::path& path) {
    auto manifold = yieldLines(path) | std::views::transform(parseRow);

    return std::ranges::fold_left(manifold, State{}, apply).nrSplits;
}

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};

    std::println("{}", solve(path));
}