#pragma once

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "broker/detail/operators.hh"

namespace broker {

/// A hierachical topic used as pub/sub communication pattern.
class topic : detail::totally_ordered<topic> {
public:
  /// The separator between topic hierarchies.
  static constexpr char sep = '/';

  /// A reserved string which must not appear in a user topic.
  static constexpr char reserved[] = "<$>";

  /// Splits a topic into a vector of its components.
  /// @param t The topic to split.
  /// @returns The components that make up the topic.
  static std::vector<std::string> split(const topic& t);

  /// Joins a sequence of components to a hierarchical topic.
  /// @param components The components that make up the topic.
  /// @returns The topic according to *components*.
  static topic join(const std::vector<std::string>& components);

  /// Default-constructs an empty topic.
  topic() = default;

  /// Constructs a topic from a type that is convertible to a string.
  /// @param x A value convertible to a string.
  template <
    class T,
    class = typename std::enable_if<
      std::is_convertible<T, std::string>::value
    >::type
  >
  topic(T&& x) : str_(std::forward<T>(x)) {
    // nop
  }

  /// Appends a topic components with a separator.
  /// @param t The topic to append to this instance.
  topic& operator/=(const topic& t);

  /// Retrieves the underlying string representation of the topic.
  /// @returns A reference to the underlying string.
  const std::string& string() const;

  /// Returns whether this topic is a prefix match for `t`.
  bool prefix_of(const topic& t) const;

  template <class Inspector>
  friend bool inspect(Inspector& f, topic& x) {
    return f.object(x).fields(f.field("str", x.str_));
  }

private:
  std::string str_;
};

/// @relates topic
bool operator==(const topic& lhs, const topic& rhs);

/// @relates topic
bool operator<(const topic& lhs, const topic& rhs);

/// @relates topic
topic operator/(const topic& lhs, const topic& rhs);

/// @relates topic
bool convert(const topic& t, std::string& str);

/// Checks whether a topic is internal, i.e., messages on this topic are always
/// only visible locally and never forwarded to peers.
/// @relates topic
bool is_internal(const topic& x);

/// Topics with a special meaning.
namespace topics {

const topic reserved = topic{topic::reserved};
const topic master = topic{"data"} / "master";
const topic clone = topic{"data"} / "clone";
const topic master_suffix = reserved / master;
const topic clone_suffix = reserved / clone;
const topic errors = reserved / "local/data/errors";
const topic statuses = reserved / "local/data/statuses";
const topic store_events = reserved / "local/data/store-events";

} // namespace topics
} // namespace broker

/// Converts a string to a topic.
/// @param str The string to convert.
/// @returns The topic according to *str*.
broker::topic operator "" _t(const char* str, size_t);

namespace std {

template <>
struct hash<broker::topic> {
  size_t operator()(const broker::topic& t) const {
    return std::hash<std::string>{}(t.string());
  }
};

} // namespace std
