#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <print>

enum class Direction { left, right };

class Safe {
public:
    void turn(Direction dir, int count) {
        assert(count > 0);

        nrZeroes_ += count / limit;
        count %= limit;

        if (dir == Direction::left) {
            location_ -= count;
            if (location_ == 0) {
                ++nrZeroes_;
            } else if (location_ < 0) {
                location_ += limit;
                if (prevLocation_ > 0) {
                    ++nrZeroes_;
                }
            }
        } else {
            location_ += count;
            if (location_ >= limit) {
                location_ -= limit;
                ++nrZeroes_;
            }
        }

        assert(location_ >= 0);
        assert(location_ < limit);

        prevLocation_ = location_;
    }

    int location() const { return location_; }
    int nrZeroes() const { return nrZeroes_; }

private:
    static constexpr auto limit = 100;

    int nrZeroes_ = 0;
    int location_ = 50;
    int prevLocation_ = location_;
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

        const auto prev = safe.location();

        safe.turn(direction, count);

        std::println("{} \t+ {} \t= {} \t{}", prev, line, safe.location(), safe.nrZeroes());
    }

    return safe.nrZeroes();
}

int main(int argc, const char** argv) {
    assert(argc >= 2);

    const auto path = std::filesystem::path{argv[1]};
    std::println("{}", solve(path));
}