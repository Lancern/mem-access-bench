#include "kv/Support/Memory.h"

#include <cstdlib>
#include <mutex>

#include "kv/Support/Intrinsics.h"
#include "kv/Support/Defer.h"

namespace kv {

static RawAllocator* globalAllocator;
static std::mutex globalAllocatorLock;

RawAllocator* GetGlobalAllocator() noexcept {
  if (UNLIKELY(!globalAllocator)) {
    globalAllocatorLock.lock();
    DEFER(1, globalAllocatorLock.unlock());

    if (!globalAllocator) {
      // Note that we CANNOT simply use `new RawAllocator()` to allocate a RawAllocator object since this will cause
      // a infinite recurse. Instead, use malloc + placement new to manually construct a RawAllocator object.
      globalAllocator = reinterpret_cast<RawAllocator *>(::malloc(sizeof(RawAllocator)));
      ::new (globalAllocator) RawAllocator();
    }
  }

  return globalAllocator;
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

void operator delete(void* ptr, std::align_val_t) noexcept {
  operator delete(ptr);
}
