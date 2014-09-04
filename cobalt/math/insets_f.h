// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MATH_INSETS_F_H_
#define MATH_INSETS_F_H_

#include <string>

#include "cobalt/math/insets_base.h"

namespace cobalt {
namespace math {

// A floating-point version of Insets.
class InsetsF : public InsetsBase<InsetsF, float> {
 public:
  InsetsF();
  InsetsF(float top, float left, float bottom, float right);
  ~InsetsF();

  // Returns a string representation of the insets.
  std::string ToString() const;
};

extern template class InsetsBase<InsetsF, float>;

}  // namespace math
}  // namespace cobalt

#endif  // MATH_INSETS_F_H_
