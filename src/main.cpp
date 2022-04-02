#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <docopt/docopt.h>
#include <spdlog/spdlog.h>

#include <internal_use_only/config.hpp>


struct Pos
{
  int x, y;
};

struct Player
{
  Pos pos;
};

void run_game()
{
  Player player{};

  auto upButton = ftxui::Button("N ᐃ", [&] { player.pos.y++; });
  auto leftButton = ftxui::Button("W ᐊ", [&] { player.pos.x--; });
  auto downButton = ftxui::Button("S ᐁ", [&] { player.pos.y--; });
  auto rightButton = ftxui::Button("E ᐅ", [&] { player.pos.x++; });

  auto moveCtl = ftxui::Container::Vertical({
    ftxui::Renderer(upButton,
      [&] {
        return ftxui::hbox({
          ftxui::filler(),
          upButton->Render(),
          ftxui::filler(),
        });
      }),
    ftxui::Container::Horizontal({
      leftButton,
      downButton,
      rightButton,
    }),
  });

  auto ui = ftxui::Renderer(moveCtl, [&] {
    return ftxui::window(ftxui::text("smoothlife"),
      ftxui::vbox({
        ftxui::text(fmt::format("{}, {}", player.pos.x, player.pos.y)),
        ftxui::separator(),
        ftxui::hbox({
          moveCtl->Render(),
        }),
      }));
  });

  auto screen = ftxui::ScreenInteractive::FitComponent();
  screen.Loop(ui);
}


int main(int argc, const char **argv)
{
  try {
    static constexpr auto USAGE =
      R"(
    Usage:
          smoothlife
          smoothlife test spdlog
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

    if (args["spdlog"].asBool()) {
      SPDLOG_INFO("spdlog works!");
      SPDLOG_WARN("careful!");
      SPDLOG_ERROR("bad stuff!");
    } else {
      run_game();
    }
  } catch (const std::exception &e) {
    fmt::print("Unhandled exception in main: {}", e.what());
  }
}
