#include "shared/shared.hpp"

#include <cassert>
#include <filesystem>
#include <functional>
#include <print>

using T = std::uint64_t;

auto parseNr(std::string_view str) {
    assert(!str.empty());
    assert(str.find_first_not_of("0123456789") == std::string_view::npos);
    auto result = T{0};
    std::from_chars(str.begin(), str.end(), result);
    return result;
}

enum class Operation { add, multiply };

auto parseOperation(std::string_view str) {
    assert(str.size() == 1);
    assert(str.find_first_not_of("+*") == std::string_view::npos);
    return (str.front() == '+') ? Operation::add : Operation::multiply;
}

struct Problem {
    std::vector<T> operands{};
    Operation operation{Operation::add};
};

T solveProblem(const Problem& problem) {
    return (problem.operation == Operation::add) ? std::ranges::fold_left(problem.operands, T{0}, std::plus<>{})
                                                 : std::ranges::fold_left(problem.operands, T{1}, std::multiplies<>{});
}

auto parse(const std::filesystem::path& path) {
    const auto chunker = [](char a, char b) { return (a == ' ') == (b == ' '); };
    const auto filter = [](std::string_view s) { return !s.contains(' '); };
    const auto elements = std::views::chunk_by(chunker) | as<std::string_view>() | std::views::filter(filter);

    const auto lines = yieldLines(path) | as<std::string>() | std::ranges::to<std::vector>();
    assert(lines.size() >= 3);

    auto problems = lines.back() | elements |
                    std::views::transform([](auto str) { return Problem{.operation = parseOperation(str)}; }) |
                    std::ranges::to<std::vector>();

    std::ranges::for_each(lines | std::views::take(lines.size() - 1), [&](const auto& line) {
        std::ranges::for_each(std::views::zip(problems, line | elements | std::views::transform(parseNr)),
                              [](const auto& tup) {
                                  auto& [problem, nr] = tup;
                                  problem.operands.push_back(nr);
                              });
    });

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