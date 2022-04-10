#ifndef SMOOTHLIFE_FIELD_HPP
#define SMOOTHLIFE_FIELD_HPP

#include "config.hpp"
#include "log.hpp"
#include "util.hpp"

namespace smoothlife {

struct Field
{
  enum struct Type : int { empty = 0, exit = 1, add = 2, sub = 3, mul = 4, div = 5 };
  using enum Type;

  Type type = empty;
  int value = 0;

  [[nodiscard]] Field inverse() const
  {
    switch (type) {
    case add:
      return { sub, value };
    case sub:
      return { add, value };
    case mul:
      return { div, value };
    case div:
      return { mul, value };
    default:
      return { empty, value };
    }
  }

  void apply(long &surface, Log *log = nullptr)
  {
    int prev = 0;
    int change = 0;

    switch (type) {
    case add:
      prev = roundness(surface);
      surface += value;
      change = roundness(surface) - prev;
      if (log) { log->post_event("Add polishing paste."); }
      break;
    case sub:
      prev = roundness(surface);
      surface -= value;
      change = roundness(surface) - prev;
      if (log) { log->post_event("Remove burrs."); }
      break;
    case mul:
      prev = roundness(surface);
      surface *= value;
      change = roundness(surface) - prev;
      if (log) { log->post_event("Use finer sanding."); }
      break;
    case div:
      prev = roundness(surface);
      surface /= value;
      change = roundness(surface) - prev;
      if (log) { log->post_event("Disassemble parts."); }
      break;
    default:
      return;
    }

    if (log) {
      using enum Log::Type;
      if (change == 1) {
        log->post_event("Nice, it's getting smooth!", good);
      } else if (change > 1) {
        log->post_event("Great, so smooth and shiny!", good);
      } else if (change < -1) {
        log->post_event("Damn, what a mess!", bad);
      }
    }

    // reset field if it was applied
    type = empty;
    value = 0;
  }
};

}// namespace smoothlife

#endif// SMOOTHLIFE_FIELD_HPP