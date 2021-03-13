#ifndef KV_JSON_JSON_OBJECT_H
#define KV_JSON_JSON_OBJECT_H

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "kv/Json/JsonException.h"
#include "kv/Support/Intrinsics.h"
#include "kv/Support/Memory.h"

namespace kv {

/**
 * @brief Types of a JSON object.
 */
enum class JsonObjectType : size_t {
  Null,
  Boolean,
  Number,
  String,
  Array,
  Map,
};

/**
 * @brief Tag type used for indicating construction of a JSON array object.
 */
class JsonArrayTag { }; // class JsonArrayTag

/**
 * @brief Tag type used for indicating construction of a JSON map object.
 */
class JsonMapTag { }; // class JsonMapTag

/**
 * @brief A JSON object.
 */
class JsonObject {
public:
  /**
   * @brief Create a new JsonObject that represents an empty array.
   *
   * @return the created JsonObject object.
   */
  [[nodiscard]]
  static JsonObject CreateArray() noexcept {
    return JsonObject(JsonArrayTag{});
  }

  /**
   * @brief Create a new JsonObject that represents an empty map.
   *
   * @return the created JsonObject object.
   */
  [[nodiscard]]
  static JsonObject CreateMap() noexcept {
    return JsonObject(JsonMapTag{});
  }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
  /**
   * @brief Construct a new JsonObject that represents the null value.
   */
  JsonObject(std::nullptr_t) noexcept
    : _data(std::in_place_type_t<NullType>(), nullptr)
  { }
#pragma clang diagnostic pop

  /**
   * @brief Construct a new JsonObject that represents a boolean value.
   *
   * @param value the boolean value.
   */
  explicit JsonObject(bool value) noexcept
    : _data(std::in_place_type_t<BooleanType>(), value)
  { }

  /**
   * @brief Construct a new JsonObject that represents a number value.
   *
   * This constructor takes part in overload resolution only if T is an arithmetic type.
   *
   * @tparam T the type of the number value.
   * @param value the number value.
   */
  template <typename T,
      std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
  explicit JsonObject(T value) noexcept
    : _data(std::in_place_type_t<NumberType>(), value)
  { }

  /**
   * @brief Construct a new JsonObject that represents a string value.
   *
   * @param value the string value.
   */
  explicit JsonObject(std::string value) noexcept
    : _data(std::in_place_type_t<StringType>(), std::move(value))
  { }

  /**
   * @brief Construct a new JsonObject that represents an array value.
   */
  explicit JsonObject(JsonArrayTag) noexcept
    : _data(std::in_place_type_t<ArrayType>())
  { }

  /**
   * @brief Construct a new JsonObject that represents a map value.
   */
  explicit JsonObject(JsonMapTag) noexcept
    : _data(std::in_place_type_t<MapType>())
  { }

  JsonObject(const JsonObject &) = default;
  JsonObject(JsonObject &&) noexcept = default;

  /**
   * @brief Destroy this JsonObject object.
   */
  virtual ~JsonObject() = default;

  JsonObject& operator=(const JsonObject &) = default;
  JsonObject& operator=(JsonObject &&) noexcept = default;

  /**
   * @brief Get the type of this JSON object.
   *
   * @return the type of this JSON object.
   */
  [[nodiscard]]
  JsonObjectType GetType() const noexcept {
    return static_cast<JsonObjectType>(_data.index());
  }

  /**
   * @brief Determine whether this JSON object is null.
   *
   * @return whether this JSON object is null.
   */
  [[nodiscard]]
  bool IsNull() const noexcept {
    return std::holds_alternative<NullType>(_data);
  }

  /**
   * @brief Determine whether this JSON object is a boolean value.
   *
   * @return whether this JSON object is boolean.
   */
  [[nodiscard]]
  bool IsBoolean() const noexcept {
    return std::holds_alternative<BooleanType>(_data);
  }

  /**
   * @brief Determine whether this JSON object is a number value.
   *
   * @return whether this JSON object is a number value.
   */
  [[nodiscard]]
  bool IsNumber() const noexcept {
    return std::holds_alternative<NumberType>(_data);
  }

  /**
   * @brief Determine whether this JSON object is a string value.
   *
   * @return whether this JSON object is a string value.
   */
  [[nodiscard]]
  bool IsString() const noexcept {
    return std::holds_alternative<StringType>(_data);
  }

  /**
   * @brief Determine whether this JSON object is an array value.
   *
   * @return whether this JSON object is an array value.
   */
  [[nodiscard]]
  bool IsArray() const noexcept {
    return std::holds_alternative<ArrayType>(_data);
  }

  /**
   * @brief Determine whether this JSON object is a map value.
   *
   * @return whether this JSON object is a map value.
   */
  [[nodiscard]]
  bool IsMap() const noexcept {
    return std::holds_alternative<MapType>(_data);
  }

