/**
 * @file main_window.cpp
 * @brief Implementation of main window
 */

#include "ui/main_window.h"
#include "ui/device_wizard.h"
#include "core/recovery_engine.h"
#include "core/file_registry.h"
#include "common/logging.h"
#include "common/utils.h"

#ifdef QT_WIDGETS_LIB
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QTimer>
#endif

namespace rsn {
namespace ui {

#ifdef QT_WIDGETS_LIB

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent),
    central_widget_(nullptr),
    main_layout_(nullptr),
    is_scanning_(false) {

  // Initialize recovery engine
  recovery_engine_ = std::make_unique<core::RecoveryEngine>();

  SetupUI();
  SetupMenuBar();
  SetupConnections();

  common::Log(common::LogLevel::INFO, "MainWindow initialized");
}

MainWindow::~MainWindow() {
  if (is_scanning_) {
    recovery_engine_->StopScan();
  }
  common::Log(common::LogLevel::INFO, "MainWindow destroyed");
}

void MainWindow::SetupUI() {
  setWindowTitle("RecoverySoftNetz - Data Recovery Solution");
  setMinimumSize(1024, 768);

  // Central widget
  central_widget_ = new QWidget(this);
  setCentralWidget(central_widget_);

  main_layout_ = new QVBoxLayout(central_widget_);
  main_layout_->setSpacing(10);
  main_layout_->setContentsMargins(10, 10, 10, 10);

  // Device info section
  lbl_device_info_ = new QLabel("No device selected", this);
  lbl_device_info_->setStyleSheet("font-weight: bold; padding: 5px;");
  main_layout_->addWidget(lbl_device_info_);

  // Control buttons section
  QHBoxLayout* control_layout = new QHBoxLayout();

  btn_select_device_ = new QPushButton("Select Device", this);
  btn_start_scan_ = new QPushButton("Start Scan", this);
  btn_stop_scan_ = new QPushButton("Stop Scan", this);

  btn_start_scan_->setEnabled(false);
  btn_stop_scan_->setEnabled(false);

  control_layout->addWidget(btn_select_device_);
  control_layout->addWidget(btn_start_scan_);
  control_layout->addWidget(btn_stop_scan_);
  control_layout->addStretch();

  main_layout_->addLayout(control_layout);

  // Progress section
  QVBoxLayout* progress_layout = new QVBoxLayout();

  lbl_status_ = new QLabel("Ready", this);
  progress_bar_ = new QProgressBar(this);
  progress_bar_->setRange(0, 100);
  progress_bar_->setValue(0);

  lbl_files_found_ = new QLabel("Files found: 0", this);

  progress_layout->addWidget(lbl_status_);
  progress_layout->addWidget(progress_bar_);
  progress_layout->addWidget(lbl_files_found_);

  main_layout_->addLayout(progress_layout);

  // Results table
  results_table_ = new QTableWidget(this);
  results_table_->setColumnCount(6);
  results_table_->setHorizontalHeaderLabels({
    "Filename", "Path", "Size", "Type", "Confidence", "Status"
  });

  results_table_->horizontalHeader()->setStretchLastSection(true);
  results_table_->setSelectionBehavior(QTableWidget::SelectRows);
  results_table_->setEditTriggers(QTableWidget::NoEditTriggers);
  results_table_->setAlternatingRowColors(true);

  main_layout_->addWidget(results_table_);

  // Recovery buttons
  QHBoxLayout* recovery_layout = new QHBoxLayout();

  btn_recover_selected_ = new QPushButton("Recover Selected", this);
  btn_recover_all_ = new QPushButton("Recover All", this);

  btn_recover_selected_->setEnabled(false);
  btn_recover_all_->setEnabled(false);

  recovery_layout->addStretch();
  recovery_layout->addWidget(btn_recover_selected_);
  recovery_layout->addWidget(btn_recover_all_);

  main_layout_->addLayout(recovery_layout);

  // Status bar
  statusBar()->showMessage("Ready");
}

void MainWindow::SetupMenuBar() {
  // File menu
  QMenu* file_menu = menuBar()->addMenu("&File");

  action_select_device_ = file_menu->addAction("&Select Device...");
  action_start_scan_ = file_menu->addAction("S&tart Scan");
  action_stop_scan_ = file_menu->addAction("St&op Scan");

  file_menu->addSeparator();
  action_exit_ = file_menu->addAction("E&xit");

  action_start_scan_->setEnabled(false);
  action_stop_scan_->setEnabled(false);

  // Help menu
  QMenu* help_menu = menuBar()->addMenu("&Help");
  action_about_ = help_menu->addAction("&About");
}

void MainWindow::SetupConnections() {
  // Button connections
  connect(btn_select_device_, &QPushButton::clicked, this, &MainWindow::onSelectDevice);
  connect(btn_start_scan_, &QPushButton::clicked, this, &MainWindow::onStartScan);
  connect(btn_stop_scan_, &QPushButton::clicked, this, &MainWindow::onStopScan);
  connect(btn_recover_selected_, &QPushButton::clicked, this, &MainWindow::onRecoverSelected);
  connect(btn_recover_all_, &QPushButton::clicked, this, &MainWindow::onRecoverAll);

  // Menu connections
  connect(action_select_device_, &QAction::triggered, this, &MainWindow::onSelectDevice);
  connect(action_start_scan_, &QAction::triggered, this, &MainWindow::onStartScan);
  connect(action_stop_scan_, &QAction::triggered, this, &MainWindow::onStopScan);
  connect(action_exit_, &QAction::triggered, this, &QMainWindow::close);

  connect(action_about_, &QAction::triggered, [this]() {
    QMessageBox::about(this, "About RecoverySoftNetz",
      "RecoverySoftNetz v0.1.0\n\n"
      "AI-Powered Data Recovery Solution\n\n"
      "© 2025 Netz Informatique\n"
      "All Rights Reserved");
  });

  // Internal signals
  connect(this, &MainWindow::scanProgress, this, &MainWindow::onUpdateProgress);
  connect(this, &MainWindow::scanCompleted, this, &MainWindow::onScanCompleted);
}

void MainWindow::onSelectDevice() {
  if (!device_wizard_) {
    device_wizard_ = std::make_unique<DeviceWizard>(this);
  }

  if (device_wizard_->exec() == QDialog::Accepted) {
    current_device_ = device_wizard_->GetSelectedDevice();
    UpdateDeviceInfo();

    // Set device in recovery engine
    if (recovery_engine_->SetDevice(current_device_)) {
      btn_start_scan_->setEnabled(true);
      action_start_scan_->setEnabled(true);
      statusBar()->showMessage("Device selected: " + QString::fromStdString(current_device_));
    } else {
      QMessageBox::warning(this, "Error",
        "Failed to set device: " + QString::fromStdString(recovery_engine_->GetLastError()));
    }
  }
}

void MainWindow::onStartScan() {
  if (current_device_.empty()) {
    QMessageBox::warning(this, "Warning", "Please select a device first");
    return;
  }

  if (recovery_engine_->StartScan()) {
    is_scanning_ = true;
    EnableControls(false);

    lbl_status_->setText("Scanning...");
    statusBar()->showMessage("Scan in progress...");

    // Simulate progress (in real implementation, use signals from engine)
    QTimer* progress_timer = new QTimer(this);
    connect(progress_timer, &QTimer::timeout, [this, progress_timer]() {
      auto stats = recovery_engine_->GetStats();
      int progress = static_cast<int>(stats.recovery_rate_percent);
      emit scanProgress(progress);

      // Update files found
      lbl_files_found_->setText(QString("Files found: %1").arg(stats.total_files_found));

      // Check if scan is complete
      if (!recovery_engine_->IsScanning()) {
        progress_timer->stop();
        progress_timer->deleteLater();
        emit scanCompleted();
      }
    });
    progress_timer->start(100);  // Update every 100ms

  } else {
    QMessageBox::critical(this, "Error",
      "Failed to start scan: " + QString::fromStdString(recovery_engine_->GetLastError()));
  }
}

void MainWindow::onStopScan() {
  recovery_engine_->StopScan();
  is_scanning_ = false;
  EnableControls(true);

  lbl_status_->setText("Scan stopped");
  statusBar()->showMessage("Scan stopped by user");
}

void MainWindow::onRecoverSelected() {
  QList<QTableWidgetItem*> selected = results_table_->selectedItems();
  if (selected.isEmpty()) {
    QMessageBox::information(this, "Info", "Please select files to recover");
    return;
  }

  QString output_dir = QFileDialog::getExistingDirectory(this, "Select Recovery Output Directory");
  if (output_dir.isEmpty()) {
    return;
  }

  recovery_engine_->SetOutputPath(output_dir.toStdString());

  // Get selected file indices
  std::vector<uint64_t> indices;
  QSet<int> rows;
  for (auto* item : selected) {
    rows.insert(item->row());
  }

  for (int row : rows) {
    indices.push_back(static_cast<uint64_t>(row));
  }

  uint64_t recovered = recovery_engine_->RecoverFiles(indices);

  QMessageBox::information(this, "Recovery Complete",
    QString("Successfully recovered %1 files").arg(recovered));
}

void MainWindow::onRecoverAll() {
  QString output_dir = QFileDialog::getExistingDirectory(this, "Select Recovery Output Directory");
  if (output_dir.isEmpty()) {
    return;
  }

  recovery_engine_->SetOutputPath(output_dir.toStdString());
  uint64_t recovered = recovery_engine_->RecoverAllFiles();

  QMessageBox::information(this, "Recovery Complete",
    QString("Successfully recovered %1 files").arg(recovered));
}

void MainWindow::onUpdateProgress(int percentage) {
  progress_bar_->setValue(percentage);
}

void MainWindow::onScanCompleted() {
  is_scanning_ = false;
  EnableControls(true);

  lbl_status_->setText("Scan completed");
  statusBar()->showMessage("Scan completed successfully");

  UpdateResultsTable();

  btn_recover_selected_->setEnabled(true);
  btn_recover_all_->setEnabled(true);
}

void MainWindow::UpdateResultsTable() {
  results_table_->setRowCount(0);

  auto registry = recovery_engine_->GetFileRegistry();
  if (!registry) {
    return;
  }

  const auto& files = registry->GetFiles();
  results_table_->setRowCount(static_cast<int>(files.size()));

  for (size_t i = 0; i < files.size(); ++i) {
    const auto& file = files[i];

    results_table_->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(file.filename)));
    results_table_->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(file.original_path)));
    results_table_->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(common::FormatBytes(file.size_bytes))));
    results_table_->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(file.file_type)));
    results_table_->setItem(i, 4, new QTableWidgetItem(QString::number(file.recovery_confidence * 100, 'f', 1) + "%"));
    results_table_->setItem(i, 5, new QTableWidgetItem(file.is_deleted ? "Deleted" : "Active"));
  }
}

void MainWindow::UpdateDeviceInfo() {
  if (current_device_.empty()) {
    lbl_device_info_->setText("No device selected");
  } else {
    lbl_device_info_->setText(QString("Device: %1").arg(QString::fromStdString(current_device_)));
  }
}

void MainWindow::EnableControls(bool enable) {
  btn_select_device_->setEnabled(enable);
  btn_start_scan_->setEnabled(enable && !current_device_.empty());
  btn_stop_scan_->setEnabled(!enable);

  action_select_device_->setEnabled(enable);
  action_start_scan_->setEnabled(enable && !current_device_.empty());
  action_stop_scan_->setEnabled(!enable);
}

#endif  // QT_WIDGETS_LIB

}  // namespace ui
}  // namespace rsn
