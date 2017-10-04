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

#include "cobalt/script/v8c/v8c_global_environment.h"

#include <algorithm>
#include <utility>

#include "base/lazy_instance.h"
#include "base/stringprintf.h"
#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/script/v8c/v8c_source_code.h"
#include "nb/memory_scope.h"

namespace cobalt {
namespace script {
namespace v8c {

V8cGlobalEnvironment::V8cGlobalEnvironment(v8::Isolate* isolate)
    : garbage_collection_count_(0),
      environment_settings_(nullptr),
      last_error_message_(nullptr),
      eval_enabled_(false),
      isolate_(isolate) {
  TRACK_MEMORY_SCOPE("Javascript");
  v8::Isolate::Scope isolate_scope(isolate_);
  v8::HandleScope handle_scope(isolate_);
  v8::Local<v8::ObjectTemplate> global_object_template;
  v8::Local<v8::Context> context =
      v8::Context::New(isolate_, nullptr, global_object_template);
  context_.Reset(isolate_, context);
}

V8cGlobalEnvironment::~V8cGlobalEnvironment() {
  DCHECK(thread_checker_.CalledOnValidThread());
}

void V8cGlobalEnvironment::CreateGlobalObject() {
  TRACK_MEMORY_SCOPE("Javascript");
  DCHECK(thread_checker_.CalledOnValidThread());
  // TODO: Global object creation happens with context creation in V8.
  // Strongly consider removing this from the interface.
  EvaluateAutomatics();
}

bool V8cGlobalEnvironment::EvaluateScript(
    const scoped_refptr<SourceCode>& source_code,
    std::string* out_result_utf8) {
  TRACK_MEMORY_SCOPE("Javascript");
  DCHECK(thread_checker_.CalledOnValidThread());

  v8::Isolate::Scope isolate_scope(isolate_);
  v8::HandleScope handle_scope(isolate_);
  v8::Local<v8::Context> context = context_.Get(isolate_);
  v8::Context::Scope scope(context);

  v8::TryCatch try_catch(isolate_);
  // TODO: Also pass in a |v8::ScriptOrigin|.
  V8cSourceCode* v8c_source_code =
      base::polymorphic_downcast<V8cSourceCode*>(source_code.get());
  v8::Local<v8::String> source =
      v8::String::NewFromUtf8(isolate_, v8c_source_code->source_utf8().c_str(),
                              v8::NewStringType::kNormal)
          .ToLocalChecked();

  auto maybe_script = v8::Script::Compile(context, source);
  v8::Local<v8::Script> script;
  if (!maybe_script.ToLocal(&script)) {
    return false;
  }

  auto maybe_result = script->Run(context);
  v8::Local<v8::Value> result;
  if (!maybe_result.ToLocal(&result)) {
    return false;
  }

  if (out_result_utf8) {
    *out_result_utf8 = *v8::String::Utf8Value(isolate_, result);
  }

  return true;
}

bool V8cGlobalEnvironment::EvaluateScript(
    const scoped_refptr<SourceCode>& source_code,
    const scoped_refptr<Wrappable>& owning_object,
    base::optional<OpaqueHandleHolder::Reference>* out_opaque_handle) {
  TRACK_MEMORY_SCOPE("Javascript");
  DCHECK(thread_checker_.CalledOnValidThread());
  // TODO: Implement this once we have |V8cObjectHandleHolder|.
  NOTIMPLEMENTED();
  return false;
}

std::vector<StackFrame> V8cGlobalEnvironment::GetStackTrace(int max_frames) {
  DCHECK(thread_checker_.CalledOnValidThread());
  NOTIMPLEMENTED();
  return {};
}

void V8cGlobalEnvironment::PreventGarbageCollection(
    const scoped_refptr<Wrappable>& wrappable) {
  DCHECK(thread_checker_.CalledOnValidThread());
  NOTIMPLEMENTED();
}

void V8cGlobalEnvironment::AllowGarbageCollection(
    const scoped_refptr<Wrappable>& wrappable) {
  TRACK_MEMORY_SCOPE("Javascript");
  DCHECK(thread_checker_.CalledOnValidThread());
  NOTIMPLEMENTED();
}

void V8cGlobalEnvironment::DisableEval(const std::string& message) {
  DCHECK(thread_checker_.CalledOnValidThread());
  eval_disabled_message_.emplace(message);
  eval_enabled_ = false;
}

void V8cGlobalEnvironment::EnableEval() {
  DCHECK(thread_checker_.CalledOnValidThread());
  eval_disabled_message_ = base::nullopt;
  eval_enabled_ = true;
}

void V8cGlobalEnvironment::DisableJit() {
  DCHECK(thread_checker_.CalledOnValidThread());
  SB_DLOG(INFO)
      << "V8 can only be run with JIT enabled, ignoring |DisableJit| call.";
}

void V8cGlobalEnvironment::SetReportEvalCallback(
    const base::Closure& report_eval) {
  DCHECK(thread_checker_.CalledOnValidThread());
  report_eval_ = report_eval;
}

void V8cGlobalEnvironment::SetReportErrorCallback(
    const ReportErrorCallback& report_error_callback) {
  DCHECK(thread_checker_.CalledOnValidThread());
  report_error_callback_ = report_error_callback;
}

void V8cGlobalEnvironment::Bind(const std::string& identifier,
                                const scoped_refptr<Wrappable>& impl) {
  TRACK_MEMORY_SCOPE("Javascript");
  NOTIMPLEMENTED();
}

ScriptValueFactory* V8cGlobalEnvironment::script_value_factory() {
  NOTIMPLEMENTED();
  return nullptr;
}

void V8cGlobalEnvironment::EvaluateAutomatics() {
  // TODO: Maybe add fetch, stream, and promise polyfills.  Investigate what
  // V8 has to natively offer first.
  NOTIMPLEMENTED();
}

}  // namespace v8c
}  // namespace script
}  // namespace cobalt
