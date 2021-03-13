#ifndef KV_JSON_JSON_SYNTHESISER_H
#define KV_JSON_JSON_SYNTHESISER_H

#include <iterator>
#include <type_traits>
#include <utility>

#include "kv/Json/JsonObject.h"

namespace kv {

namespace details {

template <typename OutputIter>
inline void WriteString(OutputIter& output, const char* s) {
  while (*s) {
    *output++ = *s++;
  }
}

template <typename OutputIter>
inline void WriteString(OutputIter& output, const std::string& s) {
  WriteString(output, s.c_str());
}

} // namespace details

/**
 * @brief Serialize JsonObject into JSON representation.
 *
 * @tparam OutputIter type of the output iterator.
 */
template <typename OutputIter>
class JsonSerializer {
public:
  static_assert(
      std::is_same_v<typename std::iterator_traits<OutputIter>::iterator_category, std::output_iterator_tag>,
      "OutputIter should be an output iterator");
  static_assert(
      std::is_same_v<typename std::iterator_traits<OutputIter>::value_type, char>,
      "value_type of OutputIter should be char");

  /**
   * @brief JsonObject visitor used for generating JSON output.
   */
  class Visitor {
  public:
    /**
     * @brief Construct a new Visitor object.
     *
     * @param output the output iterator.
     */
    explicit Visitor(OutputIter& output) noexcept
      : _output(output)
    { }

    void VisitNull(const JsonObject &) {
      details::WriteString(_output, "null");
    }

    void VisitBoolean(const JsonObject& obj) {
      auto value = obj.GetBoolean();
      if (value) {
        details::WriteString(_output, "true");
      } else {
        details::WriteString(_output, "false");
      }
    }

    void VisitNumber(const JsonObject& obj) {
      auto value = obj.template GetNumber();
      details::WriteString(_output, std::to_string(value));
    }

    void VisitString(const JsonObject& obj) {
      const auto& s = obj.GetString();

      *_output++ = '\"';

      for (auto ch : s) {
        switch (ch) {
          case '\"':
            details::WriteString(_output, "\\\"");
            break;
          case '\n':
            details::WriteString(_output, "\\n");
            break;
          case '\t':
            details::WriteString(_output, "\\t");
            break;
          default:
            *_output++ = ch;
            break;
        }
      }

      *_output++ = '\"';
    }

    void VisitArray(const JsonObject& obj) {
      *_output++ = '[';

      auto first = true;
      const auto& arr = obj.GetArray();
      for (const auto& element : arr) {
        if (UNLIKELY(first)) {
          first = false;
        } else {
          *_output++ = ',';
        }

        element->template Visit(*this);
      }

      *_output++ = ']';
    }

    void VisitMap(const JsonObject& obj) {
      *output++ = '{';

      auto first = true;
      const auto& map = obj.GetMap();
      for (const auto& [key, value] : map) {
        if (UNLIKELY(first)) {
          first = false;
        } else {
          *output++ = ',';
        }

        *output++ = '\"';
        details::WriteString(_output, key);
        *output++ = '\"';

        *output++ = ':';

        value->template Visit(*this);
      }

      *output++ = '}';
    }

  private:
    OutputIter& _output;
  }; // class Visitor

  /**
   * @brief Construct a new JsonSerializer object.
   *
   * @param iter the output iterator.
   */
  explicit JsonSerializer(OutputIter iter) noexcept
    : _output(std::move(iter))
  { }

  /**
   * @brief Serialize the specified JSON object into JSON representation.
   *
   * The generated JSON representation is written to the output iterator.
   *
   * @param obj the JsonObject to be serialized.
   */
  void Serialize(const JsonObject& obj) noexcept {
    obj.template Visit(Visitor { _output });
  }

private:
  OutputIter _output;
}; // class JsonSynthesiser

} // namespace kv

#endif // KV_JSON_JSON_SYNTHESISER_H
