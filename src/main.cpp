#include "config.hpp"
#include "gameboard.hpp"
#include "player.hpp"

#include <internal_use_only/config.hpp>

namespace smoothlife {

void game_loop()
{
  Player player;
  Log log{ config::log_length };
  auto prng = std::ranlux24{ std::random_device{}() };

  using GameBoard = GameBoard<config::board_width, config::board_height, std::ranlux24>;
  GameBoard board{ prng, player, log };
  board.generate_level();

  auto screen = ftxui::ScreenInteractive::FitComponent();
  auto quit_button = ftxui::Button("Quit", screen.ExitLoopClosure());
  auto continue_button = ftxui::Button("Continue", [&] { ++board.stage; });
  auto container = ftxui::Container::Horizontal({ player.move_ui, quit_button, continue_button });

  auto game_ui = ftxui::Renderer(container, [&] {
    using enum ftxui::Color::Palette16;
    if (board.stage == GameStage::game) { player.move_ui->TakeFocus(); }
    auto window = ftxui::window(ftxui::text(" smoothlife ") | ftxui::hcenter | ftxui::bold,
      ftxui::hbox({
        ftxui::vbox({
          ftxui::text(player.health()) | ftxui::hcenter | ftxui::color(RedLight) | ftxui::border,
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
          ftxui::text(fmt::format(" energy  {:>6}", player.energy)),
          ftxui::text(fmt::format(" surface {:>6}", player.surface)),
          ftxui::text(fmt::format(" score   {:>6}", player.score)),
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

    if (board.stage == GameStage::intro) {
      window = ftxui::dbox({
        window,
        ftxui::vbox({
          ftxui::text(""),
          ftxui::text(" Buzzing thoughts: I'm feeling so burned out... "),
          ftxui::text("     Ironic... my dream is to become a master polisher. "),
          ftxui::text("     Making things smooth, shiny and round. "),
          ftxui::text("     Why does my life have to be so rough then? "),
          ftxui::text("     I'm afraid of failing the final exams tomorrow. "),
          ftxui::text(""),
          ftxui::text(" You fall asleep at your working desk. "),
          ftxui::text(""),
          ftxui::hbox({ continue_button->Render() }),
        }) | ftxui::borderDouble
          | ftxui::clear_under | ftxui::center,
      });
    } else if (board.stage == GameStage::tutorial) {
      window = ftxui::dbox({
        window,
        ftxui::vbox({
          ftxui::text(""),
          ftxui::text(" Airy voice: Stop pitying yourself, fool! "),
          ftxui::text("     You're almost there! Keep on polishing! "),
          ftxui::separator(),
          ftxui::text(" Combine operations to make the surface-number as round as possible. "),
          ftxui::text(" Example: 592 is less round than 590 is less round than 600. "),
          ftxui::text(" Go to the exit ⋒ and your score is calculated for this level. "),
          ftxui::text(" With a roundness of 0 you lose a life."),
          ftxui::text(" The game ends if your energy or your lives reach zero. "),
          ftxui::text(""),
          ftxui::hbox({ continue_button->Render() }),
        }) | ftxui::borderDouble
          | ftxui::clear_under | ftxui::center,
      });
    } else if (board.stage == GameStage::ending) {
      if (player.lives == 0 || board.level < config::min_win_level) {
        window = ftxui::dbox({
          window,
          ftxui::vbox({
            ftxui::text(""),
            ftxui::text(" You wake up feeling terrified. What a nightmare ! "),
            ftxui::text(" Later that day you fail the exam :C"),
            ftxui::text(" You become a looser for the rest of your life."),
            ftxui::text(""),
            ftxui::text(fmt::format(" Total score: {}", player.score)) | ftxui::hcenter | ftxui::bold,
            ftxui::text(""),
            ftxui::hbox({ quit_button->Render() }),
          }) | ftxui::borderDouble
            | ftxui::clear_under | ftxui::center,
        });
      } else if (player.energy == 0) {
        window = ftxui::dbox({
          window,
          ftxui::vbox({
            ftxui::text(""),
            ftxui::text(" You wake up feeling refreshed. What a great dream ! "),
            ftxui::text(" Later that day you pass the exam *.* "),
            ftxui::text(" You become a master of your craft and live a smoothlife. "),
            ftxui::text(""),
            ftxui::text(fmt::format(" Total score: {}", player.score)) | ftxui::hcenter | ftxui::bold,
            ftxui::text(""),
            ftxui::hbox({ quit_button->Render() }),
          }) | ftxui::borderDouble
            | ftxui::clear_under | ftxui::center,
        });
      }
    }

    return window;
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
