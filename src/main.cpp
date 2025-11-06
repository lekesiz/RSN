/**
 * @file main.cpp
 * @brief Main entry point for RecoverySoftNetz application
 *
 * @author RecoverySoftNetz Team
 * @version 0.1.0
 */

#include "cli/cli.h"
#include "common/logging.h"

// Qt6 includes (conditional)
#ifdef QT_WIDGETS_LIB
#include "ui/main_window.h"
#include <QApplication>
#endif

using namespace rsn;

int main(int argc, char* argv[]) {
  common::Log(common::LogLevel::INFO, "RecoverySoftNetz v0.1.0 - Starting...");

  // Check if running in CLI mode or GUI mode
  bool use_gui = false;

#ifdef QT_WIDGETS_LIB
  // If Qt is available and no arguments provided, use GUI
  if (argc == 1) {
    use_gui = true;
  }

  // Check for explicit --gui flag
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--gui") {
      use_gui = true;
      break;
    }
  }
#endif

  if (use_gui) {
#ifdef QT_WIDGETS_LIB
    // Launch Qt6 GUI
    common::Log(common::LogLevel::INFO, "Launching Qt6 GUI mode");
    QApplication app(argc, argv);
    app.setApplicationName("RecoverySoftNetz");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("Netz Informatique");

    ui::MainWindow window;
    window.show();

    int result = app.exec();
    common::Log(common::LogLevel::INFO, "GUI application terminated");
    return result;
#else
    common::Log(common::LogLevel::ERROR, "Qt6 not available - falling back to CLI");
#endif
  }

  // Launch CLI mode
  common::Log(common::LogLevel::INFO, "Launching CLI mode");
  cli::CLI cli_app;
  int result = cli_app.Run(argc, argv);

  common::Log(common::LogLevel::INFO, "CLI application terminated");
  return result;
}
