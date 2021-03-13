#ifndef KV_JSON_JSON_EXCEPTION_H
#define KV_JSON_JSON_EXCEPTION_H

#include <exception>
#include <string>
#include <type_traits>
#include <utility>

namespace kv {

/**
 * @brief The exception thrown by the Json library.
 */
class JsonException : public std::exception {
public:
  /**
   * @brief Construct a new JsonException object.
   */
  explicit JsonException() noexcept = default;

  /**
   * @brief Construct a new JsonException object with the specified message.
   *
   * @param message the exception message.
   */
  explicit JsonException(std::string message) noexcept
    : _message(std::move(message))
  { }

  /**
   * @brief Get the exception message.
   *
   * @return the exception message.
   */
  [[nodiscard]]
  const std::string& GetMessage() const noexcept {
    return _message;
  }

  [[nodiscard]]
  const char* what() const noexcept override {
    return _message.c_str();
  }

private:
  std::string _message;
}; // class JsonException

namespace details {

template <typename F, typename ...Args>
inline decltype(auto) InterceptAsJsonException(const F& func, Args&&... args) {
  static_assert(std::is_invocable_v<F, Args...>,
      "F should be a function object type that can be invoked with the specified arguments");

  try {
    if constexpr (std::is_void_v<std::invoke_result_t<F, Args...>>) {
      func(std::forward<Args>(args)...);
    } else {
      return func(std::forward<Args>(args)...);
    }
  } catch (const JsonException &) {
    throw;
  } catch (...) {
    throw JsonException();
  }
}

} // namespace details

} // namespace kv

#endif // KV_JSON_JSON_EXCEPTION_H