  /**
   * @brief Get the boolean value represented by this JSON object.
   *
   * @return the boolean value represented by this JSON object.
   */
  [[nodiscard]]
  bool GetBoolean() const {
    return details::InterceptAsJsonException([this]() {
      return std::get<BooleanType>(_data);
    });
  }

  /**
   * @brief Get the number value represented by this JSON object.
   *
   * @tparam T the type of the number value. T should be an arithmetic type.
   *
   * @return the number value represented by this JSON object.
   */
  template <typename T = double>
  [[nodiscard]]
  T GetNumber() const {
    static_assert(std::is_arithmetic_v<T>, "T should be an arithmetic type");
    return details::InterceptAsJsonException([this]() {
      return static_cast<T>(std::get<NumberType>(_data));
    });
  }

  /**
   * @brief Get the string value represented by this JSON object.
   *
   * @return the string value represented by this JSON object.
   */
  [[nodiscard]]
  const std::string& GetString() const {
    return details::InterceptAsJsonException([this]() -> const std::string& {
      return std::get<StringType>(_data);
    });
  }

  /**
   * @brief Get the array value represented by this JSON object.
   *
   * @return the array value represented by this JSON object.
   */
  [[nodiscard]]
  std::vector<std::unique_ptr<JsonObject>>& GetArray() {
    return details::InterceptAsJsonException([this]() -> std::vector<std::unique_ptr<JsonObject>> & {
      return std::get<ArrayType>(_data);
    });
  }

  /**
   * @brief Get the array value represented by this JSON object.
   *
   * @return the array value represented by this JSON object.
   */
  [[nodiscard]]
  const std::vector<std::unique_ptr<JsonObject>>& GetArray() const {
    return details::InterceptAsJsonException([this]() -> const std::vector<std::unique_ptr<JsonObject>> & {
      return std::get<ArrayType>(_data);
    });
  }

  /**
   * @brief Get the map value represented by this JSON object.
   *
   * @return the map value represented by this JSON object.
   */
  [[nodiscard]]
  std::unordered_map<std::string, std::unique_ptr<JsonObject>>& GetMap() {
    return details::InterceptAsJsonException(
        [this]() -> std::unordered_map<std::string, std::unique_ptr<JsonObject>> & {
          return std::get<MapType>(_data);
        });
  }

  /**
   * @brief Get the map value represented by this JSON object.
   *
   * @return the map value represented by this JSON object.
   */
  [[nodiscard]]
  const std::unordered_map<std::string, std::unique_ptr<JsonObject>>& GetMap() const {
    return details::InterceptAsJsonException(
        [this]() -> const std::unordered_map<std::string, std::unique_ptr<JsonObject>> & {
          return std::get<MapType>(_data);
        });
  }

  /**
   * @brief Visit the JSON object tree rooted at this JSON object with the specified visitor.
   *
   * The visitor should define the following methods in corresponding signature:
   * * `void VisitNull(const JsonObject &)`;
   * * `void VisitBoolean(const JsonObject &)`;
   * * `void VisitNumber(const JsonObject &)`;
   * * `void VisitString(const JsonObject &)`;
   * * `void VisitArray(const JsonObject &)`;
   * * `void VisitMap(const JsonObject &)`.
   *
   * @tparam Visitor type of the visitor.
   * @param visitor the visitor.
   */
  template <typename Visitor>
  void Visit(Visitor&& visitor) const {
    switch (GetType()) {
      case JsonObjectType::Null:
        visitor.VisitNull(*this);
        break;
      case JsonObjectType::Boolean:
        visitor.VisitBoolean(*this);
        break;
      case JsonObjectType::Number:
        visitor.VisitNumber(*this);
        break;
      case JsonObjectType::String:
        visitor.VisitString(*this);
        break;
      case JsonObjectType::Array: {
        visitor.VisitArray(*this);
        break;
      }
      case JsonObjectType::Map: {
        visitor.VisitMap(*this);
        break;
      }
      default:
        UNREACHABLE();
    }
  }

  [[nodiscard]]
  bool operator==(const JsonObject& rhs) const noexcept {
    return _data == rhs._data;
  }

  [[nodiscard]]
  bool operator!=(const JsonObject& rhs) const noexcept {
    return _data != rhs._data;
  }

private:
  using NullType = std::nullptr_t;
  using BooleanType = bool;
  using NumberType = double;
  using StringType = std::string;
  using ArrayType = std::vector<std::unique_ptr<JsonObject>>;
  using MapType = std::unordered_map<std::string, std::unique_ptr<JsonObject>>;

  std::variant<
      NullType,
      BooleanType,
      NumberType,
      StringType,
      ArrayType,
      MapType> _data;
}; // class JsonObject

} // namespace kv

#endif // KV_JSON_JSON_OBJECT_H
