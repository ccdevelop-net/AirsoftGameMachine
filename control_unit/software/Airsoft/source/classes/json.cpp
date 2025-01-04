/**
 *******************************************************************************
 * @file json.cpp
 *
 * @brief JSON library, providing JSON parsing and serialization.
 *
 * @author  Cristian Croci - ccdevelop.net
 *
 * @version 1.00
 *
 * @date December 18, 2024
 *
 *******************************************************************************
 * This file is part of the Airsoft project
 * https://github.com/ccdevelop-net/AirsoftGameMachine.
 * Copyright (c) 2024 CCDevelop.NET
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <limits>
#include <climits>

#include <classes/json.hpp>

namespace Airsoft::Classes {

//======================================================================================================================
// Missing C functions library *****************************************************************************************
//======================================================================================================================


// Convert a string to a long integer.
// Ignores `locale' stuff.  Assumes that the upper and lower case
// alphabets and digits are each contiguous.
long strtol(const char *nptr, char **endptr, volatile int base) {
  volatile const char *s = nptr;
  volatile unsigned long acc;
  volatile int c;
  volatile unsigned long cutoff;
  volatile int neg = 0, any, cutlim;

  // Skip white space and pick up leading +/- sign if any.
  // If base is 0, allow 0x for hex and 0 for octal, else
  // assume decimal; if base is already 16, allow 0x.
  do {
    c = *s++;
  } while (isspace(c));

  if (c == '-') {
    neg = 1;
    c = *s++;
  } else if (c == '+') {
    c = *s++;
  }

  if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
    c = s[1];
    s += 2;
    base = 16;
  }

  if (base == 0) {
    base = c == '0' ? 8 : 10;
  }

  // Compute the cutoff value between legal numbers and illegal numbers.
  // That is the largest legal value, divided by the base.
  // An input number that is greater than this value, if followed by a legal input character, is too big.
  // One that is equal to this value may be valid or not; the limit between valid and invalid numbers is
  // then based on the last digit.
  // For instance, if the range for longs is [-2147483648..2147483647] and the input base is 10,
  // cutoff will be set to 214748364 and cutlim to either 7 (neg==0) or 8 (neg==1),
  // meaning that if we have accumulated a value > 214748364, or equal but the next digit is > 7 (or 8),
  // the number is too big, and we will return a range error.
  // Set any if any `digits' consumed; make it negative to indicate overflow.
  cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
  cutlim = cutoff % (unsigned long)base;
  cutoff /= (unsigned long)base;
  for (acc = 0, any = 0;; c = *s++) {
    if (isdigit(c)) {
      c -= '0';
    } else if (isalpha(c)) {
      c -= isupper(c) ? 'A' - 10 : 'a' - 10;
    } else {
      break;
    }

    if (c >= base) {
      break;
    }

    if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
      any = -1;
    } else {
      any = 1;
      acc *= base;
      acc += c;
    }
  }

  if (any < 0) {
    acc = neg ? LONG_MIN : LONG_MAX;
    errno = ERANGE;
  } else if (neg) {
    acc = -acc;
  }

  if (endptr != 0) {
    *endptr = (char *) (any ? s - 1 : nptr);
  }

  return (acc);
}

//======================================================================================================================
// Null struct *********************************************************************************************************
//======================================================================================================================


constexpr int32_t max_depth = 200;

/**
 * Helper for representing null - just a do-nothing struct, plus comparison
 * operators so the helpers in JsonValue work. We can't use nullptr_t because
 * it may not be orderable.
 */
struct NullStruct {
  bool operator == (NullStruct) const {
    return true;
  }
  bool operator < (NullStruct) const {
    return false;
  }
};

//======================================================================================================================
// Serialization *******************************************************************************************************
//======================================================================================================================

