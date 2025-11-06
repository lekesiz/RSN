/**
 * @file cli.cpp
 * @brief Implementation of command-line interface
 */

#include "cli/cli.h"
#include "core/recovery_engine.h"
#include "core/file_registry.h"
#include "core/file_carving.h"
#include "common/logging.h"
#include "common/utils.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstring>

namespace rsn {
namespace cli {

CLI::CLI() {
  recovery_engine_ = std::make_unique<core::RecoveryEngine>();
  options_.interactive = false;
  options_.verbose = false;
  options_.list_devices = false;
  options_.export_csv = false;
}

CLI::~CLI() {}

int CLI::Run(int argc, char* argv[]) {
  if (!ParseArguments(argc, argv)) {
    PrintHelp();
    return 1;
  }

  // Handle special commands
  if (options_.list_devices) {
    return HandleListDevices();
  }

  if (options_.interactive) {
    return HandleInteractive();
  }

  // Handle mode-based commands
  if (options_.mode == "scan") {
    return HandleScan();
  } else if (options_.mode == "recover") {
    return HandleRecover();
  } else if (options_.mode == "carve") {
    return HandleCarve();
  } else {
    std::cerr << "Error: Unknown mode '" << options_.mode << "'" << std::endl;
    return 1;
  }
}

bool CLI::ParseArguments(int argc, char* argv[]) {
  if (argc < 2) {
    return false;
  }

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      PrintHelp();
      exit(0);
    } else if (arg == "-v" || arg == "--version") {
      PrintVersion();
      exit(0);
    } else if (arg == "-l" || arg == "--list-devices") {
      options_.list_devices = true;
    } else if (arg == "-i" || arg == "--interactive") {
      options_.interactive = true;
    } else if (arg == "--verbose") {
      options_.verbose = true;
    } else if (arg == "-d" || arg == "--device") {
      if (i + 1 < argc) {
        options_.device_path = argv[++i];
      }
    } else if (arg == "-o" || arg == "--output") {
      if (i + 1 < argc) {
        options_.output_path = argv[++i];
      }
    } else if (arg == "-m" || arg == "--mode") {
      if (i + 1 < argc) {
        options_.mode = argv[++i];
      }
    } else if (arg == "-t" || arg == "--type") {
      if (i + 1 < argc) {
        options_.file_types.push_back(argv[++i]);
      }
    } else if (arg == "--export-csv") {
      options_.export_csv = true;
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        options_.export_path = argv[++i];
      } else {
        options_.export_path = "results.csv";
      }
    }
  }

  return true;
}

void CLI::PrintHelp() {
  std::cout << "\n";
  std::cout << "RecoverySoftNetz - AI-Powered Data Recovery Solution\n";
  std::cout << "Version 0.1.0\n";
  std::cout << "\n";
  std::cout << "USAGE:\n";
  std::cout << "  rsn [OPTIONS]\n";
  std::cout << "\n";
  std::cout << "OPTIONS:\n";
  std::cout << "  -h, --help                 Show this help message\n";
  std::cout << "  -v, --version              Show version information\n";
  std::cout << "  -l, --list-devices         List available devices\n";
  std::cout << "  -i, --interactive          Run in interactive mode\n";
  std::cout << "  -d, --device <path>        Device path (e.g., /dev/sda1)\n";
  std::cout << "  -o, --output <path>        Output directory for recovered files\n";
  std::cout << "  -m, --mode <mode>          Mode: scan, recover, carve\n";
  std::cout << "  -t, --type <type>          Filter by file type (can be repeated)\n";
  std::cout << "  --export-csv [path]        Export results to CSV\n";
  std::cout << "  --verbose                  Enable verbose output\n";
  std::cout << "\n";
  std::cout << "EXAMPLES:\n";
  std::cout << "  # List devices\n";
  std::cout << "  rsn --list-devices\n";
  std::cout << "\n";
  std::cout << "  # Scan a device\n";
  std::cout << "  rsn --device /dev/sda1 --mode scan\n";
  std::cout << "\n";
  std::cout << "  # Recover all files\n";
  std::cout << "  rsn --device /dev/sda1 --mode recover --output /recovery\n";
  std::cout << "\n";
  std::cout << "  # File carving for JPEG images\n";
  std::cout << "  rsn --device /dev/sda1 --mode carve --type jpeg --output /carved\n";
  std::cout << "\n";
  std::cout << "  # Interactive mode\n";
  std::cout << "  rsn --interactive\n";
  std::cout << "\n";
}

