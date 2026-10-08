#include "ShelfSyncPlan.h"

#include <gtest/gtest.h>

#include <set>
#include <string>
#include <vector>

namespace {
using Paths = std::vector<std::string>;

bool existsIn(const std::string& path, void* ctx) {
  const auto* files = static_cast<const std::set<std::string>*>(ctx);
  return files->count(path) != 0;
}

ShelfSyncPlan plan(const Paths& remote, const Paths& managed, std::set<std::string> files, bool complete = true) {
  return planShelfSync(remote, managed, complete, &existsIn, &files);
}
}  // namespace

TEST(ShelfSyncPlan, DownloadsBooksMissingLocally) {
  const auto p = plan({"/S/a.epub", "/S/b.epub"}, {}, {});
  EXPECT_EQ(p.downloads, (std::vector<size_t>{0, 1}));
  EXPECT_TRUE(p.deletions.empty());
  EXPECT_TRUE(p.kept.empty());
}

TEST(ShelfSyncPlan, KeepsManagedBooksStillOnShelf) {
  const auto p = plan({"/S/a.epub"}, {"/S/a.epub"}, {"/S/a.epub"});
  EXPECT_TRUE(p.downloads.empty());
  EXPECT_EQ(p.kept, (Paths{"/S/a.epub"}));
}

TEST(ShelfSyncPlan, RedownloadsManagedBookDeletedByHand) {
  const auto p = plan({"/S/a.epub"}, {"/S/a.epub"}, {});
  EXPECT_EQ(p.downloads, (std::vector<size_t>{0}));
  EXPECT_TRUE(p.kept.empty());
  EXPECT_TRUE(p.deletions.empty());
}

TEST(ShelfSyncPlan, DeletesManagedBookThatLeftShelf) {
  const auto p = plan({"/S/a.epub"}, {"/S/a.epub", "/S/gone.epub"}, {"/S/a.epub", "/S/gone.epub"});
  EXPECT_EQ(p.deletions, (Paths{"/S/gone.epub"}));
  EXPECT_EQ(p.kept, (Paths{"/S/a.epub"}));
}

TEST(ShelfSyncPlan, NeverDeletesUnmanagedFiles) {
  // mine.epub sits in the shelf folder but no sync downloaded it.
  const auto p = plan({"/S/a.epub"}, {}, {"/S/a.epub", "/S/mine.epub"});
  EXPECT_TRUE(p.deletions.empty());
  EXPECT_TRUE(p.downloads.empty());
  // A present-but-unmanaged shelf book is not adopted.
  EXPECT_TRUE(p.kept.empty());
}

TEST(ShelfSyncPlan, IncompleteFeedDeletesNothing) {
  const auto p = plan({"/S/a.epub"}, {"/S/a.epub", "/S/b.epub"}, {"/S/a.epub", "/S/b.epub"}, false);
  EXPECT_TRUE(p.deletions.empty());
  EXPECT_EQ(p.kept, (Paths{"/S/a.epub", "/S/b.epub"}));
}

TEST(ShelfSyncPlan, EmptyFeedDeletesNothing) {
  const auto p = plan({}, {"/S/a.epub"}, {"/S/a.epub"});
  EXPECT_TRUE(p.deletions.empty());
  EXPECT_EQ(p.kept, (Paths{"/S/a.epub"}));
}

TEST(ShelfSyncPlan, ForgetsManagedFilesAlreadyGone) {
  const auto p = plan({"/S/a.epub"}, {"/S/a.epub", "/S/gone.epub"}, {"/S/a.epub"});
  EXPECT_TRUE(p.deletions.empty());
  EXPECT_EQ(p.kept, (Paths{"/S/a.epub"}));
}

TEST(ShelfSyncPlan, DuplicateRemotePathDownloadsOnce) {
  const auto p = plan({"/S/a.epub", "/S/a.epub"}, {}, {});
  EXPECT_EQ(p.downloads, (std::vector<size_t>{0}));
}