//-----------------------------------------------------------------------------
static void dump(NullStruct, std::string &out) {
  //out += "null";
}
//-----------------------------------------------------------------------------
static void dump(double value, std::string &out) {
  /*if (std::isfinite(value)) {
    char buf[32];
    snprintf(buf, sizeof buf, "%.17g", value);
    out += buf;
  } else {
    out += "null";
  }*/
}
//-----------------------------------------------------------------------------
static void dump(int32_t value, std::string &out) {
  // Function Variables
  char buf[32];

  snprintf(buf, sizeof buf, "%d", value);

  out += buf;
}
//-----------------------------------------------------------------------------
static void dump(bool value, std::string &out) {
  out += value ? "true" : "false";
}
//-----------------------------------------------------------------------------
static void dump(const std::string &value, std::string &out) {
  out += '"';

  for (size_t i = 0; i < value.length(); i++) {
    const char ch = value[i];

    if (ch == '\\') {
      out += "\\\\";
    } else if (ch == '"') {
      out += "\\\"";
    } else if (ch == '\b') {
      out += "\\b";
    } else if (ch == '\f') {
      out += "\\f";
    } else if (ch == '\n') {
      out += "\\n";
    } else if (ch == '\r') {
      out += "\\r";
    } else if (ch == '\t') {
      out += "\\t";
    } else if (static_cast<uint8_t>(ch) <= 0x1f) {
      char buf[8];
      snprintf(buf, sizeof buf, "\\u%04x", ch);
      out += buf;
    } else if (static_cast<uint8_t>(ch) == 0xE2 &&
               static_cast<uint8_t>(value[i + 1]) == 0x80 &&
               static_cast<uint8_t>(value[i + 2]) == 0xA8) {
      out += "\\u2028";
      i += 2;
    } else if (static_cast<uint8_t>(ch) == 0xE2 &&
               static_cast<uint8_t>(value[i + 1]) == 0x80 &&
               static_cast<uint8_t>(value[i + 2]) == 0xA9) {
      out += "\\u2029";
      i += 2;
    } else {
      out += ch;
    }
  }

  out += '"';
}
//-----------------------------------------------------------------------------
static void dump(const JsonArray_t &values, std::string &out) {
  // Function Variables
  bool first { true };

  out += "[";

  for (const auto &value : values) {
    if (!first) {
      out += ", ";
    }

    value.dump(out);

    first = false;
  }

  out += "]";
}
//-----------------------------------------------------------------------------
static void dump(const JsonObject_t &values, std::string &out) {
  // Function Variables
  bool first { true };

  out += "{";

  for (const auto &kv : values) {
    if (!first) {
      out += ", ";
    }

    dump(kv.first, out);
    out += ": ";
    kv.second.dump(out);
    first = false;
  }

  out += "}";
}
//-----------------------------------------------------------------------------
void Json::dump(std::string &out) const {
  m_ptr->dump(out);
}
//-----------------------------------------------------------------------------

//======================================================================================================================
// Value Wrappers ******************************************************************************************************
//======================================================================================================================

template<Json::Type tag, typename T>
class Value: public JsonValue {
protected:
  // Constructors
  explicit Value(const T &value) :
      m_value(value) {
  }
  explicit Value(T &&value) :
      m_value(std::move(value)) {
  }

  // Get type tag
  Json::Type type() const override {
    return tag;
  }

  // Comparisons
  bool equals(const JsonValue *other) const override {
    return m_value == static_cast<const Value<tag, T>*>(other)->m_value;
  }
  bool less(const JsonValue *other) const override {
    return m_value < static_cast<const Value<tag, T>*>(other)->m_value;
  }

  const T m_value;
  void dump(std::string &out) const override {
    Airsoft::Classes::dump(m_value, out);
  }
};

class JsonDouble final : public Value<Json::Type::NUMBER, double> {
  double number_value() const override {
    return m_value;
  }
  int32_t int_value() const override {
    return static_cast<int32_t>(m_value);
  }
  bool equals(const JsonValue *other) const override {
    return m_value == other->number_value();
  }
  bool less(const JsonValue *other) const override {
    return m_value < other->number_value();
  }
public:
  explicit JsonDouble(double value) : Value(value) { }
};

class JsonInt final : public Value<Json::Type::NUMBER, int32_t> {
  double number_value() const override {
    return m_value;
  }
  int32_t int_value() const override {
    return m_value;
  }
  bool equals(const JsonValue *other) const override {
    return m_value == other->number_value();
  }
  bool less(const JsonValue *other) const override {
    return m_value < other->number_value();
  }
public:
  explicit JsonInt(int32_t value) :
      Value(value) {
  }
};

class JsonBoolean final : public Value<Json::Type::BOOL, bool> {
  bool bool_value() const override {
    return m_value;
  }
public:
  explicit JsonBoolean(bool value) :
      Value(value) {
  }
};

