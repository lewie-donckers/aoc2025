#include "shared/shared.hpp"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <functional>
#include <map>
#include <print>
#include <regex>
#include <vector>

struct Device {
    std::string name;

    constexpr auto operator<=>(const Device&) const noexcept = default;
};

using Devices = std::vector<Device>;

using Connections = std::map<Device, Devices>;

struct Rack {
    Connections connections;
};

std::pair<const Device, Devices> parseConnection(std::string_view str) {
    static const auto fullRegex = std::regex{"([[:alpha:]]{3}):( ([[:alpha:]]{3}))+"};
    assert(std::regex_match(str.begin(), str.end(), fullRegex));

    static const auto regex = std::regex{"[[:alpha:]]{3}"};

    auto iter = std::cregex_iterator{str.begin(), str.end(), regex};
    const auto end = std::cregex_iterator{};
    assert(iter != end);

    static constexpr auto toDevice = [](const auto& match) { return Device{.name{match[0]}}; };

    const auto device = toDevice(*iter);

    ++iter;
    const auto devices =
        std::ranges::subrange(iter, end) | std::views::transform(toDevice) | std::ranges::to<std::vector>();
    assert(!devices.empty());

    return {device, devices};
}

Rack parse(const std::filesystem::path& path) {
    auto result = Rack{};
    std::ranges::transform(yieldLines(path), std::inserter(result.connections, result.connections.begin()),
                           parseConnection);
    return result;
}

auto solve(const Rack& rack) {
    static const auto start = Device{.name = "you"};
    static const auto goal = Device{.name = "out"};

    const auto iStart = rack.connections.find(start);
    assert(iStart != rack.connections.end());

    struct Waypoint {
        Connections::const_iterator iter;
        std::size_t index;
    };

    auto count = 0uz;
    auto path = std::vector<Waypoint>{};
    path.emplace_back(iStart, 0);

    while (true) {
        auto& waypoint = path.back();

        if (waypoint.index == waypoint.iter->second.size()) {
            path.pop_back();
            if (path.empty()) {
                break;
            } else {
                ++path.back().index;
                continue;
            }
        }

        const auto& current = waypoint.iter->second[waypoint.index];

        if (current == goal) {
            ++count;
            ++waypoint.index;
        } else {
            const auto iCurrent = rack.connections.find(current);
            path.emplace_back(iCurrent, 0);
        }
    }

    return count;
}

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};
    const auto problem = parse(path);

    std::println("{}", solve(problem));
}