/**
 *******************************************************************************
 * @file json.hpp
 *
 * @brief JSON library, providing JSON parsing and serialization header.
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
#ifndef _JSON_HPP_
#define _JSON_HPP_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <initializer_list>

namespace Airsoft::Classes {

enum class JsonParse : uint32_t {
  STANDARD,
  COMMENTS
};

class JsonValue;
class Json;

// Array and object typedefs
using JsonArray_t = std::vector<Json>;
using JsonObject_t = std::map<std::string, Json>;

class Json final {

public:
  // Types
  enum class Type : uint32_t {
    NUL,
    NUMBER,
    BOOL,
    STRING,
    ARRAY,
    OBJECT
  };

  // Constructors for the various types of JSON value.
  Json(void) noexcept;
  Json(std::nullptr_t) noexcept;
  Json(double value);
  Json(int value);
  Json(bool value);
  Json(const std::string &value);
  Json(std::string &&value);
  Json(const char * value);
  Json(const JsonArray_t &values);
  Json(JsonArray_t &&values);
  Json(const JsonObject_t &values);
  Json(JsonObject_t &&values);

  // Implicit constructor: anything with a to_json() function.
  template <class T, class = decltype(&T::to_json)> Json(const T & t) : Json(t.to_json()) {}

  // Implicit constructor: map-like objects (std::map, std::unordered_map, etc)
  template <class M, typename std::enable_if<std::is_constructible<std::string, decltype(std::declval<M>().begin()->first)>::value
      && std::is_constructible<Json, decltype(std::declval<M>().begin()->second)>::value, int>::type = 0>
  Json(const M & m) : Json(object(m.begin(), m.end())) {}

  // Implicit constructor: vector-like objects (std::list, std::vector, std::set, etc)
  template <class V, typename std::enable_if<std::is_constructible<Json, decltype(*std::declval<V>().begin())>::value, int>::type = 0>
  Json(const V & v) : Json(array(v.begin(), v.end())) {}

  // This prevents Json(some_pointer) from accidentally producing a bool. Use
  // Json(bool(some_pointer)) if that behavior is desired.
  Json(void *) = delete;

  // Accessors
  Type type() const;

  bool is_null(void)   const { return type() == Type::NUL; }
  bool is_number(void) const { return type() == Type::NUMBER; }
  bool is_bool(void)   const { return type() == Type::BOOL; }
  bool is_string(void) const { return type() == Type::STRING; }
  bool is_array(void)  const { return type() == Type::ARRAY; }
  bool is_object(void) const { return type() == Type::OBJECT; }

  // Return the enclosed value if this is a number, 0 otherwise. Note that json11 does not
  // distinguish between integer and non-integer numbers - number_value() and int_value()
  // can both be applied to a NUMBER-typed object.
  double number_value(void) const;
  int int_value(void) const;

  // Return the enclosed value if this is a boolean, false otherwise.
  bool bool_value(void) const;
  // Return the enclosed string if this is a string, "" otherwise.
  const std::string &string_value(void) const;
  // Return the enclosed std::vector if this is an array, or an empty vector otherwise.
  const JsonArray_t &array_items(void) const;
  // Return the enclosed std::map if this is an object, or an empty map otherwise.
  const JsonObject_t &object_items(void) const;

  // Return a reference to arr[i] if this is an array, Json() otherwise.
  const Json & operator[](size_t i) const;
  // Return a reference to obj[key] if this is an object, Json() otherwise.
  const Json & operator[](const std::string &key) const;

  // Serialize.
  void dump(std::string &out) const;
  std::string dump(void) const {
      std::string out;
      dump(out);
      return out;
  }

  // Parse. If parse fails, return Json() and assign an error message to err.
  static Json parse(const std::string & in, std::string & err, JsonParse strategy = JsonParse::STANDARD);
  static Json parse(const char * in, std::string & err, JsonParse strategy = JsonParse::STANDARD) {
    if (in) {
      return parse(std::string(in), err, strategy);
    } else {
      err = "null input";
      return nullptr;
    }
  }
  // Parse multiple objects, concatenated or separated by whitespace
  static std::vector<Json> parse_multi(const std::string & in, std::string::size_type & parser_stop_pos, std::string & err, JsonParse strategy = JsonParse::STANDARD);

  static inline std::vector<Json> parse_multi(const std::string & in, std::string & err, JsonParse strategy = JsonParse::STANDARD) {
    std::string::size_type parser_stop_pos;
    return parse_multi(in, parser_stop_pos, err, strategy);
  }

  bool operator== (const Json &rhs) const;
  bool operator<  (const Json &rhs) const;
  bool operator!= (const Json &rhs) const { return !(*this == rhs); }
  bool operator<= (const Json &rhs) const { return !(rhs < *this); }
  bool operator>  (const Json &rhs) const { return  (rhs < *this); }
  bool operator>= (const Json &rhs) const { return !(*this < rhs); }

  /**
   * @brief has_shape(types, err)
   * @retval Return true if this is a JSON object and, for each item in types, has a field of
   *         the given type. If not, return false and set err to a descriptive message.
   */
  typedef std::initializer_list<std::pair<std::string, Type>> shape;
  bool has_shape(const shape & types, std::string & err) const;

private:
  std::shared_ptr<JsonValue> m_ptr;
};

// Internal class hierarchy - JsonValue objects are not exposed to users of this API.
class JsonValue {
protected:
  friend class Json;
  friend class JsonInt;
  friend class JsonDouble;

  virtual Json::Type type(void) const = 0;
  virtual bool equals(const JsonValue * other) const = 0;
  virtual bool less(const JsonValue * other) const = 0;
  virtual void dump(std::string &out) const = 0;
  virtual double number_value(void) const;
  virtual int int_value(void) const;
  virtual bool bool_value(void) const;
  virtual const std::string &string_value(void) const;
  virtual const JsonArray_t &array_items(void) const;
  virtual const Json &operator[](size_t i) const;
  virtual const JsonObject_t &object_items(void) const;
  virtual const Json &operator[](const std::string &key) const;
  virtual ~JsonValue(void) {}
};

} // Airsoft::Classes

#endif  // _JSON_HPP_