class JsonString final : public Value<Json::Type::STRING, std::string> {
  const std::string& string_value() const override {
    return m_value;
  }
public:
  explicit JsonString(const std::string &value) :
      Value(value) {
  }
  explicit JsonString(std::string &&value) :
      Value(move(value)) {
  }
};

class JsonArray final : public Value<Json::Type::ARRAY, JsonArray_t> {
  const JsonArray_t& array_items() const override {
    return m_value;
  }
  const Json& operator[](size_t i) const override;
public:
  explicit JsonArray(const JsonArray_t &value) :
      Value(value) {
  }
  explicit JsonArray(JsonArray_t &&value) :
      Value(move(value)) {
  }
};

class JsonObject final : public Value<Json::Type::OBJECT, JsonObject_t> {
  const JsonObject_t& object_items() const override {
    return m_value;
  }
  const Json& operator[](const std::string &key) const override;
public:
  explicit JsonObject(const JsonObject_t &value) :
      Value(value) {
  }
  explicit JsonObject(JsonObject_t &&value) :
      Value(move(value)) {
  }
};

class JsonNull final : public Value<Json::Type::NUL, NullStruct> {
public:
  JsonNull() : Value({ }) { }
};

//======================================================================================================================
// Static Globals ******************************************************************************************************
//======================================================================================================================

struct Statics {
  const std::shared_ptr<JsonValue>  null = std::make_shared<JsonNull>();
  const std::shared_ptr<JsonValue>  t = std::make_shared<JsonBoolean>(true);
  const std::shared_ptr<JsonValue>  f = std::make_shared<JsonBoolean>(false);
  const std::string                 empty_string;
  const std::vector<Json>           empty_vector;
  const std::map<std::string, Json> empty_map;
  Statics(void) { }
};

static const Statics& statics() {
  static const Statics s { };
  return s;
}

static const Json& static_null() {
  // This has to be separate, not in Statics, because Json() accesses statics().null.
  static const Json json_null;
  return json_null;
}

//======================================================================================================================
// Constructors ********************************************************************************************************
//======================================================================================================================

Json::Json() noexcept : m_ptr(statics().null) {}
Json::Json(std::nullptr_t) noexcept : m_ptr(statics().null) {}
Json::Json(double value) : m_ptr(std::make_shared<JsonDouble>(value)) {}
Json::Json(int32_t value) : m_ptr(std::make_shared<JsonInt>(value)) {}
Json::Json(bool value) : m_ptr(value ? statics().t : statics().f) {}
Json::Json(const std::string &value) : m_ptr(std::make_shared<JsonString>(value)) {}
Json::Json(std::string &&value) : m_ptr(std::make_shared<JsonString>(move(value))) {}
Json::Json(const char *value) : m_ptr(std::make_shared<JsonString>(value)) {}
Json::Json(const JsonArray_t &values) : m_ptr(std::make_shared<JsonArray>(values)) {}
Json::Json(JsonArray_t &&values) : m_ptr(std::make_shared<JsonArray>(move(values))) {}
Json::Json(const JsonObject_t &values) : m_ptr(std::make_shared<JsonObject>(values)) {}
Json::Json(JsonObject_t &&values) : m_ptr(std::make_shared<JsonObject>(move(values))) {}

//======================================================================================================================
// Accessors ***********************************************************************************************************
//======================================================================================================================

Json::Type Json::type() const {
  return m_ptr->type();
}
double Json::number_value() const {
  return m_ptr->number_value();
}
int32_t Json::int_value() const {
  return m_ptr->int_value();
}
bool Json::bool_value() const {
  return m_ptr->bool_value();
}
const std::string& Json::string_value() const {
  return m_ptr->string_value();
}
const std::vector<Json>& Json::array_items() const {
  return m_ptr->array_items();
}
const std::map<std::string, Json>& Json::object_items() const {
  return m_ptr->object_items();
}
const Json& Json::operator[](size_t i) const {
  return (*m_ptr)[i];
}
const Json& Json::operator[](const std::string &key) const {
  return (*m_ptr)[key];
}

double JsonValue::number_value(void) const {
  return 0;
}
int32_t JsonValue::int_value(void) const {
  return 0;
}
bool JsonValue::bool_value(void) const {
  return false;
}
const std::string& JsonValue::string_value() const {
  return statics().empty_string;
}
const std::vector<Json>& JsonValue::array_items() const {
  return statics().empty_vector;
}
const std::map<std::string, Json>& JsonValue::object_items() const {
  return statics().empty_map;
}
const Json& JsonValue::operator[](size_t) const {
  return static_null();
}
const Json& JsonValue::operator[](const std::string&) const {
  return static_null();
}

