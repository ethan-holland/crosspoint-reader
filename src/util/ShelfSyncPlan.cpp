#include "ShelfSyncPlan.h"

#include <algorithm>

namespace {
bool contains(const std::vector<std::string>& paths, const std::string& path) {
  return std::find(paths.begin(), paths.end(), path) != paths.end();
}
}  // namespace

ShelfSyncPlan planShelfSync(const std::vector<std::string>& remotePaths, const std::vector<std::string>& managedPaths,
                            const bool feedComplete, const ShelfPathExistsFn exists, void* ctx) {
  ShelfSyncPlan plan;
  plan.downloads.reserve(remotePaths.size());
  plan.kept.reserve(managedPaths.size());

  // Shelves are small (the caller caps them), so linear lookups beat building
  // hash sets on the device heap.
  for (size_t i = 0; i < remotePaths.size(); ++i) {
    const std::string& path = remotePaths[i];
    // Two books that sanitize to the same filename: the first one wins.
    if (std::find(remotePaths.begin(), remotePaths.begin() + static_cast<std::ptrdiff_t>(i), path) !=
        remotePaths.begin() + static_cast<std::ptrdiff_t>(i)) {
      continue;
    }
    if (!exists(path, ctx)) {
      plan.downloads.push_back(i);
    } else if (contains(managedPaths, path)) {
      plan.kept.push_back(path);
    }
  }

  const bool mayDelete = feedComplete && !remotePaths.empty();
  for (const auto& managed : managedPaths) {
    if (contains(remotePaths, managed) || contains(plan.kept, managed)) continue;
    if (mayDelete) {
      if (exists(managed, ctx)) plan.deletions.push_back(managed);
    } else if (exists(managed, ctx)) {
      plan.kept.push_back(managed);
    }
  }
  return plan;
}
