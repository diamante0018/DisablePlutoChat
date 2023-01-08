#pragma once

#define DLL_EXPORT extern "C" __declspec(dllexport)

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std::literals;

// clang-format off
#include "game/structs.hpp"
#include "game/game.hpp"
// clang-format on
