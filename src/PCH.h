#pragma once

#include "SKSE/SKSE.h"
#include "RE/Skyrim.h"

#include <random>
#include <unordered_set>

#include <spdlog/sinks/basic_file_sink.h>
#include <nlohmann/json.hpp>

using json = nlohmann::basic_json<nlohmann::ordered_map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;
using namespace std::literals;

namespace logger = SKSE::log;
namespace WinAPI = SKSE::WinAPI;

#define DLLEXPORT __declspec(dllexport)