const Json& JsonObject::operator[](const std::string &key) const {
  auto iter = m_value.find(key);
  return (iter == m_value.end()) ? static_null() : iter->second;
}
const Json& JsonArray::operator[](size_t i) const {
  if (i >= m_value.size())
    return static_null();
  else
    return m_value[i];
}

//======================================================================================================================
// Comparison **********************************************************************************************************
//======================================================================================================================

bool Json::operator == (const Json &other) const {
  if (m_ptr == other.m_ptr) {
    return true;
  }

  if (m_ptr->type() != other.m_ptr->type()) {
    return false;
  }

  return m_ptr->equals(other.m_ptr.get());
}

bool Json::operator<(const Json &other) const {
  if (m_ptr == other.m_ptr) {
    return false;
  }

  if (m_ptr->type() != other.m_ptr->type()) {
    return m_ptr->type() < other.m_ptr->type();
  }

  return m_ptr->less(other.m_ptr.get());
}

//======================================================================================================================
// Parsing *************************************************************************************************************
//======================================================================================================================

static inline std::string Esc(char c) {
  char buf[12];

  if (static_cast<uint8_t>(c) >= 0x20 && static_cast<uint8_t>(c) <= 0x7f) {
    snprintf(buf, sizeof buf, "'%c' (%d)", c, c);
  } else {
    snprintf(buf, sizeof buf, "(%d)", c);
  }

  return std::string(buf);
}

static inline bool InRange(int64_t x, int64_t lower, int64_t upper) {
  return (x >= lower && x <= upper);
}

//======================================================================================================================
// JsonParser - Object that tracks all state of an in-progress parse. **************************************************
//======================================================================================================================

struct JsonParser final {

  // State
  const std::string &str;
  size_t i;
  std::string &err;
  bool failed;
  const JsonParse strategy;

  Json Fail(std::string &&msg) {
    return Fail(move(msg), Json());
  }

  template<typename T>
  T Fail(std::string &&msg, const T err_ret) {
    if (!failed) {
      err = std::move(msg);
    }
    failed = true;
    return err_ret;
  }

  void ConsumeWhitespace(void) {
    while (str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == '\t') {
      i++;
    }
  }

  bool ConsumeComment(void) {
    // Function Variables
    bool comment_found {};

    if (str[i] == '/') {
      i++;

      if (i == str.size()) {
        return Fail("unexpected end of input after start of comment", false);
      }

      if (str[i] == '/') { // inline comment
        i++;
        // advance until next line, or end of input
        while (i < str.size() && str[i] != '\n') {
          i++;
        }
        comment_found = true;
      } else if (str[i] == '*') { // multiline comment
        i++;

        if (i > str.size() - 2) {
          return Fail("unexpected end of input inside multi-line comment", false);
        }

        // advance until closing tokens
        while (!(str[i] == '*' && str[i + 1] == '/')) {
          i++;
          if (i > str.size() - 2) {
            return Fail("unexpected end of input inside multi-line comment", false);
          }
        }
        i += 2;
        comment_found = true;
      } else {
        return Fail("malformed comment", false);
      }
    }
    return comment_found;
  }

  void ConsumeGarbage() {
    ConsumeWhitespace();

    if (strategy == JsonParse::COMMENTS) {
      bool comment_found {};

      do {
        comment_found = ConsumeComment();
        if (failed) {
          return;
        }
        ConsumeWhitespace();
      } while (comment_found);
    }
  }

  char GetNextToken() {
    ConsumeGarbage();
    if (failed) {
      return static_cast<char>(0);
    }

    if (i == str.size()) {
      return Fail("unexpected end of input", static_cast<char>(0));
    }

    return str[i++];
  }

  void EncodeUtf8(int64_t pt, std::string &out) {
    if (pt < 0) {
      return;
    }

    if (pt < 0x80) {
      out += static_cast<char>(pt);
    } else if (pt < 0x800) {
      out += static_cast<char>((pt >> 6) | 0xC0);
      out += static_cast<char>((pt & 0x3F) | 0x80);
    } else if (pt < 0x10000) {
      out += static_cast<char>((pt >> 12) | 0xE0);
      out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
      out += static_cast<char>((pt & 0x3F) | 0x80);
    } else {
      out += static_cast<char>((pt >> 18) | 0xF0);
      out += static_cast<char>(((pt >> 12) & 0x3F) | 0x80);
      out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
      out += static_cast<char>((pt & 0x3F) | 0x80);
    }
  }

