// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <tvm/tvm.h>
#include "core/framework/allocator.h"

namespace onnxruntime {

class NupharAllocator : public IDeviceAllocator {
 public:
  NupharAllocator(TVMContext tvm_ctx) : tvm_ctx_(tvm_ctx) {}
  virtual void* Alloc(size_t size) override;
  virtual void Free(void* p) override;
  virtual const AllocatorInfo& Info() const override;

 private:
  const TVMContext tvm_ctx_;
};

}  // namespace onnxruntime