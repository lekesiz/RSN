/**
 * @file cli.h
 * @brief Command-line interface for RecoverySoftNetz
 */

#ifndef RSN_CLI_CLI_H_
#define RSN_CLI_CLI_H_

#include <string>
#include <vector>
#include <memory>

namespace rsn {

// Forward declarations
namespace core {
class RecoveryEngine;
}

namespace cli {

/**
 * @brief Command-line argument structure
 */
struct CLIOptions {
  std::string device_path;
  std::string output_path;
  std::string mode;              // "scan", "recover", "carve"
  std::vector<std::string> file_types;  // Filter by file types
  bool interactive;
  bool verbose;
  bool list_devices;
  bool export_csv;
  std::string export_path;
};

/**
 * @brief Command-line interface handler
 */
class CLI {
 public:
  CLI();
  ~CLI();

  /**
   * @brief Run the CLI with given arguments
   */
  int Run(int argc, char* argv[]);

  /**
   * @brief Display help message
   */
  void PrintHelp();

  /**
   * @brief Display version information
   */
  void PrintVersion();

 private:
  std::unique_ptr<core::RecoveryEngine> recovery_engine_;
  CLIOptions options_;

  // Command handlers
  int HandleScan();
  int HandleRecover();
  int HandleCarve();
  int HandleListDevices();
  int HandleInteractive();

  // Helper methods
  bool ParseArguments(int argc, char* argv[]);
  void PrintDeviceList();
  void PrintScanProgress();
  void PrintResults();
  void ExportToCSV(const std::string& path);
  std::string ReadUserInput(const std::string& prompt);
  bool ConfirmAction(const std::string& message);
};

}  // namespace cli
}  // namespace rsn

#endif  // RSN_CLI_CLI_H_
