/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COBALT_H5VCC_H5VCC_ACCESSIBILITY_H_
#define COBALT_H5VCC_H5VCC_ACCESSIBILITY_H_

#include "cobalt/script/wrappable.h"

namespace cobalt {
namespace h5vcc {

class H5vccAccessibility : public script::Wrappable {
 public:
  H5vccAccessibility();
  bool high_contrast_text() const;

  DEFINE_WRAPPABLE_TYPE(H5vccAccessibility);

 private:
  bool high_contrast_text_;
  DISALLOW_COPY_AND_ASSIGN(H5vccAccessibility);
};

}  // namespace h5vcc
}  // namespace cobalt

#endif  // COBALT_H5VCC_H5VCC_ACCESSIBILITY_H_