  std::string ParseString() {
    // Function Variables
    std::string out;

    int64_t last_escaped_codepoint { -1 };

    while (true) {
      if (i == str.size()) {
        return Fail("unexpected end of input in string", "");
      }

      char ch = str[i++];

      if (ch == '"') {
        EncodeUtf8(last_escaped_codepoint, out);
        return out;
      }

      if (InRange(ch, 0, 0x1f)) {
        return Fail("unescaped " + Esc(ch) + " in string", "");
      }

      // The usual case: non-escaped characters
      if (ch != '\\') {
        EncodeUtf8(last_escaped_codepoint, out);
        last_escaped_codepoint = -1;
        out += ch;
        continue;
      }

      // Handle escapes
      if (i == str.size()) {
        return Fail("unexpected end of input in string", "");
      }

      ch = str[i++];

      if (ch == 'u') {
        // Extract 4-byte escape sequence
        std::string esc { str.substr(i, 4) };

        // Explicitly check length of the substring. The following loop
        // relies on std::string returning the terminating NUL when
        // accessing str[length]. Checking here reduces brittleness.
        if (esc.length() < 4) {
          return Fail("bad \\u escape: " + esc, "");
        }

        for (size_t j = 0; j < 4; j++) {
          if (!InRange(esc[j], 'a', 'f') && !InRange(esc[j], 'A', 'F') && !InRange(esc[j], '0', '9')) {
            return Fail("bad \\u escape: " + esc, "");
          }
        }

        int64_t codepoint = strtol(esc.data(), nullptr, 16);

        // JSON specifies that characters outside the BMP shall be encoded as a pair
        // of 4-hex-digit \u escapes encoding their surrogate pair components. Check
        // whether we're in the middle of such a beast: the previous codepoint was an
        // escaped lead (high) surrogate, and this is a trail (low) surrogate.
        if (InRange(last_escaped_codepoint, 0xD800, 0xDBFF) && InRange(codepoint, 0xDC00, 0xDFFF)) {
          // Reassemble the two surrogate pairs into one astral-plane character, per
          // the UTF-16 algorithm.
          EncodeUtf8((((last_escaped_codepoint - 0xD800) << 10) | (codepoint - 0xDC00)) + 0x10000, out);
          last_escaped_codepoint = -1;
        } else {
          EncodeUtf8(last_escaped_codepoint, out);
          last_escaped_codepoint = codepoint;
        }

        i += 4;
        continue;
      }

      EncodeUtf8(last_escaped_codepoint, out);
      last_escaped_codepoint = -1;

      if (ch == 'b') {
        out += '\b';
      } else if (ch == 'f') {
        out += '\f';
      } else if (ch == 'n') {
        out += '\n';
      } else if (ch == 'r') {
        out += '\r';
      } else if (ch == 't') {
        out += '\t';
      } else if (ch == '"' || ch == '\\' || ch == '/') {
        out += ch;
      } else {
        return Fail("invalid escape character " + Esc(ch), "");
      }
    }
  }

  Json ParseNumber(void) {
    size_t start_pos = i;

    if (str[i] == '-') {
      i++;
    }

    // Integer part
    if (str[i] == '0') {
      i++;
      if (InRange(str[i], '0', '9')) {
        return Fail("leading 0s not permitted in numbers");
      }
    } else if (InRange(str[i], '1', '9')) {
      i++;
      while (InRange(str[i], '0', '9')) {
        i++;
      }
    } else {
      return Fail("invalid " + Esc(str[i]) + " in number");
    }

    if (str[i] != '.' && str[i] != 'e' && str[i] != 'E' && (i - start_pos) <= static_cast<size_t>(std::numeric_limits<int32_t>::digits10)) {
      return std::atoi(str.c_str() + start_pos);
    }

    // Decimal part
    if (str[i] == '.') {
      i++;
      if (!InRange(str[i], '0', '9')) {
        return Fail("at least one digit required in fractional part");
      }

      while (InRange(str[i], '0', '9')) {
        i++;
      }
    }

    // Exponent part
    if (str[i] == 'e' || str[i] == 'E') {
      i++;

      if (str[i] == '+' || str[i] == '-') {
        i++;
      }

      if (!InRange(str[i], '0', '9')) {
        return Fail("at least one digit required in exponent");
      }

      while (InRange(str[i], '0', '9')) {
        i++;
      }
    }

    return std::strtod(str.c_str() + start_pos, nullptr);
  }

