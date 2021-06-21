#pragma once

#define DLL_EXPORT extern "C" __declspec(dllexport)
#include <windows.h>

#include <vector>
#include <cassert>
#include <mutex>
#include <string>
#include <iostream>
#include <algorithm>
#include <functional>
#include <queue>
#include <unordered_set>
#include <map>
#include <csetjmp>

using namespace std::literals;

#include "game/structs.hpp"
#include "game/game.hpp"
