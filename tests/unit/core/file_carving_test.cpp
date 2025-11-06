/**
 * @file file_carving_test.cpp
 * @brief Unit tests for file carving engine
 */

#include "core/file_carving.h"
#include <gtest/gtest.h>

using namespace rsn::core;

class FileCarvingEngineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    engine_ = std::make_unique<FileCarvingEngine>();
  }

  std::unique_ptr<FileCarvingEngine> engine_;
};

TEST_F(FileCarvingEngineTest, Initialize_LoadsDefaultSignatures) {
  ASSERT_TRUE(engine_->Initialize());

  auto types = engine_->GetSupportedFileTypes();
  EXPECT_GT(types.size(), 0);

  // Check for common file types across all categories
  bool has_jpeg = false;
  bool has_pdf = false;
  bool has_mp4 = false;
  bool has_rar = false;
  bool has_flac = false;

  for (const auto& type : types) {
    if (type.find("JPEG") != std::string::npos) has_jpeg = true;
    if (type.find("PDF") != std::string::npos) has_pdf = true;
    if (type.find("MP4") != std::string::npos) has_mp4 = true;
    if (type.find("RAR") != std::string::npos) has_rar = true;
    if (type.find("FLAC") != std::string::npos) has_flac = true;
  }

  EXPECT_TRUE(has_jpeg);
  EXPECT_TRUE(has_pdf);
  EXPECT_TRUE(has_mp4);
  EXPECT_TRUE(has_rar);
  EXPECT_TRUE(has_flac);
}

TEST_F(FileCarvingEngineTest, GetSupportedFileTypes_ReturnsMultipleTypes) {
  engine_->Initialize();
  auto types = engine_->GetSupportedFileTypes();

  // Should have 24 signatures: 4 images, 2 docs, 6 archives, 6 audio, 6 video
  EXPECT_EQ(types.size(), 24);
}

TEST_F(FileCarvingEngineTest, AddSignature_IncreasesTypeCount) {
  engine_->Initialize();
  size_t initial_count = engine_->GetSupportedFileTypes().size();

  FileSignature custom_sig;
  custom_sig.file_type = "Custom Type";
  custom_sig.extension = ".cst";
  custom_sig.header = {0x43, 0x53, 0x54};
  custom_sig.max_file_size = 1024;
  custom_sig.has_footer = false;

  engine_->AddSignature(custom_sig);

  size_t new_count = engine_->GetSupportedFileTypes().size();
  EXPECT_GT(new_count, initial_count);
}

TEST_F(FileCarvingEngineTest, GetStats_InitiallyZero) {
  auto stats = engine_->GetStats();
  EXPECT_EQ(stats.bytes_scanned, 0);
  EXPECT_EQ(stats.files_found, 0);
  EXPECT_EQ(stats.files_carved, 0);
}

TEST_F(FileCarvingEngineTest, SetMaxScanSize_AcceptsValue) {
  EXPECT_NO_THROW(engine_->SetMaxScanSize(1024 * 1024 * 100));
}

TEST_F(FileCarvingEngineTest, Initialize_IncludesVideoFormats) {
  ASSERT_TRUE(engine_->Initialize());
  auto types = engine_->GetSupportedFileTypes();

  // Check for video formats
  bool has_mp4 = false;
  bool has_avi = false;
  bool has_mkv = false;

  for (const auto& type : types) {
    if (type.find("MP4") != std::string::npos) has_mp4 = true;
    if (type.find("AVI") != std::string::npos) has_avi = true;
    if (type.find("Matroska") != std::string::npos) has_mkv = true;
  }

  EXPECT_TRUE(has_mp4);
  EXPECT_TRUE(has_avi);
  EXPECT_TRUE(has_mkv);
}

TEST_F(FileCarvingEngineTest, Initialize_IncludesArchiveFormats) {
  ASSERT_TRUE(engine_->Initialize());
  auto types = engine_->GetSupportedFileTypes();

  // Check for archive formats
  bool has_rar = false;
  bool has_gzip = false;
  bool has_7z = false;

  for (const auto& type : types) {
    if (type.find("RAR") != std::string::npos) has_rar = true;
    if (type.find("GZIP") != std::string::npos) has_gzip = true;
    if (type.find("7-Zip") != std::string::npos) has_7z = true;
  }

  EXPECT_TRUE(has_rar);
  EXPECT_TRUE(has_gzip);
  EXPECT_TRUE(has_7z);
}

TEST_F(FileCarvingEngineTest, Initialize_IncludesAudioFormats) {
  ASSERT_TRUE(engine_->Initialize());
  auto types = engine_->GetSupportedFileTypes();

  // Check for audio formats
  bool has_flac = false;
  bool has_wav = false;
  bool has_ogg = false;

  for (const auto& type : types) {
    if (type.find("FLAC") != std::string::npos) has_flac = true;
    if (type.find("WAV") != std::string::npos) has_wav = true;
    if (type.find("OGG") != std::string::npos) has_ogg = true;
  }

  EXPECT_TRUE(has_flac);
  EXPECT_TRUE(has_wav);
  EXPECT_TRUE(has_ogg);
}
