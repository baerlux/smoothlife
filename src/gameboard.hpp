#ifndef SMOOTHLIFE_GAMEBOARD_HPP
#define SMOOTHLIFE_GAMEBOARD_HPP

#include "config.hpp"
#include "eventlog.hpp"
#include "field.hpp"
#include "player.hpp"
#include "util.hpp"

namespace smoothlife {

template<std::size_t Width, std::size_t Height, class Prng> struct GameBoard
{
  using enum Field::Type;

  std::array<Field, Width * Height> state;
  Player &player;
  Prng &rnd;
  EventLog<config::event_log_length> &log;

  GameBoard(Prng &randomness_provider, Player &p, EventLog<config::event_log_length> &el)
    : player{ p }, rnd{ randomness_provider }, log{ el }
  {
    player.bounds.x_max = Width - 1;
    player.bounds.x_min = 0;
    player.bounds.y_max = Height - 1;
    player.bounds.y_min = 0;
    player.interaction = [&] {
      Field &f = get_field(player.x, player.y);
      switch (f.type) {
      case empty:
        break;
      case exit:
        generate_level();
        break;
      default:
        f.apply(player.surface);
        log.post_event("bla");
        break;
      }
    };
    player.energy = 100;// NOLINT
  }

  void generate_level()
  {
    // clear field
    state = std::array<Field, Width * Height>{};

    using uni = std::uniform_int_distribution<int>;
    uni in_board_width{ 0, Width - 1 };
    uni in_board_height{ 0, Height - 1 };
    uni in_field_types{ 2, 5 };// NOLINT
    uni in_one_to_nine{ 1, 9 };// NOLINT

    int goal = in_one_to_nine(rnd) * 100;// NOLINT

    std::array<bool, Width * Height> occupied{};

    // player
    occupied.at(pack2d(0, 0)) = true;
    player.x = 0;
    player.y = 0;

    // exit
    occupied.at(pack2d(Width - 1, Height - 1)) = true;
    set_field(Width - 1, Height - 1, { exit });

    // generate field chain
    for (int i = 0; i < 4; ++i) {// NOLINT

      Field rnd_f{ static_cast<Field::Type>(in_field_types(rnd)), in_one_to_nine(rnd) };

      if (rnd_f.type == mul) {
        int rem = goal % rnd_f.value;
        while (rem != 0) {
          rnd_f.value = in_one_to_nine(rnd);
          rem = goal % rnd_f.value;
        }
      }

      int rnd_x{};
      int rnd_y{};

      // search for empty field
      do {
        rnd_x = in_board_width(rnd);
        rnd_y = in_board_height(rnd);
      } while (occupied.at(pack2d(rnd_x, rnd_y)));

      set_field(rnd_x, rnd_y, rnd_f);// NOLINT
      occupied.at(pack2d(rnd_x, rnd_y)) = true;

      rnd_f.inverse().apply(goal);
    }

    player.surface = goal;
  }

  static size_t pack2d(int x, int y) { return static_cast<std::size_t>(x) + Width * static_cast<std::size_t>(y); }

  void set_field(int x, int y, Field field) { state.at(pack2d(x, y)) = field; }

  [[nodiscard]] const Field &get_field(int x, int y) const { return state.at(pack2d(x, y)); }

  [[nodiscard]] Field &get_field(int x, int y) { return state.at(pack2d(x, y)); }

  [[nodiscard]] ftxui::Element render() const
  {
    using enum ftxui::Color::Palette16;
    ftxui::Elements rows;

    for (int y = Height - 1; y >= 0; --y) {
      ftxui::Elements row;

      for (int x = 0; x < static_cast<int>(Width); ++x) {
        std::string repr = " ";
        ftxui::Color color = White;
        const Field &field = get_field(x, y);

        if (field.type != empty && field.type != exit) {
          repr = std::to_string(field.value);
          switch (field.type) {
          case add:
            color = RedLight;
            break;
          case sub:
            color = BlueLight;
            break;
          case mul:
            color = GreenLight;
            break;
          case div:
            color = YellowLight;
            break;
          default:
            break;
          }
        } else if (field.type == exit) {
          repr = "⋒";
        }
        if (x == player.x && y == player.y) { repr = "🯅"; }
        row.push_back(ftxui::text(fmt::format(" {} ", repr)) | ftxui::border | ftxui::color(color));
      }
      rows.push_back(ftxui::hbox(std::move(row)));
    }

    return ftxui::vbox(std::move(rows));
  }
};

}// namespace smoothlife

#endif// SMOOTHLIFE_GAMEBOARD_HPP
