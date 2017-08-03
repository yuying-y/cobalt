// Copyright 2017 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "starboard/log.h"
#include "starboard/shared/win32/window_internal.h"
#include "starboard/window.h"

bool SbWindowGetSize(SbWindow window, SbWindowSize* size) {
  if (!SbWindowIsValid(window)) {
    SB_LOG(ERROR) << __FUNCTION__ << ": Invalid window.";
    return false;
  }

  size->width = window->width;
  size->height = window->height;
  // The video resolution is the same as the graphics resolution.
  size->video_pixel_ratio = 1.0f;
  return true;
}
