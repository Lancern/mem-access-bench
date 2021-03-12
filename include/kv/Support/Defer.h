#ifndef KV_SUPPORT_DEFER_H
#define KV_SUPPORT_DEFER_H

#include <functional>
#include <utility>

namespace kv {

/**
 * @brief Wrapper objects whose destructor calls a function specified when constructing the object.
 *
 * Objects of this class cannot be copy constructed, move constructed, copy assigned or move assigned.
 */
class DeferredFunction {
public:
  using FunctionType = std::function<void()>;

  /**
   * @brief Construct a new DeferredFunction object.
   *
   * @param func the function that will be called during the destruction of this object.
   */
  explicit DeferredFunction(FunctionType func) noexcept
    : _func(std::move(func))
  { }

  DeferredFunction(const DeferredFunction &) = delete;
  DeferredFunction(DeferredFunction &&) noexcept = delete;

  /**
   * @brief Destroy this object. The destructor will call the function specified when constructing this object.
   */
  ~DeferredFunction() {
    _func();
  }

  DeferredFunction& operator=(const DeferredFunction &) = delete;
  DeferredFunction& operator=(DeferredFunction &&) noexcept = delete;

private:
  FunctionType _func;
}; // class DeferredFunction

/**
 * @brief Defers the execution of the specified function to the end of the enclosing block scope.
 *
 * @param func the function whose execution should be deferred.
 *
 * @return a wrapper object whose destructor invokes the specified function. The returned object should be stored in a
 * local variable to make sure that the execution of the specified function is properly delayed.
 */
[[nodiscard]]
inline DeferredFunction Defer(typename DeferredFunction::FunctionType func) noexcept {
  return DeferredFunction { std::move(func) };
}

#define DEFER(serial, expr) \
  const auto __kv_deferred_func_##serial = kv::Defer([&]() { expr; })

} // namespace kv

#endif // KV_SUPPORT_DEFER_H
