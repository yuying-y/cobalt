// Copyright 2025 The Cobalt Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/updater/constants.h"
#include "chrome/updater/updater.h"
#include "starboard/common/log.h"
#include "starboard/event.h"

namespace {
int g_argc = 0;
char** g_argv = nullptr;
}  // namespace

void SbEventHandle(const SbEvent* event) {
  if (event->type == kSbEventTypeStart) {
    updater::UpdaterMain(g_argc, g_argv);
  }
}

int main(int argc, char* argv[]) {
  g_argc = argc;
  g_argv = argv;

  SB_LOG(INFO) << "Number of arguments (argc): " << g_argc;
  for (int i = 0; i < g_argc; ++i) {
    SB_LOG(INFO) << "argv[" << i << "]: " << g_argv[i];
  }

  return SbRunStarboardMain(argc, argv, SbEventHandle);
}