void CLI::PrintVersion() {
  std::cout << "RecoverySoftNetz v0.1.0\n";
  std::cout << "© 2025 Netz Informatique\n";
  std::cout << "Build: " << __DATE__ << " " << __TIME__ << "\n";
}

int CLI::HandleScan() {
  if (options_.device_path.empty()) {
    std::cerr << "Error: Device path not specified. Use -d or --device\n";
    return 1;
  }

  std::cout << "╔══════════════════════════════════════════════════════════╗\n";
  std::cout << "║         RecoverySoftNetz - Device Scan                  ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════╝\n";
  std::cout << "\n";
  std::cout << "Device: " << options_.device_path << "\n";
  std::cout << "\n";

  if (!recovery_engine_->SetDevice(options_.device_path)) {
    std::cerr << "Error: " << recovery_engine_->GetLastError() << "\n";
    return 1;
  }

  std::cout << "Starting scan...\n";
  if (!recovery_engine_->StartScan()) {
    std::cerr << "Error: " << recovery_engine_->GetLastError() << "\n";
    return 1;
  }

  PrintScanProgress();

  std::cout << "\n";
  std::cout << "Scan complete!\n";
  std::cout << "\n";

  PrintResults();

  if (options_.export_csv) {
    ExportToCSV(options_.export_path);
  }

  return 0;
}

int CLI::HandleRecover() {
  if (options_.device_path.empty() || options_.output_path.empty()) {
    std::cerr << "Error: Device and output paths required\n";
    return 1;
  }

  // First scan
  if (HandleScan() != 0) {
    return 1;
  }

  std::cout << "\n";
  std::cout << "Output directory: " << options_.output_path << "\n";
  std::cout << "\n";

  if (!ConfirmAction("Proceed with recovery?")) {
    std::cout << "Recovery cancelled.\n";
    return 0;
  }

  recovery_engine_->SetOutputPath(options_.output_path);
  uint64_t recovered = recovery_engine_->RecoverAllFiles();

  std::cout << "\n";
  std::cout << "✓ Successfully recovered " << recovered << " files\n";
  std::cout << "\n";

  return 0;
}

int CLI::HandleCarve() {
  if (options_.device_path.empty() || options_.output_path.empty()) {
    std::cerr << "Error: Device and output paths required\n";
    return 1;
  }

  std::cout << "╔══════════════════════════════════════════════════════════╗\n";
  std::cout << "║         RecoverySoftNetz - File Carving                 ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════╝\n";
  std::cout << "\n";

  core::FileCarvingEngine carver;
  carver.Initialize();

  std::cout << "Device: " << options_.device_path << "\n";
  std::cout << "Output: " << options_.output_path << "\n";
  std::cout << "\n";

  if (!options_.file_types.empty()) {
    std::cout << "File types: ";
    for (const auto& type : options_.file_types) {
      std::cout << type << " ";
    }
    std::cout << "\n\n";
  }

  std::cout << "Starting file carving...\n";
  uint64_t carved = carver.CarveFiles(options_.device_path, options_.output_path);

  auto stats = carver.GetStats();
  std::cout << "\n";
  std::cout << "Carving complete!\n";
  std::cout << "  Files found: " << stats.files_found << "\n";
  std::cout << "  Files carved: " << stats.files_carved << "\n";
  std::cout << "  Bytes scanned: " << common::FormatBytes(stats.bytes_scanned) << "\n";
  std::cout << "  Time: " << std::fixed << std::setprecision(2) << stats.scan_time_seconds << "s\n";
  std::cout << "\n";

  return 0;
}

int CLI::HandleListDevices() {
  std::cout << "\n";
  std::cout << "Available Devices:\n";
  std::cout << "══════════════════════════════════════════════════════════\n";
  PrintDeviceList();
  std::cout << "\n";
  return 0;
}

