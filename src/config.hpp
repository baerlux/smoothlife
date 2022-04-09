#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <array>
#include <deque>
#include <random>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <docopt/docopt.h>
#include <spdlog/spdlog.h>

namespace smoothlife::config {

// gameplay config
static constexpr int base = 10;

// ui config
static constexpr std::size_t event_log_length = 4;

static constexpr std::size_t panel_width = 19;
static constexpr std::size_t panel_height = 9;

static constexpr std::size_t board_width = 7;
static constexpr std::size_t board_height = 5;

}// namespace smoothlife::config

#endif// CONFIG_HPP