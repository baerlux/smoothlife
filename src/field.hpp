#ifndef SMOOTHLIFE_FIELD_HPP
#define SMOOTHLIFE_FIELD_HPP

namespace smoothlife {

struct Field
{
  enum struct Type { empty, exit, add, sub, mul, div };
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

  void apply(int &surface)
  {
    switch (type) {
    case add:
      surface += value;
      break;
    case sub:
      surface -= value;
      break;
    case mul:
      surface *= value;
      break;
    case div:
      surface /= value;
      break;
    default:
      return;
    }

    // reset field if it was applied
    type = empty;
    value = 0;
  }
};

}// namespace smoothlife

#endif// SMOOTHLIFE_FIELD_HPP