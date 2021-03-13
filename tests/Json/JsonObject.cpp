#include "kv/Json/JsonObject.h"

#include "gtest/gtest.h"

namespace {

class Visitor {
public:
  explicit Visitor() noexcept
    : NullCount(0),
      BooleanCount(0),
      NumberCount(0),
      StringCount(0),
      ArrayCount(0),
      MapCount(0)
  { }

  int NullCount;
  int BooleanCount;
  int NumberCount;
  int StringCount;
  int ArrayCount;
  int MapCount;

  void VisitNull(const kv::JsonObject &) noexcept {
    ++NullCount;
  }

  void VisitBoolean(const kv::JsonObject &) noexcept {
    ++BooleanCount;
  }

  void VisitNumber(const kv::JsonObject &) noexcept {
    ++NumberCount;
  }

  void VisitString(const kv::JsonObject &) noexcept {
    ++StringCount;
  }

  void VisitArray(const kv::JsonObject &) noexcept {
    ++ArrayCount;
  }

  void VisitMap(const kv::JsonObject &) noexcept {
    ++MapCount;
  }
}; // class Visitor

} // namespace <anonymous>

TEST(JsonObject, TestConstructNull) {
  kv::JsonObject json = nullptr;
  ASSERT_EQ(json.GetType(), kv::JsonObjectType::Null);
  ASSERT_TRUE(json.IsNull());
}

TEST(JsonObject, TestConstructBoolean) {
  kv::JsonObject json { false };
  ASSERT_EQ(json.GetType(), kv::JsonObjectType::Boolean);
  ASSERT_TRUE(json.IsBoolean());
}

TEST(JsonObject, TestConstructNumber) {
  kv::JsonObject json { 10 };
  ASSERT_EQ(json.GetType(), kv::JsonObjectType::Number);
  ASSERT_TRUE(json.IsNumber());
}

TEST(JsonObject, TestConstructString) {
  kv::JsonObject json { "hello" };
  ASSERT_EQ(json.GetType(), kv::JsonObjectType::String);
  ASSERT_TRUE(json.IsString());
}

TEST(JsonObject, TestConstructArray) {
  kv::JsonObject json { kv::JsonArrayTag{} };
  ASSERT_EQ(json.GetType(), kv::JsonObjectType::Array);
  ASSERT_TRUE(json.IsArray());
}

TEST(JsonObject, TestConstructMap) {
  kv::JsonObject json { kv::JsonMapTag{} };
  ASSERT_EQ(json.GetType(), kv::JsonObjectType::Map);
  ASSERT_TRUE(json.IsMap());
}

TEST(JsonObject, TestVisitNull) {
  Visitor visitor;
  kv::JsonObject json = nullptr;
  json.Visit(visitor);

  ASSERT_EQ(visitor.NullCount, 1);
  ASSERT_EQ(visitor.BooleanCount, 0);
  ASSERT_EQ(visitor.NumberCount, 0);
  ASSERT_EQ(visitor.StringCount, 0);
  ASSERT_EQ(visitor.ArrayCount, 0);
  ASSERT_EQ(visitor.MapCount, 0);
}

TEST(JsonObject, TestVisitBoolean) {
  Visitor visitor;
  kv::JsonObject json { false };
  json.Visit(visitor);

  ASSERT_EQ(visitor.NullCount, 0);
  ASSERT_EQ(visitor.BooleanCount, 1);
  ASSERT_EQ(visitor.NumberCount, 0);
  ASSERT_EQ(visitor.StringCount, 0);
  ASSERT_EQ(visitor.ArrayCount, 0);
  ASSERT_EQ(visitor.MapCount, 0);
}

TEST(JsonObject, TestVisitNumber) {
  Visitor visitor;
  kv::JsonObject json { 10 };
  json.Visit(visitor);

  ASSERT_EQ(visitor.NullCount, 0);
  ASSERT_EQ(visitor.BooleanCount, 0);
  ASSERT_EQ(visitor.NumberCount, 1);
  ASSERT_EQ(visitor.StringCount, 0);
  ASSERT_EQ(visitor.ArrayCount, 0);
  ASSERT_EQ(visitor.MapCount, 0);
}

TEST(JsonObject, TestVisitString) {
  Visitor visitor;
  kv::JsonObject json { "hello" };
  json.Visit(visitor);

  ASSERT_EQ(visitor.NullCount, 0);
  ASSERT_EQ(visitor.BooleanCount, 0);
  ASSERT_EQ(visitor.NumberCount, 0);
  ASSERT_EQ(visitor.StringCount, 1);
  ASSERT_EQ(visitor.ArrayCount, 0);
  ASSERT_EQ(visitor.MapCount, 0);
}

TEST(JsonObject, TestVisitArray) {
  Visitor visitor;
  auto json = kv::JsonObject::CreateArray();
  json.Visit(visitor);

  ASSERT_EQ(visitor.NullCount, 0);
  ASSERT_EQ(visitor.BooleanCount, 0);
  ASSERT_EQ(visitor.NumberCount, 0);
  ASSERT_EQ(visitor.StringCount, 0);
  ASSERT_EQ(visitor.ArrayCount, 1);
  ASSERT_EQ(visitor.MapCount, 0);
}

TEST(JsonObject, TestVisitMap) {
  Visitor visitor;
  auto json = kv::JsonObject::CreateMap();
  json.Visit(visitor);

  ASSERT_EQ(visitor.NullCount, 0);
  ASSERT_EQ(visitor.BooleanCount, 0);
  ASSERT_EQ(visitor.NumberCount, 0);
  ASSERT_EQ(visitor.StringCount, 0);
  ASSERT_EQ(visitor.ArrayCount, 0);
  ASSERT_EQ(visitor.MapCount, 1);
}
