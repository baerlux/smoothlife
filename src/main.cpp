#include <array>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <docopt/docopt.h>
#include <spdlog/spdlog.h>

#include <internal_use_only/config.hpp>


struct Player
{
  // position
  int x, y;
  int energy = 4;

  // ui
  ftxui::Components buttons;
  ftxui::Component move_ui;

  // defined by GameBoard
  ftxui::Box bounds;
  std::function<void()> interaction;

  Player(int x_, int y_) : x{ x_ }, y{ y_ }
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


struct Field
{
  enum struct Type { empty, exit, add, sub, mul, div };
  Type type = Type::empty;
  int value = 0;

  void apply(Player &player)
  {
    using enum Field::Type;
    switch (type) {
    case add:
      player.energy += value;
      break;
    case sub:
      player.energy -= value;
      break;
    case mul:
      player.energy *= value;
      break;
    case div:
      player.energy /= value;
      break;
    case exit:
      return;
    default:
      break;
    }
    // reset field
    type = empty;
    value = 0;
  }
};

template<std::size_t Width, std::size_t Height> struct GameBoard
{
  std::array<Field, Width * Height> state{};
  Player &player;

  explicit GameBoard(Player &p) : player{ p }
  {
    player.bounds.x_max = Width - 1;
    player.bounds.x_min = 0;
    player.bounds.y_max = Height - 1;
    player.bounds.y_min = 0;
    player.interaction = [&] { get_field(player.x, player.y).apply(player); };
  }

  [[nodiscard]] const Field &get_field(int x, int y) const
  {
    return state.at(static_cast<std::size_t>(x) + Width * static_cast<std::size_t>(y));
  }

  [[nodiscard]] Field &get_field(int x, int y)
  {
    return state.at(static_cast<std::size_t>(x) + Width * static_cast<std::size_t>(y));
  }

  void set_field(int x, int y, Field field)
  {
    state.at(static_cast<std::size_t>(x) + Width * static_cast<std::size_t>(y)) = field;
  }

  [[nodiscard]] ftxui::Element render() const
  {
    std::vector<ftxui::Element> rows;

    for (int y = Height - 1; y >= 0; --y) {
      std::vector<ftxui::Element> row;
      for (int x = 0; x < static_cast<int>(Width); ++x) {
        std::string repr = " ";
        const Field &field = get_field(x, y);
        using enum Field::Type;
        if (field.type != empty && field.type != exit && field.value > 0 && field.value < 10) {// NOLINT
          repr = std::to_string(field.value);
        } else if (field.type == exit) {
          repr = "⛝";
        }
        if (x == player.x && y == player.y) { repr = "@"; }
        row.push_back(ftxui::text(fmt::format(" {} ", repr)) | ftxui::border);
      }
      rows.push_back(ftxui::hbox(std::move(row)));
    }

    return ftxui::vbox(std::move(rows));
  }
};

void run_game()
{
  Player player{ 2, 2 };
  GameBoard<5, 5> board{ player };// NOLINT magic number okay

  using enum Field::Type;
  board.set_field(0, 3, { div, 2 });// NOLINT magic number
  board.set_field(4, 1, { mul, 7 });// NOLINT magic number
  board.set_field(1, 2, { add, 4 });// NOLINT magic number
  board.set_field(4, 0, { exit });// NOLINT magic number

  auto game_ui = ftxui::Renderer(player.move_ui, [&] {
    return ftxui::window(ftxui::text(" smoothlife "),
      ftxui::vbox({
        ftxui::text(fmt::format("energy: {}", player.energy)),
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

    run_game();

  } catch (const std::exception &e) {
    SPDLOG_ERROR("Unhandled exception in main: {}", e.what());
  }
}
