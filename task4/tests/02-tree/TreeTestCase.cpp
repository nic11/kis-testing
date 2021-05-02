#include "TreeTestCase.h"

#include "Tree.h"

#include <gmock/gmock-matchers.h>

namespace {

void Print(std::ostream& os, const FileNode& file_node, int indent) {
  for (int i = 0; i < indent; ++i) {
    os << ' ';
  }
  os << file_node.name;
  if (file_node.is_dir) {
    os << '/';
  }
  os << '\n';
  for (const auto& child : file_node.children) {
    Print(os, child, indent + 2);
  }
}

std::ostream& operator<<(std::ostream& os, const FileNode& file_node) {
  Print(os, file_node, 0);
  return os;
}

std::string ToString(const FileNode& file_node) {
  std::stringstream ss;
  Print(ss, file_node, 0);
  return ss.str();
}

}

TEST_F(TreeTestCase, NonexistentDir) {
  EXPECT_THAT(
    [this] {
      GetTree((tmp_dir_path_ / "no_such_file").string(), false);
    },
    testing::ThrowsMessage<std::invalid_argument>(testing::HasSubstr("not exist"))
  );
}

TEST_F(TreeTestCase, FileNotDir) {
  boost::filesystem::ofstream(tmp_dir_path_ / "this_is_a_file");
  boost::filesystem::create_symlink(tmp_dir_path_ / "this_is_a_file", tmp_dir_path_ / "symlink");
  EXPECT_THAT(
    [this] {
      GetTree((tmp_dir_path_ / "symlink").string(), false);
    },
    testing::ThrowsMessage<std::invalid_argument>(testing::HasSubstr("not directory"))
  );
}

TEST_F(TreeTestCase, EmptyDir) {
  FileNode root = GetTree(tmp_dir_path_.string(), false);
  ASSERT_EQ(root.name, tmp_dir_path_.filename()) << ToString(root);
  ASSERT_TRUE(root.is_dir) << ToString(root);
  ASSERT_EQ(root.children.size(), 0) << ToString(root);
}

TEST_F(TreeTestCase, SimpleDirsOnly) {
  CreateSimpleStructure();
  FileNode root = GetTree(tmp_dir_path_.string(), true);

  ASSERT_EQ(root.name, tmp_dir_path_.filename()) << ToString(root);
  ASSERT_TRUE(root.is_dir) << ToString(root);
  ASSERT_EQ(root.children.size(), 1) << ToString(root);

  ASSERT_EQ(root.children[0].name, "lol") << ToString(root);
  ASSERT_TRUE(root.children[0].is_dir) << ToString(root);
  ASSERT_EQ(root.children[0].children.size(), 1) << ToString(root);

  ASSERT_EQ(root.children[0].children[0].name, "kek") << ToString(root);
  ASSERT_TRUE(root.children[0].children[0].is_dir) << ToString(root);
  ASSERT_EQ(root.children[0].children[0].children.size(), 0) << ToString(root);
}

TEST_F(TreeTestCase, SimpleWithFiles) {
  CreateSimpleStructure();
  FileNode root = GetTree(tmp_dir_path_.string(), false);

  ASSERT_EQ(root.name, tmp_dir_path_.filename()) << ToString(root);
  ASSERT_TRUE(root.is_dir) << ToString(root);
  ASSERT_EQ(root.children.size(), 2) << ToString(root);

  for (const auto& child : root.children) {
    if (child.is_dir) {
      ASSERT_EQ(child.name, "lol") << ToString(root);
      ASSERT_TRUE(child.is_dir) << ToString(root);
      ASSERT_EQ(child.children.size(), 2) << ToString(root);

      for (const auto& child2 : child.children) {
        if (child2.is_dir) {
          ASSERT_EQ(child2.name, "kek") << ToString(root);
          ASSERT_EQ(child2.children.size(), 0) << ToString(root);
        } else {
          ASSERT_EQ(child2.name, "some_other_file") << ToString(root);
          ASSERT_EQ(child2.children.size(), 0) << ToString(root);
        }
      }
    } else {
      ASSERT_EQ(child.name, "some_file") << ToString(root);
      ASSERT_FALSE(child.is_dir) << ToString(root);
      ASSERT_EQ(child.children.size(), 0) << ToString(root);
    }
  }
}

TEST_F(TreeTestCase, FilterEmptyNodes) {
  CreateSimpleStructure();
  FileNode root = GetTree(tmp_dir_path_.string(), false);
  FilterEmptyNodes(root, tmp_dir_path_);
  EXPECT_TRUE(boost::filesystem::is_directory(tmp_dir_path_ / "lol"));
  EXPECT_FALSE(boost::filesystem::exists(tmp_dir_path_ / "lol" / "kek"));
  EXPECT_TRUE(boost::filesystem::is_regular_file(tmp_dir_path_ / "some_file"));
  EXPECT_TRUE(boost::filesystem::is_regular_file(tmp_dir_path_ / "lol" / "some_other_file"));
}

namespace {

testing::AssertionResult TreesEq(
    const char* expr_a,
    const char* expr_b,
    const char*,
    const FileNode& tree_a,
    const FileNode& tree_b,
    bool should_equal
) {
  if ((tree_a == tree_b) == should_equal) {
    return testing::AssertionSuccess();
  }
  return testing::AssertionFailure()
    << "tree_a:\n" << ToString(tree_a)
    << "tree_b:\n" << ToString(tree_b)
    << "should be "
    << std::vector<std::string>{"not equal", "equal"}[should_equal];
}

}

TEST_F(TreeTestCase, OperatorEq) {
  FileNode a{
    "hello", // name
    true,    // is_dir
    {},      // children
  };

  FileNode b = a;
  EXPECT_PRED_FORMAT3(TreesEq, a, b, true);

  b.name = "other";
  EXPECT_PRED_FORMAT3(TreesEq, a, b, false);

  b = a;
  b.is_dir = false;
  EXPECT_PRED_FORMAT3(TreesEq, a, b, false);

  b = a;
  b.children.emplace_back(a);
  EXPECT_PRED_FORMAT3(TreesEq, a, b, false);
}

// This one fails, because code is bugged
// https://www.boost.org/doc/libs/1_69_0/libs/filesystem/doc/reference.html#Class-directory_iterator
// > The order of directory entries obtained by dereferencing successive
// > increments of a directory_iterator is unspecified.
TEST_F(TreeTestCase, DISABLED_OperatorEqReorderedChildren) {
  FileNode inner_a{
    "a",     // name
    true,    // is_dir
    {},      // children
  };
  FileNode inner_b{
    "b",     // name
    true,    // is_dir
    {},      // children
  };
  FileNode outer_1{
    "outer", // name
    true,    // is_dir
    {        // children
      inner_a,
      inner_b,
    },
  };
  FileNode outer_2{
    "outer", // name
    true,    // is_dir
    {        // children
      inner_b,
      inner_a,
    },
  };

  EXPECT_PRED_FORMAT3(TreesEq, outer_1, outer_2, true);
}
