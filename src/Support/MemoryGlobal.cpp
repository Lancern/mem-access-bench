#include "kv/Support/Memory.h"

namespace kv {

static RawAllocator globalAllocator;

RawAllocator* GetGlobalAllocator() noexcept {
  return &globalAllocator;
}

} // namespace kv
