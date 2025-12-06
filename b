#!/bin/sh

g++-14 -std=c++23 -Wall -Wextra -Wpedantic -Werror -O3 -I. $1 -o build/app -fconcepts-diagnostics-depth=5