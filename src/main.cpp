#include "config.hpp"
#include "gameboard.hpp"
#include "player.hpp"

#include <internal_use_only/config.hpp>

void game_loop()
{
  Player player;
  auto prng = std::ranlux24{ std::random_device{}() };
  GameBoard<7, 5, std::ranlux24> board{ prng, player };// NOLINT magic number okay
  board.generate_level();

  auto game_ui = ftxui::Renderer(player.move_ui, [&] {
    return ftxui::window(ftxui::text(" smoothlife "),
      ftxui::hbox({
        ftxui::vbox({
          ftxui::filler(),
          player.move_ui->Render(),
          ftxui::filler(),
        }),
        ftxui::separator(),
        board.render(),
        ftxui::separator(),
        ftxui::vbox({
          ftxui::text(""),
          ftxui::text(fmt::format("  energy  : {}    ", player.energy)),
          ftxui::text(fmt::format("  surface : {}    ", player.surface)),
        }),
      }));
  });

  auto screen = ftxui::ScreenInteractive::FitComponent();
  screen.Loop(game_ui);
}

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
    game_loop();

  } catch (const std::exception &e) {
    SPDLOG_ERROR("Unhandled exception in main: {}", e.what());
  }
}
