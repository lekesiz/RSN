/**
 * @file main_window.h
 * @brief Main application window (Qt6)
 */

#ifndef RSN_UI_MAIN_WINDOW_H_
#define RSN_UI_MAIN_WINDOW_H_

#include <memory>
#include <string>
#include <vector>

// Forward declarations for when Qt6 is available
#ifdef QT_WIDGETS_LIB
#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>
#endif

namespace rsn {

// Forward declarations
namespace core {
class RecoveryEngine;
struct RecoverableFile;
}

namespace ui {

// Forward declaration
class DeviceWizard;

/**
 * @brief Main application window
 *
 * The main window provides the primary user interface for RecoverySoftNetz,
 * including device selection, scan progress, and results display.
 */
#ifdef QT_WIDGETS_LIB
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow() override;

 signals:
  void scanStarted();
  void scanProgress(int percentage);
  void scanCompleted();

 private slots:
  void onStartScan();
  void onStopScan();
  void onSelectDevice();
  void onRecoverSelected();
  void onRecoverAll();
  void onUpdateProgress(int percentage);
  void onScanCompleted();

 private:
  // UI Components
  QWidget* central_widget_;
  QVBoxLayout* main_layout_;

  // Menu bar actions
  QAction* action_select_device_;
  QAction* action_start_scan_;
  QAction* action_stop_scan_;
  QAction* action_exit_;
  QAction* action_about_;

  // Control buttons
  QPushButton* btn_select_device_;
  QPushButton* btn_start_scan_;
  QPushButton* btn_stop_scan_;
  QPushButton* btn_recover_selected_;
  QPushButton* btn_recover_all_;

  // Progress display
  QProgressBar* progress_bar_;
  QLabel* lbl_status_;
  QLabel* lbl_device_info_;
  QLabel* lbl_files_found_;

  // Results table
  QTableWidget* results_table_;

  // Core engine
  std::unique_ptr<core::RecoveryEngine> recovery_engine_;
  std::unique_ptr<DeviceWizard> device_wizard_;

  // State
  std::string current_device_;
  bool is_scanning_;

  // Private methods
  void SetupUI();
  void SetupMenuBar();
  void SetupConnections();
  void UpdateResultsTable();
  void UpdateDeviceInfo();
  void EnableControls(bool enable);
};
#else
// Stub implementation when Qt is not available
class MainWindow {
 public:
  MainWindow() {}
  ~MainWindow() {}
};
#endif

}  // namespace ui
}  // namespace rsn

#endif  // RSN_UI_MAIN_WINDOW_H_
