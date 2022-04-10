#ifndef SMOOTHLIFE_CONFIG_HPP
#define SMOOTHLIFE_CONFIG_HPP

#include <array>
#include <deque>
#include <random>
#include <utility>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <docopt/docopt.h>
#include <spdlog/spdlog.h>

namespace smoothlife::config {

// gameplay config
static constexpr int base = 10;
static constexpr int player_lives = 5;
static constexpr int player_energy = 100;
static constexpr int chain_length = 3;
static constexpr int surface_size = 100;
static constexpr int min_win_level = 4;

// ui config
static constexpr std::size_t log_length = 4;

static constexpr std::size_t panel_width = 19;
static constexpr std::size_t panel_height = 9;

static constexpr std::size_t board_width = 7;
static constexpr std::size_t board_height = 5;

}// namespace smoothlife::config

#endif// SMOOTHLIFE_CONFIG_HPP