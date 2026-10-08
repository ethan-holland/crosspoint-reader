#pragma once
#include <cstddef>
#include <string>
#include <vector>

// What one shelf sync does. `remotePaths` are the local paths the shelf feed's
// books map to; `managedPaths` are the files earlier syncs downloaded.
struct ShelfSyncPlan {
  std::vector<size_t> downloads;       // indices into remotePaths missing locally
  std::vector<std::string> deletions;  // managed files that left the shelf
  std::vector<std::string> kept;       // managed files to keep tracking
};

using ShelfPathExistsFn = bool (*)(const std::string& path, void* ctx);

// Only files the sync downloaded are ever deleted: a book already present but
// not managed is left alone and stays unmanaged. Nothing is deleted unless the
// whole feed was read (`feedComplete`) and listed at least one book, so a
// failed or truncated fetch can't empty the shelf. Pure: file existence comes
// from `exists`.
ShelfSyncPlan planShelfSync(const std::vector<std::string>& remotePaths, const std::vector<std::string>& managedPaths,
                            bool feedComplete, ShelfPathExistsFn exists, void* ctx);
