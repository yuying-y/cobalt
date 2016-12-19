// Copyright 2016 Google Inc. All Rights Reserved.
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

#include "starboard/raspi/shared/open_max/open_max_component.h"

#include <algorithm>

#include "starboard/configuration.h"
#include "starboard/once.h"
#include "starboard/thread.h"

namespace starboard {
namespace raspi {
namespace shared {
namespace open_max {

OpenMaxComponent::OpenMaxComponent(const char* name)
    : OpenMaxComponentBase(name),
      output_setting_changed_(false),
      fill_buffer_thread_(kSbThreadInvalid),
      kill_fill_buffer_thread_(false),
      output_available_condition_variable_(mutex_) {}

void OpenMaxComponent::Start() {
  SendCommandAndWaitForCompletion(OMX_CommandStateSet, OMX_StateIdle);
  EnableInputPortAndAllocateBuffers();
  SendCommandAndWaitForCompletion(OMX_CommandStateSet, OMX_StateExecuting);
}

void OpenMaxComponent::Flush() {
  output_setting_changed_ = SbThreadIsValid(fill_buffer_thread_);
  DisableOutputPort();
  SendCommandAndWaitForCompletion(OMX_CommandFlush, input_port_);
  SendCommandAndWaitForCompletion(OMX_CommandFlush, output_port_);
}

int OpenMaxComponent::WriteData(const void* data, int size, SbTime timestamp) {
  int offset = 0;

  while (offset != size) {
    OMX_BUFFERHEADERTYPE* buffer_header = GetUnusedInputBuffer();
    if (buffer_header == NULL) {
      return offset;
    }

    int size_to_append = std::min<int>(size - offset, buffer_header->nAllocLen);
    buffer_header->nOffset = 0;
    buffer_header->nFilledLen = size_to_append;
    buffer_header->nFlags = 0;

    buffer_header->nTimeStamp.nLowPart = timestamp;
    buffer_header->nTimeStamp.nHighPart = timestamp >> 32;

    memcpy(buffer_header->pBuffer, reinterpret_cast<const char*>(data) + offset,
           size_to_append);
    offset += size_to_append;

    OMX_ERRORTYPE error = OMX_EmptyThisBuffer(handle_, buffer_header);
    SB_DCHECK(error == OMX_ErrorNone);
  }

  return offset;
}

void OpenMaxComponent::WriteEOS() {
  OMX_BUFFERHEADERTYPE* buffer_header = GetUnusedInputBuffer();

  buffer_header->nOffset = 0;
  buffer_header->nFilledLen = 0;
  buffer_header->nFlags = OMX_BUFFERFLAG_EOS;

  OMX_ERRORTYPE error = OMX_EmptyThisBuffer(handle_, buffer_header);
  SB_DCHECK(error == OMX_ErrorNone);
}

OMX_BUFFERHEADERTYPE* OpenMaxComponent::PeekNextOutputBuffer() {
  {
    ScopedLock scoped_lock(mutex_);

    if (!output_setting_changed_ && output_buffers_.empty()) {
      return NULL;
    }
  }

  if (output_setting_changed_ && filled_output_buffers_.empty()) {
    EnableOutputPortAndAllocateBuffer();
    output_setting_changed_ = false;
  }

  ScopedLock scoped_lock(mutex_);
  return filled_output_buffers_.empty() ? NULL : filled_output_buffers_.front();
}

void OpenMaxComponent::DropNextOutputBuffer() {
  ScopedLock scoped_lock(mutex_);
  SB_DCHECK(!filled_output_buffers_.empty());
  OMX_BUFFERHEADERTYPE* buffer = filled_output_buffers_.front();
  filled_output_buffers_.pop();

  if (output_setting_changed_) {
    return;
  }

  free_output_buffers_.push(buffer);
  output_available_condition_variable_.Signal();
}

OpenMaxComponent::~OpenMaxComponent() {
  if (!handle_) {
    return;
  }

  SendCommandAndWaitForCompletion(OMX_CommandStateSet, OMX_StateIdle);

  SendCommandAndWaitForCompletion(OMX_CommandFlush, input_port_);
  SendCommandAndWaitForCompletion(OMX_CommandFlush, output_port_);

  SendCommand(OMX_CommandPortDisable, input_port_);
  for (size_t i = 0; i < input_buffers_.size(); ++i) {
    OMX_ERRORTYPE error =
        OMX_FreeBuffer(handle_, input_port_, input_buffers_[i]);
    SB_DCHECK(error == OMX_ErrorNone);
  }
  WaitForCommandCompletion();

  DisableOutputPort();

  SendCommandAndWaitForCompletion(OMX_CommandStateSet, OMX_StateLoaded);
}

void OpenMaxComponent::DisableOutputPort() {
  if (SbThreadIsValid(fill_buffer_thread_)) {
    {
      ScopedLock scoped_lock(mutex_);
      kill_fill_buffer_thread_ = true;
      output_available_condition_variable_.Signal();
    }
    SbThreadJoin(fill_buffer_thread_, NULL);
    fill_buffer_thread_ = kSbThreadInvalid;
    kill_fill_buffer_thread_ = false;
  }
  if (!output_buffers_.empty()) {
    SendCommandAndWaitForCompletion(OMX_CommandFlush, output_port_);

    SendCommand(OMX_CommandPortDisable, output_port_);
    for (size_t i = 0; i < output_buffers_.size(); ++i) {
      OMX_ERRORTYPE error =
          OMX_FreeBuffer(handle_, output_port_, output_buffers_[i]);
      SB_DCHECK(error == OMX_ErrorNone);
    }
    output_buffers_.clear();
    WaitForCommandCompletion();
    while (!filled_output_buffers_.empty()) {
      filled_output_buffers_.pop();
    }
    while (!free_output_buffers_.empty()) {
      free_output_buffers_.pop();
    }
  }
}

void OpenMaxComponent::EnableInputPortAndAllocateBuffers() {
  SB_DCHECK(input_buffers_.empty());

  OMXParamPortDefinition port_definition;
  GetInputPortParam(&port_definition);
  if (OnEnableInputPort(&port_definition)) {
    SetPortParam(port_definition);
  }

  SendCommand(OMX_CommandPortEnable, input_port_);

  for (int i = 0; i < port_definition.nBufferCountActual; ++i) {
    OMX_BUFFERHEADERTYPE* buffer;
    OMX_ERRORTYPE error = OMX_AllocateBuffer(handle_, &buffer, input_port_,
                                             NULL, port_definition.nBufferSize);
    SB_DCHECK(error == OMX_ErrorNone);
    input_buffers_.push_back(buffer);
    free_input_buffers_.push(buffer);
  }

  WaitForCommandCompletion();
}

void OpenMaxComponent::EnableOutputPortAndAllocateBuffer() {
  DisableOutputPort();

  OMXParamPortDefinition output_port_definition;

  GetOutputPortParam(&output_port_definition);
  if (OnEnableOutputPort(&output_port_definition)) {
    SetPortParam(output_port_definition);
  }

  SendCommand(OMX_CommandPortEnable, output_port_);

  output_buffers_.reserve(output_port_definition.nBufferCountActual);
  for (int i = 0; i < output_port_definition.nBufferCountActual; ++i) {
    OMX_BUFFERHEADERTYPE* buffer;
    OMX_ERRORTYPE error =
        OMX_AllocateBuffer(handle_, &buffer, output_port_, NULL,
                           output_port_definition.nBufferSize);
    SB_DCHECK(error == OMX_ErrorNone);
    output_buffers_.push_back(buffer);
  }

  WaitForCommandCompletion();

  for (size_t i = 0; i < output_buffers_.size(); ++i) {
    free_output_buffers_.push(output_buffers_[i]);
  }

  SB_DCHECK(!kill_fill_buffer_thread_);
  fill_buffer_thread_ =
      SbThreadCreate(0, kSbThreadPriorityHigh, kSbThreadNoAffinity, true,
                     "output_buffer_filler",
                     &OpenMaxComponent::FillBufferThreadEntryPoint, this);
}

OMX_BUFFERHEADERTYPE* OpenMaxComponent::GetUnusedInputBuffer() {
  ScopedLock scoped_lock(mutex_);
  if (!free_input_buffers_.empty()) {
    OMX_BUFFERHEADERTYPE* buffer_header = free_input_buffers_.front();
    free_input_buffers_.pop();
    return buffer_header;
  }
  return NULL;
}

void OpenMaxComponent::OnOutputSettingChanged() {
  ScopedLock scoped_lock(mutex_);
  output_setting_changed_ = true;
  OnReadyToPeekOutputBuffer();
}

OMX_ERRORTYPE OpenMaxComponent::OnEmptyBufferDone(
    OMX_BUFFERHEADERTYPE* buffer) {
  ScopedLock scoped_lock(mutex_);
  free_input_buffers_.push(buffer);
}

void OpenMaxComponent::OnFillBufferDone(OMX_BUFFERHEADERTYPE* buffer) {
  {
    ScopedLock scoped_lock(mutex_);
    if (output_setting_changed_) {
      return;
    }
    filled_output_buffers_.push(buffer);
  }
  OnReadyToPeekOutputBuffer();
}

void OpenMaxComponent::RunFillBufferLoop() {
  for (;;) {
    OMX_BUFFERHEADERTYPE* buffer = NULL;
    {
      ScopedLock scoped_lock(mutex_);
      if (kill_fill_buffer_thread_) {
        break;
      }
      if (!free_output_buffers_.empty()) {
        buffer = free_output_buffers_.front();
        free_output_buffers_.pop();
      } else {
        output_available_condition_variable_.Wait();
        continue;
      }
    }
    if (buffer) {
      buffer->nFilledLen = 0;

      OMX_ERRORTYPE error = OMX_FillThisBuffer(handle_, buffer);
      SB_DCHECK(error == OMX_ErrorNone);
    }
  }
}

// static
void* OpenMaxComponent::FillBufferThreadEntryPoint(void* context) {
  OpenMaxComponent* component = reinterpret_cast<OpenMaxComponent*>(context);
  component->RunFillBufferLoop();
  return NULL;
}

}  // namespace open_max
}  // namespace shared
}  // namespace raspi
}  // namespace starboard
