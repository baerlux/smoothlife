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
  Pos pos{};
  ftxui::Components buttons;
  ftxui::Component move_ui;

  Player()
  {
    buttons.push_back(ftxui::Button("N ᐃ", [&] { pos.y++; }));
    buttons.push_back(ftxui::Button("W ᐊ", [&] { pos.x--; }));
    buttons.push_back(ftxui::Button("S ᐁ", [&] { pos.y--; }));
    buttons.push_back(ftxui::Button("E ᐅ", [&] { pos.x++; }));

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

void run_game()
{
  Player player;

  auto game_ui = ftxui::Renderer(player.move_ui, [&] {
    return ftxui::window(ftxui::text(" smoothlife "),
      ftxui::vbox({
        ftxui::text(fmt::format("{}, {}", player.pos.x, player.pos.y)),
        ftxui::separator(),
        ftxui::hbox({
          player.move_ui->Render(),
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
    SPDLOG_ERROR("Unhandled exception in main: {}", e.what());
  }
}
