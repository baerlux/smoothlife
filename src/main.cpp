#include "config.hpp"
#include "gameboard.hpp"
#include "player.hpp"

#include <internal_use_only/config.hpp>

namespace smoothlife {

void game_loop()
{
  Player player;
  EventLog<config::event_log_length> log;

  auto prng = std::ranlux24{ std::random_device{}() };
  GameBoard<config::board_width, config::board_height, std::ranlux24> board{ prng, player, log };
  board.generate_level();

  auto screen = ftxui::ScreenInteractive::FitComponent();
  auto quit_button = ftxui::Button("Quit", screen.ExitLoopClosure());

  auto container = ftxui::Container::Horizontal({ player.move_ui, quit_button });

  auto game_ui = ftxui::Renderer(container, [&] {
    using enum ftxui::Color::Palette16;
    return ftxui::window(ftxui::text(" smoothlife ") | ftxui::hcenter | ftxui::bold,
      ftxui::hbox({
        ftxui::vbox({
          ftxui::text(""),
          ftxui::text(""),
          ftxui::text(""),
          ftxui::vbox({
            ftxui::hbox({
              ftxui::filler(),
              player.move_ui->Render(),
              ftxui::filler(),
            }),
            ftxui::text(" try arrow keys!") | ftxui::color(GrayDark),
          }) | ftxui::border
            | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, config::panel_width),
          ftxui::hbox({ quit_button->Render() }),
        }),
        ftxui::separator(),
        board.render(),
        ftxui::separator(),
        ftxui::vbox({
          ftxui::text(""),
          ftxui::text(fmt::format(" energy  {:>6}", player.energy)),
          ftxui::text(fmt::format(" surface {:>6}", player.surface)),
          log.render(),
          ftxui::text(" legend:") | ftxui::color(GrayDark),
          ftxui::hbox({
            ftxui::text(" + ") | ftxui::bold | ftxui::color(RedLight),
            ftxui::text(" − ") | ftxui::bold | ftxui::color(BlueLight),
            ftxui::text(" × ") | ftxui::bold | ftxui::color(GreenLight),
            ftxui::text(" ÷ ") | ftxui::bold | ftxui::color(YellowLight),
          }),
        }),
      }));
  });

  screen.Loop(game_ui);
}

}// namespace smoothlife

int main(int argc, const char **argv)
{
  try {
    static constexpr auto USAGE =
      R"(
    Usage:
          smoothlife
          smoothlife --version
          smoothlife (-h | --help)
    Options:
          -h --help     Show this screen.
          --version     Show version.
)";

    auto args = docopt::docopt(USAGE,
      { std::next(argv), std::next(argv, argc) },
      true,
      fmt::format("{} {}", smoothlife::cmake::project_name, smoothlife::cmake::project_version));

    // start the game
    smoothlife::game_loop();

  } catch (const std::exception &e) {
    SPDLOG_ERROR("Unhandled exception in main: {}", e.what());
  }
}