  Json Expect(const std::string &expected, Json res) {
    assert(i != 0);

    i--;

    if (str.compare(i, expected.length(), expected) == 0) {
      i += expected.length();
      return res;
    } else {
      return Fail("parse error: expected " + expected + ", got " + str.substr(i, expected.length()));
    }
  }

  Json ParseJson(int32_t depth) {
    if (depth > max_depth) {
      return Fail("exceeded maximum nesting depth");
    }

    char ch { GetNextToken() };
    if (failed) {
      return Json();
    }

    if (ch == '-' || (ch >= '0' && ch <= '9')) {
      i--;
      return ParseNumber();
    }

    if (ch == 't') {
      return Expect("true", true);
    }

    if (ch == 'f') {
      return Expect("false", false);
    }

    if (ch == 'n') {
      return Expect("null", Json());
    }

    if (ch == '"') {
      return ParseString();
    }

    if (ch == '{') {
      std::map<std::string, Json> data;
      ch = GetNextToken();

      if (ch == '}') {
        return data;
      }

      while (true) {
        if (ch != '"') {
          return Fail("expected '\"' in object, got " + Esc(ch));
        }

        std::string key = ParseString();
        if (failed) {
          return Json();
        }

        ch = GetNextToken();
        if (ch != ':') {
          return Fail("expected ':' in object, got " + Esc(ch));
        }

        data[std::move(key)] = ParseJson(depth + 1);
        if (failed) {
          return Json();
        }

        ch = GetNextToken();

        if (ch == '}') {
          break;
        }

        if (ch != ',') {
          return Fail("expected ',' in object, got " + Esc(ch));
        }

        ch = GetNextToken();
      }
      return data;
    }

    if (ch == '[') {
      std::vector<Json> data;
      ch = GetNextToken();
      if (ch == ']') {
        return data;
      }

      while (true) {
        i--;
        data.push_back(ParseJson(depth + 1));
        if (failed) {
          return Json();
        }

        ch = GetNextToken();
        if (ch == ']') {
          break;
        }

        if (ch != ',') {
          return Fail("expected ',' in list, got " + Esc(ch));
        }

        ch = GetNextToken();
        (void) ch;
      }

      return data;
    }

    return Fail("expected value, got " + Esc(ch));
  }
};

//======================================================================================================================
// Public Functions ****************************************************************************************************
//======================================================================================================================

//-----------------------------------------------------------------------------
Json Json::Parse(const std::string &in, std::string &err, JsonParse strategy) {
  // Function Variables
  JsonParser parser { in, 0, err, false, strategy };
  Json result = parser.ParseJson(0);

  // Check for any trailing garbage
  parser.ConsumeGarbage();
  if (parser.failed) {
    return Json();
  }

  if (parser.i != in.size()) {
    return parser.Fail("unexpected trailing " + Esc(in[parser.i]));
  }

  return result;
}
//-----------------------------------------------------------------------------
std::vector<Json> Json::ParseMulti(const std::string &in, std::string::size_type &parser_stop_pos, std::string &err, JsonParse strategy) {
  // Function Variables
  JsonParser parser { in, 0, err, false, strategy };
  std::vector<Json> json_vec;

  parser_stop_pos = 0;

  while (parser.i != in.size() && !parser.failed) {
    json_vec.push_back(parser.ParseJson(0));
    if (parser.failed) {
      break;
    }

    // Check for another object
    parser.ConsumeGarbage();
    if (parser.failed) {
      break;
    }

    parser_stop_pos = parser.i;
  }

  return json_vec;
}
//-----------------------------------------------------------------------------
bool Json::HasShape(const shape & types, std::string & err) const {
  if (!is_object()) {
    err = "expected JSON object, got " + dump();
    return false;
  }

  const auto &obj_items { object_items() };

  for (auto &item : types) {
    const auto it = obj_items.find(item.first);

    if (it == obj_items.cend() || it->second.type() != item.second) {
      err = "bad type for " + item.first + " in " + dump();
      return false;
    }
  }

  return true;
}
//-----------------------------------------------------------------------------

} // namespace Airsoft::Classes
