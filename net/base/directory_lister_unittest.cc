// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/file_path.h"
#include "base/file_util.h"
#include "base/i18n/file_util_icu.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "net/base/directory_lister.h"
#include "net/base/net_errors.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {
class DirectoryListerTest : public testing::Test {};
}

class ListerDelegate : public net::DirectoryLister::DirectoryListerDelegate {
 public:
  ListerDelegate() : error_(-1) {
  }
  void OnListFile(const file_util::FileEnumerator::FindInfo& data) {
    file_list_.push_back(data);
  }
  void OnListDone(int error) {
    error_ = error;
    MessageLoop::current()->Quit();
    // Check that we got files in the right order.
    if (!file_list_.empty()) {
      for (size_t previous = 0, current = 1;
           current < file_list_.size();
           previous++, current++) {
        // Directories should come before files.
        if (file_util::FileEnumerator::IsDirectory(file_list_[previous]) &&
            !file_util::FileEnumerator::IsDirectory(file_list_[current])) {
          continue;
        }
        EXPECT_FALSE(file_util::IsDotDot(
            file_util::FileEnumerator::GetFilename(file_list_[current])));
        EXPECT_EQ(file_util::FileEnumerator::IsDirectory(file_list_[previous]),
                  file_util::FileEnumerator::IsDirectory(file_list_[current]));
        EXPECT_TRUE(file_util::LocaleAwareCompareFilenames(
            file_util::FileEnumerator::GetFilename(file_list_[previous]),
            file_util::FileEnumerator::GetFilename(file_list_[current])));
      }
    }
  }
  int error() const { return error_; }
 private:
  int error_;
  std::vector<file_util::FileEnumerator::FindInfo> file_list_;
};

TEST(DirectoryListerTest, BigDirTest) {
  FilePath path;
  ASSERT_TRUE(PathService::Get(base::DIR_SOURCE_ROOT, &path));

  ListerDelegate delegate;
  scoped_refptr<net::DirectoryLister> lister =
      new net::DirectoryLister(path, &delegate);

  lister->Start();

  MessageLoop::current()->Run();

  EXPECT_EQ(delegate.error(), net::OK);
}

TEST(DirectoryListerTest, CancelTest) {
  FilePath path;
  ASSERT_TRUE(PathService::Get(base::DIR_SOURCE_ROOT, &path));

  ListerDelegate delegate;
  scoped_refptr<net::DirectoryLister> lister =
      new net::DirectoryLister(path, &delegate);

  lister->Start();
  lister->Cancel();

  MessageLoop::current()->Run();

  EXPECT_EQ(delegate.error(), net::ERR_ABORTED);
}