int CLI::HandleInteractive() {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════╗\n";
  std::cout << "║   RecoverySoftNetz - Interactive Mode                   ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════╝\n";
  std::cout << "\n";

  while (true) {
    std::cout << "Menu:\n";
    std::cout << "  1. List devices\n";
    std::cout << "  2. Scan device\n";
    std::cout << "  3. Recover files\n";
    std::cout << "  4. File carving\n";
    std::cout << "  5. Exit\n";
    std::cout << "\n";

    std::string choice = ReadUserInput("Enter choice (1-5): ");

    if (choice == "1") {
      HandleListDevices();
    } else if (choice == "2") {
      options_.device_path = ReadUserInput("Device path: ");
      HandleScan();
    } else if (choice == "3") {
      options_.device_path = ReadUserInput("Device path: ");
      options_.output_path = ReadUserInput("Output directory: ");
      HandleRecover();
    } else if (choice == "4") {
      options_.device_path = ReadUserInput("Device path: ");
      options_.output_path = ReadUserInput("Output directory: ");
      HandleCarve();
    } else if (choice == "5") {
      std::cout << "Goodbye!\n";
      break;
    } else {
      std::cout << "Invalid choice. Please try again.\n\n";
    }
  }

  return 0;
}

void CLI::PrintDeviceList() {
  // Mock device list (TODO: implement real device scanning)
  std::cout << "  /dev/sda1    System Disk - Partition 1    NTFS    500 GB\n";
  std::cout << "  /dev/sdb1    External Drive                ext4    1 TB\n";
  std::cout << "  /dev/disk2s1 USB Flash Drive              FAT32   32 GB\n";
}

void CLI::PrintScanProgress() {
  // Simulate progress (TODO: use real progress from engine)
  const int total_steps = 20;
  for (int i = 0; i <= total_steps; ++i) {
    int percentage = (i * 100) / total_steps;

    std::cout << "\r[";
    for (int j = 0; j < total_steps; ++j) {
      if (j < i) {
        std::cout << "=";
      } else if (j == i) {
        std::cout << ">";
      } else {
        std::cout << " ";
      }
    }
    std::cout << "] " << percentage << "% ";
    std::cout.flush();

    // Small delay to simulate progress
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << "\n";
}

void CLI::PrintResults() {
  auto registry = recovery_engine_->GetFileRegistry();
  if (!registry) {
    std::cout << "No files found.\n";
    return;
  }

  const auto& files = registry->GetFiles();
  if (files.empty()) {
    std::cout << "No files found.\n";
    return;
  }

  std::cout << "Found " << files.size() << " recoverable files:\n";
  std::cout << "══════════════════════════════════════════════════════════\n";
  std::cout << std::left
            << std::setw(30) << "Filename"
            << std::setw(15) << "Size"
            << std::setw(20) << "Type"
            << std::setw(10) << "Confidence"
            << "\n";
  std::cout << "──────────────────────────────────────────────────────────\n";

  for (const auto& file : files) {
    std::cout << std::left
              << std::setw(30) << file.filename.substr(0, 28)
              << std::setw(15) << common::FormatBytes(file.size_bytes)
              << std::setw(20) << file.file_type.substr(0, 18)
              << std::setw(10) << (static_cast<int>(file.recovery_confidence * 100)) << "%"
              << "\n";
  }
}

void CLI::ExportToCSV(const std::string& path) {
  auto registry = recovery_engine_->GetFileRegistry();
  if (!registry) {
    return;
  }

  std::ofstream csv(path);
  if (!csv.is_open()) {
    std::cerr << "Error: Cannot create CSV file: " << path << "\n";
    return;
  }

  // Header
  csv << "Filename,Path,Size,Type,Confidence,Status,Fragmented\n";

  // Data
  const auto& files = registry->GetFiles();
  for (const auto& file : files) {
    csv << "\"" << file.filename << "\","
        << "\"" << file.original_path << "\","
        << file.size_bytes << ","
        << "\"" << file.file_type << "\","
        << (file.recovery_confidence * 100) << ","
        << (file.is_deleted ? "Deleted" : "Active") << ","
        << (file.is_fragmented ? "Yes" : "No") << "\n";
  }

  csv.close();
  std::cout << "Results exported to: " << path << "\n";
}

std::string CLI::ReadUserInput(const std::string& prompt) {
  std::cout << prompt;
  std::string input;
  std::getline(std::cin, input);
  return input;
}

bool CLI::ConfirmAction(const std::string& message) {
  std::cout << message << " (y/n): ";
  std::string response;
  std::getline(std::cin, response);
  return (response == "y" || response == "Y" || response == "yes");
}

}  // namespace cli
}  // namespace rsn
