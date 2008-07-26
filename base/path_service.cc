// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

#include <hash_map>
#include <hash_set>

#include "base/path_service.h"

#include "base/lock.h"
#include "base/logging.h"
#include "base/file_util.h"

namespace base {
  bool PathProvider(int key, std::wstring* result);
}

namespace {

typedef stdext::hash_map<int,std::wstring> PathMap;
typedef stdext::hash_set<int> PathSet;

// We keep a linked list of providers.  In a debug build we ensure that no two
// providers claim overlapping keys.
struct Provider {
  PathService::ProviderFunc func;
  struct Provider* next;
#ifndef NDEBUG
  int key_start;
  int key_end;
#endif
};

static Provider base_provider = {
  base::PathProvider,
  NULL,
#ifndef NDEBUG
  base::PATH_START,
  base::PATH_END
#endif
};

struct PathData {
  Lock      lock;
  PathMap   cache;      // Track mappings from path key to path value.
  PathSet   overrides;  // Track whether a path has been overridden.
  Provider* providers;  // Linked list of path service providers.

  PathData() : providers(&base_provider) {
  }
};

// We rely on the path service not being used prior to 'main' execution, and
// we are happy to let this data structure leak at process exit.
PathData* path_data = new PathData();

}  // namespace

// TODO(brettw): this function does not handle long paths (filename > MAX_PATH)
// characters). This isn't supported very well by Windows right now, so it is
// moot, but we should keep this in mind for the future.
// static
bool PathService::Get(int key, std::wstring* result) {
  DCHECK(path_data);
  DCHECK(result);
  DCHECK(key >= base::DIR_CURRENT);

  // special case the current directory because it can never be cached
  if (key == base::DIR_CURRENT) {
    wchar_t system_buffer[MAX_PATH];
    system_buffer[0] = 0;
    DWORD len = GetCurrentDirectory(MAX_PATH, system_buffer);
    if (len == 0 || len > MAX_PATH)
      return false;
    *result = system_buffer;
    file_util::TrimTrailingSeparator(result);
    return true;
  }

  // TODO(darin): it would be nice to avoid holding this lock while calling out
  // to the path providers.
  AutoLock scoped_lock(path_data->lock);

  // check for a cached version
  PathMap::const_iterator it = path_data->cache.find(key);
  if (it != path_data->cache.end()) {
    *result = it->second;
    return true;
  }

  std::wstring path;

  // search providers for the requested path
  Provider* provider = path_data->providers;
  while (provider) {
    if (provider->func(key, &path))
      break;
    DCHECK(path.empty()) << "provider should not have modified path";
    provider = provider->next;
  }

  if (path.empty())
    return false;

  // Save the computed path in our cache.
  path_data->cache[key] = path;

  result->swap(path);
  return true;
}

bool PathService::IsOverridden(int key) {
  DCHECK(path_data);

  AutoLock scoped_lock(path_data->lock);
  return path_data->overrides.find(key) != path_data->overrides.end();
}

bool PathService::Override(int key, const std::wstring& path) {
  DCHECK(path_data);
  DCHECK(key > base::DIR_CURRENT) << "invalid path key";

  wchar_t file_path_buf[MAX_PATH];
  if (!_wfullpath(file_path_buf, path.c_str(), MAX_PATH))
    return false;
  std::wstring file_path(file_path_buf);

  // make sure the directory exists:
  if (!file_util::PathExists(file_path) &&
      // TODO(darin): what if this path is not that of a directory?
      !file_util::CreateDirectory(file_path))
    return false;

  file_util::TrimTrailingSeparator(&file_path);

  AutoLock scoped_lock(path_data->lock);
  path_data->cache[key] = file_path;
  path_data->overrides.insert(key);
  return true;
}

bool PathService::SetCurrentDirectory(const std::wstring& current_directory) {
  BOOL ret = ::SetCurrentDirectory(current_directory.c_str());
  return (ret ? true : false);
}

void PathService::RegisterProvider(ProviderFunc func, int key_start,
                                   int key_end) {
  DCHECK(path_data);
  DCHECK(key_end > key_start);

  AutoLock scoped_lock(path_data->lock);

  Provider* p;

#ifndef NDEBUG
  p = path_data->providers;
  while (p) {
    DCHECK(key_start >= p->key_end || key_end <= p->key_start) <<
      "path provider collision";
    p = p->next;
  }
#endif

  p = new Provider;
  p->func = func;
  p->next = path_data->providers;
#ifndef NDEBUG
  p->key_start = key_start;
  p->key_end = key_end;
#endif
  path_data->providers = p;
}
