// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "media/base/data_buffer.h"

namespace media {

DataBuffer::DataBuffer(scoped_array<uint8> buffer, int buffer_size)
    : Buffer(base::TimeDelta(), base::TimeDelta()),
      data_(buffer.Pass()),
      buffer_size_(buffer_size),
      data_size_(buffer_size) {
#if defined(__LB_SHELL__)
  // passing a refptr isn't going to work for our C-based memalign pointer
  NOTREACHED();
#endif
}

DataBuffer::DataBuffer(int buffer_size)
    : Buffer(base::TimeDelta(), base::TimeDelta()),
      buffer_size_(buffer_size),
      data_size_(0) {
  Initialize();
}

DataBuffer::DataBuffer(const uint8* data, int data_size)
    : Buffer(base::TimeDelta(), base::TimeDelta()),
      buffer_size_(data_size),
      data_size_(data_size) {
  Initialize();
  memcpy(data_.get(), data, data_size_);
}

DataBuffer::~DataBuffer() {
#if defined(__LB_SHELL__)
  // use free instead of delete[]
  uint8* old_buffer = data_.release();
  if (old_buffer) free(old_buffer);
#endif
}

void DataBuffer::Initialize() {
  // Prevent arbitrary pointers.
  if (buffer_size_ <= 0) {
    buffer_size_ = data_size_ = 0;
    data_.reset();
    return;
  }
#if defined(__LB_SHELL__)
  // align size to platform requirement bytes as well
  buffer_size_ = ((buffer_size_ + kShellMediaBufferAlignment - 1) /
                  kShellMediaBufferAlignment) * kShellMediaBufferAlignment;
  data_.reset((uint8*)memalign(kShellMediaBufferAlignment, buffer_size_));
#else
  data_.reset(new uint8[buffer_size_]);
#endif
}

scoped_refptr<DataBuffer> DataBuffer::CopyFrom(const uint8* data,
                                               int data_size) {
  return make_scoped_refptr(new DataBuffer(data, data_size));
}

const uint8* DataBuffer::GetData() const {
  return data_.get();
}

int DataBuffer::GetDataSize() const {
  return data_size_;
}

uint8* DataBuffer::GetWritableData() {
  return data_.get();
}

void DataBuffer::SetDataSize(int data_size) {
  DCHECK_LE(data_size, buffer_size_);
  data_size_ = data_size;
}

int DataBuffer::GetBufferSize() const {
  return buffer_size_;
}

}  // namespace media
