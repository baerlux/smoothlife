#include <array>
#include <random>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <docopt/docopt.h>
#include <spdlog/spdlog.h>

#include <internal_use_only/config.hpp>


struct Player
{
  // set by GameBoard
  int x{}, y{};
  int energy{};
  int surface{};

  std::function<void()> interaction;
  ftxui::Box bounds;

  // player control ui
  ftxui::Components buttons;
  ftxui::Component move_ui;


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


struct Field
{
  enum struct Type { empty, exit, add, sub, mul, div };
  Type type = Type::empty;
  int value = 0;

  using enum Type;

  [[nodiscard]] Field inverse() const
  {
    switch (type) {
    case add:
      return { sub, value };
    case sub:
      return { add, value };
    case mul:
      return { div, value };
    case div:
      return { mul, value };
    default:
      return { empty, value };
    }
  }

  void apply(int &surface)
  {
    switch (type) {
    case add:
      surface += value;
      break;
    case sub:
      surface -= value;
      break;
    case mul:
      surface *= value;
      break;
    case div:
      surface /= value;
      break;
    default:
      return;
    }
    // reset field if it was applied
    type = empty;
    value = 0;
  }
};


int roundness(int num)
{
  int r = 0;
  int d = 10;// NOLINT
  while (true) {
    if (num % d == 0) {
      ++r;
    } else {
      break;
    }
    d *= 10;// NOLINT
  }
  return r;
}


template<std::size_t Width, std::size_t Height, class Prng> struct GameBoard
{
  std::array<Field, Width * Height> state;
  Player &player;
  Prng &rnd;

  using enum Field::Type;
  explicit GameBoard(Prng &randomness_provider, Player &p) : player{ p }, rnd{ randomness_provider }
  {
    player.bounds.x_max = Width - 1;
    player.bounds.x_min = 0;
    player.bounds.y_max = Height - 1;
    player.bounds.y_min = 0;
    player.interaction = [&] {
      Field &f = get_field(player.x, player.y);
      switch (f.type) {
      case empty:
        break;
      case exit:
        generate_level();
        break;
      default:
        f.apply(player.surface);
        break;
      }
    };
  }

  void generate_level()
  {
    // clear field
    state = std::array<Field, Width * Height>{};

    using uni = std::uniform_int_distribution<int>;
    uni in_board_width{ 0, Width - 1 };
    uni in_board_height{ 0, Height - 1 };
    uni in_field_types{ 2, 5 };// NOLINT
    uni in_one_to_nine{ 1, 9 };// NOLINT

    int goal = 10000;// in_one_to_nine(rnd) * 1000;// NOLINT

    std::array<bool, Width * Height> occupied{};

    // player
    occupied.at(pack2d(0, 0)) = true;
    player.energy = 100;// NOLINT
    player.x = 0;
    player.y = 0;

    // exit
    occupied.at(pack2d(Width - 1, Height - 1)) = true;
    set_field(Width - 1, Height - 1, { exit });

    // generate field chain
    for (int i = 0; i < 5; ++i) {// NOLINT

      Field rnd_f{ static_cast<Field::Type>(in_field_types(rnd)), in_one_to_nine(rnd) };

      if (rnd_f.type == mul) {
        int rem = goal % rnd_f.value;
        while (rem != 0) {
          rnd_f.value = in_one_to_nine(rnd);
          rem = goal % rnd_f.value;
        }
      }

      int rnd_x{};
      int rnd_y{};

      // search for empty field
      do {
        rnd_x = in_board_width(rnd);
        rnd_y = in_board_height(rnd);
      } while (occupied.at(pack2d(rnd_x, rnd_y)));

      set_field(rnd_x, rnd_y, rnd_f);// NOLINT
      occupied.at(pack2d(rnd_x, rnd_y)) = true;

      rnd_f.inverse().apply(goal);
    }

    player.surface = goal;
  }

  static size_t pack2d(int x, int y) { return static_cast<std::size_t>(x) + Width * static_cast<std::size_t>(y); }

  [[nodiscard]] const Field &get_field(int x, int y) const { return state.at(pack2d(x, y)); }

  [[nodiscard]] Field &get_field(int x, int y) { return state.at(pack2d(x, y)); }

  void set_field(int x, int y, Field field) { state.at(pack2d(x, y)) = field; }

  [[nodiscard]] ftxui::Element render() const
  {
    std::vector<ftxui::Element> rows;

    for (int y = Height - 1; y >= 0; --y) {
      std::vector<ftxui::Element> row;
      for (int x = 0; x < static_cast<int>(Width); ++x) {
        using enum Field::Type;
        using enum ftxui::Color::Palette16;
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

void run_game()
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

    run_game();

  } catch (const std::exception &e) {
    SPDLOG_ERROR("Unhandled exception in main: {}", e.what());
  }
}
