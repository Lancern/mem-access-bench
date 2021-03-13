#include "kv/Support/Memory.h"

namespace kv {

static RawAllocator globalAllocator;

RawAllocator* GetGlobalAllocator() noexcept {
  return &globalAllocator;
}

} // namespace kv

void* operator new(size_t count) {
  return kv::GetGlobalAllocator()->Allocate(count);
}

void* operator new(size_t count, std::align_val_t alignment) {
  return kv::GetGlobalAllocator()->Allocate(count, static_cast<size_t>(alignment));
}

void operator delete(void* ptr) noexcept {
  kv::GetGlobalAllocator()->Release(ptr);
}

void operator delete(void* ptr, std::align_val_t) {
  operator delete(ptr);
}
