#include "shared/shared.hpp"

#include <cassert>
#include <filesystem>
#include <functional>
#include <optional>
#include <print>

using T = std::uint64_t;

template <typename... Ts>
auto parseNr(const std::tuple<Ts...>& tup) {
    auto result = T{0};

    const auto parse = [&](char c) {
        static constexpr auto table = std::string_view{"0123456789 "};
        assert(table.contains(c));

        const auto pos = table.find(c);
        if (pos == 10) return;

        result *= 10;
        result += pos;
    };

    std::apply([&](auto... c) { (parse(c), ...); }, tup);

    return result;
}

enum class Operation { add, multiply };

std::optional<Operation> parseOperation(char c) {
    if (c == '+') {
        return Operation::add;
    }
    if (c == '*') {
        return Operation::multiply;
    }
    return std::nullopt;
}

struct Problem {
    std::vector<T> operands{};
    Operation operation{Operation::add};
};

T solveProblem(const Problem& problem) {
    assert(!problem.operands.empty());

    return (problem.operation == Operation::add) ? std::ranges::fold_left(problem.operands, T{0}, std::plus<>{})
                                                 : std::ranges::fold_left(problem.operands, T{1}, std::multiplies<>{});
}

template <typename... Ts>
auto allSpace(const std::tuple<Ts...>& tup) {
    return std::apply([](auto... c) { return ((c == ' ') && ...); }, tup);
}

template <typename... Ts>
auto allButLast(const std::tuple<Ts...>& tup) {
    return [&tup]<std::size_t... Indices>(std::index_sequence<Indices...>) {
        return std::tuple{std::get<Indices>(tup)...};
    }(std::make_index_sequence<sizeof...(Ts) - 1>());
}

template <typename... Ts>
auto last(const std::tuple<Ts...>& tup) {
    return std::get<sizeof...(Ts) - 1>(tup);
}

auto parse(const std::filesystem::path& path) {
    const auto lines = yieldLines(path) |
                       std::views::transform([](auto str) { return std::string{str.rbegin(), str.rend()}; }) |
                       std::ranges::to<std::vector>();
    assert((lines.size() >= 4) && (lines.size() <= 5));

    auto problems = std::vector<Problem>{1};
    const auto parser = [&](const auto& tup) {
        if (allSpace(tup)) {
            problems.push_back(Problem{});
            return;
        }

        auto& problem = problems.back();

        problem.operands.push_back(parseNr(allButLast(tup)));

        const auto operation = parseOperation(last(tup));
        if (operation.has_value()) {
            problem.operation = operation.value();
        }
    };

    const auto impl = [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
        std::ranges::for_each(std::views::zip(lines[Indices]...), parser);
    };

    if (lines.size() == 4) {
        impl(std::make_index_sequence<4>());
    } else if (lines.size() == 5) {
        impl(std::make_index_sequence<5>());
    }

    return problems;
}

auto solve(const auto& problems) {
    return std::ranges::fold_left(problems | std::views::transform(solveProblem), T{0}, std::plus<>{});
}

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};
    const auto problems = parse(path);

    std::println("{}", solve(problems));
}