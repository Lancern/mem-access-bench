#include "kv/Support/Memory.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <optional>

#include "kv/Support/Defer.h"
#include "kv/Support/Intrinsics.h"

namespace kv {

constexpr static const size_t NewChunkSize = 4096;

struct RawAllocator::Chunk {
  bool isFirst;  // Is this chunk the first chunk within a memory block that is allocated using `malloc`?
  bool isFree;   // Is this chunk in free state?
  size_t size;   // The size of this chunk
  void* ptr;     // Pointer to the first byte of this memory chunk

  /**
   * @brief Determine whether the specified memory allocation request can be satisfied with this chunk.
   *
   * @param requestSize the size of the memory allocation request.
   * @param alignment the alignment of the memory allocation request.
   *
   * @return whether the specified memory allocation request can be satisfied with this chunk.
   */
  [[nodiscard]]
  bool CanFit(size_t requestSize, size_t alignment) const noexcept {
    assert(isFree && "the chunk under examine should be in free state");
    assert(requestSize > 0 && "size is out of range");
    assert(alignment > 0 && (alignment & (alignment - 1)) == 0 && "alignment should be a power of 2");

    auto basePtr = reinterpret_cast<uintptr_t>(ptr);
    auto arithPtr = reinterpret_cast<uintptr_t>(ptr);
    if (arithPtr & (alignment - 1)) {
      // The chunk pointer is not aligned.
      arithPtr += alignment - (arithPtr & (alignment - 1));
    }

    arithPtr += requestSize;
    return arithPtr - basePtr <= size;
  }

  /**
   * @brief Split this chunk to get a new chunk whose base address is aligned with respect to the specified alignment.
   *
   * This function splits this chunk C into two sub-chunks C1 and C2 such that C = C1 + C2 and the alignment of C2 is
   * at least `alignment`.
   *
   * @param alignment the alignment.
   *
   * @return If this chunk is already aligned, returns empty; otherwise returns the aligned chunk. In the latter case,
   * the `size` field of this chunk will be truncated to the distance between the base of this chunk and the base of the
   * aligned chunk.
   */
  [[nodiscard]]
  std::optional<Chunk> SplitAlignment(size_t alignment) noexcept {
    assert(isFree && "the splitting chunk must be in free state");
    assert(alignment > 0 && (alignment & (alignment - 1)) == 0 && "alignment should be a power of 2");

    auto arithPtr = reinterpret_cast<uintptr_t>(ptr);
    if (LIKELY((arithPtr & (alignment - 1)) == 0)) {
      return std::nullopt;
    }

    auto alignedArithPtr = arithPtr + (alignment - (arithPtr & (alignment - 1)));
    assert(alignedArithPtr <= arithPtr + size && "the pointer is out of bound after alignment");

    Chunk alignedChunk{};
    alignedChunk.isFirst = false;
    alignedChunk.isFree = true;
    alignedChunk.size = size - (alignedArithPtr - arithPtr);
    alignedChunk.ptr = reinterpret_cast<void *>(alignedArithPtr);

    size = alignedArithPtr - arithPtr;

    return alignedChunk;
  }

  /**
   * @brief Split this chunk to get a new chunk whose size is the specified size.
   *
   * This function splits this chunk C into two sub-chunks C1 and C2 such that C = C1 + C2 and the size of C1 is the
   * specified size.
   *
   * @param splitSize the size of the split chunk. This argument should be less than or equal to the size of this chunk.
   *
   * @return If splitSize is equal to the size of this chunk, then this function returns empty; otherwise this function
   * returns the chunk C2 mentioned above. In the latter case, the size field of this chunk will be set to splitSize.
   */
  [[nodiscard]]
  std::optional<Chunk> SplitSize(size_t splitSize) noexcept {
    assert(isFree && "the splitting chunk must be in free state");
    assert(splitSize > 0 && splitSize <= size && "splitSize is out of range");

    if (splitSize == size) {
      return std::nullopt;
    }

    Chunk rest{};
    rest.isFirst = false;
    rest.isFree = true;
    rest.size = size - splitSize;
    rest.ptr = reinterpret_cast<uint8_t *>(ptr) + splitSize;

    size = splitSize;
    return rest;
  }

  /**
   * @brief Try to merge the specified chunk into this chunk.
   *
   * The specified chunk can be merged into this chunk only if the following conditions hold:
   * * another.ptr > this->ptr
   * * this->ptr + this->size == another.ptr
   *
   * @param another another chunk to merge.
   * @return whether the merge is successful or not.
   */
  bool Merge(Chunk& another) noexcept {
    assert(isFree && another.isFree && "the merging chunks should be in free state");

    if (another.isFirst) {
      return false;
    }

    if (another.ptr < ptr) {
      return false;
    }

    auto arithPtr = reinterpret_cast<uintptr_t>(ptr);
    if (arithPtr + size != reinterpret_cast<uintptr_t>(another.ptr)) {
      return false;
    }

    size += another.size;
    return true;
  }
};

RawAllocator::RawAllocator() = default;

RawAllocator::~RawAllocator() noexcept = default;

void* RawAllocator::Allocate(size_t size, size_t alignment) {
  assert(size > 0 && "size should be a positive value");
  assert(alignment > 0 && (alignment & (alignment - 1)) == 0 && "alignment should be a power of 2");

  _mutex.lock();
  DEFER(1, _mutex.unlock());

  // Scan existing chunks for any suitable chunk.
  auto it = std::find_if(_chunks.begin(), _chunks.end(), [size, alignment](const Chunk& chunk) noexcept {
    return chunk.isFree && chunk.CanFit(size, alignment);
  });

  if (it == _chunks.end()) {
    // No suitable chunk found. Allocate a new chunk to meet the request.
    Chunk chunk{};
    chunk.isFirst = true;
    chunk.isFree = true;
    chunk.size = std::max(size + alignment, NewChunkSize);
    chunk.ptr = ::malloc(chunk.size);
    if (!chunk.ptr) {
      throw std::bad_alloc();
    }

    it = _chunks.insert(_chunks.begin(), chunk);
  }

  auto alignedChunk = it->SplitAlignment(alignment);
  if (alignedChunk) {
    it = _chunks.insert(std::next(it), alignedChunk.value());
  }

  auto restChunk = it->SplitSize(size);
  if (restChunk) {
    _chunks.insert(std::next(it), restChunk.value());
  }

  it->isFree = false;
  return it->ptr;
}

void RawAllocator::Release(void *ptr) noexcept {
  _mutex.lock();
  DEFER(1, _mutex.unlock());

  auto it = std::find_if(_chunks.begin(), _chunks.end(), [ptr](const Chunk& chunk) noexcept {
    return !chunk.isFree && chunk.ptr == ptr;
  });

  if (UNLIKELY(it == _chunks.end())) {
    return;
  }

  it->isFree = true;

  // Try to merge with the previous chunk
  if (it != _chunks.begin()) {
    auto pv = std::prev(it);
    if (pv->isFree && pv->Merge(*it)) {
      _chunks.erase(it);
      it = pv;
    }
  }

  // Try to merge with the next chunk
  auto nx = std::next(it);
  if (nx != _chunks.end()) {
    if (nx->isFree && it->Merge(*nx)) {
      _chunks.erase(nx);
    }
  }
}

} // namespace kv
