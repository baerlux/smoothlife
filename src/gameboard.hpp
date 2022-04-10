#ifndef SMOOTHLIFE_GAMEBOARD_HPP
#define SMOOTHLIFE_GAMEBOARD_HPP

#include "config.hpp"
#include "field.hpp"
#include "log.hpp"
#include "player.hpp"
#include "util.hpp"

namespace smoothlife {

enum struct GameStage : int { intro = 1, tutorial_1 = 2, tutorial_2 = 3, game = 4, ending = 5 };

// Make GameStage incrementable
GameStage &operator++(GameStage &stage)
{
  using IntType = typename std::underlying_type<GameStage>::type;
  if (stage != GameStage::ending) { stage = static_cast<GameStage>(static_cast<IntType>(stage) + 1); }
  return stage;
}

template<std::size_t Width, std::size_t Height, class Prng> struct GameBoard
{
  using enum Field::Type;

  GameStage stage = GameStage::intro;
  std::array<Field, Width * Height> state;

  Player &player;
  Prng &rnd;
  Log &log;

  int level = 0;

  GameBoard(Prng &randomness_provider, Player &p, Log &l) : player{ p }, rnd{ randomness_provider }, log{ l }
  {
    player.bounds.x_max = Width - 1;
    player.bounds.x_min = 0;
    player.bounds.y_max = Height - 1;
    player.bounds.y_min = 0;

    player.interaction = [&] {
      if (player.energy == 0) {
        ++stage;
        return;
      }

      Field &f = get_field(player.x, player.y);
      ++player.steps;

      if (f.type == empty) {
        return;
      } else if (f.type == exit) {
        using enum Log::Type;
        int r = roundness(player.surface);
        int score = (r + level) * (config::player_energy - player.steps);
        if (r == 0) {
          log.post_event("You failed this time :(", bad);
          if (--player.lives == 0) {
            ++stage;
            return;
          }
        } else if (r == 1) {
          log.post_event(fmt::format("An okay polish. score +{}, energy +{}", score, config::ok_energy_gain), neutral);
          player.energy += config::ok_energy_gain;
          player.score += score;
          ++level;
        } else if (r == 2) {
          log.post_event(
            fmt::format("A really fine polish ^.^ score +{}, energy +{}", score, config::fine_energy_gain), good);
          player.energy += config::fine_energy_gain;
          player.score += score;
          ++level;
        } else if (r > 2) {
          log.post_event(
            fmt::format("Truly a masterpiece *.* score +{}, energy +{}", score, config::master_energy_gain), amazing);
          player.energy += config::master_energy_gain;
          player.score += score;
          ++level;
        }
        player.steps = 0;
        generate_level();
      } else {
        f.apply(player.surface, &log);
      }
    };
  }

  void generate_level()
  {
    using uni = std::uniform_int_distribution<int>;
    uni in_board_width{ 0, Width - 1 };
    uni in_board_height{ 0, Height - 1 };
    uni in_field_types{ static_cast<int>(Field::Type::add), static_cast<int>(Field::Type::div) };
    uni in_one_to_nine{ 1, config::base - 1 };

    // generate a surface and operation chain until roundness is zero
    long surface;
    do {
      // clear field
      state = std::array<Field, Width * Height>{};

      surface =
        in_one_to_nine(rnd)
        * std::min(config::surface_size * static_cast<int>(std::pow(10, level / 2)), config::max_surface_factor);

      std::array<bool, Width * Height> occupied{};

      // player
      occupied.at(pack2d(0, 0)) = true;
      player.x = 0;
      player.y = 0;

      // exit
      occupied.at(pack2d(Width - 1, Height - 1)) = true;
      set_field(Width - 1, Height - 1, { exit });

      // generate field chain, increase length every two levels
      for (int i = 0; i < config::chain_length + level / 2; ++i) {
        // random operation field
        Field rnd_f;

        rnd_f.type = static_cast<Field::Type>(in_field_types(rnd));
        rnd_f.value = in_one_to_nine(rnd);

        // search for op to result in a whole number
        if (rnd_f.type == mul) {
          long rem = surface % rnd_f.value;
          while (rem != 0) {
            rnd_f.value = in_one_to_nine(rnd);
            rem = surface % rnd_f.value;
          }
        }

        // search for empty field
        int rnd_x, rnd_y;
        do {
          rnd_x = in_board_width(rnd);
          rnd_y = in_board_height(rnd);
        } while (occupied.at(pack2d(rnd_x, rnd_y)));

        // place operation field
        set_field(rnd_x, rnd_y, rnd_f);
        occupied.at(pack2d(rnd_x, rnd_y)) = true;

        // make surface rough
        rnd_f.inverse().apply(surface);
      }
    } while (roundness(surface) > 0);

    player.surface = surface;
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
