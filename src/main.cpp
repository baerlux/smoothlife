#include <array>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <docopt/docopt.h>
#include <spdlog/spdlog.h>

#include <internal_use_only/config.hpp>


struct Player
{
  int x, y;
  ftxui::Components buttons;
  ftxui::Component move_ui;

  Player(int x_, int y_) : x{ x_ }, y{ y_ }
  {
    buttons.push_back(ftxui::Button("N ᐃ", [&] { y++; }));
    buttons.push_back(ftxui::Button("W ᐊ", [&] { x--; }));
    buttons.push_back(ftxui::Button("S ᐁ", [&] { y--; }));
    buttons.push_back(ftxui::Button("E ᐅ", [&] { x++; }));

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

template<std::size_t Width, std::size_t Height> struct GameBoard
{
  explicit GameBoard(Player &p) : player{ p } {}

  [[nodiscard]] int get_field(std::size_t x, std::size_t y) const { return state.at(x + Width * y); }

  void set_field(std::size_t x, std::size_t y, int value)
  {
    assert(value >= 0 && value < 10);// NOLINT magic number okay here
    state.at(x + Width * y) = value;
  }

  [[nodiscard]] ftxui::Element render() const
  {
    std::vector<ftxui::Element> rows;

    for (std::size_t y = Height - 1; y < Height; --y) {
      std::vector<ftxui::Element> row;
      for (std::size_t x = 0; x < Width; ++x) {
        std::string field = " ";
        int value = get_field(x, y);
        if (value > 0) { field = std::to_string(value); }
        if (static_cast<int>(x) == player.x && static_cast<int>(y) == player.y) { field = "@"; }
        row.push_back(ftxui::text(fmt::format(" {} ", field)) | ftxui::border);
      }
      rows.push_back(ftxui::hbox(std::move(row)));
    }

    return ftxui::vbox(std::move(rows));
  }

private:
  std::array<int, Width * Height> state{};
  Player &player;
};

void run_game()
{
  Player player{ 2, 2 };
  GameBoard<5, 5> board{ player };// NOLINT magic number okay
  board.set_field(0, 3, 6);// NOLINT magic number
  board.set_field(4, 1, 7);// NOLINT magic number
  board.set_field(1, 2, 2);// NOLINT magic number

  auto game_ui = ftxui::Renderer(player.move_ui, [&] {
    return ftxui::window(ftxui::text(" smoothlife "),
      ftxui::vbox({
        ftxui::text(fmt::format("{}, {}", player.x, player.y)),
        ftxui::separator(),
        board.render(),
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
          smoothlife test numbers
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
