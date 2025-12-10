#include "shared/shared.hpp"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <functional>
#include <print>
#include <vector>

enum class Light { off, on };

bool isEven(int i) { return i % 2 == 0; }

Light operator*(Light l, int i) {
    if ((l == Light::off) || isEven(i)) {
        return Light::off;
    }
    return Light::on;
}

Light operator+(Light a, Light b) { return (a == b) ? Light::off : Light::on; }

using IndicatorLightDiagram = std::vector<Light>;

using WiringSchematic = std::vector<int>;
using WiringSchematics = std::vector<WiringSchematic>;

using JoltageRequirements = std::vector<int>;

struct Machine {
    IndicatorLightDiagram lights;
    WiringSchematics wiring;
    JoltageRequirements joltage;
};

using Machines = std::vector<Machine>;

IndicatorLightDiagram makeIndicatorLightDiagram(std::size_t size) { return IndicatorLightDiagram(size, Light::off); }

IndicatorLightDiagram add(IndicatorLightDiagram a, const IndicatorLightDiagram& b) {
    assert(a.size() == b.size());
    std::ranges::transform(a, b, a.begin(), std::plus<>{});
    return a;
}

IndicatorLightDiagram multiply(std::size_t size, const WiringSchematic& schematic, int factor) {
    const auto diagram = [&]() {
        auto result = makeIndicatorLightDiagram(size);
        std::ranges::for_each(schematic, [&result](auto i) {
            assert((i >= 0) && std::cmp_less(i, result.size()));
            result[i] = Light::on;
        });
        return result;
    }();

    return diagram | std::views::transform([factor](auto i) { return i * factor; }) | std::ranges::to<std::vector>();
}

IndicatorLightDiagram parseIndicatorLightDiagram(std::string_view str) {
    assert(str.size() >= 3);
    assert(str.front() == '[');
    assert(str.back() == ']');

    str.remove_prefix(1);
    str.remove_suffix(1);

    static constexpr auto parseLight = [](char c) {
        assert((c == '.') || (c == '#'));

        return (c == '.') ? Light::off : Light::on;
    };

    return str | std::views::transform(parseLight) | std::ranges::to<std::vector>();
}

WiringSchematic parseWiringSchematic(std::string_view str) {
    assert(str.size() >= 3);
    assert(str.front() == '(');
    assert(str.back() == ')');

    str.remove_prefix(1);
    str.remove_suffix(1);

    return str | std::views::split(',') | as<std::string_view>() | std::views::transform(parseInt<int>) |
           std::ranges::to<std::vector>();
}

JoltageRequirements parseJoltageRequirements(std::string_view str) {
    assert(str.size() >= 3);
    assert(str.front() == '{');
    assert(str.back() == '}');

    str.remove_prefix(1);
    str.remove_suffix(1);

    return str | std::views::split(',') | as<std::string_view>() | std::views::transform(parseInt<int>) |
           std::ranges::to<std::vector>();
}

Machine parseMachine(std::string_view str) {
    const auto parts = str | std::views::split(' ') | as<std::string_view>() | std::ranges::to<std::vector>();

    assert(parts.size() >= 3);

    auto lights = parseIndicatorLightDiagram(parts.front());
    auto wiring = std::span{parts}.subspan(1, parts.size() - 2) | std::views::transform(parseWiringSchematic) |
                  std::ranges::to<std::vector>();
    auto joltage = parseJoltageRequirements(parts.back());

    return Machine{lights, wiring, joltage};
}

Machines parse(const std::filesystem::path& path) {
    return yieldLines(path) | std::views::transform(parseMachine) | std::ranges::to<std::vector>();
}

using Selection = std::span<const int>;

std::generator<Selection> yieldSelections(std::size_t size) {
    auto data = std::vector<int>(size, 0);

    const auto next = [&data]() {
        for (auto& i : data) {
            i = (i + 1) % 2;
            if (i == 1) {
                break;
            }
        }
    };

    const auto isDone = [&data]() { return std::ranges::all_of(data, [](auto i) { return i == 1; }); };

    while (!isDone()) {
        co_yield data;
        next();
    }
}

auto solve(const Machines& machines) {
    static constexpr auto impl = [](const Machine& machine) {
        const auto isValid = [&machine](Selection selection) {
            const auto& target = machine.lights;
            const auto size = target.size();
            const auto mul = std::bind_front(multiply, size);
            const auto result = std::ranges::fold_left(std::views::zip_transform(mul, machine.wiring, selection),
                                                       makeIndicatorLightDiagram(size), add);

            return target == result;
        };
        const auto count = [](Selection s) { return std::ranges::fold_left(s, 0, std::plus<>{}); };

        return std::ranges::min(yieldSelections(machine.wiring.size()) | std::views::filter(isValid) |
                                std::views::transform(count));
    };

    return std::ranges::fold_left(machines | std::views::transform(impl), 0, std::plus<>{});
}

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};
    const auto problem = parse(path);

    std::println("{}", solve(problem));
}