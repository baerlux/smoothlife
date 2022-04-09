#ifndef SMOOTHLIFE_EVENTLOG_HPP
#define SMOOTHLIFE_EVENTLOG_HPP

#include "config.hpp"

namespace smoothlife {

template<std::size_t Length> struct EventLog
{
  std::deque<std::string> log;

  void post_event(const std::string &event)
  {
    log.push_front(event);
    if (log.size() > Length) { log.pop_back(); }
  }

  [[nodiscard]] ftxui::Element render() const
  {
    using enum ftxui::Color::Palette16;

    ftxui::Elements events;
    std::array<ftxui::Color, 3> shades{ White, GrayLight, GrayDark };

    for (std::size_t i = 0; i < log.size(); ++i) {
      events.push_back(ftxui::paragraph(log.at(i)) | ftxui::color(shades.at(std::min(i, shades.size() - 1))));
    }


    return ftxui::vbox(std::move(events)) | ftxui::border | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, config::panel_width)
           | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, config::panel_height);
  }
};

}// namespace smoothlife

#endif// SMOOTHLIFE_EVENTLOG_HPP