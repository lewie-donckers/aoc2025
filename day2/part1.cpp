#include <algorithm>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <print>
#include <ranges>
#include <string>

using T = std::uint64_t;

class Id {
public:
    Id() = default;
    explicit Id(T val) : str_{std::to_string(val)}, val_{val} {}
    explicit Id(std::string_view str) : str_{str}, val_{std::stoull(str_)} {}

    bool isValid() const {
        const auto str = std::string_view{str_};
        const auto strSize = str.size();
        const auto strSizeIsEven = (strSize % 2) == 0;

        if (!strSizeIsEven) {
            return true;
        }

        const auto front = str_.substr(0, strSize / 2);
        const auto back = str_.substr(strSize / 2);

        return front != back;
    }

    Id& operator++() {
        ++val_;
        str_ = std::to_string(val_);
        return *this;
    }

    Id operator++(int) {
        const auto prev = *this;
        operator++();
        return prev;
    }

    bool operator==(const Id& rhs) const { return val_ == rhs.val_; }

    Id operator+(T rhs) const { return Id{val_ + rhs}; }
    T operator-(const Id& rhs) const { return val_ - rhs.val_; }

    T value() const { return val_; }

private:
    std::string str_{"0"};
    T val_{0};
};

T valueIfValid(const Id& i) { return i.isValid() ? 0 : i.value(); };

template <typename T>
struct to {};

template <typename T, typename R>
auto operator|(R&& range, to<T>) {
    return std::forward<R>(range) | std::views::transform([](const auto& e) { return T{e}; });
}

auto split(char delim) { return std::views::split(delim) | to<std::string_view>{}; }

template <typename R>
T sum(R&& range) {
    return std::ranges::fold_left(std::forward<R>(range), T{0}, std::plus<T>{});
}

template <typename R>
auto elem(R&& range, int index) {
    auto result = std::ranges::next(range.begin(), index, range.end());
    assert(result != range.end());
    return *result;
}

T sumRange(std::string_view str) {
    const auto range = [str]() {
        auto items = str | split('-') | to<Id>{};
        const auto from = elem(items, 0);
        const auto to = elem(items, 1);
        return std::views::iota(from, to + 1);
    }();

    return sum(range | std::views::transform(valueIfValid));
}

T solve(const std::filesystem::path& path) {
    const auto input = [&path]() {
        auto stream = std::ifstream{path};
        auto string = std::string{};
        std::getline(stream, string);
        return string;
    }();

    return sum(input | split(',') | std::views::transform(sumRange));
}

int main(int argc, const char** argv) {
    assert(argc >= 2);
    const auto path = std::filesystem::path{argv[1]};
    std::println("{}", solve(path));
}