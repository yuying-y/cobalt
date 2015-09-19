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
#ifndef SCRIPT_SCRIPT_RUNNER_H_
#define SCRIPT_SCRIPT_RUNNER_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "cobalt/base/source_location.h"

namespace cobalt {
namespace script {

class GlobalObjectProxy;

// Maintains a handle to a JavaScript global object, and provides an interface
// to execute JavaScript code.
class ScriptRunner {
 public:
  static scoped_ptr<ScriptRunner> CreateScriptRunner(
      const scoped_refptr<GlobalObjectProxy>& global_object_proxy);

  virtual void Execute(const std::string& script_utf8,
                       const base::SourceLocation& script_location) = 0;
  virtual ~ScriptRunner() {}
};

}  // namespace script
}  // namespace cobalt

#endif  // SCRIPT_SCRIPT_RUNNER_H_
