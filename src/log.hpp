#ifndef SMOOTHLIFE_LOG_HPP
#define SMOOTHLIFE_LOG_HPP

#include "config.hpp"

namespace smoothlife {

struct Log
{
  enum struct Type { amazing, good, bad, neutral };
  using enum Type;

  std::size_t length;
  std::deque<std::pair<std::string, Type>> log;

  explicit Log(std::size_t len) : length{ len } {}

  void post_event(const std::string &event, Type type = neutral)
  {
    log.push_front({ event, type });
    if (log.size() > length) { log.pop_back(); }
  }

  [[nodiscard]] ftxui::Element render() const
  {
    using enum ftxui::Color::Palette16;

    ftxui::Elements events;
    std::array<ftxui::Color, 3> shades{ White, GrayLight, GrayDark };

    for (std::size_t i = 0; i < log.size(); ++i) {
      ftxui::Decorator color = ftxui::color(shades.at(std::min(i, shades.size() - 1)));
      if (i == 0 && log.at(i).second == amazing) {
        color = ftxui::color(GreenLight);
      } else if (i == 0 && log.at(i).second == good) {
        color = ftxui::color(BlueLight);
      } else if (i == 0 && log.at(i).second == bad) {
        color = ftxui::color(RedLight);
      }
      events.push_back(ftxui::paragraph(log.at(i).first) | color);
    }

    return ftxui::vbox(std::move(events)) | ftxui::border | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, config::panel_width)
           | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, config::panel_height);
  }
};

}// namespace smoothlife

#endif// SMOOTHLIFE_LOG_HPP