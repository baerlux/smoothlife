#ifndef SMOOTHLIFE_PLAYER_HPP
#define SMOOTHLIFE_PLAYER_HPP

#include "config.hpp"

namespace smoothlife {

struct Player
{
  int x = 0;
  int y = 0;
  int surface = 0;
  int score = 0;
  int steps = 0;
  int energy = config::player_energy;
  int lives = config::player_lives;

  // set by GameBoard
  std::function<void()> interaction;
  ftxui::Box bounds;

  // player control ui
  ftxui::Components buttons;
  ftxui::Component move_ui;

  [[nodiscard]] std::string health() const
  {
    std::string health_str = "";
    for (int i = 0; i < config::player_lives; ++i) {
      if (i < lives) {
        health_str.append("♥");
      } else {
        health_str.append("♡");
      }
      if (i < config::player_lives - 1) { health_str.append(" "); }
    }
    return health_str;
  }

  Player()
  {
    buttons.push_back(ftxui::Button(" ᐃ ", [&] {
      if (y < bounds.y_max && energy > 0) {
        y++;
        energy--;
        interaction();
      }
    }));
    buttons.push_back(ftxui::Button(" ᐊ ", [&] {
      if (x > bounds.x_min && energy > 0) {
        x--;
        energy--;
        interaction();
      }
    }));
    buttons.push_back(ftxui::Button(" ᐁ ", [&] {
      if (y > bounds.y_min && energy > 0) {
        y--;
        energy--;
        interaction();
      }
    }));
    buttons.push_back(ftxui::Button(" ᐅ ", [&] {
      if (x < bounds.x_max && energy > 0) {
        x++;
        energy--;
        interaction();
      }
    }));

    move_ui = ftxui::Container::Vertical({
      ftxui::Renderer(buttons[0],
        [&] {
          return ftxui::hbox({
            ftxui::filler(),
            buttons[0]->Render(),
            ftxui::filler(),
          });
        }),
      ftxui::Container::Horizontal({
        buttons[1],
        buttons[2],
        buttons[3],
      }),
    });

    // use arrow keys as hotkeys for movement
    move_ui = ftxui::CatchEvent(move_ui, [&](const ftxui::Event &event) {
      if (event == ftxui::Event::ArrowUp) {
        buttons[0]->TakeFocus();
        buttons[0]->OnEvent(ftxui::Event::Return);
        return true;
      } else if (event == ftxui::Event::ArrowLeft) {
        buttons[1]->TakeFocus();
        buttons[1]->OnEvent(ftxui::Event::Return);
        return true;
      } else if (event == ftxui::Event::ArrowDown) {
        buttons[2]->TakeFocus();
        buttons[2]->OnEvent(ftxui::Event::Return);
        return true;
      } else if (event == ftxui::Event::ArrowRight) {
        buttons[3]->TakeFocus();
        buttons[3]->OnEvent(ftxui::Event::Return);
        return true;
      }
      return false;
    });
  }
};

}// namespace smoothlife

#endif// SMOOTHLIFE_PLAYER_HPP
