/**
 * @file device_wizard.cpp
 * @brief Implementation of device wizard
 */

#include "ui/device_wizard.h"
#include "common/logging.h"
#include "common/utils.h"

#ifdef QT_WIDGETS_LIB
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#endif

// For device scanning (platform-specific)
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

namespace rsn {
namespace ui {

#ifdef QT_WIDGETS_LIB

DeviceWizard::DeviceWizard(QWidget* parent)
  : QDialog(parent),
    selected_index_(-1) {

  SetupUI();
  LoadDevices();

  common::Log(common::LogLevel::INFO, "DeviceWizard initialized");
}

DeviceWizard::~DeviceWizard() {}

void DeviceWizard::SetupUI() {
  setWindowTitle("Select Device to Scan");
  setMinimumSize(600, 400);

  QVBoxLayout* main_layout = new QVBoxLayout(this);

  // Instructions
  QLabel* lbl_instructions = new QLabel(
    "Select a device or partition to scan for recoverable files:", this);
  main_layout->addWidget(lbl_instructions);

  // Device list
  device_list_ = new QListWidget(this);
  device_list_->setMinimumHeight(200);
  main_layout->addWidget(device_list_);

  // Device info group
  info_group_ = new QGroupBox("Device Information", this);
  QVBoxLayout* info_layout = new QVBoxLayout(info_group_);

  lbl_device_info_ = new QLabel("No device selected", this);
  lbl_device_info_->setWordWrap(true);
  info_layout->addWidget(lbl_device_info_);

  main_layout->addWidget(info_group_);

  // Buttons
  QHBoxLayout* button_layout = new QHBoxLayout();

  btn_refresh_ = new QPushButton("Refresh", this);
  btn_ok_ = new QPushButton("OK", this);
  btn_cancel_ = new QPushButton("Cancel", this);

  btn_ok_->setEnabled(false);
  btn_ok_->setDefault(true);

  button_layout->addWidget(btn_refresh_);
  button_layout->addStretch();
  button_layout->addWidget(btn_ok_);
  button_layout->addWidget(btn_cancel_);

  main_layout->addLayout(button_layout);

  // Connections
  connect(device_list_, &QListWidget::itemClicked, this, &DeviceWizard::onDeviceSelected);
  connect(device_list_, &QListWidget::itemDoubleClicked, [this](QListWidgetItem*) {
    if (selected_index_ >= 0) {
      accept();
    }
  });

  connect(btn_refresh_, &QPushButton::clicked, this, &DeviceWizard::onRefreshDevices);
  connect(btn_ok_, &QPushButton::clicked, this, &QDialog::accept);
  connect(btn_cancel_, &QPushButton::clicked, this, &QDialog::reject);
}

void DeviceWizard::LoadDevices() {
  device_list_->clear();
  devices_.clear();
  selected_index_ = -1;
  btn_ok_->setEnabled(false);

  devices_ = ScanAvailableDevices();

  if (devices_.empty()) {
    QListWidgetItem* item = new QListWidgetItem("No devices found", device_list_);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    lbl_device_info_->setText("No devices detected. Try refreshing or check permissions.");
    return;
  }

  for (size_t i = 0; i < devices_.size(); ++i) {
    const auto& dev = devices_[i];

    QString display = QString("%1 - %2 (%3)")
      .arg(QString::fromStdString(dev.path))
      .arg(QString::fromStdString(dev.name))
      .arg(QString::fromStdString(common::FormatBytes(dev.size_bytes)));

    QListWidgetItem* item = new QListWidgetItem(display, device_list_);
    item->setData(Qt::UserRole, static_cast<int>(i));

    // Add icon based on filesystem
    if (dev.filesystem == "NTFS") {
      item->setIcon(qApp->style()->standardIcon(QStyle::SP_DriveHDIcon));
    } else if (dev.filesystem == "APFS" || dev.filesystem == "HFS+") {
      item->setIcon(qApp->style()->standardIcon(QStyle::SP_DriveFDIcon));
    } else {
      item->setIcon(qApp->style()->standardIcon(QStyle::SP_DriveNetIcon));
    }
  }

  common::Log(common::LogLevel::INFO,
              "Found " + std::to_string(devices_.size()) + " devices");
}

void DeviceWizard::onRefreshDevices() {
  LoadDevices();
}

void DeviceWizard::onDeviceSelected(QListWidgetItem* item) {
  if (!item) {
    return;
  }

  int index = item->data(Qt::UserRole).toInt();
  if (index >= 0 && index < static_cast<int>(devices_.size())) {
    selected_index_ = index;
    UpdateDeviceInfo(index);
    btn_ok_->setEnabled(true);
  }
}

void DeviceWizard::UpdateDeviceInfo(int index) {
  if (index < 0 || index >= static_cast<int>(devices_.size())) {
    return;
  }

  const auto& dev = devices_[index];

  QString info = QString(
    "<b>Path:</b> %1<br>"
    "<b>Name:</b> %2<br>"
    "<b>Filesystem:</b> %3<br>"
    "<b>Size:</b> %4<br>"
    "<b>Mounted:</b> %5")
    .arg(QString::fromStdString(dev.path))
    .arg(QString::fromStdString(dev.name))
    .arg(QString::fromStdString(dev.filesystem))
    .arg(QString::fromStdString(common::FormatBytes(dev.size_bytes)))
    .arg(dev.is_mounted ? "Yes" : "No");

  lbl_device_info_->setText(info);
}

std::string DeviceWizard::GetSelectedDevice() const {
  if (selected_index_ >= 0 && selected_index_ < static_cast<int>(devices_.size())) {
    return devices_[selected_index_].path;
  }
  return "";
}

std::vector<DeviceInfo> DeviceWizard::ScanAvailableDevices() {
  std::vector<DeviceInfo> devices;

  // TODO: Implement proper device scanning
  // For now, create some mock devices for demonstration

  DeviceInfo mock_dev1;
  mock_dev1.path = "/dev/sda1";
  mock_dev1.name = "System Disk - Partition 1";
  mock_dev1.filesystem = "NTFS";
  mock_dev1.size_bytes = 500ULL * 1024 * 1024 * 1024;  // 500 GB
  mock_dev1.is_mounted = true;
  devices.push_back(mock_dev1);

  DeviceInfo mock_dev2;
  mock_dev2.path = "/dev/sdb1";
  mock_dev2.name = "External Drive";
  mock_dev2.filesystem = "ext4";
  mock_dev2.size_bytes = 1024ULL * 1024 * 1024 * 1024;  // 1 TB
  mock_dev2.is_mounted = false;
  devices.push_back(mock_dev2);

  DeviceInfo mock_dev3;
  mock_dev3.path = "/dev/disk2s1";
  mock_dev3.name = "USB Flash Drive";
  mock_dev3.filesystem = "FAT32";
  mock_dev3.size_bytes = 32ULL * 1024 * 1024 * 1024;  // 32 GB
  mock_dev3.is_mounted = true;
  devices.push_back(mock_dev3);

  return devices;
}

#endif  // QT_WIDGETS_LIB

}  // namespace ui
}  // namespace rsn
