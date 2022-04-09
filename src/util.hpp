#ifndef UTIL_HPP
#define UTIL_HPP

#include "config.hpp"

/**
 * @brief calculates the roundness of a number
 * see: https://en.wikipedia.org/wiki/Round_number
 *
 * 'A round number is an integer that ends with one or more "0"s (zero-digit)
 * in a given base. So, 590 is rounder than 592, but 590 is less round than 600.'
 *
 * example:
 *  roundness(100) -> 2
 *  roundness(318) -> 0
 *  roundness(20)  -> 1
 *
 * @param number integer to determine the roundness of.
 * @return int
 */
int roundness(int number)
{
  int roundness = 0;
  int places = config::base;
  while (true) {
    if (number % places == 0) {
      ++roundness;
    } else {
      break;
    }
    places *= config::base;
  }
  return roundness;
}

#endif// UTIL_HPP
