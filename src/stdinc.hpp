#pragma once

#define DLL_EXPORT extern "C" __declspec(dllexport)

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

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

#include "game/game.hpp"
#include "game/structs.hpp"
