#include "config.hpp"
#include "gameboard.hpp"
#include "player.hpp"

#include <internal_use_only/config.hpp>

namespace smoothlife {

void game_loop(bool skip_tutorial = false)
{
  Player player;
  Log log{ config::log_length };
  auto prng = std::ranlux24{ std::random_device{}() };

  using GameBoard = GameBoard<config::board_width, config::board_height, std::ranlux24>;
  GameBoard board{ prng, player, log };
  board.generate_level();

  if (skip_tutorial) { board.stage = GameStage::game; }

  auto screen = ftxui::ScreenInteractive::FitComponent();
  auto quit_button = ftxui::Button("Quit", screen.ExitLoopClosure());
  auto continue_button = ftxui::Button("Continue", [&] { ++board.stage; });
  auto retry_button = ftxui::Button("Retry", [&] { game_loop(true); });
  auto container = ftxui::Container::Horizontal({ player.move_ui, quit_button, continue_button, retry_button });

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
          ftxui::hbox(
            { ftxui::text(" energy  "), ftxui::text(fmt::format("{:>8}", player.energy)) | ftxui::color(Yellow) }),
          ftxui::hbox(
            { ftxui::text(" surface "), ftxui::text(fmt::format("{:>8}", player.surface)) | ftxui::color(Blue) }),
          ftxui::hbox(
            { ftxui::text(" score   "), ftxui::text(fmt::format("{:>8}", player.score)) | ftxui::color(Magenta) }),
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
    } else if (board.stage == GameStage::tutorial_1) {
      window = ftxui::dbox({
        window,
        ftxui::vbox({
          ftxui::text(""),
          ftxui::text(" Airy voice: Stop pitying yourself, fool! "),
          ftxui::text("     You're almost there! Keep on polishing! "),
          ftxui::separator(),
          ftxui::text(" Tutorial: ") | ftxui::bold,
          ftxui::hbox({
            ftxui::text(" Combine operations to make the "),
            ftxui::text("surface-number") | ftxui::color(Blue),
            ftxui::text(" as round as possible. "),
          }),
          ftxui::text(" A round number is an integer that ends with one or more '0's. "),
          ftxui::text(" Example: 592 is less round than 590 is less round than 600. "),
          ftxui::text(""),
          ftxui::hbox({ continue_button->Render() }),
        }) | ftxui::borderDouble
          | ftxui::clear_under | ftxui::center,
      });
    } else if (board.stage == GameStage::tutorial_2) {
      window = ftxui::dbox({
        window,
        ftxui::vbox({
          ftxui::text(""),
          ftxui::text(" Tutorial: ") | ftxui::bold,
          ftxui::hbox({
            ftxui::text(" Go to the exit ⋒ and your "),
            ftxui::text("score") | ftxui::color(Magenta),
            ftxui::text(" for this level is calculated. "),
          }),
          ftxui::text(" With a non-round number like 592 you lose a life. "),
          ftxui::hbox({
            ftxui::text(" The game ends if your "),
            ftxui::text("energy-value") | ftxui::color(Yellow),
            ftxui::text(" or your "),
            ftxui::text("lives") | ftxui::color(Red),
            ftxui::text(" reach zero. "),
          }),
          ftxui::text(fmt::format(
            " Each step and every {} seconds you lose one energy point. ", config::energy_decrement_time.count())),
          ftxui::text(" You can regain energy with a good polish. "),
          ftxui::text(fmt::format(" You need a minimum score of {} to win. ", config::min_win_score)),
          ftxui::text(""),
          ftxui::hbox({ continue_button->Render() }),
        }) | ftxui::borderDouble
          | ftxui::clear_under | ftxui::center,
      });
    } else if (board.stage == GameStage::ending) {
      if (player.score < config::min_win_score) {
        window = ftxui::dbox({
          window,
          ftxui::vbox({
            ftxui::text(""),
            ftxui::text(" You wake up feeling terrified. What a nightmare! "),
            ftxui::text(" Later that day you fail the exam :C"),
            ftxui::text(" You become a looser for the rest of your life."),
            ftxui::text(""),
            ftxui::text(fmt::format(" Total score: {}", player.score)) | ftxui::hcenter | ftxui::bold,
            ftxui::text(""),
            ftxui::hbox({ quit_button->Render(), retry_button->Render() }),
          }) | ftxui::borderDouble
            | ftxui::clear_under | ftxui::center,
        });
      } else {
        window = ftxui::dbox({
          window,
          ftxui::vbox({
            ftxui::text(""),
            ftxui::text(" You wake up feeling refreshed. What a great dream! "),
            ftxui::text(" Later that day you pass the exam *.* "),
            ftxui::text(" You become a master of your craft and live a smoothlife. "),
            ftxui::text(""),
            ftxui::text(fmt::format(" Total score: {}", player.score)) | ftxui::hcenter | ftxui::bold,
            ftxui::text(""),
            ftxui::hbox({ quit_button->Render(), retry_button->Render() }),
          }) | ftxui::borderDouble
            | ftxui::clear_under | ftxui::center,
        });
      }
    }

    return window;
  });

  std::atomic<bool> refresh_ui_continue = true;

  // This thread exists to decrement player.energy every few seconds
  std::thread refresh_ui([&] {
    while (refresh_ui_continue) {
      std::this_thread::sleep_for(config::energy_decrement_time);
      if (board.stage == GameStage::game) { player.energy -= 1; }
      screen.PostEvent(ftxui::Event::Custom);
    }
  });

  screen.Loop(game_ui);
  refresh_ui_continue = false;
  refresh_ui.join();
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
