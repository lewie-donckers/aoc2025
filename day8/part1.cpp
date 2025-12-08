#include "shared/shared.hpp"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <print>
#include <vector>

struct Box {
    std::uint64_t x{0};
    std::uint64_t y{0};
    std::uint64_t z{0};
};

using Boxes = std::vector<Box>;

auto distance(Box a, Box b) {
    static constexpr auto d = [](auto a, auto b) { return static_cast<double>(a) - static_cast<double>(b); };

    return std::hypot(d(a.x, b.x), d(a.y, b.y), d(a.z, b.z));
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

Box parseBox(std::string_view str) {
    return makeFromRange<Box, 3>(str | std::views::split(',') | as<std::string_view>() |
                                 std::views::transform(parseInt<std::uint64_t>));
}

Boxes parse(const std::filesystem::path& path) {
    return yieldLines(path) | std::views::transform(parseBox) | std::ranges::to<std::vector>();
}

using Indices = std::vector<std::size_t>;

auto solve(const Boxes& boxes, std::size_t nrConnections) {
    const auto size = boxes.size();
    auto boxToCircuit = std::views::iota(0uz, size) | std::ranges::to<Indices>();
    auto circuits = std::views::iota(0uz, size) | std::views::transform([](auto i) { return Indices{{i}}; }) |
                    std::ranges::to<std::vector>();

    auto combinations =
        std::views::iota(0uz, size) | std::views::transform([size](auto i) {
            return std::views::iota(i + 1, size) | std::views::transform([i](auto j) { return std::pair{i, j}; });
        }) |
        std::views::join | std::ranges::to<std::vector>();
    assert(nrConnections < combinations.size());
    std::ranges::partial_sort(combinations, std::next(combinations.begin(), nrConnections), {}, [&](auto t) {
        const auto [i1, i2] = t;
        return distance(boxes.at(i1), boxes.at(i2));
    });

    for (auto [boxIndex1, boxIndex2] : combinations | std::views::take(nrConnections)) {
        assert(boxIndex1 != boxIndex2);

        const auto circuitIndex1 = boxToCircuit.at(boxIndex1);
        const auto circuitIndex2 = boxToCircuit.at(boxIndex2);
        if (circuitIndex1 == circuitIndex2) continue;

        auto& circuit1 = circuits.at(circuitIndex1);
        auto& circuit2 = circuits.at(circuitIndex2);
        assert(!circuit1.empty() && !circuit2.empty());

        circuit1.insert(circuit1.begin(), circuit2.begin(), circuit2.end());
        circuit2.clear();
        std::ranges::for_each(boxToCircuit, [circuitIndex1, circuitIndex2](auto& i) {
            if (i == circuitIndex2) i = circuitIndex1;
        });
    }

    static constexpr auto toSum = std::size_t{3};
    assert(toSum <= circuits.size());

    std::ranges::partial_sort(circuits, std::next(circuits.begin(), toSum), std::ranges::greater{}, &Indices::size);

    return std::ranges::fold_left(circuits | std::views::take(toSum) | std::views::transform(&Indices::size), 1uz,
                                  std::multiplies<>{});
}

int main(int argc, const char** argv) {
    assert(argc >= 3);
    const auto path = std::filesystem::path{argv[1]};
    const auto nrConnections = parseInt<int>(argv[2]);
    auto problem = parse(path);

    std::println("{}", solve(problem, nrConnections));
}