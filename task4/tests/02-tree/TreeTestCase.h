#pragma once

#include <gtest/gtest.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/optional.hpp>

class TreeTestCase : public ::testing::Test {
 protected:
  void SetUp() {
    tmp_dir_path_ =
        boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    create_directories(tmp_dir_path_);
  }

  void TearDown() {
    boost::filesystem::remove_all(tmp_dir_path_);
  }

  void CreateSimpleStructure(
      boost::optional<boost::filesystem::path> base = boost::none) {
    if (!base) {
      base = tmp_dir_path_;
    }
    boost::filesystem::create_directories(*base / "lol" / "kek");
    boost::filesystem::ofstream(*base / "some_file");
    boost::filesystem::ofstream(*base / "lol" / "some_other_file");
    // boost::filesystem::ofstream(tmp_dir_path_ / "lol" / "some_other_file2");
  }

 protected:
  boost::filesystem::path tmp_dir_path_;
};
