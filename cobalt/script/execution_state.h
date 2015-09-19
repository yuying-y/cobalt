/*
 * Copyright 2014 Google Inc. All Rights Reserved.
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
#ifndef SCRIPT_EXECUTION_STATE_H_
#define SCRIPT_EXECUTION_STATE_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

namespace cobalt {
namespace script {

class GlobalObjectProxy;

// Provides access to the state of JavaScript execution.
class ExecutionState {
 public:
  static scoped_ptr<ExecutionState> CreateExecutionState(
      const scoped_refptr<GlobalObjectProxy>& global_object_proxy);

  virtual std::string GetStackTrace() const = 0;
  virtual ~ExecutionState() {}
};

}  // namespace script
}  // namespace cobalt

#endif  // SCRIPT_EXECUTION_STATE_H_
