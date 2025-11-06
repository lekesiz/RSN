/**
 * @file device_wizard.h
 * @brief Device selection wizard (Qt6)
 */

#ifndef RSN_UI_DEVICE_WIZARD_H_
#define RSN_UI_DEVICE_WIZARD_H_

#include <string>
#include <vector>

#ifdef QT_WIDGETS_LIB
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#endif

namespace rsn {
namespace ui {

/**
 * @brief Device information structure
 */
struct DeviceInfo {
  std::string path;           // e.g., /dev/sda1
  std::string name;           // User-friendly name
  std::string filesystem;     // NTFS, APFS, ext4, etc.
  uint64_t size_bytes;        // Total size
  bool is_mounted;            // Mount status
};

/**
 * @brief Device selection wizard dialog
 */
#ifdef QT_WIDGETS_LIB
class DeviceWizard : public QDialog {
  Q_OBJECT

 public:
  explicit DeviceWizard(QWidget* parent = nullptr);
  ~DeviceWizard() override;

  /**
   * @brief Get the selected device path
   */
  std::string GetSelectedDevice() const;

 private slots:
  void onRefreshDevices();
  void onDeviceSelected(QListWidgetItem* item);

 private:
  // UI Components
  QListWidget* device_list_;
  QPushButton* btn_refresh_;
  QPushButton* btn_ok_;
  QPushButton* btn_cancel_;
  QLabel* lbl_device_info_;
  QGroupBox* info_group_;

  // Data
  std::vector<DeviceInfo> devices_;
  int selected_index_;

  // Private methods
  void SetupUI();
  void LoadDevices();
  void UpdateDeviceInfo(int index);
  std::vector<DeviceInfo> ScanAvailableDevices();
};
#else
// Stub implementation
class DeviceWizard {
 public:
  DeviceWizard() {}
  ~DeviceWizard() {}
  int exec() { return 0; }
  std::string GetSelectedDevice() const { return ""; }
};
#endif

}  // namespace ui
}  // namespace rsn

#endif  // RSN_UI_DEVICE_WIZARD_H_
