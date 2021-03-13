#ifndef KV_SUPPORT_MEMORY_H
#define KV_SUPPORT_MEMORY_H

#include <cstddef>
#include <list>
#include <memory>
#include <mutex>
#include <utility>

namespace kv {

/**
 * @brief Allocate raw memory chunks.
 *
 * Objects of this class cannot be copy constructed, move constructed, copy assigned or move assigned.
 */
class RawAllocator {
public:
  constexpr static const size_t DefaultAlignment = 8;

  /**
   * @brief Construct a new RawAllocator object.
   */
  explicit RawAllocator() noexcept;

  RawAllocator(const RawAllocator &) = delete;
  RawAllocator(RawAllocator &&) noexcept = delete;

  /**
   * @brief Destroy this RawAllocator object.
   */
  ~RawAllocator() noexcept;

  RawAllocator& operator=(const RawAllocator &) = delete;
  RawAllocator& operator=(RawAllocator &&) noexcept = delete;

  /**
   * @brief Allocates a new memory chunk with the specified chunk size and alignment requirements.
   *
   * If the arguments do not meet the corresponding requirements, this function will trigger an assertion failure.
   *
   * @param size the minimal size of the allocated memory chunk. The size must be positive.
   * @param alignment the alignment of the allocated memory chunk. The alignment must be a power of 2.
   *
   * @return pointer to the allocated memory chunk.
   * @throw std::bad_alloc if the allocation fails.
   */
  void* Allocate(size_t size, size_t alignment = DefaultAlignment);

  /**
   * @brief Releases the memory chunk referred to by the specified pointer.
   *
   * @param ptr pointer to the memory chunk to be freed.
   */
  void Release(void* ptr) noexcept;

private:
  struct Chunk;

  std::mutex _mutex;
  std::list<Chunk> _chunks;
}; // class Allocator

/**
 * @brief Get the global memory allocator.
 *
 * @return pointer to the global memory allocator.
 */
RawAllocator* GetGlobalAllocator() noexcept;

/**
 * @brief Allocate memory chunks for storing objects of the specified type.
 *
 * ObjectAllocator<T> meets the C++ named requirement Allocation.
 *
 * @tparam T the type of the objects.
 */
template <typename T>
class ObjectAllocator {
public:
  /**
   * @brief Type of the allocated object.
   */
  using value_type = T;

  /**
   * @brief Construct a new ObjectAllocator object using the global raw memory allocator.
   */
  explicit ObjectAllocator() noexcept
    : _raw(GetGlobalAllocator())
  { }

  /**
   * @brief Construct a new ObjectAllocator object.
   *
   * @param raw the underlying raw memory allocator.
   */
  explicit ObjectAllocator(RawAllocator& raw) noexcept
    : _raw(&raw)
  { }

  /**
   * @brief Construct a new ObjectAllocator object.
   *
   * @param another another ObjectAllocator whose underlying raw memory allocator will be shared.
   */
  template <typename U>
  explicit ObjectAllocator(const ObjectAllocator<U>& another) noexcept
    : _raw(another._raw)
  { }

  ObjectAllocator(const ObjectAllocator &) noexcept = default;
  ObjectAllocator(ObjectAllocator &&) noexcept = default;

  /**
   * @brief Destroy this ObjectAllocator object.
   */
  ~ObjectAllocator() noexcept = default;

  ObjectAllocator& operator=(const ObjectAllocator &) noexcept = default;
  ObjectAllocator& operator=(ObjectAllocator &&) noexcept = default;

  template <typename U>
  ObjectAllocator& operator=(const ObjectAllocator<U>& another) noexcept {
    _raw = another._raw;
  }

  /**
   * @brief Allocates memory space for storing `n` objects.
   *
   * @param n the number of objects.
   *
   * @return pointer to the memory space that can be used for storing `n` objects.
   * @throw std::bad_alloc if allocation fails.
   */
  [[nodiscard]]
  T* allocate(size_t n) const {
    return reinterpret_cast<T *>(_raw->Allocate(n * sizeof(T), alignof(T)));
  }

  /**
   * @brief Releases memory space that was previously allocated by this allocator.
   *
   * @param ptr pointer to the memory space.
   */
  void deallocate(T* ptr) const noexcept {
    _raw->Release(ptr);
  }

  [[nodiscard]]
  bool operator==(const ObjectAllocator& rhs) const noexcept {
    return _raw == rhs._raw;
  }

  [[nodiscard]]
  bool operator!=(const ObjectAllocator& rhs) const noexcept {
    return _raw != rhs._raw;
  }

private:
  RawAllocator* _raw;
}; // class ObjectAllocator

/**
 * @brief Deleter for smart pointers whose pointee is allocated by RawAllocator.
 *
 * @tparam T the type of the pointee object.
 */
template <typename T>
class ObjectDeleter {
public:
  /**
   * @brief Construct a new ObjectDeleter object with the specified object allocator.
   *
   * @param allocator the object allocator used for deallocating objects.
   */
  explicit ObjectDeleter(ObjectAllocator<T> allocator = ObjectAllocator<T>()) noexcept
    : _allocator(allocator)
  { }

  void operator()(T* obj) const noexcept {
    _allocator.deallocate(obj);
  }

private:
  ObjectAllocator<T> _allocator;
}; // class ObjectDeleter

} // namespace kv

[[nodiscard]]
void* operator new(size_t count);

[[nodiscard]]
void* operator new(size_t count, std::align_val_t alignment);

void operator delete(void* ptr) noexcept;

void operator delete(void* ptr, std::align_val_t alignment) noexcept;

#endif // KV_SUPPORT_MEMORY_H
