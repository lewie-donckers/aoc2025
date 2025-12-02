#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

enum class Direction { left, right };

class Safe {
public:
    void turn(Direction dir, int count) {
        assert(count >= 0);

        if (dir == Direction::left) {
            count %= limit;
            count = limit - count;
        }

        location_ += count;
        location_ %= limit;

        if (location_ == 0) {
            ++nrZeroes_;
        }
    }

    int nrZeroes() const { return nrZeroes_; }

private:
    static constexpr auto limit = 100;

    int nrZeroes_ = 0;
    int location_ = 50;
};

int solve(const std::filesystem::path& path) {
    auto input = std::ifstream{path};
    auto safe = Safe{};

    auto line = std::string{};
    while (std::getline(input, line)) {
        assert(line.size() >= 2);

        const auto dirRaw = line[0];
        assert((dirRaw == 'L') || (dirRaw == 'R'));
        const auto direction = (dirRaw == 'L') ? Direction::left : Direction::right;

        const auto count = std::atoi(line.c_str() + 1);

        safe.turn(direction, count);
    }

    return safe.nrZeroes();
}

int main(int argc, const char** argv) {
    assert(argc >= 2);

    const auto path = std::filesystem::path{argv[1]};
    std::cout << solve(path) << '\n';
}